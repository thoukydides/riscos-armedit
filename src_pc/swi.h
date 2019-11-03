/*
    File        : swi.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1995-2001, 2019
    Description : Call RISC OS SWIs by name. Note that this is a very
                  inefficient operation.

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
#ifndef SWI_H
#define SWI_H

// Include project header files
#include "talk.h"

// Define some useful SWI numbers
#define OS_CLI 0x05
#define OS_File 0x08
#define OS_GBPB 0x0c
#define OS_Module 0x1e
#define OS_ReadVarVal 0x23
#define OS_ReadModeVariable 0x35
#define OS_NumberFromString 0x39
#define OS_ReadMonotonicTime 0x42

#ifdef __cplusplus
extern "C" {
#endif

/*
    Parameters  : swi           - The name of the SWI to call.
                  in            - Pointer to the values for the ARM registers
                                  on entry to the SWI.
                  out           - Pointer to the values that the ARM
                                  registers contained on exit from the SWI.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Call the specified RISC OS SWI. The SWI is always called
                  with the X bit set.
*/
const talk_error *swi_swi(const char *swi, const talk_swi_regs *in,
                          talk_swi_regs *out);

#ifdef __cplusplus
}
#endif

#endif
