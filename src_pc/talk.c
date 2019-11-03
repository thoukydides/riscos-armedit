/*
    File        : talk.c
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

// Include header file for this module
#include "talk.h"

// Include system header files
#include <ctype.h>
#include <string.h>
#include <stdio.h>

// Include project header files
#include "hpc.h"

// The HPC service identifier
#define HPC_SERVICE_ID 0x0105

// The possible reason codes within that service
#define CODE_SWI 0x0000
#define CODE_READ 0x0001
#define CODE_WRITE 0x0002
#define CODE_ALLOC 0x0003
#define CODE_FREE 0x0004
#define CODE_EXTTYPE 0x0005
#define CODE_TYPEEXT 0x0006
#define CODE_FOPEN 0x0007
#define CODE_FCLOSE 0x0008
#define CODE_FREAD 0x0009
#define CODE_FWRITE 0x000a
#define CODE_CSTART 0x000b
#define CODE_CEND 0x000c
#define CODE_CTX 0x000d
#define CODE_CRX 0x000e
#define CODE_DEVINIT 0x000f
#define CODE_DEVBPB 0x0010
#define CODE_DEVCHNG 0x0011
#define CODE_DEVREAD 0x0012
#define CODE_DEVWRITE 0x0013
#define CODE_DATEDOS 0x0014
#define CODE_DATERISCOS 0x0015
#define CODE_OSCLISTART 0x0016
#define CODE_OSCLIPOLL 0x0017
#define CODE_OSCLIEND 0x0018
#define CODE_CREPLY 0x0019
#define CODE_FASTER 0x001a
#define CODE_TEMPORARY 0x001b

// The possible return codes
#define RETURN_SUCCESS 0x00000000L
#define RETURN_FAILURE 0x00000001L
#define RETURN_UNKNOWN 0xffffffffL

// A shared error block
static talk_error err = {0, ""};

// Shared error message text
static const char talk_in_use[] =
    "Communications with ARM are already in use";
static const char talk_not_working[] =
    "Communications with ARM are not working";
static const char talk_service_unknown[] =
    "The required ARM service is unknown";
static const char talk_memory_unknown[] =
    "The requested ARM memory does not exist";
static const char talk_dosmap_fail[] =
    "Unable to perform DOSMap conversion";

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
                           talk_swi_regs *out)
{
    struct
    {
        int service, reason;
        long swi;
    } head_in;
    static struct
    {
        unsigned long code;
        talk_error err;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_SWI;
    head_in.swi = no;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in),
                       in, in ? sizeof(talk_swi_regs) : 0,
                       &head_out, sizeof(head_out),
                       out, out ? sizeof(talk_swi_regs) : 0);

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE) return &head_out.err;
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

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
const talk_error *talk_read(void *buf, size_t len, long start)
{
    struct
    {
        int service, reason;
        long start, len;
    } head_in;
    struct
    {
        unsigned long code;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_READ;
    head_in.start = start;
    head_in.len = len;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), NULL, 0,
                       &head_out, sizeof(head_out), buf, len);

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE)
    {
        strcpy(err.errmess, talk_memory_unknown);
        return &err;
    }
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

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
const talk_error *talk_write(const void *buf, size_t len, long start)
{
    struct
    {
        int service, reason;
        long start, len;
    } head_in;
    struct
    {
        unsigned long code;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_WRITE;
    head_in.start = start;
    head_in.len = len;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), buf, len,
                       &head_out, sizeof(head_out), NULL, 0);

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE)
    {
        strcpy(err.errmess, talk_memory_unknown);
        return &err;
    }
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

/*
    Parameters  : len           - Amount of memory to allocate.
                  buf           - Variable to contain address of memory.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Claim the specified amount of ARM memory.
*/
const talk_error *talk_malloc(size_t len, long *buf)
{
    struct
    {
        int service, reason;
        long len;
    } head_in;
    struct
    {
        unsigned long code;
        long ptr;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_ALLOC;
    head_in.len = len;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), NULL, 0,
                       &head_out, sizeof(head_out), NULL, 0);
    *buf = head_out.ptr;

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE)
    {
        strcpy(err.errmess, "Unable to claim required ARM memory");
        return &err;
    }
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

/*
    Parameters  : buf           - Address of block of memory to free.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Free a block of memory previously claimed using talk_alloc.
*/
const talk_error *talk_free(long buf)
{
    struct
    {
        int service, reason;
        long ptr;
    } head_in;
    struct
    {
        unsigned long code;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_FREE;
    head_in.ptr = buf;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), NULL, 0,
                       &head_out, sizeof(head_out), NULL, 0);

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE)
    {
        strcpy(err.errmess, "Unable to release ARM memory");
        return &err;
    }
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

/*
    Parameters  : ext           - A file extension.
                  type          - Variable to receive the filetype.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Convert a DOS file extension into a RISC OS filetype.
*/
const talk_error *talk_ext_to_filetype(const char *ext, int *type)
{
    struct
    {
        int service, reason;
        long ext;
    } head_in;
    struct
    {
        unsigned long code, type;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_EXTTYPE;
    head_in.ext = 0;
    if (ext[0])
    {
        head_in.ext |= (unsigned long) toupper(ext[0]);
        if (ext[1])
        {
            head_in.ext |= (unsigned long) toupper(ext[1]) << 8;
            if (ext[2])
            {
                head_in.ext |= (unsigned long) toupper(ext[2]) << 16;
            }
        }
    }

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), NULL, 0,
                       &head_out, sizeof(head_out), NULL, 0);
    *type = (int) head_out.type;

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE)
    {
        strcpy(err.errmess, talk_dosmap_fail);
        return &err;
    }
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

/*
    Parameters  : type          - A RISC OS filetype.
                  ext           - Variable to receive file extension.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Convert a RISC OS filetype into a DOS file extension.
*/
const talk_error *talk_filetype_to_ext(int type, char *ext)
{
    struct
    {
        int service, reason;
        long type;
    } head_in;
    struct
    {
        unsigned long code;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_TYPEEXT;
    head_in.type = type;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), NULL, 0,
                       &head_out, sizeof(head_out), ext, 4);

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE)
    {
        strcpy(err.errmess, talk_dosmap_fail);
        return &err;
    }
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

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
                                 long *handle)
{
    struct
    {
        int service, reason;
        long size, del;
    } head_in;
    struct
    {
        unsigned long code;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_FOPEN;
    head_in.size = size;
    head_in.del = del;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), name, strlen(name) + 1,
                       &head_out, sizeof(head_out), handle, sizeof(long));

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE)
    {
        strcpy(err.errmess, "Unable to open RISC OS file");
        return &err;
    }
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

/*
    Parameters  : handle        - Handle of the file to close.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Close a RISC OS file.
*/
const talk_error *talk_file_close(long handle)
{
    struct
    {
        int service, reason;
        long handle;
    } head_in;
    struct
    {
        unsigned long code;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_FCLOSE;
    head_in.handle = handle;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), NULL, 0,
                       &head_out, sizeof(head_out), NULL, 0);

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE)
    {
        strcpy(err.errmess, "Unable to close RISC OS file");
        return &err;
    }
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

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
                                 void *buf, long *done)
{
    struct
    {
        int service, reason;
        long handle, ptr, size;
    } head_in;
    struct
    {
        unsigned long code;
        long size;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_FREAD;
    head_in.handle = handle;
    head_in.ptr = ptr;
    head_in.size = size;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), NULL, 0,
                       &head_out, sizeof(head_out), buf, size);
    *done = head_out.size;

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE)
    {
        strcpy(err.errmess, "Unable to read from RISC OS file");
        return &err;
    }
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

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
                                  const void *buf)
{
    struct
    {
        int service, reason;
        long handle, ptr, size;
    } head_in;
    struct
    {
        unsigned long code;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_FWRITE;
    head_in.handle = handle;
    head_in.ptr = ptr;
    head_in.size = size;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), buf, size,
                       &head_out, sizeof(head_out), NULL, 0);

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE)
    {
        strcpy(err.errmess, "Unable to write to RISC OS file");
        return &err;
    }
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

/*
    Parameters  : handle        - The unique handle allocated for this
                                  client.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Register a communications client.
*/
const talk_error *talk_comms_start(long *handle)
{
    struct
    {
        int service, reason;
    } head_in;
    struct
    {
        unsigned long code;
        long handle;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_CSTART;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), NULL, 0,
                       &head_out, sizeof(head_out), NULL, 0);
    *handle = head_out.handle;

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE)
    {
        strcpy(err.errmess, "Unable to register as a communications client");
        return &err;
    }

    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

/*
    Parameters  : handle        - The unique handle previously allocated to
                                  this client.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Deregister a communications client.
*/
const talk_error *talk_comms_end(long handle)
{
    struct
    {
        int service, reason;
        long handle;
    } head_in;
    struct
    {
        unsigned long code;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_CEND;
    head_in.handle = handle;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), NULL, 0,
                       &head_out, sizeof(head_out), NULL, 0);

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE)
    {
        strcpy(err.errmess, "Unable to deregister as a communications client");
        return &err;
    }
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

/*
    Parameters  : handle        - The unique handle previously allocated to
                                  this client.
                  dest          - The destination ID or handle.
                  msg           - The message to send.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Transmit a message to another client.
*/
const talk_error *talk_comms_tx(long handle, long dest, const void *msg)
{
    struct
    {
        int service, reason;
        long handle, dest;
    } head_in;
    struct
    {
        unsigned long code;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_CTX;
    head_in.handle = handle;
    head_in.dest = dest;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), msg, 1024,
                       &head_out, sizeof(head_out), NULL, 0);

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE)
    {
        strcpy(err.errmess, "Unable to transmit message");
        return &err;
    }
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

/*
    Parameters  : handle        - The unique handle previously allocated to
                                  this client.
                  src_id        - The ID of the sending task.
                  src_handle    - The handle of the sending task.
                  msg           - The buffer to receive the message.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Receive a message from another task. Note that an error is
                  returned if no message is available.
*/
const talk_error *talk_comms_rx(long handle, long *src_id, long *src_handle,
                                void *msg)
{
    struct
    {
        int service, reason;
        long handle;
    } head_in;
    struct
    {
        unsigned long code;
        long id, handle;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_CRX;
    head_in.handle = handle;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), NULL, 0,
                       &head_out, sizeof(head_out), msg, 1024);
    *src_id = head_out.id;
    *src_handle = head_out.handle;

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE)
    {
        strcpy(err.errmess, "There is no message to receive");
        return &err;
    }
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

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
                                   unsigned *date)
{
    struct
    {
        int service, reason;
    } head_in;
    struct
    {
        unsigned long code;
        unsigned time, date;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_DATEDOS;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), in, sizeof(talk_date),
                       &head_out, sizeof(head_out), NULL, 0);
    *time = head_out.time;
    *date = head_out.date;

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if ((head_out.code == RETURN_FAILURE)
             || (head_out.code == RETURN_UNKNOWN))
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

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
                                      talk_date out)
{
    struct
    {
        int service, reason;
        unsigned time, date;
    } head_in;
    struct
    {
        unsigned long code;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_DATERISCOS;
    head_in.time = time;
    head_in.date = date;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), NULL, 0,
                       &head_out, sizeof(head_out), out, sizeof(talk_date));

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if ((head_out.code == RETURN_FAILURE)
             || (head_out.code == RETURN_UNKNOWN))
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

/*
    Parameters  : cmd           - The command to execute.
                  handle        - The returned handle for this command.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Start executing a RISC OS *command.
*/
const talk_error *talk_oscli_start(const char *cmd, long *handle)
{
    struct
    {
        int service, reason;
    } head_in;
    struct
    {
        unsigned long code;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_OSCLISTART;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), cmd, strlen(cmd) + 1,
                       &head_out, sizeof(head_out), handle, sizeof(*handle));

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if ((head_out.code == RETURN_FAILURE)
             || (head_out.code == RETURN_UNKNOWN))
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

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
                                  long *status, long *out_size, char *out)
{
    struct
    {
        int service, reason;
        long handle;
        long size;
    } head_in;
    struct
    {
        unsigned long code;
        long status;
        long size;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_OSCLIPOLL;
    head_in.handle = handle;
    head_in.size = in_size;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in),
                       0 < in_size ? in : NULL, 0 < in_size ? in_size : 0,
                       &head_out, sizeof(head_out), out, 256);
    *status = head_out.status;
    *out_size = head_out.size;

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if ((head_out.code == RETURN_FAILURE)
             || (head_out.code == RETURN_UNKNOWN))
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

/*
    Parameters  : handle        - The command handle.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Stop executing a RISC OS *command.
*/
const talk_error *talk_oscli_end(long handle)
{
    struct
    {
        int service, reason;
        long handle;
    } head_in;
    static struct
    {
        unsigned long code;
        talk_error err;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_OSCLIEND;
    head_in.handle = handle;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), NULL, 0,
                       &head_out, sizeof(head_out), NULL, 0);

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE) return &head_out.err;
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

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
const talk_error *talk_comms_reply(long handle, long dest, const void *buf)
{
    struct
    {
        int service, reason;
        long handle, dest;
    } head_in;
    struct
    {
        unsigned long code;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_CREPLY;
    head_in.handle = handle;
    head_in.dest = dest;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), buf, 1024,
                       &head_out, sizeof(head_out), NULL, 0);

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if (head_out.code == RETURN_FAILURE)
    {
        strcpy(err.errmess, "Unable to transmit message");
        return &err;
    }
    else if (head_out.code == RETURN_UNKNOWN)
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

/*
    Parameters  : centiseconds  - Time in centiseconds before multitasking
                                  should be reenabled, or 0 to reenable
                                  normal operation.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Disable multitasking for a specified length of time.
*/
const talk_error *talk_faster(long centiseconds)
{
    struct
    {
        int service, reason;
        long centiseconds;
    } head_in;
    struct
    {
        unsigned long code;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_FASTER;
    head_in.centiseconds = centiseconds;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), NULL, 0,
                       &head_out, sizeof(head_out), NULL, 0);

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if ((head_out.code == RETURN_FAILURE)
             || (head_out.code == RETURN_UNKNOWN))
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}

/*
    Parameters  : buf           - Buffer to receive the filename.
                  len           - Size of the buffer.
    Returns     : talk_error    - A pointer to a RISC OS style error block
                                  (in PC memory), or NULL if there was no
                                  error.
    Description : Generate a unique RISC OS filename for a temporary file.
*/
const talk_error *talk_temporary(char *buf, size_t len)
{
    struct
    {
        int service, reason;
    } head_in;
    struct
    {
        unsigned long code;
    } head_out;
    int code;

    // Set up the required header
    head_in.service = HPC_SERVICE_ID;
    head_in.reason = CODE_TEMPORARY;

    // Perform the HPC call
    code = hpc_message(&head_in, sizeof(head_in), NULL, 0,
                       &head_out, sizeof(head_out), buf, len);

    // Return pointer to error block if appropriate
    if (code == HPC_RETURN_FAILURE_IN_USE)
    {
        strcpy(err.errmess, talk_in_use);
        return &err;
    }
    else if (code == HPC_RETURN_FAILURE_NOT_WORKING)
    {
        strcpy(err.errmess, talk_not_working);
        return &err;
    }
    else if ((head_out.code == RETURN_FAILURE)
             || (head_out.code == RETURN_UNKNOWN))
    {
        strcpy(err.errmess, talk_service_unknown);
        return &err;
    }
    else return NULL;
}
