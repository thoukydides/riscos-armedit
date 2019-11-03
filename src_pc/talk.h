/*
    File        : talk.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1995-2001, 2019
    Description : Communications with the RISC OS ARMEdit module. This
                  provides an interface to the services provided by that
                  module without requiring any knowledge of the underlying
                  interface.

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
#ifndef TALK_H
#define TALK_H

// Include system header files
#include <stdlib.h>

// RISC OS scrap directory to use
#define TALK_SCRAP "<ARMEdit$ScrapDir>"

// OSCLI status codes
#define TALK_OSCLI_ACTIVE 0x00
#define TALK_OSCLI_FINISHED 0x01
#define TALK_OSCLI_WAITING 0x02

// ARM registers used on entry and exit from SWIs
typedef struct
{
    long r[10];                         // Only R0 to R9 matter for SWIs
} talk_swi_regs;

// A RISC OS style error block
typedef struct
{
    long errnum;                        // Error number
    char errmess[252];                  // Error message (zero terminated)
} talk_error;

// A RISC OS date and time
typedef char talk_date[5];

#ifdef __cplusplus
extern "C" {
#endif

/*
    Paramaters  : no            - The number of the SWI to call.
                  in            - Pointer to the values for the ARM registers
                                  on entry to the SWI.
                  out           - Pointer to the values that the ARM
                                  registers contained on exit from the SWI.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Call the specified RISC OS SWI. The SWI is always called
                  with the X bit set.
*/
const talk_error *talk_swi(long no, const talk_swi_regs *in,
                           talk_swi_regs *out);

/*
    Parameters  : buf           - Pointer to buffer to receive data.
                  len           - The number of bytes to read.
                  start         - The start ARM memory address.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Read up to 16380 bytes (less than 4092 bytes recomended)
                  of ARM memory.
*/
const talk_error *talk_read(void *buf, size_t len, long start);

/*
    Parameters  : buf           - Pointer to buffer containing data.
                  len           - The number of bytes to write.
                  start         - The start ARM memory address.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Write up to 16372 bytes (less than 4084 byte recommended)
                  of ARM memory.
*/
const talk_error *talk_write(const void *buf, size_t len, long start);

/*
    Parameters  : len           - Amount of memory to allocate.
                  buf           - Variable to contain address of memory.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Claim the specified amount of ARM memory.
*/
const talk_error *talk_malloc(size_t len, long *buf);

/*
    Parameters  : buf           - Address of block of memory to free.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Free a block of memory previously claimed using talk_alloc.
*/
const talk_error *talk_free(long buf);

/*
    Parameters  : ext           - A file extension.
                  type          - Variable to receive the filetype.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Convert a DOS file extension into a RISC OS filetype.
*/
const talk_error *talk_ext_to_filetype(const char *ext, int *type);

/*
    Parameters  : type          - A RISC OS filetype.
                  ext           - Variable to receive file extension.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Convert a RISC OS filetype into a DOS file extension.
*/
const talk_error *talk_filetype_to_ext(int type, char *ext);


/*
    Parameters  : name          - The name of the file to open.
                  size          - The initial size of the file, or -1 to open
                                  an existing file.
                  del           - Should the file be deleted when closed.
                  handle        - Variable to receive the file handle.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Open a RISC OS file.
*/
const talk_error *talk_file_open(const char *name, long size, int del,
                                 long *handle);

/*
    Parameters  : handle        - Handle of the file to close.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Close a RISC OS file.
*/
const talk_error *talk_file_close(long handle);

/*
    Parameters  : handle        - Handle of the file to read from.
                  ptr           - Sequential file pointer position to read
                                  from, or -1 to use current position.
                  size          - Number of bytes to read.
                  buf           - Buffer to receive the data.
                  done          - Variable to receive number of bytes read.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Read up to 16376 bytes (less than 4088 recommended) from
                  a RISC OS file.
*/
const talk_error *talk_file_read(long handle, long ptr, long size,
                                 void *buf, long *done);

/*
    Parameters  : handle        - Handle of the file to write to.
                  ptr           - Sequential file pointer position to write
                                  at, or -1 to use current position.
                  size          - Number of bytes to write.
                  buf           - Buffer containing the data.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Write up to 16368 bytes (less than 4080 recommended) to a
                  RISC OS file.
*/
const talk_error *talk_file_write(long handle, long ptr, long size,
                                  const void *buf);

/*
    Parameters  : handle        - Variable to receive the unique handle for
                                  this task. This should be used with all
                                  future communications.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Register a communications client.
*/
const talk_error *talk_comms_start(long *handle);

/*
    Parameters  : handle        - The previously allocated handle for this
                                  client.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Deregister a communications client.
*/
const talk_error *talk_comms_end(long handle);

/*
    Parameters  : handle        - The previously allocated client handle for
                                  this task.
                  dest          - The destination task ID or handle.
                  buf           - The message to send.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Send a message to one or more other clients.
*/
const talk_error *talk_comms_tx(long handle, long dest, const void *buf);

/*
    Parameters  : handle        - The previously allocated client handle for
                                  this task.
                  src_id        - The ID of the sending task.
                  src_handle    - The handle of the sending task.
                  buf           - The buffer to receive the message in.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Check for any waiting messages, and read the next one if
                  possible. Note that this (currently) returns an error if
                  there is no message to read.
*/
const talk_error *talk_comms_rx(long handle, long *src_id, long *src_handle,
                                void *buf);

/*
    Parameters  : in            - The RISC OS date and time to convert.
                  time          - The DOS style time.
                  date          - The DOS style date.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Convert RISC OS format date and times to the equivalent DOS
                  values.
*/
const talk_error *talk_date_to_dos(const talk_date in, unsigned *time,
                                   unsigned *date);

/*
    Parameters  : time          - The DOS style time.
                  date          - The DOS style date.
                  out           - The equivalent RISC OS style date and time.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Convert DOS format date and times to the equivalent RISC OS
                  value.
*/
const talk_error *talk_date_to_riscos(unsigned time, unsigned date,
                                      talk_date out);

/*
    Parameters  : cmd           - The command to execute.
                  handle        - The returned handle for this command.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Start executing a RISC OS *command.
*/
const talk_error *talk_oscli_start(const char *cmd, long *handle);

/*
    Parameters  : handle        - The command handle.
                  in_size       - Number of input bytes to send.
                  in            - Bytes to send.
                  status        - Returned status.
                  out_size      - Number of output bytes received.
                  out           - Buffer for received output (at least 256
                                  bytes).
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Continue executing a RISC OS *command.
*/
const talk_error *talk_oscli_poll(long handle, long in_size, const char *in,
                                  long *status, long *out_size, char *out);

/*
    Parameters  : handle        - The command handle.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Stop executing a RISC OS *command.
*/
const talk_error *talk_oscli_end(long handle);

/*
    Parameters  : handle        - The previously allocated client handle for
                                  this task.
                  dest          - The destination task handle.
                  buf           - The message to send.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Reply to a message from another client.
*/
const talk_error *talk_comms_reply(long handle, long dest, const void *buf);

/*
    Parameters  : centiseconds  - Time in centiseconds before multitasking
                                  should be reenabled, or 0 to reenable
                                  normal operation.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Disable multitasking for a specified length of time.
*/
const talk_error *talk_faster(long centiseconds);

/*
    Parameters  : buf           - Buffer to receive the filename.
                  len           - Size of the buffer.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Generate a unique RISC OS filename for a temporary file.
*/
const talk_error *talk_temporary(char *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif
