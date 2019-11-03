/*
    File        : wimp.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Interface to the messages supported by the !ARMEdit
                  front-end application.

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

// Only include header file once
#ifndef WIMP_H
#define WIMP_H

// Include system header files
#include <stdlib.h>

// Include project header files
#include "talk.h"

// Polling flags
#define WIMP_POLL_TX_FLAG_RETRIEVE 0x01
#define WIMP_POLL_TX_FLAG_ABORT 0x02
#define WIMP_POLL_TX_FLAG_SAFE 0x04
#define WIMP_POLL_RX_FLAG_MODIFIED 0x01
#define WIMP_POLL_RX_FLAG_ABORTED 0x02

#ifdef __cplusplus
extern "C" {
#endif

/*
    Parameters  : void
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Send the message to obtain the client handle of the
                  front-end application if it is running.
*/
const talk_error *wimp_find(void);

/*
    Parameters  : timeout       - Timeout period in centiseconds.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Attempt to receive the reply to a wimp_find message.
*/
const talk_error *wimp_find_receive(long timeout);

/*
    Parameters  : cursor        - Byte offset to required cursor position.
                  leaf          - The leafname to display.
                  path          - The RISC OS pathname of the file.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Send the message to start an external edit.
*/
const talk_error *wimp_start(long cursor, const char *leaf,
                             const char *path);

/*
    Parameters  : timeout       - Timeout period in centiseconds.
                  handle        - Variable to receive the edit handle.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Attempt to receive the reply to a wimp_start message.
*/
const talk_error *wimp_start_receive(long timeout, long *handle);

/*
    Parameters  : handle        - The handle of the external edit.
                  flags         - The flags to send.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Send the message to poll the status of an external edit.
*/
const talk_error *wimp_poll(long handle, long flags);

/*
    Parameters  : timeout       - Timeout period in centiseconds.
                  flags         - Variable to receive the returned flags.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Attempt to receive the reply to a wimp_poll message.
*/
const talk_error *wimp_poll_receive(long timeout, long *flags);

/*
    Parameters  : path          - The RISC OS pathname of the file.
                  suggest       - A suggested filename to save as.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Send the message to save a file.
*/
const talk_error *wimp_saveas(const char *path, const char *suggest);

/*
    Parameters  : timeout       - Timeout period in centiseconds.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Attempt to receive the reply to a wimp_saveas message.
*/
const talk_error *wimp_saveas_receive(long timeout);

/*
    Parameters  : command       - The null terminated command to execute.
                  name          - Name of the task to create.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Start executing a specified *command in a TaskWindow.
*/
const talk_error *wimp_oscli_start(const char *command, const char *name);

/*
    Parameters  : timeout       - Timeout period in centiseconds.
                  handle        - Handle for this command.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Attempt to receive the reply to a wimp_oscli_start message.
*/
const talk_error *wimp_oscli_start_receive(long timeout, long *handle);

/*
    Parameters  : handle        - The handle of the command to poll.
                  input         - Number of bytes of input.
                  data          - The input data.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Continue execution of a *command in a TaskWindow.
*/
const talk_error *wimp_oscli_poll(long handle, long input, const char *data);

/*
    Parameters  : timeout       - Timeout period in centiseconds.
                  status        - Variable to receive the command status.
                  output        - Variable to receive the number of bytes of
                                  output.
                  data          - Buffer to receive the output data.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Attempt to receive the reply to a wimp_oscli_poll message.
*/
const talk_error *wimp_oscli_poll_receive(long timeout, long *status,
                                          long *output, char *data);

/*
    Parameters  : handle        - The handle of the command to terminate.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Terminate execution of a *command in a TaskWindow.
*/
const talk_error *wimp_oscli_end(long handle);

/*
    Parameters  : timeout       - Timeout period in centiseconds.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Attempt to receive the reply to a wimp_oscli_end message.
*/
const talk_error *wimp_oscli_end_receive(long timeout);

#ifdef __cplusplus
}
#endif

#endif
