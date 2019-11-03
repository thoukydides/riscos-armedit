/*
    File        : configapp.c++
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : The !ARMEdit configuration program.

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
#include "kernel.h"
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include oslib header files
#include "gadget.h"
#include "wimp.h"
#include "wimpspriteop.h"
#include "window.h"
extern "C" {
#include "event.h"
}

// Include project header files
#include "armeditmsg.h"
#include "configfile.h"
#include "configinst.h"
#include "configwin.h"
#include "quit.h"
#include "utils.h"

// The directory which contains the resource files
#define ResDirectory "<ARMEdit$Dir>.Configure"

// Should a shutdown be resumed
wimp_t quit_shutdown;

// Wimp messages of interest, or 0 for all
static wimp_message_list wimp_messages[] = {0};

// Array of toolbox events of interest, or 0 for all events
static toolbox_action_list toolbox_events[] = {0};

// Jump position for signal handling
static jmp_buf signal_jump;

// Flag to store whether an error is being handled
static sig_atomic_t handler_active;

// Error message for nested errors
static const os_error fatal_error = {0, "A serious error has occurred within the error handler. Application must quit immediately."};

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle key presses.
*/
static bool handler_key(wimp_event_no event_code, wimp_block *block,
                        toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(id_block);
    NOT_USED(handle);

    wimp_process_key(block->key.c);

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_ARMEditOpen messages. This either opens the
                  specified window, or move the window to the front if it is
                  already open.
*/
static int message_armedit_open(wimp_message *message, void *handle)
{
    NOT_USED(handle);

    // Acknowledge the message
    message->your_ref = message->my_ref;
    wimp_send_message(wimp_USER_MESSAGE_ACKNOWLEDGE, message, message->sender);

    // Take the required action
    switch (((wimp_message_armedit_open *) &message->data)->reason)
    {
        case ARMEDIT_OPEN_CONFIGURE:
            // Open the configuration editor
            configwin_open();
            break;

        case ARMEDIT_OPEN_INSTALL:
            // Open the installation window
            configinst_open();
            break;

        default:
            // Do not know how to handle other messages
            break;
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the toolbox system.
*/
static void tbox_initialise(void)
{
    int used;

    // GSTrans the resource directory name
    os_gs_trans(ResDirectory, res_directory, sizeof(res_directory), &used);
    res_directory[used] = 0;

    // Initialise event library
    event_initialise(&id_block);

    // Not interested in null polls
    event_set_mask(wimp_MASK_NULL);

    // Register WIMP message handlers
    event_register_message_handler(message_ARMEDIT_OPEN,
                                   message_armedit_open, NULL);

    // Register toolbox handlers
    event_register_wimp_handler(event_ANY, wimp_KEY_PRESSED,
                                handler_key, NULL);

    // All handlers setup, so initialise as a toolbox task
    task_handle = toolbox_initialise(0, WimpVersion, wimp_messages,
                                     toolbox_events, res_directory,
                                     &message_block, &id_block,
                                     &wimp_version, &sprite_area);
}

/*
    Parameters  : sig   - The signal that was raised.
    Returns     : void
    Description : Signal handler.
*/
extern "C" void signal_handler(int sig)
{
    // Check if it is a nested error
    if (handler_active)
    {
        // Take panic action
        wimp_report_error(&fatal_error, wimp_ERROR_BOX_OK_ICON, "application");
        os_exit(&fatal_error, EXIT_FAILURE);
    }

    // Set flag to detect nested errors
    handler_active = TRUE;

    // Resume execution from just before the main program loop
    longjmp(signal_jump, sig);
}

/*
    Parameters  : sig   - The signal that was raised.
    Returns     : void
    Description : Display a suitable error for the signal that was raised,
                  and offer the user the opportunity to continue or quit the
                  program.
*/
static void signal_error(int sig)
{
    os_error er;
    char msg[256];
    int quit;

    // Choose an appropriate error message
    switch (sig)
    {
        case SIGFPE:
            lookup_token(msg, sizeof(msg), "SigFPE");
            break;

        case SIGILL:
            lookup_token(msg, sizeof(msg), "SigIll");
            break;

        case SIGSEGV:
            lookup_token(msg, sizeof(msg), "SigSegV");
            break;

        case SIGSTAK:
            lookup_token(msg, sizeof(msg), "SigStack");
            break;

        case SIGOSERROR:
            {
                _kernel_oserror *ptr;
                ptr = _kernel_last_oserror();
                lookup_token(msg, sizeof(msg), "SigErr", ptr->errmess);
            }
            break;

        default:
            lookup_token(msg, sizeof(msg), "SigUnk");
            break;
    }

    // Use a suitable form of error window
    if (wimp_VERSION_RO35 <= wimp_version)
    {
        // Construct the error block
        er.errnum = 0;
        lookup_token(er.errmess, sizeof(er.errmess), "SigTpl:%0", msg);

        // Display the error message
        quit = wimp_report_error_by_category(&er, 0, task_name, AppSprite,
                   wimpspriteop_AREA, lookup_token("SigBut:Continue,Quit"))
               != 3;
    }
    else
    {
        // Construct the error block
        er.errnum = 0;
        lookup_token(er.errmess, sizeof(er.errmess), "SigTplO:%0", msg);

        // Display the error message
        quit = wimp_report_error(&er, wimp_ERROR_BOX_OK_ICON
                                 | wimp_ERROR_BOX_CANCEL_ICON, task_name)
               != wimp_ERROR_BOX_SELECTED_OK;
    }

    // Exit if the Quit option was selected
    if (quit)
    {
        xwimp_create_menu((wimp_menu *) -1, 0, 0);
        os_exit(&er, EXIT_FAILURE);
    }
}

/*
    Parameters  : argc  - The number of command line arguments.
                  argv  - The command line arguments.
    Returns     : int   - The return code.
    Description : The main program!
*/
int main(int argc, char *argv[])
{
    wimp_block poll_block;
    wimp_event_no event_code;
    int sig;

    NOT_USED(argc);
    NOT_USED(argv);

    // Initialise as a toolbox task
    tbox_initialise();

    // Initialise the quit handler
    quit_initialise();

    // Read the current configuration
    configfile_read();

    // Initialise the configuration windows
    configwin_initialise();

    // Initialise the installation windows
    configinst_initialise();

    // Set the jump point for signal handling
    sig = setjmp(signal_jump);
    if (sig) signal_error(sig);

    // Clear the nested handler flag
    handler_active = FALSE;

    // Install signal handlers
    signal(SIGFPE, signal_handler);
    signal(SIGILL, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGSTAK, signal_handler);
    signal(SIGOSERROR, signal_handler);

    // Keep polling until quit flag is set
    while (!quit) event_poll(&event_code, &poll_block, NULL);

    // If the program gets this far then everything went alright
    return EXIT_SUCCESS;
}
