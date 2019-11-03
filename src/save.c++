/*
    File        : save.c++
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Handle ARMEdit messages related to saving of files.

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

// Include oslib header files
#include "osfile.h"
#include "osfscontrol.h"
#include "saveas.h"
extern "C" {
#include "event.h"
}

// Include other project header files
#include "main.h"
#include "msg.h"

// Linked list of files being saved
struct save_list
{
    save_list *prev;
    save_list *next;
    char path[256];
    toolbox_o id;
};
static save_list *save_head = NULL;

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Save to a specified file.
*/
static bool save_save(bits event_code, toolbox_action *action,
                      toolbox_block *id_block, void *handle)
{
    saveas_action_save_to_file *save =
        (saveas_action_save_to_file *) &action->data;
    save_list *ptr = (save_list *) handle;
    os_error *er;

    NOT_USED(event_code);
    NOT_USED(id_block);

    // Copy the file to the required location
    er = xosfscontrol_copy(ptr->path, save->file_name,
                           osfscontrol_COPY_FORCE, 0, 0, 0, 0, NULL);

    // Let the SaveAs module know the status
    saveas_file_save_completed(er ? 0 : 1, ptr->id, save->file_name);

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description :
*/
static bool save_complete(bits event_code, toolbox_action *action,
                          toolbox_block *id_block, void *handle)
{
    save_list *ptr = (save_list *) handle;

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(id_block);

    // Deregister handlers
    event_deregister_toolbox_handler(ptr->id, action_SAVE_AS_SAVE_TO_FILE,
                                     save_save, ptr);
    event_deregister_toolbox_handler(ptr->id,
                                     action_SAVE_AS_DIALOGUE_COMPLETED,
                                     save_complete, ptr);

    // Delete the SaveAs object
    toolbox_delete_object(0, ptr->id);

    // Delete the file
    xosfile_delete(ptr->path, NULL, NULL, NULL, NULL, NULL);

    // Unlink the data structure
    if (ptr->prev) ptr->prev->next = ptr->next;
    else save_head = ptr->next;
    if (ptr->next) ptr->next->prev = ptr->prev;

    // Delete the object record
    delete ptr;

    // Claim the event
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
    Description : Handle ARMEdit SaveAs messages.
*/
static bool save_start(char *msg, int msg_id, int msg_handle, void *handle)
{
    struct rx_struct
    {
        int reason;
        char path[256];
        char suggest[256];
    } *rx = (struct rx_struct *) msg;
    struct tx_struct
    {
        int reason;
    } *tx = (struct tx_struct *) msg;
    save_list *ptr;
    fileswitch_object_type obj_type;
    int file_size;
    bits file_type;
    os_error *er;

    NOT_USED(msg_id);
    NOT_USED(msg_handle);
    NOT_USED(handle);

    // Create a new save record
    ptr = new save_list;
    if (!ptr) return FALSE;

    // Canonicalise the filename
    er = xosfscontrol_canonicalise_path(rx->path, ptr->path, NULL, NULL,
                                        sizeof(ptr->path), NULL);

    // Read the file details
    if (!er)
    {
        er = xosfile_read_stamped(ptr->path, &obj_type, NULL, NULL,
                                  &file_size, NULL, &file_type);
    }

    // Ignore the request if file is unsuitable
    if (er || (obj_type != fileswitch_IS_FILE))
    {
        delete ptr;
        return FALSE;
    }

    // Create the SaveAs object
    ptr->id = toolbox_create_object(0, (toolbox_id) "SaveAs");
    saveas_set_file_size(0, ptr->id, file_size);
    saveas_set_file_name(0, ptr->id, rx->suggest);
    saveas_set_file_type(0, ptr->id, file_type);

    // Register handlers
    event_register_toolbox_handler(ptr->id, action_SAVE_AS_DIALOGUE_COMPLETED,
                                   save_complete, ptr);
    event_register_toolbox_handler(ptr->id, action_SAVE_AS_SAVE_TO_FILE,
                                   save_save, ptr);

    // Link the entry in
    ptr->prev = NULL;
    ptr->next = save_head;
    if (save_head) save_head->prev = ptr;
    save_head = ptr;

    // Send the reply to the message
    return TRUE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the handling of file saving messages.
*/
void save_initialise(void)
{
    // Register handlers for the required messages
    msg_register_handler(MSG_ID_PC, MSG_HANDLE_ANY, MSG_REASON_FRONTEND_SAVEAS,
                         save_start, NULL);
}
