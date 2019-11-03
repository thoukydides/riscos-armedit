/*
    File        : utils.c++
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

// Inlcude header file for this module
#include "utils.h"

// Include clib header files
#include "kernel.h"
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include oslib header files
#include "gadget.h"
#include "menu.h"
#include "proginfo.h"
#include "quit.h"
#include "taskmanager.h"
#include "wimp.h"
#include "wimpspriteop.h"
extern "C" {
#include "event.h"
}

// Include project header files
#include "edit.h"
#include "extedit.h"
#include "config.h"
#include "info.h"
#include "msg.h"
#include "multi.h"
#include "save.h"

// Global quit flag
int quit = FALSE;

// Toolbox and wimp variables
char res_directory[256];                // GSTrans'd resource directory
toolbox_block id_block;                 // The toolbox event ID block
messagetrans_control_block message_block;// Toolbox message block
osspriteop_area *sprite_area;           // Sprite area pointer
int wimp_version;                       // The current wimp version
wimp_t task_handle;                     // Task handle for this task
char *task_name;                        // Name of the task

/*
    Parameters  : er    - Pointer to a standard error block.
    Returns     : void
    Description : Report an error.
*/
void report_error(os_error *er)
{
    if (er)
    {
        er->errnum = 0;
        if (wimp_VERSION_RO35 <= wimp_version)
        {
            wimp_report_error_by_category(er, wimp_ERROR_BOX_CATEGORY_INFO << 9,
                                          task_name, AppSprite,
                                          wimpspriteop_AREA, 0);
        }
        else wimp_report_error(er, 0, task_name);
    }
}

/*
    Parameters  : dest  - Pointer to string in which to place result.
                  size  - The length of the destination string.
                  token - The token to lookup.
                  arg0  - The optional first parameter.
                  arg1  - The optional second parameter.
                  arg2  - The optional third parameter.
                  arg3  - The optional fourth parameter.
    Returns     : void
    Description : Lookup the specified token in the message file opened by
                  the toolbox, substituting up to four parameters.
*/
void lookup_token(char *dest, int size, const char *token, const char *arg0,
                  const char *arg1, const char *arg2, const char *arg3)
{
    int used;

    messagetrans_lookup(&message_block, token, dest, size,
                        arg0, arg1, arg2, arg3, &used);
    if (used < size) dest[used] = 0;
}

/*
    Parameters  : token - The token to lookup.
                  arg0  - The optional first parameter.
                  arg1  - The optional second parameter.
                  arg2  - The optional third parameter.
                  arg3  - The optional fourth parameter.
    Returns     : char  - Pointer to the token (in the RMA).
    Description : Lookup the specified token in the message file opened by
                  the toolbox, substituting up to four parameters.
*/
char *lookup_token(const char *token, const char *arg0, const char *arg1,
                   const char *arg2, const char *arg3)
{
    return messagetrans_lookup(&message_block, token, 0, 0,
                               arg0, arg1, arg2, arg3, 0);
}
