/*
    File        : msg.h
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

#ifndef msg_h
#define msg_h

// Include project header files
#include "types.h"

// Predefined client IDs
#define MSG_ID_ANY -1
#define MSG_ID_PC 0x0000
#define MSG_ID_MODULE 0x0001
#define MSG_ID_FRONTEND 0x0100

// Predefined handle codes
#define MSG_HANDLE_ANY 0

// Predefined reason codes
#define MSG_REASON_ANY -1
#define MSG_REASON_MODULE_RESET 0x00
#define MSG_REASON_MODULE_SHUTDOWN 0x01
#define MSG_REASON_FRONTEND_FIND 0x00
#define MSG_REASON_FRONTEND_START 0x01
#define MSG_REASON_FRONTEND_POLL 0x02
#define MSG_REASON_FRONTEND_SAVEAS 0x03
#define MSG_REASON_FRONTEND_OSCLI_START 0x04
#define MSG_REASON_FRONTEND_OSCLI_POLL 0x05
#define MSG_REASON_FRONTEND_OSCLI_END 0x06

// Polling flags
#define MSG_FRONTEND_POLL_TX_FLAG_RETRIEVE 0x01
#define MSG_FRONTEND_POLL_TX_FLAG_ABORT 0x02
#define MSG_FRONTEND_POLL_TX_FLAG_SAFE 0x04
#define MSG_FRONTEND_POLL_RX_FLAG_MODIFIED 0x01
#define MSG_FRONTEND_POLL_RX_FLAG_ABORTED 0x02

/*
    Parameters  : msg           - Pointer to the message block.
                  msg_id        - The ID of the sender.
                  msg_handle    - The handle of the sender.
                  handle        - A user defined pointer.
    Returns     : bool          - Has the message buffer been updated to
                                  contain a reply. If this is set then no
                                  more handlers will be called for this
                                  message.
    Description : A function to handle messages.
*/
typedef bool msg_handler(char *msg, int msg_id, int msg_handle, void *handle);

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
                          msg_handler *handler, void *handle);

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
                            msg_handler *handler, void *handle);

/*
    Parameters  : void
    Returns     : int * - Pointer to a poll word.
    Description : Initialise the message passing system.
*/
int *msg_initialise(void);

#endif
