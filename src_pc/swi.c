/*
    File        : swi.c
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

// Include header file for this module
#include "swi.h"

// Include system header files
#include <string.h>

// Include project header files
#include "hpc.h"

// Amount of ARM memory required
#define ARM_MEM 256

// Pointer to some ARM memory
static long arm_mem = 0;

// Possible error blocks
static const talk_error error_initialise =
    {-100, "Unable to initialise SWI handler"};

/*
    Parameters  : void
    Returns     : void
    Description : This should be called just before the program exits to free
                  any ARM memory claimed.
*/
void finalise(void)
{
    // Free the ARM memory
    talk_free(arm_mem);
}

/*
    Parameters  : void
    Returns     : int    - Has the module been initialised.
    Description : Ensure that the module is initialised. In particular this
                  allocates some ARM memory.
*/
int initialise(void)
{
    // Check if module already initialised
    if (arm_mem) return TRUE;

    // Claim some ARM memory
    if (talk_malloc(ARM_MEM, &arm_mem)) return FALSE;

    // Set up an exit handler to free the memory
    atexit(finalise);

    // Return success
    return TRUE;
}

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
                          talk_swi_regs *out)
{
    const talk_error *code;
    talk_swi_regs regs;

    // Ensure that some ARM memory has been claimed
    if (!initialise()) return &error_initialise;

    // Copy the SWI name into ARM memory
    code = talk_write(swi, strlen(swi) + 1, arm_mem);
    if (code) return code;

    // Convert the name into a number
    regs.r[1] = arm_mem;
    code = talk_swi(OS_NumberFromString, &regs, &regs);
    if (code) return code;

    // Call the required SWI
    return talk_swi(regs.r[0], in, out);
}
