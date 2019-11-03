/*
    File        : vdu.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1997-2001, 2019
    Description : Process VDU control codes.

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
#ifndef VDU_H
#define VDU_H

// Include system header files
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
    Parameters  : stream    - The stream to redirect output to.
                  bytes     - Pointer to the bytes of data to output.
                  size      - The number of bytes to output.
    Returns     : void
    Description : An output stream prefilter. This emulates RISC OS treatment
                  of control codes when output is to the screen.
*/
void vdu_output_filter(FILE *stream, const char *bytes, size_t size);

#ifdef __cplusplus
}
#endif

#endif
