/*
    File        : wimp.c
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

// Include header file for this module
#include "wimp.h"

// Include system header files
#include <string.h>
#include <stdio.h>
#include "dos.h"

// Include other project header files
#include "hpc.h"
#include "swi.h"
#include "util.h"

// The ID of the front-end client
#define WIMP_ID 256

// Reason codes
#define WIMP_REASON_FIND 0x00
#define WIMP_REASON_START 0x01
#define WIMP_REASON_POLL 0x02
#define WIMP_REASON_SAVEAS 0x03
#define WIMP_REASON_OSCLI_START 0x04
#define WIMP_REASON_OSCLI_POLL 0x05
#define WIMP_REASON_OSCLI_END 0x06

// The client handle for this task
static long client_handle = 0;

// The client handle for the front-end task
static long frontend_handle;

// Shared message buffer
struct
{
    long reason;
    union
    {
        char raw[1020];
        struct
        {
            long cursor;
            char leaf[20];
            char path[256];
        } start_tx;
        struct
        {
            long handle;
        } start_rx;
        struct
        {
            long handle;
            long flags;
        } poll_tx;
        struct
        {
            long flags;
        } poll_rx;
        struct
        {
            char path[256];
            char suggest[256];
        } saveas_tx;
        struct
        {
            char cmd[256];
            char name[256];
        } oscli_start_tx;
        struct
        {
            long handle;
        } oscli_start_rx;
        struct
        {
            long handle;
            long input;
            char data[1000];
        } oscli_poll_tx;
        struct
        {
            long status;
            long output;
            char data[1000];
        } oscli_poll_rx;
        struct
        {
            long handle;
        } oscli_end_tx;
    } data;
} buffer;

// A shared error block
static talk_error err = {0, ""};

// Shared error message text
static const char wimp_wrong_message[] =
    "Unexpected message received";

/*
    Parameters  : void
    Returns     : long  - The RISC OS system time in centi-seconds.
    Description : Read the monotonic RISC OS system clock.
*/
static long read_monotonic_time(void)
{
    const talk_error *err;
    talk_swi_regs regs;

    err = talk_swi(OS_ReadMonotonicTime, &regs, &regs);

    return err ? 0 : regs.r[0];
}

/*
    Parameters  : void
    Returns     : void
    Description : Attempt to ensure multitasking behaviour.
*/
static void multitask(void)
{
    union REGS regs;

    // Prevent disabling of multitasking
    talk_faster(0);

    // Ensure in multitasking mode
    regs.x.ax = 0xba00;
    int86(0x15, &regs, &regs);
}

/*
    Parameters  : void
    Returns     : void
    Description : This functions should be called before the program quits to
                  tidy up.
*/
static void tidy(void)
{
    // Deregister as a communications client
    if (client_handle) talk_comms_end(client_handle);
}

/*
    Parameters  : void
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : This function initialises this module. It should be called
                  on entry by each of the externally accessible functions.
*/
static const talk_error *initialise(void)
{
    static int done = FALSE;
    const talk_error *err;

    // Only perform initialisation once
    if (!done)
    {
        // Ensure things are tidied up before quitting
        atexit(tidy);

        // Register as a communications client
        err = talk_comms_start(&client_handle);
        if (err) return err;

        // Set flag to prevent reinitialisation
        done = TRUE;
    }

    // If this point reached then successful
    return NULL;
}

/*
    Parameters  : void
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Send the message to obtain the client handle of the
                  front-end application if it is running.
*/
const talk_error *wimp_find(void)
{
    const talk_error *err;

    // Initialise this module
    err = initialise();
    if (err) return err;

    // Prepare the message
    buffer.reason = WIMP_REASON_FIND;

    // Send the message
    return talk_comms_tx(client_handle, WIMP_ID, &buffer);
}

/*
    Parameters  : long          - Timeout period in centiseconds.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Attempt to receive the reply to a wimp_find message.
*/
const talk_error *wimp_find_receive(long timeout)
{
    long src_handle;
    const talk_error *e;
    long end;

    // Attempt to receive the message
    e = talk_comms_rx(client_handle, NULL, &src_handle, &buffer);
    do
    {
        // Prepare to retry
        multitask();
        end = read_monotonic_time() + timeout;

        // Keep trying until successful or timeout occurs
        while (e && (read_monotonic_time() < end))
        {
            e = talk_comms_rx(client_handle, NULL, &src_handle, &buffer);
        }

        // Return an error if failed
        if (!e && (buffer.reason != WIMP_REASON_FIND))
        {
            strcpy(err.errmess, wimp_wrong_message);
            e = &err;
        }
    }
    while (e && util_retry(e->errmess, "Failed to connect", TRUE));

    // Return an error if required
    if (e) return e;

    // No error if this point reached
    frontend_handle = src_handle;
    return NULL;
}

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
                             const char *path)
{
    const talk_error *err;

    // Initialise this module
    err = initialise();
    if (err) return err;

    // Prepare the message
    buffer.reason = WIMP_REASON_START;
    buffer.data.start_tx.cursor = cursor;
    strncpy(buffer.data.start_tx.leaf, leaf,
            sizeof(buffer.data.start_tx.leaf));
    strncpy(buffer.data.start_tx.path, path,
            sizeof(buffer.data.start_tx.path));

    // Send the message
    return talk_comms_tx(client_handle, frontend_handle, &buffer);
}

/*
    Parameters  : long          - Timeout period in centiseconds.
                  handle        - Variable to receive the edit handle.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Attempt to receive the reply to a wimp_start message.
*/
const talk_error *wimp_start_receive(long timeout, long *handle)
{
    long src_handle;
    const talk_error *e;
    long end;

    // Attempt to receive the message
    e = talk_comms_rx(client_handle, NULL, &src_handle, &buffer);
    do
    {
        // Prepare to reply
        multitask();
        end = read_monotonic_time() + timeout;

        // Keep trying until successful or timeout occurs
        while ((e || (src_handle != frontend_handle))
               && (read_monotonic_time() < end))
        {
            e = talk_comms_rx(client_handle, NULL, &src_handle, &buffer);
        }

        // Return an error if failed
        if (!e && (buffer.reason != WIMP_REASON_START))
        {
            strcpy(err.errmess, wimp_wrong_message);
            e = &err;
        }
    }
    while (e && util_retry(e->errmess, "Failed to start edit", TRUE));

    // Return an error if required
    if (e) return e;

    // No error if this point reached
    *handle = buffer.data.start_rx.handle;
    return NULL;
}

/*
    Parameters  : handle        - The handle of the external edit.
                  flags         - The flags to send.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Send the message to poll the status of an external edit.
*/
const talk_error *wimp_poll(long handle, long flags)
{
    const talk_error *err;

    // Initialise this module
    err = initialise();
    if (err) return err;

    // Prepare the message
    buffer.reason = WIMP_REASON_POLL;
    buffer.data.poll_tx.handle = handle;
    buffer.data.poll_tx.flags = flags;

    // Send the message
    return talk_comms_tx(client_handle, frontend_handle, &buffer);
}

/*
    Parameters  : long          - Timeout period in centiseconds.
                  flags         - Variable to receive the returned flags.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Attempt to receive the reply to a wimp_poll message.
*/
const talk_error *wimp_poll_receive(long timeout, long *flags)
{
    long src_handle;
    const talk_error *e;
    long end;

    // Attempt to receive the message
    e = talk_comms_rx(client_handle, NULL, &src_handle, &buffer);
    do
    {
        // Prepare to reply
        multitask();
        end = read_monotonic_time() + timeout;

        // Keep trying until successful or timeout occurs
        while ((e || (src_handle != frontend_handle))
               && (read_monotonic_time() < end))
        {
            e = talk_comms_rx(client_handle, NULL, &src_handle, &buffer);
        }

        // Return an error if failed
        if (!e && (buffer.reason != WIMP_REASON_POLL))
        {
            strcpy(err.errmess, wimp_wrong_message);
            e = &err;
        }
    }
    while (e && util_retry(e->errmess, "Failed to poll edit status", TRUE));

    // Return an error if required
    if (e) return e;

    // No error if this point reached
    *flags = buffer.data.poll_rx.flags;
    return NULL;
}

/*
    Parameters  : path          - The RISC OS pathname of the file.
                  suggest       - A suggested filename to save as.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Send the message to save a file.
*/
const talk_error *wimp_saveas(const char *path, const char *suggest)
{
    const talk_error *err;

    // Initialise this module
    err = initialise();
    if (err) return err;

    // Prepare the message
    buffer.reason = WIMP_REASON_SAVEAS;
    strcpy(buffer.data.saveas_tx.path, path);
    strcpy(buffer.data.saveas_tx.suggest, suggest);

    // Send the message
    return talk_comms_tx(client_handle, frontend_handle, &buffer);
}

/*
    Parameters  : long          - Timeout period in centiseconds.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Attempt to receive the reply to a wimp_saveas message.
*/
const talk_error *wimp_saveas_receive(long timeout)
{
    long src_handle;
    const talk_error *e;
    long end;

    // Attempt to receive the message
    e = talk_comms_rx(client_handle, NULL, &src_handle, &buffer);
    do
    {
        // Prepare to reply
        multitask();
        end = read_monotonic_time() + timeout;

        // Keep trying until successful or timeout occurs
        while ((e || (src_handle != frontend_handle))
               && (read_monotonic_time() < end))
        {
            e = talk_comms_rx(client_handle, NULL, &src_handle, &buffer);
        }

        // Return an error if failed
        if (!e && (buffer.reason != WIMP_REASON_SAVEAS))
        {
            strcpy(err.errmess, wimp_wrong_message);
            e = &err;
        }
    }
    while (e && util_retry(e->errmess, "Failed to open SaveAs window", TRUE));

    // Return an error if required
    if (e) return e;

    // No error if this point reached
    return NULL;
}

/*
    Parameters  : command       - The null terminated command to execute.
                  name          - Name of the task to create.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Start executing a specified *command in a TaskWindow.
*/
const talk_error *wimp_oscli_start(const char *command, const char *name)
{
    const talk_error *err;

    // Initialise this module
    err = initialise();
    if (err) return err;

    // Prepare the message
    buffer.reason = WIMP_REASON_OSCLI_START;
    strcpy(buffer.data.oscli_start_tx.cmd, command);
    strcpy(buffer.data.oscli_start_tx.name, name);

    // Send the message
    return talk_comms_tx(client_handle, frontend_handle, &buffer);
}

/*
    Parameters  : timeout       - Timeout period in centiseconds.
                  handle        - Handle for this command.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Attempt to receive the reply to a wimp_oscli_start message.
*/
const talk_error *wimp_oscli_start_receive(long timeout, long *handle)
{
    long src_handle;
    const talk_error *e;
    long end;

    // Attempt to receive the message
    e = talk_comms_rx(client_handle, NULL, &src_handle, &buffer);
    do
    {
        // Prepare to reply
        multitask();
        end = read_monotonic_time() + timeout;

        // Keep trying until successful or timeout occurs
        while ((e || (src_handle != frontend_handle))
               && (read_monotonic_time() < end))
        {
            e = talk_comms_rx(client_handle, NULL, &src_handle, &buffer);
        }

        // Return an error if failed
        if (!e && (buffer.reason != WIMP_REASON_OSCLI_START))
        {
            strcpy(err.errmess, wimp_wrong_message);
            e = &err;
        }
    }
    while (e && util_retry(e->errmess, "Failed to start TaskWindow", TRUE));

    // Return an error if required
    if (e) return e;

    // No error if this point reached
    *handle = buffer.data.oscli_start_rx.handle;
    return NULL;
}

/*
    Parameters  : handle        - The handle of the command to poll.
                  input         - Number of bytes of input.
                  data          - The input data.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Continue execution of a *command in a TaskWindow.
*/
const talk_error *wimp_oscli_poll(long handle, long input, const char *data)
{
    const talk_error *err;

    // Initialise this module
    err = initialise();
    if (err) return err;

    // Prepare the message
    buffer.reason = WIMP_REASON_OSCLI_POLL;
    buffer.data.oscli_poll_tx.handle = handle;
    buffer.data.oscli_poll_tx.input = input;
    if (input && data) memcpy(buffer.data.oscli_poll_tx.data, data, input);

    // Send the message
    return talk_comms_tx(client_handle, frontend_handle, &buffer);
}

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
                                          long *output, char *data)
{
    long src_handle;
    const talk_error *e;
    long end;

    // Attempt to receive the message
    e = talk_comms_rx(client_handle, NULL, &src_handle, &buffer);
    do
    {
        // Prepare to reply
        multitask();
        end = read_monotonic_time() + timeout;

        // Keep trying until successful or timeout occurs
        while ((e || (src_handle != frontend_handle))
               && (read_monotonic_time() < end))
        {
            e = talk_comms_rx(client_handle, NULL, &src_handle, &buffer);
        }

        // Return an error if failed
        if (!e && (buffer.reason != WIMP_REASON_OSCLI_POLL))
        {
            strcpy(err.errmess, wimp_wrong_message);
            e = &err;
        }
    }
    while (e && util_retry(e->errmess, "Failed to poll TaskWindow", TRUE));

    // Return an error if required
    if (e) return e;

    // No error if this point reached
    *status = buffer.data.oscli_poll_rx.status;
    *output = buffer.data.oscli_poll_rx.output;
    if (*output && data)
    {
        memcpy(data, buffer.data.oscli_poll_rx.data, *output);
    }
    return NULL;
}

/*
    Parameters  : handle        - The handle of the command to terminate.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Terminate execution of a *command in a TaskWindow.
*/
const talk_error *wimp_oscli_end(long handle)
{
    const talk_error *err;

    // Initialise this module
    err = initialise();
    if (err) return err;

    // Prepare the message
    buffer.reason = WIMP_REASON_OSCLI_END;
    buffer.data.oscli_end_tx.handle = handle;

    // Send the message
    return talk_comms_tx(client_handle, frontend_handle, &buffer);
}

/*
    Parameters  : timeout       - Timeout period in centiseconds.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Attempt to receive the reply to a wimp_oscli_end message.
*/
const talk_error *wimp_oscli_end_receive(long timeout)
{
    long src_handle;
    const talk_error *e;
    long end;

    // Attempt to receive the message
    e = talk_comms_rx(client_handle, NULL, &src_handle, &buffer);
    do
    {
        // Prepare to reply
        multitask();
        end = read_monotonic_time() + timeout;

        // Keep trying until successful or timeout occurs
        while ((e || (src_handle != frontend_handle))
               && (read_monotonic_time() < end))
        {
            e = talk_comms_rx(client_handle, NULL, &src_handle, &buffer);
        }

        // Return an error if failed
        if (!e && (buffer.reason != WIMP_REASON_OSCLI_END))
        {
            strcpy(err.errmess, wimp_wrong_message);
            e = &err;
        }
    }
    while (e && util_retry(e->errmess, "Failed to terminate TaskWindow", TRUE));

    // Return an error if required
    if (e) return e;

    // No error if this point reached
    return NULL;
}

