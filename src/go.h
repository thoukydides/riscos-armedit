/*
    File        : go.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Shared !ARMEdit utilities.

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

#ifndef go_h
#define go_h

// Include cpplib header files
#include "cpp:string.h"

// Include oslib header files
#include "wimp.h"

// Callback function when application has been launched
typedef void (* go_callback_func)(wimp_t task, void *handle);

/*
    Parameters  : cmd       - The command to execute.
                  func      - The function to call when the command has been
                              performed.
                  handle    - The handle of the task started.
    Returns     : void
    Description : Start a new WIMP task indirectly. This is useful for
                  launching other Toolbox applications.
*/
void go_start_task(const string &cmd, go_callback_func func, void *handle);

#endif
