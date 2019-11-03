/*
    File        : go.c++
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

// Include header file for this module
#include "go.h"

// Include cpplib header files
#include "sstream.h"

// Include oslib header files
extern "C" {
#include "event.h"
}

// Include project header files
#include "armeditmsg.h"
#include "utils.h"

// Callback function when application has been launched
typedef void (* go_callback_func)(wimp_t task, void *handle);

// The path of the launcher application
#define GO_PATH "<ARMEdit$Dir>.Launch"

// List of pending applications to start
struct go_list
{
    go_list *next;
    go_list *prev;
    go_callback_func func;
    void *handle;
};
static go_list *go_head = NULL;

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_ARMEditLaunched messages. This is used to
                  callback the required function.
*/
static int go_launched(wimp_message *message, void *handle)
{
    go_list *ptr = (go_list *) ((wimp_message_armedit_launched *)
                                &message->data)->id;

    NOT_USED(handle);

    // Not all launches have associated records
    if (ptr)
    {
        // Call the callback function
        (*(ptr->func))(((wimp_message_armedit_launched *)
                       &message->data)->handle,
                       ptr->handle);

        // Unlink the record
        if (ptr->next) ptr->next->prev = ptr->prev;
        if (ptr->prev) ptr->prev->next = ptr->next;
        else go_head = ptr->next;

        // Release the memory
        delete ptr;
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Initialise this module.
*/
static void go_initialise(void)
{
    static int done = FALSE;

    // Only initialise once
    if (!done)
    {
        // Register event handlers
        event_register_message_handler(message_ARMEDIT_LAUNCHED,
                                       go_launched, NULL);

        // Prevent multiple initialisations
        done = TRUE;
    }
}

/*
    Parameters  : cmd       - The command to execute.
                  func      - The function to call when the command has been
                              performed.
                  handle    - The handle of the task started.
    Returns     : void
    Description : Start a new WIMP task indirectly. This is useful for
                  launching other Toolbox applications.
*/
void go_start_task(const string &cmd, go_callback_func func, void *handle)
{
    go_list *ptr = NULL;

    // Initialise if it was the first call
    go_initialise();

    // If a callback function is specified then add a suitable record
    if (func)
    {
        // Allocate memory for this record
        ptr = new go_list;

        // Complete the record details
        ptr->func = func;
        ptr->handle = handle;

        // Link the record in
        ptr->prev = NULL;
        ptr->next = go_head;
        if (go_head) go_head->prev = ptr;
        go_head = ptr;
    }

    // Build the actual command line
    stringstream real_cmd;
    real_cmd << GO_PATH << " " << (int) task_handle << " " << (int) ptr
             << " " << cmd;

    // Start the launcher
    wimp_start_task(real_cmd.str().c_str());
}
