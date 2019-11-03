/*
    File        : armfile.c
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1995-2001, 2019
    Description : Transfer of files between DOS and RISC OS.

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
#include "armfile.h"

// Include system header files
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Include project header files
#include "talk.h"
#include "swi.h"
#include "hpc.h"

// Should multitasking be enabled during transfers (default is FALSE)
int armfile_mtask = FALSE;

// Amount of ARM memory required
#define ARM_MEM 256

// Variable to contain any error message
char armfile_error[256] = "";

// Open file handles
static int dos_file = -1;
static long arm_file = 0;
static long arm_mem = 0;

// Characters to convert
static const char char_dos[] = "?#&@%$^.\\/";
static const char char_arm[] = "#?+=;<>/.\\";

// Linked list of files to delete
typedef struct armfile_files_struct
{
    struct armfile_files_struct *next;
    char name[256];
} armfile_files;
static armfile_files *files_head = NULL;

/*
    Parameters  : void
    Returns     : void
    Description : Close any open files.
*/
static void close_files(void)
{
    // Close any open RISC OS file
    if (arm_file)
    {
        talk_file_close(arm_file);
        arm_file = 0;
    }

    // Close any open DOS file
    if (dos_file != -1)
    {
        close(dos_file);
        dos_file = -1;
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : This functions should be called before the program quits to
                  tidy up.
*/
static void tidy(void)
{
    const talk_error *err;
    talk_swi_regs regs;

    // Ensure any open files are closed
    close_files();

    // Delete any temporary files
    while (arm_mem && files_head)
    {
        err = talk_write(files_head->name, strlen(files_head->name) + 1,
                         arm_mem);
        if (!err)
        {
            regs.r[0] = 6;
            regs.r[1] = arm_mem;
            err = talk_swi(OS_File, &regs, &regs);
        }

        files_head = files_head->next;
    }

    // Free any memory that has been claimed
    if (arm_mem) talk_free(arm_mem);

    // Restore multitasking
    talk_faster(0);
}

/*
    Parameters  : void
    Returns     : int    - Standard C return code giving status of operation.
    Description : This function initialises this module. It should be called
                  on entry by each of the externally accessible functions.
*/
static int initialise(void)
{
    static int done = FALSE;
    const talk_error *err;

    // Only perform initialisation once
    if (!done)
    {
        // Ensure things are tidied up before quitting
        atexit(tidy);

        // Allocate some memory
        err = talk_malloc(ARM_MEM, &arm_mem);
        if (err)
        {
            sprintf(armfile_error, "Failed to allocate memory (%s)\n",
                    err->errmess);
            return EXIT_FAILURE;
        }

        // Set flag to prevent reinitialisation
        done = TRUE;
    }

    // If this point reached then successful
    return EXIT_SUCCESS;
}

/*
    Parameters  : src    - The source DOS file.
                  dest    - The destination RISC OS file.
    Returns     : int    - Standard C return code giving status of operation.
    Description : Copy a file from DOS to RISC OS. If there is an error then
                  it is stored in the variable armfile_error and a non-zero
                  value is returned, otherwise zero is returned.
*/
int armfile_copy_dos_riscos(const char *src, const char *dest)
{
    long size;
    const talk_error *err;
    char buffer[4080];
    char name[256];
    unsigned time, date;
    talk_date riscos_date;
    int type;
    talk_swi_regs regs;

    // Initialise this module
    if (initialise()) return EXIT_FAILURE;
    if (!armfile_mtask) talk_faster(100);

    // Check that both filenames are specified
    if (!dest) armfile_translate_dos_riscos(src, (char *) dest = name);

    // Open the DOS source file
    dos_file = open(src, O_RDONLY | O_BINARY | O_DENYALL);
    if (dos_file == -1)
    {
        return EXIT_FAILURE;
    }

    // Read original date and time stamp
    _dos_getftime(dos_file, &date, &time);

    // Convert date and time to RISC OS format
    err = talk_date_to_riscos(time, date, riscos_date);
    if (err)
    {
        sprintf(armfile_error, "Unable to convert date stamp for file '%s'",
                src);
        close_files();
        return EXIT_FAILURE;
    }

    // Open the RISC OS destination file
    err = talk_file_open(dest, filelength(dos_file), FALSE, &arm_file);
    if (err)
    {
        sprintf(armfile_error, "Unable to open destination file '%s' (%s)",
                dest, err->errmess);
        close_files();
        return EXIT_FAILURE;
    }

    // Copy the data
    size = TRUE;
    while (size)
    {
        if (!armfile_mtask) talk_faster(100);
        size = read(dos_file, buffer, sizeof(buffer));
        if (size == -1)
        {
            sprintf(armfile_error, "Error reading from file '%s'", src);
            close_files();
            return EXIT_FAILURE;
        }
        if (size)
        {
            err = talk_file_write(arm_file, -1, size, buffer);
            if (err)
            {
                sprintf(armfile_error, "Error writing to file '%s' (%s)",
                        dest, err->errmess);
                close_files();
                return EXIT_FAILURE;
            }
        }
    }

    // Close the files
    close_files();

    // Write the destination filename into a RISC OS buffer
    err = talk_write(dest, strlen(dest) + 1, arm_mem);
    if (err)
    {
        sprintf(armfile_error, "Unable to write filename for RISC OS (%s)",
                err->errmess);
        return EXIT_FAILURE;
    }

    // Find the extension of the file
    while (*src && (*src != '.')) src++;

    // Choose a suitable filetype
    if (*src) src++;
    err = talk_ext_to_filetype(src, &type);
    if (err)
    {
        sprintf(armfile_error,
                "Unable to convert extension '%s' to filetype (%s)",
                src, err->errmess);
        return EXIT_FAILURE;
    }

    // Set the filetype and set the date stamp
    regs.r[0] = 2;
    regs.r[1] = arm_mem;
    regs.r[2] = ((unsigned long) type) << 8;
    regs.r[2] |= riscos_date[4];
    regs.r[2] |= 0xfff00000;
    err = talk_swi(OS_File, &regs, &regs);
    if (!err)
    {
        // Attempt to set execution address
        regs.r[0] = 3;
        regs.r[1] = arm_mem;
        regs.r[3] = * ((unsigned long *) riscos_date);
        err = talk_swi(OS_File, &regs, &regs);
    }
    if (err)
    {
        sprintf(armfile_error,
                "Unable to set filetype &%x (%s)",
                type, err->errmess);
        return EXIT_FAILURE;
    }

    // Return success
    talk_faster(0);
    return EXIT_SUCCESS;
}

/*
    Parameters  : src    - The source RISC OS file.
                  dest    - The destination DOS file.
    Returns     : int    - Standard C return code giving status of operation.
    Description : Copy a file from RISC OS to DOS. If there is an error then
                  it is stored in the variable armfile_error and a non-zero
                  value is returned, otherwise zero is returned.
*/
int armfile_copy_riscos_dos(const char *src, const char *dest)
{
    long size;
    const talk_error *err;
    char buffer[4088];
    char name[256];
    unsigned time, date;
    talk_date riscos_date;
    talk_swi_regs regs;

    // Initialise this module
    if (initialise()) return EXIT_FAILURE;
    if (!armfile_mtask) talk_faster(100);

    // Check that both filenames are specified
    if (!dest) armfile_translate_riscos_dos(src, (char *) dest = name);

    // Open the RISC OS source file
    err = talk_file_open(src, -1, FALSE, &arm_file);
    if (err)
    {
        sprintf(armfile_error, "Unable to open source file '%s' (%s)",
                src, err->errmess);
        return EXIT_FAILURE;
    }

    // Write the source filename into a RISC OS buffer
    err = talk_write(src, strlen(src) + 1, arm_mem);
    if (err)
    {
        sprintf(armfile_error, "Unable to write filename for RISC OS (%s)",
                err->errmess);
        return EXIT_FAILURE;
    }

    // Read details of the RISC OS file
    regs.r[0] = 23;
    regs.r[1] = arm_mem;
    err = talk_swi(OS_File, &regs, &regs);
    if (err)
    {
        sprintf(armfile_error,
                "Unable to read date stamp for file '%s' (%s)",
                src, err->errmess);
        return EXIT_FAILURE;
    }
    if (regs.r[6] == -1)
    {
        // Object does not have a date stamp
        riscos_date[0] = riscos_date[1] = riscos_date[2] =
        riscos_date[3] = riscos_date[4] = 0;
    }
    else
    {
        // Extract the date stamp
        *((unsigned long *) riscos_date) = regs.r[3];
        riscos_date[4] = regs.r[2];
    }

    // Convert date and time to DOS format
    err = talk_date_to_dos(riscos_date, &time, &date);
    if (err)
    {
        sprintf(armfile_error, "Unable to convert date stamp for file '%s'",
                src);
        close_files();
        return EXIT_FAILURE;
    }

    // Open the DOS destination file
    dos_file = open(dest,
                    O_WRONLY | O_CREAT | O_TRUNC | O_BINARY | O_DENYALL,
                    S_IREAD | S_IWRITE);
    if (dos_file == -1)
    {
        sprintf(armfile_error, "Unable to open destination file '%s'", dest);
        close_files();
        return EXIT_FAILURE;
    }

    // Copy the data
    size = TRUE;
    while (size)
    {
        if (!armfile_mtask) talk_faster(100);
        err = talk_file_read(arm_file, -1, sizeof(buffer), buffer, &size);
        if (err)
        {
            sprintf(armfile_error, "Error reading from file '%s' (%s)",
                    src, err->errmess);
            close_files();
            return EXIT_FAILURE;
        }
        if (size)
        {
            size = write(dos_file, buffer, size);
            if (size == -1)
            {
                sprintf(armfile_error, "Error writing to file '%s'", dest);
            }
        }
    }

    // Set the date and time stamp
    _dos_setftime(dos_file, date, time);

    // Close the files
    close_files();

    // Return success
    talk_faster(0);
    return EXIT_SUCCESS;
}

/*
    Parameters  : src    - The source DOS filename.
                  dest    - The resulting RISC OS filename.
    Returns     : void
    Description : Perform character translation from DOS to RISC OS
                  filenames.
*/
void armfile_translate_dos_riscos(const char *src, char *dest)
{
    const char *ptr;

    // Translate the name
    while (*src)
    {
        ptr = strchr(char_dos, *src);
        if (ptr) *dest++ = char_arm[ptr - char_dos];
        else *dest++ = *src;
        src++;
    }

    // Terminate the resulting string
    *dest = 0;
}

/*
    Parameters  : src    - The source RISC OS filename.
                  dest    - The resulting DOS filename.
    Returns     : void
    Description : Perform character translation from RISC OS to DOS
                  filenames.
*/
void armfile_translate_riscos_dos(const char *src, char *dest)
{
    const char *ptr;

    // Translate the name
    while (*src)
    {
        ptr = strchr(char_arm, *src);
        if (ptr) *dest++ = char_dos[ptr - char_arm];
        else *dest++ = *src;
        src++;
    }

    // Terminate the resulting string
    *dest = 0;
}

/*
    Parameters  : buf    - Buffer to receive the filename.
                  len    - Size of the buffer.
    Returns     : int    - Standard C return code giving status of operation.
    Description : Generate a filename for a temporary RISC OS file. The file
                  is deleted before exiting.
*/
int armfile_temporary(char *buf, size_t len)
{
    const talk_error *err;
    armfile_files *ptr;

    // Initialise this module
    if (initialise()) return EXIT_FAILURE;

    // Get the temporary filename
    err = talk_temporary(buf, len);
    if (err)
    {
        sprintf(armfile_error, "Unable to generate temporary filename (%s)",
                err->errmess);
        return EXIT_FAILURE;
    }

    // Add to list of files to delete
    ptr = (armfile_files *) malloc(sizeof(armfile_files));
    if (ptr)
    {
        ptr->next = files_head;
        strcpy(ptr->name, buf);
        files_head = ptr;
    }

    // Return success
    return EXIT_SUCCESS;
}

/*
    Parameters  : dir    - The name of the RISC OS directory to create.
    Returns     : int    - Standard C return code giving status of operation.
    Description : Create a RISC OS directory.
*/
int armfile_create_dir(const char *dir)
{
    const talk_error *err;
    talk_swi_regs regs;

    // Initialise this module
    if (initialise()) return EXIT_FAILURE;

    // Write the directory pathname into a RISC OS buffer
    err = talk_write(dir, strlen(dir) + 1, arm_mem);
    if (err)
    {
        sprintf(armfile_error, "Unable to write directory name for RISC OS (%s)",
                err->errmess);
        return EXIT_FAILURE;
    }

    // Create the directory
    regs.r[0] = 8;
    regs.r[1] = arm_mem;
    regs.r[4] = 0;
    err = talk_swi(OS_File, &regs, &regs);
    if (err)
    {
        sprintf(armfile_error,
                "Unable to create directory '%s' (%s)",
                dir, err->errmess);
        return EXIT_FAILURE;
    }

    // Return success
    return EXIT_SUCCESS;
}
