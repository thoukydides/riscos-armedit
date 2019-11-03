/*
    File        : svc.h
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

// Only include header file once
#ifndef SVC_H
#define SVC_H

// ARM registers used on entry and exit from SWIs
typedef struct
{
    long r[16];                         // All registers included
} svc_swi_regs;

// SWI return codes
#define SVC_RETURN_SUCCESS 0x0000
#define SVC_RETURN_FAILURE_GENERAL 0x0001
#define SVC_RETURN_FAILURE_SIGNATURE 0x0002
#define SVC_RETURN_FAILURE_NOT_RAM 0x0003
#define SVC_RETURN_FAILURE_ALIGNMENT 0x0004
#define SVC_RETURN_FAILURE_SWI_RANGE 0x0005
#define SVC_RETURN_FAILURE_PROTECTED 0x0006

#ifdef __cplusplus
extern "C" {
#endif

/*
    Parameters  : ptr           - An 8086 address.
    Returns     : unsigned long - The equivalent ARM address, or 0 if unable
                                  to convert.
    Description : Translate an 8086 address to an ARM address.
*/
unsigned long svc_address(void far *ptr);

/*
    Parameters  : no    - The number of the SWI to call.
                  in    - Pointer to the values for the ARM registers on
                          entry to the SWI.
                  out   - Pointer to the values that the ARM registers
                          contained on exit from the SWI.
    Returns     : int   - The return code.
    Description : Call the specified RISC OS SWI.
*/
int svc_swi(long no, const svc_swi_regs *in, svc_swi_regs *out);

#ifdef __cplusplus
}
#endif

#endif
