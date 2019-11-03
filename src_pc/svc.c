/*
    File        : svc.c
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1995-2001, 2019
    Description : Perform SVC calls as provided by the Acorn software PC
                  emulator.

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
#include "svc.h"

// Include system header files
#include <dos.h>

/*
    Parameters  : ptr           - An 8086 address.
    Returns     : unsigned long - The equivalent ARM address, or 0 if unable
                                  to convert.
    Description : Translate an 8086 address to an ARM address.
*/
unsigned long svc_address(void far *ptr)
{
    unsigned long x86_ptr = (unsigned long) ptr;
    unsigned long arm_ptr;

    // Translate the address
    asm {
        mov ax, 0                           // Start with the registers clear
        mov dx, ax
        les bx, dword ptr ss:[x86_ptr]      // Get address to translate
        dw  -1, 257                         // SVC257 translates 8086 address
        jnc ok                              // Carry clear if OK
        mov ax, 0                           // Clear ax
        mov dx, ax                          // Clear dx also
    }
    ok:
    asm {
        mov word ptr ss:[arm_ptr], ax       // Store least significant word
        mov word ptr ss:[arm_ptr + 2], dx   // Store most significant word
    }

    // Return the converted address
    return arm_ptr;
}

/*
    Parameters  : no    - The number of the SWI to call.
                  in    - Pointer to the values for the ARM registers on
                          entry to the SWI.
                  out   - Pointer to the values that the ARM registers
                          contained on exit from the SWI.
    Returns     : int   - The return code.
    Description : Call the specified RISC OS SWI.
*/
int svc_swi(long no, const svc_swi_regs *in, svc_swi_regs *out)
{
    short i;
    typedef struct
    {
        long no;
        svc_swi_regs regs;
    } record;
    record far *block;
    char buffer[sizeof(record) + 3];
    unsigned offset;
    short ok = 1;
    unsigned long x86_ptr;

    // Align the block to a 4 byte boundary
    offset = FP_SEG(buffer);
    block = (record far *) MK_FP(FP_SEG(buffer), (offset + 3) & ~0x0003);
    x86_ptr = (unsigned long) block;

    // Copy the input registers and set the SWI number
    block->no = no;
    for (i = 0; i < 16; i++) block->regs.r[i] = in ? in->r[i] : 0;

    // Attempt to call the SWI
    asm {
        les bx, dword ptr ss:[x86_ptr]      // Get pointer to parameter block
        mov dx, 'sa'                        // SA
        mov ax, 'fe'                        // FE
        dw  -1, 258                         // SVC258 call general SWI
        jnc ok                              // Carry clear if OK
        mov word ptr ss:[ok], 0             // Clear flag if failed
    }
    ok:

    // Copy the return registers
    if (out) for (i = 0; i < 16; i++) out->r[i] = block->regs.r[i];

    // Return the status
    return ok ? SVC_RETURN_SUCCESS
              : SVC_RETURN_FAILURE_GENERAL + (block->no & 0xf);
}
