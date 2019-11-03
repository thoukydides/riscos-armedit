/*
    File        : edit.c++
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Handle ARMEdit messages related to external editing of files.

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
#include "edit.h"

// Include clib header files
#include <string.h>

// Include other project header files
#include "extedit.h"
#include "main.h"
#include "msg.h"

// Linked list of edits
struct edit_list
{
    edit_list *prev;
    edit_list *next;
    ExtEdit *edit;
};
static edit_list *edit_head = NULL;

/*
    Parameters  : msg           - Pointer to the message block.
                  msg_id        - The ID of the sender.
                  msg_handle    - The handle of the sender.
                  handle        - A user defined pointer.
    Returns     : bool          - Has the message buffer been updated to
                                  contain a reply. If this is set then no
                                  more handlers will be called for this
                                  message.
    Description : Handle ARMEdit start edit messages.
*/
static bool edit_start(char *msg, int msg_id, int msg_handle, void *handle)
{
    struct rx_struct
    {
        int reason;
        int cursor;
        char leaf[20];
        char path[256];
    } *rx = (struct rx_struct *) msg;
    struct tx_struct
    {
        int reason;
        int handle;
    } *tx = (struct tx_struct *) msg;
    edit_list *ptr;

    NOT_USED(msg_id);
    NOT_USED(msg_handle);
    NOT_USED(handle);

    // Create a new edit structure
    ptr = new edit_list;
    if (!ptr) return FALSE;

    // Start an external edit
    ptr->edit = new ExtEdit(rx->path, rx->leaf, "ARMEdit");
    if (!(ptr->edit))
    {
        delete ptr;
        return FALSE;
    }

    // Set the initial cursor position
    ptr->edit->cursor(rx->cursor);

    // Link the entry in
    ptr->prev = NULL;
    ptr->next = edit_head;
    if (edit_head) edit_head->prev = ptr;
    edit_head = ptr;

    // Send the reply to the message
    tx->handle = (int) ptr;
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
    Description : Handle ARMEdit poll edit messages.
*/
static bool edit_poll(char *msg, int msg_id, int msg_handle, void *handle)
{
    struct rx_struct
    {
        int reason;
        int handle;
        int flags;
    } *rx = (struct rx_struct *) msg;
    struct tx_struct
    {
        int reason;
        int flags;
    } *tx = (struct tx_struct *) msg;
    edit_list *ptr;

    NOT_USED(msg_id);
    NOT_USED(msg_handle);
    NOT_USED(handle);

    // Obtain a pointer to the edit record
    ptr = (edit_list *) rx->handle;

    // Update the status of the edit
    if (rx->flags & MSG_FRONTEND_POLL_TX_FLAG_RETRIEVE) ptr->edit->retrieve();
    if (rx->flags & MSG_FRONTEND_POLL_TX_FLAG_ABORT) ptr->edit->abort();
    if (rx->flags & MSG_FRONTEND_POLL_TX_FLAG_SAFE) ptr->edit->saved();

    // Return the current status
    tx->flags = 0;
    if (ptr->edit->modified) tx->flags |= MSG_FRONTEND_POLL_RX_FLAG_MODIFIED;
    if (!ptr->edit->active)
    {
        // Kill the edit, unlink and delete the record
        tx->flags |= MSG_FRONTEND_POLL_RX_FLAG_ABORTED;
        delete ptr->edit;
        if (ptr->prev) ptr->prev->next = ptr->next;
        else edit_head = ptr->next;
        if (ptr->next) ptr->next->prev = ptr->prev;
        delete ptr;
    }
    else ptr->edit->modified = FALSE;

    // Simply reply to the message
    return TRUE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the handling of external edit messages.
*/
void edit_initialise(void)
{
    // Register handlers for the required messages
    msg_register_handler(MSG_ID_PC, MSG_HANDLE_ANY, MSG_REASON_FRONTEND_START,
                         edit_start, NULL);
    msg_register_handler(MSG_ID_PC, MSG_HANDLE_ANY, MSG_REASON_FRONTEND_POLL,
                         edit_poll, NULL);
}
