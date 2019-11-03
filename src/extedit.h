/*
    File        : extedit.h
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

#ifndef extedit_h
#define extedit_h

// Include oslib header files
#include "osfile.h"
#include "toolbox.h"
#include "wimp.h"

// An External Data Edit Protocol class
class ExtEdit
{
    // The private part of the class

    /*
        Parameters  : event_code    - The event number.
                      wimp_block    - The wimp poll block.
                      id_block      - The toolbox ID block.
                      handle        - An unused handle.
                                      object.
        Returns     : int           - Was the event claimed.
        Description : Handle User_Message_Acknowledge to detect messages that
                      were not acknowledged.
    */
    static int handler_message_acknowledge(wimp_event_no event_code,
                                           wimp_block *block,
                                           toolbox_block *id_block,
                                           void *handle);

    /*
        Parameters  : message   - The wimp message.
                      handle    - A handle used to reference the relevant
                                  object.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_EditAck messages. This is used to start
                      data transfer when an editor has replied to the request
                      message.
    */
    static int handler_message_edit_ack(wimp_message *message, void *handle);

    /*
        Parameters  : message   - The wimp message.
                      handle    - A handle used to reference the relevant
                                  object.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_EditAbort messages. This is used to
                      abandon an external edit by the editor.
    */
    static int handler_message_edit_abort(wimp_message *message, void *handle);

    /*
        Parameters  : message   - The wimp message.
                      handle    - A handle used to reference the relevant
                                  object.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_EditDataSave messages. This is used to
                      retrieve the file from the editor.
    */
    static int handler_message_edit_data_save(wimp_message *message,
                                              void *handle);

    /*
        Parameters  : message   - The wimp message.
                      handle    - A handle used to reference the relevant
                                  object.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_DataSaveAck messages. This is used to start
                      the actual transfer of data to the editor.
    */
    static int handler_message_data_save_ack(wimp_message *message,
                                             void *handle);

    /*
        Parameters  : message   - The wimp message.
                      handle    - A handle used to reference the relevant
                                  object.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_DataLoad messages. This is used to when the
                      editor has written the data to a file.
    */
    static int handler_message_data_load(wimp_message *message, void *handle);

    /*
        Parameters  : message   - The wimp message.
                      handle    - A handle used to reference the relevant
                                  object.
        Returns     : int       - Was the event claimed.
        Description : Handle Message_DataLoadAck messages. This is used to check
                      that the data has been received by the editor.
    */
    static int handler_message_data_load_ack(wimp_message *message,
                                             void *handle);

    static int count;                   // Unique edit handle counter
    int first;                          // Used to mark as unmodified
    int job;                            // Handle for this job
    int save;                           // Is file automatically saved
    int data_saved_ref;                 // Reference for marking data as safe
    int cursor_pos;                     // Required cursor position
    wimp_t editor;                      // Editor task handle
    int my_ref;                         // Reference of last message
    bits file_type;                     // File type of the data
    int file_size;                      // Length of the file
    char file_name[256];                // Actual filename
    char file_leaf[256];                // Leafname for display purposes
    static ExtEdit *head;               // Head of list of objects
    ExtEdit *prev, *next;               // The previous and next objects

public:

    // The public part of the class

    /*
        Parameters  : name      - The full pathname of the file to edit.
                      leaf      - Optional suggested leaf name for display.
                      parent    - Name of this application.
                      saved     - Automatically mark the file as saved when
                                  modified.
        Returns     : -
        Description : Constructor function.
    */
    ExtEdit(const char *name, const char *leaf = NULL,
            const char *parent = NULL, int saved = FALSE);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor function.
    */
    ~ExtEdit(void);

    /*
        Parameters  : void
        Returns     : void
        Description : Abort an external edit.
    */
    void abort(void);

    /*
        Parameters  : void
        Returns     : void
        Description : Retrieve the file from the editor.
    */
    void retrieve(void);

    /*
        Parameters  : pos   - The required cursor position.
        Returns     : void
        Description : Set the cursor position within the editor for this file.
    */
    void cursor(int pos);

    /*
        Parameters  : void
        Returns     : void
        Description : Mark the external edit as unmodified in the external
                      editor. This can only be used after the file has been
                      either retrieved, or returned by the editor (modified
                      flag set).
    */
    void saved(void);

    /*
        Parameters  : void
        Returns     : int   - The number of active external edits.
        Description : Count the number of external edits that are still active.
    */
    static int count_active(void);

    /*
        Parameters  : void
        Returns     : void
        Description : Abort all active external edits.
    */
    static void abort_all(void);

    int modified;                       // Has a modified version been saved
    int active;                         // Is the edit still active
    int started;                        // Has an external edit been started
};

#endif
