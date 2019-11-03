/*
    File        : armeditswi.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Interface to the ARMEdit module.

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

#ifndef armeditswi_h
#define armeditswi_h

// Include oslib header files
#include "OS:os.h"

// SWI names and numbers
#define ARMEdit_ControlPC 0x4BC40
#define ARMEdit_TalkStart 0x4BC41
#define ARMEdit_TalkEnd 0x4BC42
#define ARMEdit_TalkTX 0x4BC43
#define ARMEdit_TalkRX 0x4BC44
#define ARMEdit_TalkAck 0x4BC45
#define ARMEdit_HPC 0x4BC46
#define ARMEdit_Polling 0x4BC47
#define ARMEdit_TalkReply 0x4BC48

// Operations to perform using ARMEdit_ControlPC
#define ARMEditControlPC_FreezeFullScreen 0x0
#define ARMEditControlPC_FreezeWindow 0x1
#define ARMEditControlPC_Reset 0x2
#define ARMEditControlPC_Quit 0x3

// Flags defined for ARMEdit_TalkStart
#define ARMEditTalkStart_FlagsARMEditMessages 0x1

// Message buffers
#define ARMEditTalk_BufferSize 1024
typedef char armedit_talk_buffer[ARMEditTalk_BufferSize];

#ifdef __cplusplus
extern "C" {
#endif

/*
    Parameters  : operation - The operation to perform.
    Returns     : os_error  - Pointer to a standard error block.
    Description : Control the PC front-end.
                  This calls XARMEdit_ControlPC.
*/
os_error *xarmedit_control_pc(int operation);

/*
    Parameters  : operation - The operation to perform.
    Returns     : void
    Description : Control the PC front-end.
                  This calls ARMEdit_ControlPC.
*/
void armedit_control_pc(int operation);

/*
    Parameters  : id        - Pre-allocated ID for this task.
                  flags     - The flags.
                  func      - Pointer to a function to be called when a
                              message is available, or 0 for none.
                  r12       - Value for r12 to contain when the function is
                              called.
                  rhandle   - Optional variable to receive the unique client
                              handle for this task.
                  rpoll     - Optional variable to receive the pointer to a
                              poll word for this task.
    Returns     : os_error  - Pointer to a standard error block.
    Description : Register a new client task.
                  This calls XARMEdit_TalkStart.
*/
os_error *xarmedit_talk_start(int id, int flags, void *func, int r12,
                              int *rhandle, int **rpoll);

/*
    Parameters  : id        - Pre-allocated ID for this task.
                  flags     - The flags.
                  func      - Pointer to a function to be called when a
                              message is available, or 0 for none.
                  r12       - Value for r12 to contain when the function is
                              called.
                  rhandle   - Optional variable to receive the unique client
                              handle for this task.
                  rpoll     - Optional variable to receive the pointer to a
                              poll word for this task.
    Returns     : int       - The unique client handle for this task.
    Description : Register a new client task.
                  This calls ARMEdit_TalkStart.
*/
int armedit_talk_start(int id, int flags, void *func, int r12, int *rhandle,
                       int **rpoll);

/*
    Parameters  : handle    - The previously assigned handle for this client
                              task.
    Returns     : os_error  - Pointer to a standard error block.
    Description : Deregister a client task.
                  This calls XARMEdit_TalkEnd.
*/
os_error *xarmedit_talk_end(int handle);

/*
    Parameters  : handle    - The previously assigned handle for this client
                              task.
    Returns     : void
    Description : Deregister a client task.
                  This calls ARMEdit_TalkEnd.
*/
void armedit_talk_end(int handle);

/*
    Parameters  : handle    - Client handle for this task.
                  dest      - Either the ID or client handle for the recipient
                              (if msg is a valid pointer).
                  msg       - Pointer to block containing the message to send,
                              or NULL to check if the buffer already contains a
                              message.
                  rmsg      - Optional variable to receive a pointer to the
                              message buffer, or NULL if no message is waiting
                              to be delivered.
    Returns     : os_error  - Pointer to a standard error block.
    Description : Send a message to another client task.
                  This calls XARMEdit_TalkTx.
*/
os_error *xarmedit_talk_tx(int handle, int dest, void *msg, void **rmsg);

/*
    Parameters  : handle    - Client handle for this task.
                  dest      - Either the ID or client handle for the recipient
                              (if msg is a valid pointer).
                  msg       - Pointer to block containing the message to send,
                              or NULL to check if the buffer already contains a
                              message.
                  rmsg      - Optional variable to receive a pointer to the
                              message buffer, or NULL if no message is waiting
                              to be delivered.
    Returns     : void *    - Pointer to the message buffer, or NULL if no
                              message is waiting to be delivered.
    Description : Send a message to another client task.
                  This calls ARMEdit_TalkTx.
*/
void *armedit_talk_tx(int handle, int dest, void *msg, void **rmsg);

/*
    Parameters  : handle    - Client handle for this task.
                  rmsg      - Optional variable to receive a pointer to the
                              block containing the waiting message, or NULL if
                              no messages are waiting.
                  rid       - Optional variable to receive the source ID.
                  rhandle   - Optional variable to receive the source handle.
    Returns     : os_error  - Pointer to a standard error block.
    Description : Check for any waiting messages for this client task.
                  This calls XARMEdit_TalkRx.
*/
os_error *xarmedit_talk_rx(int handle, void **rmsg, int *rid, int *rhandle);

/*
    Parameters  : handle    - Client handle for this task.
                  rmsg      - Optional variable to receive a pointer to the
                              block containing the waiting message, or NULL if
                              no messages are waiting.
                  rid       - Optional variable to receive the source ID.
                  rhandle   - Optional variable to receive the source handle.
    Returns     : void *    - Pointer to the block containing the waiting
                              message, or NULL if no messages are waiting.
    Description : Check for any waiting messages for this client task.
                  This calls ARMEdit_TalkRx.
*/
void *armedit_talk_rx(int handle, void **rmsg, int *rid, int *rhandle);

/*
    Parameters  : handle    - Client handle for this task.
    Returns     : os_error  - Pointer to a standard error block.
    Description : Claim the most recently read message.
                  This calls XARMEdit_TalkAck.
*/
os_error *xarmedit_talk_ack(int handle);

/*
    Parameters  : handle    - Client handle for this task.
    Returns     : void
    Description : Claim the most recently read message.
                  This calls ARMEdit_TalkAck.
*/
void armedit_talk_ack(int handle);

/*
    Parameters  : tx1_size  - Length of first input block.
                  tx1_buf   - Pointer to first input block.
                  tx2_size  - Length of second input block.
                  tx2_buf   - Pointer to second input block.
                  rx1_size  - Length of first output block.
                  rx1_buf   - Pointer to first output block.
                  rx2_size  - Length of second output block.
                  rx2_buf   - Pointer to second output block.
    Returns     : os_error  - Pointer to a standard error block.
    Description : Call an ARMEdit HPC service.
                  This calls XARMEdit_HPC.
*/
os_error *xarmedit_hpc(int tx1_size, void *tx1_buf, int tx2_size,
                       void *tx2_buf, int rx1_size, void *rx1_buf,
                       int rx2_size, void *rx2_buf);

/*
    Parameters  : tx1_size  - Length of first input block.
                  tx1_buf   - Pointer to first input block.
                  tx2_size  - Length of second input block.
                  tx2_buf   - Pointer to second input block.
                  rx1_size  - Length of first output block.
                  rx1_buf   - Pointer to first output block.
                  rx2_size  - Length of second output block.
                  rx2_buf   - Pointer to second output block.
    Returns     : void
    Description : Call an ARMEdit HPC service.
                  This calls ARMEdit_HPC.
*/
void armedit_hpc(int tx1_size, void *tx1_buf, int tx2_size, void *tx2_buf,
                 int rx1_size, void *rx1_buf, int rx2_size, void *rx2_buf);

/*
    Parameters  : fore      - Foreground speed, or -1 to read current setting.
                  back      - Background speed, or -1 to read current setting.
                  rfore     - Optional variable to receive the foreground
                              speed.
                  rback     - Optional variable to receive the background
                              speed.
    Returns     : os_error  - Pointer to a standard error block.
    Description : Control the multitasking speed of the PC card.
                  This calls XARMEdit_Polling.
*/
os_error *xarmedit_polling(int fore, int back, int *rfore, int *rback);

/*
    Parameters  : fore      - Foreground speed, or -1 to read current setting.
                  back      - Background speed, or -1 to read current setting.
                  rfore     - Optional variable to receive the foreground
                              speed.
                  rback     - Optional variable to receive the background
                              speed.
    Returns     : void
    Description : Control the multitasking speed of the PC card.
                  This calls ARMEdit_Polling.
*/
void armedit_polling(int fore, int back, int *rfore, int *rback);

/*
    Parameters  : handle    - Client handle for this task.
                  dest      - The client handle for the recipient.
                  msg       - Pointer to block containing the message to send,
                              or NULL to check if the buffer already contains a
                              message.
    Returns     : os_error  - Pointer to a standard error block.
    Description : Reply to a message from another client task.
                  This calls XARMEdit_TalkReply.
*/
os_error *xarmedit_talk_reply(int handle, int dest, void *msg);

/*
    Parameters  : handle    - Client handle for this task.
                  dest      - The client handle for the recipient.
                  msg       - Pointer to block containing the message to send,
                              or NULL to check if the buffer already contains a
                              message.
    Returns     : void *    - Pointer to the message buffer, or NULL if no
                              message is waiting to be delivered.
    Description : Reply to a message from another client task.
                  This calls ARMEdit_TalkReply.
*/
void *armedit_talk_reply(int handle, int dest, void *msg);

#ifdef __cplusplus
}
#endif

#endif
