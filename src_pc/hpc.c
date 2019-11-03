/*
    File        : hpc.c
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1995-2001, 2019
    Description : Aleph One High-level Procedure Call (HPC) interface.
                  If HPC support is not present then a less efficient, but
                  more widely supported, scheme is used without any extra
                  work by the client software.

    License     : ARMEdit is free software: you can redistribute it and/or
                  modify it under the terms of the GNU General Public License
                  as published by the Free Software Foundation, either
                  version 3 of the License, or (at your option) any later
                  version.

                  ARMEdit is distributed in the hope that it will be useful,
                  but WITHOUT ANY WARRANTY; without even the implied warranty
                  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
                  the GNU General Public License for more details.

                  You should have received a copy of the GNU General Public
                  License along with ARMEdit. If not, see
                  <http://www.gnu.org/licenses/>.
*/

// Include header file for this module
#include "hpc.h"

// Include system header files
#include <dos.h>
#include <stdlib.h>
#include <mem.h>
#include <string.h>

// Include project header files
#include "svc.h"

// BIOS interrupt number for HPC services
#define HPC_INT_SERVICE 0x4d

// The HPC reason codes
#define HPC_CODE_IDENTIFY 0x0000
#define HPC_CODE_EXECUTE 0x0001

// The HPC return codes
#define HPC_RETURN_HPC 0x4850

// The HPC buffer to use
#define HPC_BUFFER 1

// Interrupt generated when ARM requests HPC transfer
#define HPC_INT_NOTIFY 0x2f

// Reason code given for ARM requests
#define HPC_CODE_NOTIFY 0x4d4d

// The ports to use if HPC is not available
#define EMULATE_PORT_STATUS 0x2e0
#define EMULATE_PORT_CMD 0x2e0
#define EMULATE_PORT_DATA 0x2e2

// The commands
#define EMULATE_CODE_TX 0x0000
#define EMULATE_CODE_PROCESS 0x0001
#define EMULATE_CODE_RX 0x0002
#define EMULATE_CODE_RELEASE 0x0003

// The emulated HPC return codes
#define EMULATE_RETURN_EMULATE 0x454d
#define EMULATE_RETURN_BUSY 0x4d45

// The SWI to emulate HPC calls
#define ARMEdit_HPC 0x4bc46

// System variable to indicate software emulator
#define EMULATE_VAR "SOFTPC"

/*
    Parameters  : void
    Returns     : int   - Is it the software emulator.
    Description : Check whether the code is running under the Acorn software
                  PC emulator.
*/
int hpc_is_software(void)
{
    unsigned value;

    // Read a word from the BIOS ROM
    movedata(0xff00, 0x0ffe, _DS, (unsigned) &value, sizeof(value));

    // Return the status
    return value == 0xffff;
}

/*
    Parameters  : void
    Returns     : int   - Are HPC services available.
    Description : Check whether HPC services are present.
*/
static int hpc_present(void)
{
    union REGS inregs, outregs;

    // Set the entry registers
    inregs.x.ax = HPC_CODE_IDENTIFY;

    // Identify HPC services
    return int86(HPC_INT_SERVICE, &inregs, &outregs) == HPC_RETURN_HPC;
}

/*
    Parameters  : tx1_buf   - Pointer to the first portion of the data to
                              send.
                  tx1_size  - The size of the first portion of the data to
                              send.
                  tx2_buf   - Pointer to the second portion of the data to
                              send.
                  tx2_size  - The size of the second portion of the data to
                              send.
                  rx1_buf   - Pointer to the buffer to receive the first
                              portion of the reply.
                  rx1_size  - The size of the buffer to receive the first
                              portion of the reply.
                  tx1_buf   - Pointer to the buffer to receive the second
                              portion of the reply.
                  tx2_size  - The size of the buffer to receive the second
                              portion of the reply.
    Returns     : int       - The code returned by the HPC call.
    Description : Execute an HPC call.
                  Note that the HPC protocol imposes a maximum length of
                  transfer of 16384 bytes.
*/
static int hpc_execute(const void far *tx1_buf, size_t tx1_size,
                       const void far *tx2_buf, size_t tx2_size,
                       void far *rx1_buf, size_t rx1_size,
                       void far *rx2_buf, size_t rx2_size)
{
    union REGS inregs, outregs;
    struct SREGS segregs;
    struct
    {
        int buffer;
        size_t tx1s;
        const void far *tx1b;
        size_t tx2s;
        const void far *tx2b;
        size_t rx1s;
        void far *rx1b;
        size_t rx2s;
        void far *rx2b;
    } block;

    // Construct the parameter block
    block.buffer = HPC_BUFFER;
    block.tx1s = tx1_size;
    block.tx1b = tx1_buf;
    block.tx2s = tx2_size;
    block.tx2b = tx2_buf;
    block.rx1s = rx1_size;
    block.rx1b = rx1_buf;
    block.rx2s = rx2_size;
    block.rx2b = rx2_buf;

    // Set the entry registers
    inregs.x.ax = HPC_CODE_EXECUTE;
    inregs.x.bx = FP_OFF(&block);
    segregs.es = FP_SEG(&block);

    // Perform the HPC call, and return the result
    return int86x(HPC_INT_SERVICE, &inregs, &outregs, &segregs);
}

/*
    Parameters  : tx1_buf   - Pointer to the first portion of the data to
                              send.
                  tx1_size  - The size of the first portion of the data to
                              send.
                  tx2_buf   - Pointer to the second portion of the data to
                              send.
                  tx2_size  - The size of the second portion of the data to
                              send.
                  rx1_buf   - Pointer to the buffer to receive the first
                              portion of the reply.
                  rx1_size  - The size of the buffer to receive the first
                              portion of the reply.
                  tx1_buf   - Pointer to the buffer to receive the second
                              portion of the reply.
                  tx2_size  - The size of the buffer to receive the second
                              portion of the reply.
    Returns     : int       - The code returned by the HPC call.
    Description : Emulate an HPC call.
                  Note that the HPC protocol imposes a maximum length of
                  transfer of 16384 bytes.
*/
static int hpc_emulate(const void far *tx1_buf, size_t tx1_size,
                       const void far *tx2_buf, size_t tx2_size,
                       void far *rx1_buf, size_t rx1_size,
                       void far *rx2_buf, size_t rx2_size)
{
    int status;
    size_t i;
    unsigned char far *ptr;

    // Check the status of the emulated HPC
    status = inport(EMULATE_PORT_STATUS);
    if (status == EMULATE_RETURN_BUSY)
    {
        return HPC_RETURN_FAILURE_IN_USE;
    }
    else if (status != EMULATE_RETURN_EMULATE)
    {
        return HPC_RETURN_FAILURE_NOT_WORKING;
    }

    // Send the data
    outport(EMULATE_PORT_CMD, EMULATE_CODE_TX);
    ptr = (unsigned char far *) tx1_buf;
    for (i = 0; i < tx1_size; i++) outportb(EMULATE_PORT_DATA, *ptr++);
    ptr = (unsigned char far *) tx2_buf;
    for (i = 0; i < tx2_size; i++) outportb(EMULATE_PORT_DATA, *ptr++);

    // Perform the call
    outport(EMULATE_PORT_CMD, EMULATE_CODE_PROCESS);

    // Receive the reply
    outport(EMULATE_PORT_CMD, EMULATE_CODE_RX);
    ptr = (unsigned char far *) rx1_buf;
    for (i = 0; i < rx1_size; i++) *ptr++ = inportb(EMULATE_PORT_DATA);
    ptr = (unsigned char far *) rx2_buf;
    for (i = 0; i < rx2_size; i++) *ptr++ = inportb(EMULATE_PORT_DATA);

    // End the transfer to allow the buffer to be released
    outport(EMULATE_PORT_CMD, EMULATE_CODE_RELEASE);

    // Return success code
    return HPC_RETURN_SUCCESS;
}

/*
    Parameters  : tx1_buf   - Pointer to the first portion of the data to
                              send.
                  tx1_size  - The size of the first portion of the data to
                              send.
                  tx2_buf   - Pointer to the second portion of the data to
                              send.
                  tx2_size  - The size of the second portion of the data to
                              send.
                  rx1_buf   - Pointer to the buffer to receive the first
                              portion of the reply.
                  rx1_size  - The size of the buffer to receive the first
                              portion of the reply.
                  tx1_buf   - Pointer to the buffer to receive the second
                              portion of the reply.
                  tx2_size  - The size of the buffer to receive the second
                              portion of the reply.
    Returns     : int       - The code returned by the HPC call.
    Description : Emulate an HPC call.
                  Note that the HPC protocol imposes a maximum length of
                  transfer of 16384 bytes.
*/
static int hpc_software(const void far *tx1_buf, size_t tx1_size,
                        const void far *tx2_buf, size_t tx2_size,
                        void far *rx1_buf, size_t rx1_size,
                        void far *rx2_buf, size_t rx2_size)
{
    svc_swi_regs regs;
    int status;

    // Set the SWI registers
    regs.r[0] = tx1_size;
    regs.r[1] = svc_address(tx1_buf);
    regs.r[2] = tx2_size;
    regs.r[3] = svc_address(tx2_buf);
    regs.r[4] = rx1_size;
    regs.r[5] = svc_address(rx1_buf);
    regs.r[6] = rx2_size;
    regs.r[7] = svc_address(rx2_buf);

    // Call the SWI to perform the HPC call
    status = svc_swi(ARMEdit_HPC, &regs, &regs);

    // Check the status
    return (status == SVC_RETURN_SUCCESS) && !(regs.r[15] & (1 << 28))
           ? HPC_RETURN_SUCCESS
           : HPC_RETURN_FAILURE_NOT_WORKING;
}

/*
    Parameters  : tx1_buf   - Pointer to the first portion of the data to
                              send.
                  tx1_size  - The size of the first portion of the data to
                              send.
                  tx2_buf   - Pointer to the second portion of the data to
                              send.
                  tx2_size  - The size of the second portion of the data to
                              send.
                  rx1_buf   - Pointer to the buffer to receive the first
                              portion of the reply.
                  rx1_size  - The size of the buffer to receive the first
                              portion of the reply.
                  tx1_buf   - Pointer to the buffer to receive the second
                              portion of the reply.
                  tx2_size  - The size of the buffer to receive the second
                              portion of the reply.
    Returns     : int       - The code returned by the HPC call.
    Description : Execute an HPC call, or a substitute if HPC not available.
                  Note that the HPC protocol imposes a maximum length of
                  transfer of 16384 bytes, although transfers over 4096 bytes
                  may be unreliable and are discouraged.
*/
int hpc_message(const void far *tx1_buf, size_t tx1_size,
                const void far *tx2_buf, size_t tx2_size,
                void far *rx1_buf, size_t rx1_size,
                void far *rx2_buf, size_t rx2_size)
{
    int status;
    static int first = TRUE;
    static int is_software;
    static int is_hpc;
    union REGS regs;

    regs.x.ax = 0x1681;
    int86(0x2F, &regs, &regs);

    // Check system type the first time only
    if (first)
    {
        // Check the system type
        is_software = hpc_is_software();
        if (!is_software) is_hpc = hpc_present();

        // Clear first time only flag
        first = FALSE;
    }

    // Behaviour depends upon the system being used
    if (is_software)
    {
        // It is the software emulator
        status = hpc_software(tx1_buf, tx1_size, tx2_buf, tx2_size,
                              rx1_buf, rx1_size, rx2_buf, rx2_size);
    }
    else
    {
        // Assume PC card is being used, so reset the emulation
        outport(EMULATE_PORT_CMD, EMULATE_CODE_RELEASE);

        // Action depends upon whether HPC is available
        if (is_hpc)
        {
            // HPC is available so do the real thing
            status = hpc_execute(tx1_buf, tx1_size, tx2_buf, tx2_size,
                                 rx1_buf, rx1_size, rx2_buf, rx2_size);
        }
        else
        {
            // HPC is not available so emulate it
            status = hpc_emulate(tx1_buf, tx1_size, tx2_buf, tx2_size,
                                 rx1_buf, rx1_size, rx2_buf, rx2_size);
        }
    }

    // Reenable task switching
    regs.x.ax = 0x1682;
    int86(0x2F, &regs, &regs);

    // Return the status
    return status;
}
