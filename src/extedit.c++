/*
    File        : extedit.c++
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : An implementation of the client end of the External Data
                  Editing Protocol.

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

/*
    Change the started field to a status
*/

// Include header file for this modue
#include "extedit.h"

// Include clib header files
#include <string.h>

// Include oslib header files
#include "osfscontrol.h"
extern "C" {
#include "event.h"
}

// Message numbers
#define message_EDIT_RQ 0x45D80         // Request external editing session
#define message_EDIT_ACK 0x45D81        // Acknowledge external edit
#define message_EDIT_RETURN 0x45D82     // Request return of external edit data
#define message_EDIT_ABORT 0x45D83      // Close editing session completely
#define message_EDIT_DATA_SAVE 0x45D84  // External edit equivalent of DataSave
#define message_EDIT_CURSOR 0x45D85     // Cursor/Selection placement

// Flag bits
#define flag_EDIT_CONTINUE (1 << 0)     // Continue editing
#define flag_EDIT_SELECTION (1 << 1)    // Selection only
#define flag_EDIT_READ_ONLY (1 << 2)    // Read only
#define flag_EDIT_EXECUTE (1 << 3)      // Immediate execution
#define flag_EDIT_ADJUST (1 << 4)       // Adjust selection

// Maximum sizes of various fields
#define EXTEDIT_PARENT_NAME_SIZE 20
#define EXTEDIT_LEAF_NAME_SIZE 204

// The message contents for Message_EditRq
struct wimp_message_edit_rq
{
    bits type;
    int handle;
    bits flags;
    char parent[EXTEDIT_PARENT_NAME_SIZE];
    char leaf[EXTEDIT_LEAF_NAME_SIZE];
};

// The message contents for Message_EditAck
struct wimp_message_edit_ack
{
    bits type;
    int handle;
    bits flags;
};

// The message contents for Message_EditReturn
struct wimp_message_edit_return
{
    bits type;
    int handle;
    bits flags;
};

// The message contents for Message_EditAbort
struct wimp_message_edit_abort
{
    int reserved;
    int handle;
};

// The message contents for Message_EditDataSave
struct wimp_message_edit_data_save
{
    int handle;
    int i, x, y;
    int est_size;
    bits file_type;
    char file_name[212];
};

// The message contents for Message_EditCursor
struct wimp_message_edit_cursor
{
    int reserved;
    int handle;
    bits flags;
    int cursor;
    int sel_start;
    int sel_end;
    int old_cursor;
    int old_sel_start;
    int old_sel_end;
};

// The unique edit handle counter
int ExtEdit::count = 0;

// The head of the list of objects
ExtEdit *ExtEdit::head = NULL;

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle User_Message_Acknowledge to detect messages that were
                  not acknowledged.
*/
int ExtEdit::handler_message_acknowledge(wimp_event_no event_code,
                                         wimp_block *block,
                                         toolbox_block *id_block, void *handle)
{
    ExtEdit *this_ptr = (ExtEdit *) handle;

    NOT_USED(event_code);
    NOT_USED(block);
    NOT_USED(id_block);

    // Split into different message types
    switch (block->message.action)
    {
        case message_EDIT_RQ:
            // Message_EditRq was not acknowledged
            if ((!this_ptr->started)
                && (block->message.my_ref == this_ptr->my_ref))
            {
                // No editor found
                this_ptr->active = FALSE;

                // Claim the event
                return TRUE;
            }
            break;

        case message_EDIT_DATA_SAVE:
            // Message_EditDataSave was not acknowledged
        case message_DATA_LOAD:
            // Message_DataLoad was not acknowledged
        default:
            // Do not care about any other messages
            break;
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - A handle used to reference the relevant object.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_EditAck messages. This is used to start
                  data transfer when an editor has replied to the request
                  message.
*/
int ExtEdit::handler_message_edit_ack(wimp_message *message, void *handle)
{
    ExtEdit *this_ptr = (ExtEdit *) handle;

    // Check the reference field
    if ((!this_ptr->started) && (message->your_ref == this_ptr->my_ref))
    {
        wimp_message msg;
        wimp_message_edit_ack *edit_ack;
        wimp_message_edit_data_save *edit_data_save;

        // Copy the complete job and task handle
        edit_ack = (wimp_message_edit_ack *) &message->data;
        this_ptr->job = edit_ack->handle;
        this_ptr->editor = message->sender;

        // Status is now active
        this_ptr->active = this_ptr->started = TRUE;

        // Start sending the data by sending Message_DataSave
        msg.size = sizeof(wimp_message);
        msg.your_ref = 0;
        msg.action = message_EDIT_DATA_SAVE;
        edit_data_save = (wimp_message_edit_data_save *) &msg.data;
        edit_data_save->handle = this_ptr->job;
        edit_data_save->est_size = this_ptr->file_size;
        edit_data_save->file_type = this_ptr->file_type & 0xfff;
        strcpy(edit_data_save->file_name, this_ptr->file_leaf);
        wimp_send_message(wimp_USER_MESSAGE_RECORDED, &msg, this_ptr->editor);
        this_ptr->my_ref = this_ptr->data_saved_ref = msg.my_ref;

        // Claim the event
        return TRUE;
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - A handle used to reference the relevant object.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_EditAbort messages. This is used to abandon
                  an external edit by the editor.
*/
int ExtEdit::handler_message_edit_abort(wimp_message *message, void *handle)
{
    ExtEdit *this_ptr = (ExtEdit *) handle;
    wimp_message_edit_abort *edit_abort;

    // Check the job handle
    edit_abort = (wimp_message_edit_abort *) &message->data;
    if (this_ptr->active && (edit_abort->handle == this_ptr->job))
    {
        // The edit has been abandoned
        this_ptr->active = FALSE;
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - A handle used to reference the relevant object.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_EditDataSave messages. This is used to
                  retrieve the file from the editor.
*/
int ExtEdit::handler_message_edit_data_save(wimp_message *message, void *handle)
{
    ExtEdit *this_ptr = (ExtEdit *) handle;
    wimp_message_edit_data_save *edit_data_save;

    // Check that the job handle is correct
    edit_data_save = (wimp_message_edit_data_save *) &message->data;
    if (this_ptr->active && (edit_data_save->handle == this_ptr->job))
    {
        if (this_ptr->first)
        {
            // Mark the data as unmodified
            this_ptr->data_saved_ref = message->my_ref;
            this_ptr->saved();
            this_ptr->first = FALSE;
        }
        else
        {
            // Request the data by sending Message_DataSaveAck
            message->size = sizeof(wimp_message);
            message->your_ref = this_ptr->data_saved_ref = message->my_ref;
            message->action = message_DATA_SAVE_ACK;
            message->data.data_xfer.w = 0;
            message->data.data_xfer.i = 0;
            message->data.data_xfer.pos.x = 0;
            message->data.data_xfer.pos.y = 0;
            message->data.data_xfer.est_size = -1;
            strcpy(message->data.data_xfer.file_name, "<Wimp$Scrap>");
            wimp_send_message(wimp_USER_MESSAGE, message, message->sender);
            this_ptr->my_ref = message->my_ref;
        }

        // Claim the event
        return TRUE;
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - A handle used to reference the relevant object.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataSaveAck messages. This is used to start
                  the actual transfer of data to the editor.
*/
int ExtEdit::handler_message_data_save_ack(wimp_message *message, void *handle)
{
    ExtEdit *this_ptr = (ExtEdit *) handle;
    os_error *er;

    // Check the reference field
    if (this_ptr->started && (message->your_ref == this_ptr->my_ref))
    {
        // Copy the file to the required location
        er = xosfscontrol_copy(this_ptr->file_name,
                               message->data.data_xfer.file_name,
                               osfscontrol_COPY_FORCE, 0, 0, 0, 0, NULL);

        // Only send the data if successful
        if (!er)
        {
            // Start sending the data by sending Message_DataLoad
            message->size = sizeof(wimp_message);
            message->your_ref = message->my_ref;
            message->action = message_DATA_LOAD;
            message->data.data_xfer.est_size = this_ptr->file_size;
            message->data.data_xfer.file_type = this_ptr->file_type & 0xfff;
            wimp_send_message(wimp_USER_MESSAGE_RECORDED, message,
                              message->sender);
            this_ptr->my_ref = message->my_ref;
        }

        // Claim the event
        return TRUE;
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - A handle used to reference the relevant object.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataLoad messages. This is used to when the
                  editor has written the data to a file.
*/
int ExtEdit::handler_message_data_load(wimp_message *message, void *handle)
{
    ExtEdit *this_ptr = (ExtEdit *) handle;
    os_error *er;

    // Check the reference field
    if (this_ptr->started && (message->your_ref == this_ptr->my_ref))
    {
        // Copy the updated file to the required location
        er = xosfscontrol_copy(message->data.data_xfer.file_name,
                               this_ptr->file_name,
                               osfscontrol_COPY_FORCE
                               | osfscontrol_COPY_DELETE,
                               0, 0, 0, 0, NULL);

        // Set the modified flag if successful
        if (!er) this_ptr->modified = TRUE;

        // Acknowledge the data by sending Message_DataLoadAck
        message->size = sizeof(wimp_message);
        message->your_ref = message->my_ref;
        message->action = message_DATA_LOAD_ACK;
        wimp_send_message(wimp_USER_MESSAGE, message, message->sender);
        this_ptr->my_ref = message->my_ref;

        // Mark as a safe save if required
        if (this_ptr->save) this_ptr->saved();

        // Claim the event
        return TRUE;
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - A handle used to reference the relevant object.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataLoadAck messages. This is used to check
                  that the data has been received by the editor.
*/
int ExtEdit::handler_message_data_load_ack(wimp_message *message, void *handle)
{
    ExtEdit *this_ptr = (ExtEdit *) handle;

    // Check the reference field
    if (this_ptr->started && (message->your_ref == this_ptr->my_ref))
    {
        // Mark data as unmodified
        this_ptr->first = TRUE;
        this_ptr->retrieve();

        // Position the cursor at the required position
        this_ptr->cursor(this_ptr->cursor_pos);

        // Claim the event
        return TRUE;
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : name      - The full pathname of the file to edit.
                  leaf      - Optional suggested leaf name for display.
                  parent    - Name of this application.
                  saved     - Automatically mark the file as saved when
                              modified.
    Returns     : -
    Description : Constructor function.
*/
ExtEdit::ExtEdit(const char *name, const char *leaf, const char *parent,
                 int saved)
{
    wimp_message msg;
    wimp_message_edit_rq *edit_rq;
    fileswitch_object_type obj_type;
    os_error *er;

    NOT_USED(name);

    // Install handlers for the External Data Editing Protocol messages
    event_register_wimp_handler(event_ANY, wimp_USER_MESSAGE_ACKNOWLEDGE,
                                handler_message_acknowledge, this);
    event_register_message_handler(message_EDIT_ACK,
                                   handler_message_edit_ack, this);
    event_register_message_handler(message_EDIT_ABORT,
                                   handler_message_edit_abort, this);
    event_register_message_handler(message_EDIT_DATA_SAVE,
                                   handler_message_edit_data_save, this);
    event_register_message_handler(message_DATA_SAVE_ACK,
                                   handler_message_data_save_ack, this);
    event_register_message_handler(message_DATA_LOAD,
                                   handler_message_data_load, this);
    event_register_message_handler(message_DATA_LOAD_ACK,
                                   handler_message_data_load_ack, this);

    // Initialise variables
    job = count;
    cursor_pos = 0;
    strcpy(file_leaf, leaf);
    active = TRUE;
    started = modified = first = FALSE;
    save = saved;

    // Canonicalise the filename
    er = xosfscontrol_canonicalise_path(name, file_name, NULL, NULL,
                                        sizeof(file_name), NULL);

    // Read the file details
    if (!er)
    {
        er = xosfile_read_stamped(file_name, &obj_type, NULL, NULL,
                                  &file_size, NULL, &file_type);
    }
    if (er || (obj_type != fileswitch_IS_FILE)) active = FALSE;

    if (active)
    {
        // Fake the filetype
        file_type = 0xfff;

        // Send Message_EditRq message to find an editor
        msg.size = sizeof(wimp_message);
        msg.your_ref = 0;
        msg.action = message_EDIT_RQ;
        edit_rq = (wimp_message_edit_rq *) &msg.data;
        edit_rq->type = file_type;
        edit_rq->handle = job;
        edit_rq->flags = flag_EDIT_CONTINUE;
        if (parent) strcpy(edit_rq->parent, parent);
        else edit_rq->parent[0] = 0;
        strcpy(edit_rq->leaf, leaf);
        wimp_send_message(wimp_USER_MESSAGE_RECORDED, &msg, wimp_BROADCAST);
        my_ref = msg.my_ref;

        // Update the handle so that any new files are distinct
        count = (count + 1) & 0xffff;
    }

    // Add this object to the list
    prev = NULL;
    next = head;
    if (next) next->prev = this;
    head = this;
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor function.
*/
ExtEdit::~ExtEdit(void)
{
    // Abort the edit if it is still active
    abort();

    // Remove this record from the list
    if (prev) prev->next = next;
    else head = next;
    if (next) next->prev = prev;

    // Deinstall all handlers
    event_deregister_message_handler(message_DATA_LOAD_ACK,
                                     handler_message_data_load_ack, this);
    event_deregister_message_handler(message_DATA_LOAD,
                                     handler_message_data_load, this);
    event_deregister_message_handler(message_DATA_SAVE_ACK,
                                     handler_message_data_save_ack, this);
    event_deregister_message_handler(message_EDIT_DATA_SAVE,
                                     handler_message_edit_data_save, this);
    event_deregister_message_handler(message_EDIT_ABORT,
                                     handler_message_edit_abort, this);
    event_deregister_message_handler(message_EDIT_ACK,
                                     handler_message_edit_ack, this);
    event_deregister_wimp_handler(event_ANY, wimp_USER_MESSAGE_ACKNOWLEDGE,
                                  handler_message_acknowledge, this);
}

/*
    Parameters  : void
    Returns     : void
    Description : Abort an external edit.
*/
void ExtEdit::abort(void)
{
    // Check if the external edit is still active
    if (started && active)
    {
        wimp_message msg;
        wimp_message_edit_abort *edit_abort;

        // Abandon the edit by sending Message_EditAbort
        msg.size = sizeof(wimp_message);
        msg.your_ref = 0;
        msg.action = message_EDIT_ABORT;
        edit_abort = (wimp_message_edit_abort *) &msg.data;
        edit_abort->reserved = 0;
        edit_abort->handle = job;
        wimp_send_message(wimp_USER_MESSAGE, &msg, editor);

        // Clear active edit flag
        active = FALSE;
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Retrieve the file from the editor.
*/
void ExtEdit::retrieve(void)
{
    // Check if the external edit is still active
    if (started && active)
    {
        wimp_message msg;
        wimp_message_edit_return *edit_return;

        // Retrieve the data by sending Message_EditReturn
        msg.size = sizeof(wimp_message);
        msg.your_ref = 0;
        msg.action = message_EDIT_RETURN;
        edit_return = (wimp_message_edit_return *) &msg.data;
        edit_return->type = file_type;
        edit_return->handle = job;
        edit_return->flags = flag_EDIT_CONTINUE;
        wimp_send_message(wimp_USER_MESSAGE, &msg, editor);
    }
}

/*
    Parameters  : pos   - The required cursor position.
    Returns     : void
    Description : Set the cursor position within the editor for this file.
*/
void ExtEdit::cursor(int pos)
{
    cursor_pos = pos;

    // Only set the position if active
    if (active && started)
    {
        wimp_message msg;
        wimp_message_edit_cursor *edit_cursor;

        // Position the cursor at the start by sending Message_EditCursor
        msg.size = sizeof(wimp_message);
        msg.your_ref = 0;
        msg.action = message_EDIT_CURSOR;
        edit_cursor = (wimp_message_edit_cursor *) &msg.data;
        edit_cursor->handle = job;
        edit_cursor->cursor = cursor_pos;
        edit_cursor->sel_start = -1;
        edit_cursor->sel_end = -1;
        wimp_send_message(wimp_USER_MESSAGE, &msg, editor);
        my_ref = msg.my_ref;
    }
}

/*
    Parameters  : void
    Returns     : int   - The number of active external edits.
    Description : Count the number of external edits that are still active.
*/
int ExtEdit::count_active(void)
{
    int count = 0;
    ExtEdit *ptr = head;

    // Check every object
    while (ptr)
    {
        if (ptr->active) count++;
        ptr = ptr->next;
    }

    // Return the total
    return count;
}

/*
    Parameters  : void
    Returns     : void
    Description : Abort all active external edits.
*/
void ExtEdit::abort_all(void)
{
    ExtEdit *ptr = head;

    // Check every object
    while (ptr)
    {
        ptr->abort();
        ptr = ptr->next;
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Mark the external edit as unmodified in the external
                  editor. This can only be used after the file has been
                  either retrieved, or returned by the editor (modified
                  flag set).
*/
void ExtEdit::saved(void)
{
    // Only mark the data as saved if active
    if (active && started)
    {
        wimp_message msg;

        // Send Message_DataSaved to mark the data as safe
        msg.size = sizeof(wimp_message);
        msg.your_ref = data_saved_ref;
        msg.action = message_DATA_SAVED;
        wimp_send_message(wimp_USER_MESSAGE, &msg, editor);
        my_ref = msg.my_ref;
    }
}
