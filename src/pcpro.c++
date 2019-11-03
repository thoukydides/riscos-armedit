/*
    File        : pcpro.c++
    Date        : 15-May-01
    Author      : Â© A.Thoukydides, 1997, 1998, 2019
    Description : Operations and definitions related to new versions of PCPro.
                  These include support for the multiple configuration chooser
                  and the updated configuration editor.

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
#include "pcpro.h"

// Include cpplib header files
#include "string.h"
#include "fstream.h"

// Include clib header file
#include <stdio.h>
#include <string.h>

// Include oslib header files
#include "wimp.h"
extern "C" {
#include "event.h"
}

// Include project header files
#include "utils.h"

// A flag to control the emulation of older PCConfig versions
int pcpro_fake_old = FALSE;

// The suffix to access the !Run file
#define PCPRO_RUN_SUFFIX ".!Run"

// The configuration file to edit if the fake option is selected
#define PCPRO_CONFIG_FILE "<Diva$Dir>.Config"

/*
    Parameters  : path          - The complete path of the PC front-end
                                  software to check.
    Returns     : const char *  - The version number string, or NULL if unable
                                  to read it.
    Description : Read the version number string from the first line of the
                  !Run file.
*/
const char *pcpro_read_version(const char *path)
{
    static char str[256];
    char c;
    char *ptr = NULL;

    // Construct the path of the !Run file
    sprintf(str, "%s%s", path, PCPRO_RUN_SUFFIX);

    // Attempt to read the first line of the !Run file
    ifstream file(str);
    file.get(str, sizeof(str), '\n');

    // Process the line if successful
    if (file.get(c) && c == '\n')
    {
        // Find a likely start position
        ptr = strstr(str, " v");
        if (ptr)
        {
            char *space;

            // Advance to the first real character
            ptr++;

            // Check for any other text afterwards
            space = strchr(ptr, ' ');
            if (space) *space = 0;
        }
    }

    // Return a pointer to the version number if successful
    return ptr;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle PCConfig wimp message events.
*/
static bool pcpro_config_message(wimp_message *message, void *handle)
{
    wimp_message_pcconfig *details = (wimp_message_pcconfig *) &message->data;
    os_error err;
    wimp_t task = wimp_BROADCAST;

    NOT_USED(handle);

    // No action required unless the fake flag is set and correct task
    if (pcpro_fake_old
        && ((task == wimp_BROADCAST) || (task == message->sender)))
    {
        static int selected = FALSE;
        int kill = FALSE;

        // Action depends upon the reason code
        switch (details->reason)
        {
            case PCPRO_PCCONFIG_STARTING:
                // Ensure that the required configuration is being edited
                task = message->sender;
                message->size = sizeof(wimp_message);
                message->your_ref = 0;
                message->action = message_PCCONFIG;
                details->reason = PCPRO_PCCONFIG_EDIT_SPECIFIED;
                strcpy(details->path, PCPRO_CONFIG_FILE);
                wimp_send_message(wimp_USER_MESSAGE, message, task);
                selected = FALSE;
                break;

            case PCPRO_PCCONFIG_SELECTED:
                // This may signify time to kill the editor
                if (selected) kill = TRUE;
                else if (!strcmp(details->path, PCPRO_CONFIG_FILE))
                {
                    selected = TRUE;
                }
                break;

            case PCPRO_PCCONFIG_EXITING:
                // Clear the task handle
                task = wimp_BROADCAST;
                break;

            case PCPRO_PCCONFIG_EDIT_FAILED:
                // Display an error message and kill the configuration editor
                lookup_token(err.errmess, sizeof(err.errmess), "EditActive");
                report_error(&err);
                kill = TRUE;
                break;

            default:
                // Not interested in other messages
                break;
        }

        // Kill the configuration editor if no longer required
        if (kill && (task != wimp_BROADCAST))
        {
            message->size = sizeof(wimp_message);
            message->your_ref = 0;
            message->action = message_QUIT;
            wimp_send_message(wimp_USER_MESSAGE, message, task);
        }
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the PCPro handlers.
*/
void pcpro_initialise(void)
{
    // Register event handlers
    event_register_message_handler(message_PCCONFIG, pcpro_config_message,
                                   NULL);
}
