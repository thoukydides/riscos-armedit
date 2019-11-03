/*
    File        : hpc.h
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

// Only include header file once
#ifndef HPC_H
#define HPC_H

// Include system header files
#include <stdlib.h>

// Define useful constants
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// The HPC return codes
#define HPC_RETURN_SUCCESS 0x0000
#define HPC_RETURN_FAILURE_IN_USE 0x0001
#define HPC_RETURN_FAILURE_NOT_WORKING 0x0002

#ifdef __cplusplus
extern "C" {
#endif

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
                void far *rx2_buf, size_t rx2_size);

#ifdef __cplusplus
}
#endif

#endif
