/*
    File        : quit.c++
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1997-2001, 2019
    Description : Quit and pre-quit handling.

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
#include "quit.h"

// Include system header files
#include <stdlib.h>

// Include oslib header files
#include "toolbox.h"
#include "os:quit.h"
extern "C" {
#include "event.h"
}

// Include project header files
#include "armeditmsg.h"
#include "utils.h"

// List of child task handles
struct quit_child_record
{
    quit_child_record *next;
    quit_child_record *prev;
    wimp_t handle;
};
static quit_child_record *quit_child_head = NULL;

// List of registered pre quit functions
struct quit_func_record
{
    quit_func_record *next;
    quit_func_record *prev;
    quit_func_check func;
    void *handle;
};
static quit_func_record *quit_func_head = NULL;

// The quit toolbox object
static toolbox_o quit_id;

// The message that started the quit sequence
static wimp_message quit_message;

// The current function to notify if a quit option is selected
static quit_func_quit quit_func_done = NULL;
static void *quit_func_done_handle = NULL;

/*
    Parameters  : void
    Returns     : int   - Was the quit accepted by all the functions.
    Description : Call all of the registered functions, handling any
                  objections.
*/
static int quit_check_func(void)
{
    quit_func_record *ptr;
    int ok;

    // Loop through all the registered functions until one objects
    ptr = quit_func_head;
    ok = TRUE;
    while (ptr && ok)
    {
        const char *msg;

        // Allow this function to object
        msg = (*(ptr->func))(ptr->handle, &quit_func_done);

        // Handle the result
        if (msg)
        {
            // Record the details of the function to callback
            quit_func_done_handle = ptr->handle;

            // Set the message and display the dialogue box
            quit_set_message(0, quit_id, msg);
            toolbox_show_object(toolbox_SHOW_AS_MENU, quit_id,
                                toolbox_POSITION_DEFAULT, NULL,
                                toolbox_NULL_OBJECT,
                                toolbox_NULL_COMPONENT);

            // Do not call any other functions
            ok = FALSE;
        }
        else
        {
            // Advance to the next registered function
            ptr = ptr->next;
        }
    }

    // Return the status
    return ok;
}

/*
    Parameters  : void
    Returns     : int   - Was the quit accepted by all the children.
    Description : Allow any active children to object to the shutdown.
*/
static int quit_check_children(void)
{
    // If there are any children then let the first one object
    if (quit_child_head)
    {
        wimp_message msg;

        // Send a Message_ARMEditPreQuit message to the child
        msg.size = sizeof(msg);
        msg.your_ref = 0;
        msg.action = message_ARMEDIT_PRE_QUIT;
        wimp_send_message(wimp_USER_MESSAGE_RECORDED, &msg,
                          quit_child_head->handle);
    }

    // Return the status
    return quit_child_head == NULL;
}

/*
    Parameters  : void
    Returns     : void
    Description : Start a shutdown sequence. This calls all registered
                  functions, and offers child tasks the opportunity to
                  object if required. If there are any objections then the
                  shutdown is suspended.
*/
static void quit_start(void)
{
    int ok;

    // Start by calling any registered functions
    ok = quit_check_func();

    // If still alright then check with children
    if (ok && (!quit_message.sender
               || ((quit_message.action == message_PREQUIT)
                   && (quit_message.data.prequit.flags
                       & wimp_PRE_QUIT_TASK_ONLY))
               || (quit_message.action == message_ARMEDIT_PRE_QUIT)))
    {
        ok = quit_check_children();
    }

    // Check the result of all the checks
    if (ok)
    {
        // No objections, so quit if required
        if (!quit_message.sender
            || ((quit_message.action == message_PREQUIT)
                 && (quit_message.data.prequit.flags
                     & wimp_PRE_QUIT_TASK_ONLY))
            || (quit_message.action == message_ARMEDIT_PRE_QUIT))
        {
            quit = TRUE;
        }
    }
    else
    {
        // Acknowledge the shutdown to suspend the process
        if (quit_message.sender)
        {
            wimp_t sender = quit_message.sender;

            wimp_send_message(wimp_USER_MESSAGE_ACKNOWLEDGE, &quit_message,
                              quit_message.sender);
            quit_message.sender = sender;
        }
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Restart a shutdown sequence as required if the user has
                  selected the quit option from a dialogue.
*/
static void quit_restart(void)
{
    // Check whether the sequence was started externally
    if (quit_message.sender)
    {
        // The action depends upon the message type
        switch (quit_message.action)
        {
            case message_PREQUIT:
                // Is this a single task quit
                if (quit_message.data.prequit.flags & wimp_PRE_QUIT_TASK_ONLY)
                {
                    // Restart the internal sequence
                    quit_start();
                }
                else
                {
                    wimp_block block;

                    // Send Ctrl-Shift-F12 to the task
                    wimp_get_caret_position((wimp_caret *) &block);
                    block.key.c = wimp_KEY_CONTROL | wimp_KEY_SHIFT
                                  | wimp_KEY_F12;
                    wimp_send_message(wimp_KEY_PRESSED,
                                      (wimp_message *) &block,
                                      quit_message.sender);
                }
                break;

            case message_ARMEDIT_PRE_QUIT:
                // Send Message_ARMEditRestartQuit to restart
                quit_message.action = message_ARMEDIT_RESTART_QUIT;
                wimp_send_message(wimp_USER_MESSAGE, &quit_message,
                                  quit_message.sender);
                break;

            default:
                // Do not know how to handle other messages
                break;
        }
    }
    else
    {
        // Restart the internal sequence
        quit_start();
    }
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle quit toolbox events.
*/
static bool quit_tb_quit(bits event_code, toolbox_action *action,
                         toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Call the current quit function
    if (quit_func_done) (*quit_func_done)(quit_func_done_handle);

    // Restart the shutdown
    quit_restart();

    // Claim the event
    return TRUE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Start a quit sequence. This calls any registered quit
                  handlers and offers any child tasks the opportunity to
                  object. This should normally only be used when a menu
                  option is selected; quit messages from the task manager
                  or other tasks are handled automatically.
*/
void quit_quit(void)
{
    // Clear the sender field to mark this as internal
    quit_message.sender = 0;

    // Start the shutdown procedure
    quit_start();
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_TaskCloseDown messages. This is used to
                  detect when child tasks exit.
*/
static int quit_message_task_close_down(wimp_message *message, void *handle)
{
    quit_child_record *ptr;

    NOT_USED(handle);

    // Remove any matching registered child tasks
    ptr = quit_child_head;
    while (ptr)
    {
        quit_child_record *next;

        next = ptr->next;
        if (ptr->handle == message->sender)
        {
            // Unlink this record
            if (ptr->prev) ptr->prev->next = ptr->next;
            else quit_child_head = ptr->next;
            if (ptr->next) ptr->next->prev = ptr->prev;

            // Release the memory used by this record
            delete ptr;
        }
        ptr = next;
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_PreQuit wimp message events.
*/
static int quit_message_pre_quit(wimp_message *message, void *handle)
{
    NOT_USED(message);
    NOT_USED(handle);

    // Copy the message to allow an acknowledgement or restart to be sent
    quit_message = *message;

    // Copy the message reference
    quit_message.your_ref = quit_message.my_ref;

    // Special case for old style messages
    if (quit_message.size < 24) quit_message.data.prequit.flags = 0;

    // Start the shutdown procedure
    quit_start();

    // Claim the event
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_Quit wimp message events.
*/
static int quit_message_quit(wimp_message *message, void *handle)
{
    NOT_USED(message);
    NOT_USED(handle);

    // Set the global quit flag
    quit = TRUE;

    // Claim the event
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_ARMEditPreQuit wimp message events.
*/
static int quit_message_armedit_pre_quit(wimp_message *message, void *handle)
{
    NOT_USED(message);
    NOT_USED(handle);

    // Copy the message to allow an acknowledgement or restart to be sent
    quit_message = *message;

    // Copy the message reference
    quit_message.your_ref = quit_message.my_ref;

    // Start the shutdown procedure
    quit_start();

    // Claim the event
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_ARMEditRestartQuit wimp message events.
*/
static int quit_message_armedit_restart_quit(wimp_message *message,
                                             void *handle)
{
    NOT_USED(message);
    NOT_USED(handle);

    // Restart the shutdown
    quit_restart();

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle undelivered messages.
*/
static int quit_message_acknowledge(wimp_event_no event_code,
                                    wimp_block *block,
                                    toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(handle);
    NOT_USED(id_block);

    // Split into different message types
    switch (block->message.action)
    {
        case message_ARMEDIT_PRE_QUIT:
            // Restart the quit sequence
            quit_restart();
            break;

        default:
            // Do not care about other messages
            break;
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : handle    - The WIMP handle of the child task.
    Returns     : void
    Description : Register a child task that should be asked before quitting.
                  There is no need to deregister the task; the task is
                  automatically unregistered when it exits.
*/
void quit_child(wimp_t handle)
{
    quit_child_record *ptr;

    // Create a new record for this task
    ptr = new quit_child_record;
    if (ptr)
    {
        // Fill in the details
        ptr->handle = handle;

        // Place this record at the head of the list
        ptr->prev = NULL;
        ptr->next = quit_child_head;
        if (quit_child_head) quit_child_head->prev = ptr;
        quit_child_head = ptr;
    }
}

/*
    Parameters  : func      - The function to register.
                  handle    - User specified data to be passed to the function.
    Returns     : void
    Description : Register a function that will be called when a quit sequence
                  has been started.
*/
void quit_register(quit_func_check func, void *handle)
{
    quit_func_record *ptr;

    // Create a new record for this function
    ptr = new quit_func_record;
    if (ptr)
    {
        // Fill in the details
        ptr->func = func;
        ptr->handle = handle;

        // Place this record at the head of the list
        ptr->prev = NULL;
        ptr->next = quit_func_head;
        if (quit_func_head) quit_func_head->prev = ptr;
        quit_func_head = ptr;
    }
}

/*
    Parameters  : func      - The function to deregister.
                  handle    - User specified data specified when the function
                              was registered.
    Returns     : void
    Description : Deregister a previously registered function.
*/
void quit_deregister(quit_func_check func, void *handle)
{
    quit_func_record *ptr;

    // Find the first matching registered function
    ptr = quit_func_head;
    while (ptr && ((ptr->func != func) || (ptr->handle != handle)))
    {
        ptr = ptr->next;
    }

    // Remove any entry found
    if (ptr)
    {
        // Unlink this record
        if (ptr->prev) ptr->prev->next = ptr->next;
        else quit_func_head = ptr->next;
        if (ptr->next) ptr->next->prev = ptr->prev;

        // Free the memory
        delete ptr;
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Ensure that all child tasks quit at the same time as this
                  software.
*/
static void quit_exit(void)
{
    quit_child_record *ptr;

    // Kill all child tasks
    ptr = quit_child_head;
    while (ptr)
    {
        wimp_message msg;

        // Send Message_Quit to kill the child
        msg.size = sizeof(wimp_message);
        msg.your_ref = 0;
        msg.action = message_QUIT;
        wimp_send_message(wimp_USER_MESSAGE, &msg, ptr->handle);

        // Advance to the next child
        ptr = ptr->next;
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the quit handler.
*/
void quit_initialise(void)
{
    // Create a quit dialogue box
    quit_id = toolbox_create_object(0, (toolbox_id) "Quit");

    // Register message handlers
    event_register_wimp_handler(event_ANY, wimp_USER_MESSAGE_ACKNOWLEDGE,
                                quit_message_acknowledge, NULL);
    event_register_message_handler(message_QUIT,
                                   quit_message_quit, NULL);
    event_register_message_handler(message_PREQUIT,
                                   quit_message_pre_quit, NULL);
    event_register_message_handler(message_TASK_CLOSE_DOWN,
                                   quit_message_task_close_down, NULL);
    event_register_message_handler(message_ARMEDIT_PRE_QUIT,
                                   quit_message_armedit_pre_quit, NULL);
    event_register_message_handler(message_ARMEDIT_RESTART_QUIT,
                                   quit_message_armedit_restart_quit, NULL);

    // Register toolbox handlers
    event_register_toolbox_handler(event_ANY, action_QUIT_QUIT,
                                   quit_tb_quit, NULL);

    // Ensure that all child tasks are killed at the same time
    atexit(quit_exit);
}
