/*
    File        : msg.c++
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Handling of message passing via the ARMEdit module.

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
#include "msg.h"

// Include clib header files
#include <stdlib.h>
#include <stdio.h>

// Include oslib header files
#include "toolbox.h"
extern "C" {
#include "event.h"
}

// Include other project header files
#include "armeditswi.h"
#include "main.h"

// The client handle for this task
static int msg_frontend_handle;

// Linked list of registered handlers
struct msg_list
{
    msg_list *next;
    int msg_id;
    int msg_handle;
    int reason;
    msg_handler *handler;
    void *handle;
};
static msg_list *msg_head = NULL;

/*
    Parameters  : msg           - Pointer to the message block.
                  msg_id        - The ID of the sender.
                  msg_handle    - The handle of the sender.
                  handle        - A user defined pointer.
    Returns     : bool          - Has the message buffer been updated to
                                  contain a reply. If this is set then no
                                  more handlers will be called for this
                                  message.
    Description : Handle ARMEdit find messages.
*/
static bool msg_find(char *msg, int msg_id, int msg_handle, void *handle)
{
    NOT_USED(msg);
    NOT_USED(msg_id);
    NOT_USED(msg_handle);
    NOT_USED(handle);

    // Simply reply to the message
    return TRUE;
}

/*
    Parameters  : msg_id        - Client ID to add handler for.
                  msg_handle    - Client handle to add handler for.
                  reason        - Message reason code to add handler for.
                  handler       - The handler function.
                  handle        - A user defined pointer.
    Returns     : void
    Description : Add a handler for the specified message(s). If the reason
                  code is specified, then either the client ID or handle
                  should also be included.
*/
void msg_register_handler(int msg_id, int msg_handle, int reason,
                          msg_handler *handler, void *handle)
{
    msg_list *ptr;

    // Allocate memory for a new record
    ptr = new msg_list;
    if (!ptr) return;

    // Fill in all the details
    ptr->msg_id = msg_id;
    ptr->msg_handle = msg_handle;
    ptr->reason = reason;
    ptr->handler = handler;
    ptr->handle = handle;

    // Link in this record
    ptr->next = msg_head;
    msg_head = ptr;
}

/*
    Parameters  : msg_id        - Client ID to add handler for.
                  msg_handle    - Client handle to add handler for.
                  reason        - Message reason code to add handler for.
                  handler       - The handler function.
                  handle        - A user defined pointer.
    Returns     : void
    Description : Deregister a handler for the specified message(s). All of
                  the parameters must match.
*/
void msg_deregister_handler(int msg_id, int msg_handle, int reason,
                            msg_handler *handler, void *handle)
{
    msg_list *ptr, **prev;

    // Find the required handler
    prev = &msg_head;
    ptr = *prev;
    while (ptr && ((ptr->msg_id != msg_id) || (ptr->msg_handle != msg_handle)
                   || (ptr->reason != reason) || (ptr->handler != handler)
                   || (ptr->handle != handle)))
    {
        prev = &(ptr->next);
        ptr = *prev;
    }
    if (!ptr) return;

    // Unlink the record
    *prev = ptr->next;

    // Free the memory
    delete ptr;
}

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle wimp pollword non-zero events.
*/
bool msg_handler_pollword(wimp_event_no event_code, wimp_block *block,
                          toolbox_block *id_block, void *handle)
{
    int msg_id, msg_handle;
    char *msg;
    os_error *er;

    NOT_USED(event_code);
    NOT_USED(block);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Handle any waiting messages
    er = xarmedit_talk_rx(msg_frontend_handle, (void **) &msg, &msg_id,
                          &msg_handle);
    while (!er && msg)
    {
        int reply = FALSE;
        msg_list *ptr = msg_head;
        int reason = *((int *) msg);

        // Call all matching handlers until one replies
        while (ptr && !reply)
        {

            // Call this handler if it matches
            if (((ptr->msg_id == MSG_ID_ANY)
                 || (ptr->msg_id == msg_id))
                && ((ptr->msg_handle == MSG_HANDLE_ANY)
                    || (ptr->msg_handle == msg_handle))
                && ((ptr->reason == MSG_REASON_ANY)
                    || (ptr->reason == reason)))
            {
                reply = (*ptr->handler)(msg, msg_id, msg_handle, ptr->handle);
            }

            // Advance to the next handler
            ptr = ptr->next;
        }

        // Either send a reply, or acknowledge the message
        if (reply) xarmedit_talk_reply(msg_frontend_handle, msg_handle, msg);
        else xarmedit_talk_ack(msg_frontend_handle);

        // Check for any more pending messages
        er = xarmedit_talk_rx(msg_frontend_handle, (void **) &msg, &msg_id,
                              &msg_handle);
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Tidy up before exiting.
*/
static void msg_tidy(void)
{
    // Deregister as a message passing client
    xarmedit_talk_end(msg_frontend_handle);
}

/*
    Parameters  : void
    Returns     : int * - Pointer to a poll word.
    Description : Initialise the message passing system.
*/
int *msg_initialise(void)
{
    int *poll_word;

    // Initialise message handler
    armedit_talk_start(MSG_ID_FRONTEND, ARMEditTalkStart_FlagsARMEditMessages,
                       NULL, 0, &msg_frontend_handle, &poll_word);
    event_set_mask(wimp_GIVEN_POLLWORD);
    event_register_wimp_handler(event_ANY, wimp_POLLWORD_NON_ZERO,
                                msg_handler_pollword, NULL);
    atexit(msg_tidy);

    // The only message handled internally
    msg_register_handler(MSG_ID_PC, MSG_HANDLE_ANY, MSG_REASON_FRONTEND_FIND,
                         msg_find, NULL);

    // Return pointer to the poll word
    return poll_word;
}
