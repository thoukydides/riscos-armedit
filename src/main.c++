/*
    File        : main.c++
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : The main !ARMEdit program.

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
#include "main.h"

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
#include "configfile.h"
#include "info.h"
#include "msg.h"
#include "multi.h"
#include "quit.h"
#include "save.h"
#include "speed.h"
#include "task.h"
#include "utils.h"

// The directory which contains the resource files
#define ResDirectory "<ARMEdit$Dir>.Resources"

// The help file to run from the iconbar menu
#define HelpFile "<ARMEdit$Dir>.!Help"

// Time between polls
int poll_interval = POLL_INTERVAL_DEFAULT;

// Should a shutdown be resumed
wimp_t quit_shutdown;

// Toolbox and wimp variables
toolbox_o menu_id;                      // ID of iconbar menu object

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
    Parameters  : handle    - The user specified handle.
    Returns     : void
    Description : A function that is called after the user has responded to a
                  prompt to quit by selecting the continue option. This should
                  perform an operation equivalent to marking the data as read.
                  This allows the quit to continue further if it is restarted.
*/
static void quit_selected(void *handle)
{
    NOT_USED(handle);

    // Discard any external edits
    ExtEdit::abort_all();
}

/*
    Parameters  : handle    - The user specified handle.
                  func      - Variable to receive a pointer to the function
                              to call if the user selects the quit option.
    Returns     : char *    - Prompt for the user, or NULL if no objection.
    Description : A function that is called when the software is about to quit.
                  If a string is returned then the user is prompted to allow
                  the quit to be aborted or continued.
*/
const char *quit_check(void *handle, quit_func_quit *func)
{
    static char str[256];
    int edits;

    NOT_USED(handle);

    // Decide whether to allow the quit
    if (edits = ExtEdit::count_active())
    {
        // Construct the required message
        sprintf(str, lookup_token(edits == 1 ? "Quit1" : "Quit2"), edits);
    }

    // Set the quit function
    *func = quit_selected;

    // Return the string if required
    return edits ? str : NULL;
}

/*
    Parameters  : msg           - Pointer to the message block.
                  msg_id        - The ID of the sender.
                  msg_handle    - The handle of the sender.
                  handle        - A user defined pointer.
    Returns     : bool          - Has the message buffer been updated to
                                  contain a reply. If this is set then no
                                  more handlers will be called for this
                                  message.
    Description : Handle PC reset messages.
*/
static bool handler_msg_reset(char *msg, int msg_id, int msg_handle,
                              void *handle)
{
    NOT_USED(msg);
    NOT_USED(msg_id);
    NOT_USED(msg_handle);
    NOT_USED(handle);

    // Run the reset obey file
    if (!configfile_auto_boot.empty())
    {
        wimp_start_task(configfile_auto_boot.c_str());
    }

    // No reply to send
    return FALSE;
}

/*
    Parameters  : msg           - Pointer to the message block.
                  msg_id        - The ID of the sender.
                  msg_handle    - The handle of the sender.
                  handle        - A user defined pointer.
    Returns     : bool          - Has the message buffer been updated to
                                  contain a reply. If this is set then no
                                  more handlers will be called for this
                                  message.
    Description : Handle PC shutdown messages.
*/
static bool handler_msg_shutdown(char *msg, int msg_id, int msg_handle,
                                 void *handle)
{
    NOT_USED(msg);
    NOT_USED(msg_id);
    NOT_USED(msg_handle);
    NOT_USED(handle);

    // Run the quit obey file
    if (!configfile_auto_quit.empty())
    {
        wimp_start_task(configfile_auto_quit.c_str());
    }

    // Start a quit if required
    if (configfile_frontend_auto_quit) quit_quit();

    // No reply to send
    return FALSE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle menu selection toolbox events.
*/
static bool handler_tb_menu(bits event_code, toolbox_action *action,
                            toolbox_block *id_block, void *handle)
{
    os_error *er;

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // Decode the component ID
    switch (id_block->this_cmp)
    {
        case IBAR_MENU_QUIT:
            // Quit option selected
            quit_quit();
            break;

        case IBAR_MENU_CONFIGURE:
            // Configuration option selected
            config_edit();
            break;

        case IBAR_MENU_INSTALL:
            // Installation option selected
            config_install();
            break;

        case IBAR_MENU_RELOG:
            // Relog device driver option selected
            er = xos_cli("ARMEdit_DevicesRelog");
            if (er) report_error(er);
            break;

        case IBAR_MENU_RELOG_NOW:
            // Relog device driver option selected
            er = xos_cli("ARMEdit_DevicesRelog -now");
            if (er) report_error(er);
            break;

        case IBAR_MENU_HELP:
            // Open the online help
            er = xos_cli(HelpFile);
            if (er) report_error(er);
            break;

        default:
            // Do not care about any other entries
            break;
    }

    // Do not claim the event
    return FALSE;
}

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

    // Pass all unrecognised keys to the next task
    wimp_process_key(block->key.c);

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Attach handlers to an auto-created object.
*/
static bool handler_tb_autocreated(bits event_code, toolbox_action *action,
                                   toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(handle);

    if (!strcmp(action->data.created.name, "SpeedWin"))
    {
        // It is the speed control window
        speed_created(id_block->this_obj);
    }
    else if (!strcmp(action->data.created.name, "IBarMenu"))
    {
        // It is the main menu
        menu_id = id_block->this_obj;
        event_register_toolbox_handler(menu_id,
                                       action_MENU_SELECTION,
                                       handler_tb_menu, NULL);
    }
    else if (!strcmp(action->data.created.name, "RelogMenu"))
    {
        // It is the device driver relog submenu
        event_register_toolbox_handler(id_block->this_obj,
                                       action_MENU_SELECTION,
                                       handler_tb_menu, NULL);
    }

    // Do not claim the event - it is too generally useful
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

    // Register toolbox handlers
    event_register_wimp_handler(event_ANY, wimp_KEY_PRESSED,
                                handler_key, NULL);
    event_register_toolbox_handler(event_ANY, action_OBJECT_AUTO_CREATED,
                                   handler_tb_autocreated, NULL);
    event_register_toolbox_handler(event_ANY, action_OBJECT_AUTO_CREATED,
                                   multi_tb_autocreated, NULL);
    event_register_toolbox_handler(event_ANY, action_OBJECT_AUTO_CREATED,
                                   config_tb_autocreated, NULL);
    event_register_toolbox_handler(event_ANY,
                                   action_PROG_INFO_ABOUT_TO_BE_SHOWN,
                                   info_handler_shown, NULL);
    event_register_toolbox_handler(event_ANY,
                                   action_PROG_INFO_DIALOGUE_COMPLETED,
                                   info_handler_hidden, NULL);

    // All handlers setup, so initialise as a toolbox task
    task_handle = toolbox_initialise(0, WimpVersion, wimp_messages,
                                     toolbox_events, res_directory,
                                     &message_block, &id_block,
                                     &wimp_version, &sprite_area);
}

/*
    Parameters  : str1  - The first string.
                  str2  - The second string.
    Returns     : bool  - Do the strings match.
    Description : Compare two control character terminated strings.
*/
static bool strctrlcmp(const char *str1, const char *str2)
{
    while ((31 < *str1) && (31 < *str2) && (*str1 == *str2))
    {
        str1++;
        str2++;
    }

    return (*str1 < 32) && (*str2 < 32);
}

/*
    Parameters  : name  - The name of the task to check for.
    Returns     : bool  - Is the specified task running.
    Description : Check if a specified task is running.
*/
static bool running(const char *name)
{
    int context = 0;
    bool match = FALSE;
    taskmanager_task task;

    // Enumerate all the running tasks
    context = taskmanager_enumerate_tasks(context, &task, sizeof(task), NULL);
    while (0 <= context)
    {
        // Check if this task matches - string is terminated by ctrl char
        if (strctrlcmp(name, task.name)) match = TRUE;

        // Get the next task
        context = taskmanager_enumerate_tasks(context, &task, sizeof(task),
                                              NULL);
    }

    // Return the status
    return match;
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
    os_t old_time, new_time;
    int sig;
    int *poll_word;

    NOT_USED(argc);
    NOT_USED(argv);

    // Initialise as a toolbox task
    tbox_initialise();

    // Initialise the quit handler
    quit_initialise();
    quit_register(quit_check);

    // Read and handler the configuration
    config_initialise();

    // Initialise message handler
    poll_word = msg_initialise();
    msg_register_handler(MSG_ID_MODULE, MSG_HANDLE_ANY,
                         MSG_REASON_MODULE_RESET, handler_msg_reset,
                         NULL);
    msg_register_handler(MSG_ID_MODULE, MSG_HANDLE_ANY,
                         MSG_REASON_MODULE_SHUTDOWN, handler_msg_shutdown,
                         NULL);
    edit_initialise();
    task_initialise();
    save_initialise();

    // Ensure task is not already running
    task_name = lookup_token("_TaskName");
    if (running(task_name))
    {
        os_error er;

        lookup_token(er.errmess, sizeof(er.errmess), "Running");
        report_error(&er);

        // Exit if already running
        return EXIT_FAILURE;
    }

    // Run the loaded obey file
    if (!configfile_auto_load.empty())
    {
        wimp_start_task(configfile_auto_load.c_str());
    }

    // Start the installer if no valid configuration
    if (!configfile_present) config_install();

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
    old_time = os_read_monotonic_time();
    while (!quit)
    {
        event_poll_idle(&event_code, &poll_block, old_time, poll_word);
        if (event_code == wimp_NULL_REASON_CODE)
        {
            new_time = os_read_monotonic_time();
            while (0 <= new_time - old_time) old_time += poll_interval;
        }
    }

    // Ensure that the menu is closed
    xtoolbox_hide_object(0, menu_id);

    // Run the exit obey file
    if (!configfile_auto_exit.empty())
    {
        wimp_start_task(configfile_auto_exit.c_str());
    }

    // If the program gets this far then everything went alright
    return EXIT_SUCCESS;
}
