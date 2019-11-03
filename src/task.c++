/*
    File        : task.c++
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1997-2001, 2019
    Description : Handle ARMEdit messages related to executing commands in
                  TaskWindows.

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
#include "task.h"

// Include clib header files
#include <string.h>
#include <stdio.h>

// Include oslib header files
#include "taskwindow.h"
#include "wimp.h"

// Include other project header files
#include "buffer.h"
#include "main.h"
#include "msg.h"
#include "utils.h"
extern "C" {
#include "event.h"
}

/* The details required for a single TaskWindow */
struct task_list
{
    task_list *prev;
    task_list *next;
    wimp_t handle;
    Buffer output;
    int active;
    int suspended;
};
static task_list *task_head = NULL;

/*
    Parameters  : msg           - Pointer to the message block.
                  msg_id        - The ID of the sender.
                  msg_handle    - The handle of the sender.
                  handle        - A user defined pointer.
    Returns     : bool          - Has the message buffer been updated to
                                  contain a reply. If this is set then no
                                  more handlers will be called for this
                                  message.
    Description : Handle ARMEdit start *command messages.
*/
static bool task_start(char *msg, int msg_id, int msg_handle, void *handle)
{
    struct rx_struct
    {
        int reason;
        char cmd[256];
        char name[256];
    } *rx = (struct rx_struct *) msg;
    struct tx_struct
    {
        int reason;
        int handle;
    } *tx = (struct tx_struct *) msg;
    task_list *ptr;
    char cmd[256];

    NOT_USED(msg_id);
    NOT_USED(msg_handle);
    NOT_USED(handle);

    // Create a new TaskWindow structure
    ptr = new task_list;
    if (!ptr) return FALSE;

    // Start a TaskWindow
    // May need to add -wimpslot switch
    sprintf(cmd, "TaskWindow \"%s\" -name \"%s\" -ctrl -quit"
            " -task &%8.8X -txt &%8.8X",
            rx->cmd, rx->name, task_handle, (int) ptr);
    wimp_start_task(cmd);

    // Link the entry in
    ptr->prev = NULL;
    ptr->next = task_head;
    if (task_head) task_head->prev = ptr;
    task_head = ptr;

    // Fill in other fields
    ptr->active = TRUE;
    ptr->suspended = FALSE;

    // Send the reply to the message
    tx->handle = (int) ptr;

    // Simply reply to the message.
    return TRUE;
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
    Description : Handle ARMEdit poll *command messages.
*/
static bool task_poll(char *msg, int msg_id, int msg_handle, void *handle)
{
    struct rx_struct
    {
        int reason;
        int handle;
        int input;
        char data[1000];
    } *rx = (struct rx_struct *) msg;
    struct tx_struct
    {
        int reason;
        int status;
        int output;
        char data[1000];
    } *tx = (struct tx_struct *) msg;
    task_list *ptr = (task_list *) rx->handle;
    char *input = rx->data;

    NOT_USED(msg_id);
    NOT_USED(msg_handle);
    NOT_USED(handle);

    // Find the record for the specified task
    ptr = task_head;
    while (ptr && ((int) ptr != rx->handle)) ptr = ptr->next;
    if (!ptr) return FALSE;

    // Send any input data to the task
    while (rx->input && ptr->active)
    {
        wimp_message wmsg;
        taskwindow_message_data *data = (taskwindow_message_data *) &wmsg.data;

        // Send a block of input to the task
        wmsg.size = sizeof(wmsg);
        wmsg.your_ref = 0;
        wmsg.action = message_TASK_WINDOW_INPUT;
        data->size = rx->input < sizeof(data->data)
                     ? rx->input : sizeof(data->data);
        memcpy(data->data, input, data->size);
        wimp_send_message(wimp_USER_MESSAGE, &wmsg, ptr->handle);

        // Update the source pointer
        input += data->size;
        rx->input -= data->size;
    }

    // Store any output data from the buffer
    tx->output = ptr->output.extract(sizeof(tx->data), tx->data);

    // Set the status
    tx->status = tx->output ? TRUE : ptr->active;

    // Restart the TaskWindow if output buffer not full
    if (ptr->suspended && ptr->active && !ptr->output.full())
    {
        wimp_message wmsg;

        // Resume execution
        wmsg.size = sizeof(wmsg);
        wmsg.your_ref = 0;
        wmsg.action = message_TASK_WINDOW_RESUME;
        wimp_send_message(wimp_USER_MESSAGE, &wmsg, ptr->handle);

        // Store the suspension state
        ptr->suspended = FALSE;
    }

    // Send the reply to the message.
    return TRUE;
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
    Description : Handle ARMEdit end *command messages.
*/
static bool task_end(char *msg, int msg_id, int msg_handle, void *handle)
{
    struct rx_struct
    {
        int reason;
        int handle;
    } *rx = (struct rx_struct *) msg;
    struct tx_struct
    {
        int reason;
    } *tx = (struct tx_struct *) msg;
    task_list *ptr = (task_list *) rx->handle;

    NOT_USED(msg_id);
    NOT_USED(msg_handle);
    NOT_USED(handle);

    // Find the record for the specified task
    ptr = task_head;
    while (ptr && ((int) ptr != rx->handle)) ptr = ptr->next;
    if (!ptr) return FALSE;

    // Check if the task is still active
    if (ptr->active)
    {
        wimp_message wmsg;

        // Kill the task
        wmsg.size = sizeof(wmsg);
        wmsg.your_ref = 0;
        wmsg.action = message_TASK_WINDOW_MORITE;
        wimp_send_message(wimp_USER_MESSAGE, &wmsg, ptr->handle);
    }

    // Unlink the record and free the memory
    if (ptr->next) ptr->next->prev = ptr->prev;
    if (ptr->prev) ptr->prev->next = ptr->next;
    else task_head = ptr->next;
    delete ptr;

    // Simply reply to the message.
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - A handle used to reference the relevant object.
    Returns     : int       - Was the event claimed.
    Description : Handle TaskWindow_Output messages used to obtain output from
                  a TaskWindow.
*/
static int handler_message_output(wimp_message *message, void *handle)
{
    taskwindow_message_data *data = (taskwindow_message_data *) &message->data;
    task_list *ptr;

    NOT_USED(handle);

    // Find the record for the specified task
    ptr = task_head;
    while (ptr && (ptr->handle != message->sender)) ptr = ptr->next;

    // Store the data if task has been found
    if (ptr)
    {
        // Add the data to the output buffer
        ptr->output.add(data->size, data->data);

        // Suspend the TaskWindow if output buffer full
        if (!ptr->suspended && ptr->active && ptr->output.full())
        {
            wimp_message wmsg;

            // Suspend execution
            wmsg.size = sizeof(wmsg);
            wmsg.your_ref = 0;
            wmsg.action = message_TASK_WINDOW_SUSPEND;
            wimp_send_message(wimp_USER_MESSAGE, &wmsg, ptr->handle);

            // Store the suspension state
            ptr->suspended = TRUE;
        }

        // Claim the event
        return TRUE;
    }

    // No not claim the event
    return FALSE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - A handle used to reference the relevant object.
    Returns     : int       - Was the event claimed.
    Description : Handle TaskWindow_Output messages used to obtain the child
                  task handle.
*/
static int handler_message_ego(wimp_message *message, void *handle)
{
    taskwindow_message_ego *data = (taskwindow_message_ego *) &message->data;
    task_list *ptr = (task_list *) data->txt;

    NOT_USED(handle);

    // Copy the child task handle
    ptr->handle = message->sender;

    // Claim the event
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - A handle used to reference the relevant object.
    Returns     : int       - Was the event claimed.
    Description : Handle TaskWindow_Morio messages used to notify the parent
                  when a TaskWindow has died.
*/
static int handler_message_morio(wimp_message *message, void *handle)
{
    task_list *ptr;

    NOT_USED(handle);

    // Find the record for the specified task
    ptr = task_head;
    while (ptr && (ptr->handle != message->sender)) ptr = ptr->next;

    // Update the status if task has been found
    if (ptr)
    {
        // Mark the task as inactive
        ptr->active = FALSE;

        // Claim the event
        return TRUE;
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the handling of TaskWindow messages.
*/
void task_initialise(void)
{
    // Register handlers for the required ARMEdit messages
    msg_register_handler(MSG_ID_PC, MSG_HANDLE_ANY,
                         MSG_REASON_FRONTEND_OSCLI_START, task_start, NULL);
    msg_register_handler(MSG_ID_PC, MSG_HANDLE_ANY,
                         MSG_REASON_FRONTEND_OSCLI_POLL, task_poll, NULL);
    msg_register_handler(MSG_ID_PC, MSG_HANDLE_ANY,
                         MSG_REASON_FRONTEND_OSCLI_END, task_end, NULL);

    // Install handlers for the TaskWindow messages
    event_register_message_handler(message_TASK_WINDOW_OUTPUT,
                                   handler_message_output, NULL);
    event_register_message_handler(message_TASK_WINDOW_EGO,
                                   handler_message_ego, NULL);
    event_register_message_handler(message_TASK_WINDOW_MORIO,
                                   handler_message_morio, NULL);
}
