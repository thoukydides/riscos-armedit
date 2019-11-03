/*
    File        : launch.c
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Wimp task launcher for !ARMEdit. This is used to avoid the
                  problems associated with calling Wimp_StartTask to start a
                  Toolbox application from another Toolbox application.

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

// Include clib header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include oslib header files
#include "wimp.h"

// Include project header files
#include "armeditmsg.h"

/*
    Parameters  : argc  - The number of command line arguments.
                  argv  - The command line arguments.
    Returns     : int   - The return code.
    Description : The main program!
*/
int main(int argc, char *argv[])
{
    wimp_t task_handle;
    wimp_t child_task_handle = wimp_BROADCAST;
    wimp_t parent_task_handle;
    int launch_id;
    int quit = FALSE;
    int i;
    wimp_event_no event;
    wimp_block poll_block;
    char cmd[256] = "";
    wimp_message msg;

    // Process the command line
    if (argc < 4) return EXIT_FAILURE;
    parent_task_handle = (wimp_t) atoi(argv[1]);
    launch_id = atoi(argv[2]);
    for (i = 3; i < argc; i++)
    {
        if (cmd[0]) strcat(cmd, " ");
        strcat(cmd, argv[i]);
    }

    // Start up as a WIMP task
    task_handle = wimp_initialise(wimp_VERSION_RO3, "ARMEdit Launcher",
                                  NULL, NULL);

    // Poll until a NULL poll is received
    while (!quit)
    {
        // Poll the WIMP
        event = wimp_poll(0, &poll_block, NULL);

        // Decode the event
        switch (event)
        {
            case wimp_NULL_REASON_CODE:
                // Launch the required application
                if (xwimp_start_task(cmd, &child_task_handle))
                {
                    child_task_handle = wimp_BROADCAST;
                }
                quit = TRUE;
                break;

            case wimp_USER_MESSAGE:
            case wimp_USER_MESSAGE_RECORDED:
                // The only message of interest is Message_Quit
                if (poll_block.message.action == message_QUIT)
                {
                    // Must quit immediately
                    quit = TRUE;
                }
                break;
        }
    }

    // Send Message_ARMEditLaunched to the parent task
    msg.size = sizeof(wimp_message);
    msg.your_ref = 0;
    msg.action = message_ARMEDIT_LAUNCHED;
    ((wimp_message_armedit_launched *) &msg.data)->id = launch_id;
    ((wimp_message_armedit_launched *) &msg.data)->handle = child_task_handle;
    wimp_send_message(wimp_USER_MESSAGE, &msg, parent_task_handle);

    // Close down as a WIMP task
    xwimp_close_down(task_handle);

    // If the program gets this far then everything went alright
    return EXIT_SUCCESS;
}
