/*
    File        : util.c
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : General utility functions.

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
#include "util.h"

// Include system header files
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "conio.h"
#include "dir.h"
#include "dos.h"

// Include project header files
#include "hpc.h"
#include "swi.h"

// Characters used to split filenames
#define UTIL_SPLIT_CHAR ".:"

// Object types returned by OS_GBPB
#define UTIL_OBJECT_NOT_FOUND (0)
#define UTIL_OBJECT_FILE (1)
#define UTIL_OBJECT_DIR (2)
#define UTIL_OBJECT_IMAGE (3)

// A block of ARM memory
typedef struct
{
    char dir[256];
    char leaf[256];
    struct
    {
        long load;
        long exec;
        long length;
        long attrib;
        long type;
        char name[256];
    } buffer;
} arm_mem_struct;
static arm_mem_struct arm_mem_dos;
static long arm_mem = 0;

/*
    Parameters  : void
    Returns     : void
    Description : This functions should be called before the program quits to
                  tidy up.
*/
static void tidy(void)
{
    // Free any memory that has been claimed
    if (arm_mem) talk_free(arm_mem);
}

/*
    Parameters  : void
    Returns     : int    - Standard C return code giving status of operation.
    Description : This function initialises this module. It should be called
                  on entry by each of the externally accessible functions
                  that use ARM services.
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
        err = talk_malloc(sizeof(arm_mem_struct), &arm_mem);
        if (err) return EXIT_FAILURE;

        // Set flag to prevent reinitialisation
        done = TRUE;
    }

    // If this point reached then successful
    return EXIT_SUCCESS;
}

/*
    Parameters  : path  - The RISC OS path to split.
                  dir   - The path excluding the leafname.
                  leaf  - The leafname of the file.
    Returns     : void
    Description : Split a RISC OS path into a directory and leafname. The
                  full pathname may be reconstructed by concatenating the
                  returned directory and leafnames; no extra characters
                  need be inserted.
*/
void util_arm_split(const char *path, char *dir, char *leaf)
{
    const char *ptr = path;
    const char *next;

    // Locate important characters
    next = strpbrk(path, UTIL_SPLIT_CHAR);
    while (next)
    {
        ptr = next + 1;
        next = strpbrk(ptr, UTIL_SPLIT_CHAR);
    }

    // Set the directory component
    if (dir)
    {
        strncpy(dir, path, ptr - path);
        dir[ptr - path] = 0;
    }

    // Set the leaf component
    if (leaf) strcpy(leaf, ptr);
}

/*
    Parameters  : path      - A wildcarded pathname.
                  attrib    - Required DOS file-attribute byte.
                  func      - The function to call for each match.
    Returns     : int       - The number of matches.
    Description : Call a specified function for all the files that match.
*/
int util_wildcard(const char *path, int attrib, util_func *func)
{
    struct ffblk ffblk;
    char name[MAXPATH];
    char drive[MAXDRIVE];
    char dir[MAXDIR];
    char file[MAXFILE];
    char ext[MAXEXT];
    int done, count;

    // Decompose the original path and find the first match
    fnsplit(path, drive, dir, NULL, NULL);
    done = findfirst(path, &ffblk, attrib);

    // Loop until no more matches
    count = 0;
    while (!done)
    {
        // Check that the attributes really do match
        if ((ffblk.ff_name[0] != '.') && !(attrib & ~ffblk.ff_attrib))
        {
            // Increase count of matching files
            count++;

            // Construct the full pathname
            fnsplit(ffblk.ff_name, NULL, NULL, file, ext);
            fnmerge(name, drive, dir, file, ext);

            // Call the specified function
            if (func) (*func)(name);
        }

        // Find the next matching file
        done = findnext(&ffblk);
    }

    // Return the number of matching files
    return count;
}

/*
    Parameters  : dir        - The directory path.
                  leaf       - The leafname.
    Returns     : talk_error - Any error returned.
    Description : Set the ARM memory block before calling OS_GBPB.
*/
static const talk_error *util_arm_wildcard_prepare(char *dir, char *leaf)
{
    strcpy(arm_mem_dos.dir, dir);
    if (strrchr(arm_mem_dos.dir, '.') == strrchr(arm_mem_dos.dir, 0) - 1)
    {
        *strrchr(arm_mem_dos.dir, '.') = 0;
    }
    strcpy(arm_mem_dos.leaf, leaf);
    return talk_write(&arm_mem_dos, sizeof(arm_mem_dos), arm_mem);
}

/*
    Parameters  : path      - A wildcarded pathname.
                  attrib    - The required DOS file-attribute byte. Only the
                              directory bit is used.
                  func      - The function to call for each match.
    Returns     : int       - The number of matches.
    Description : Call a specified function for all the RISC OS files that
                  match.
*/
int util_arm_wildcard(const char *path, int attrib, util_func *func)
{
    const talk_error *err;
    talk_swi_regs regs;
    char name[256];
    char dir[256];
    char leaf[256];
    int count;

    // Initialise this module
    if (initialise()) return 0;

    // Decompose the original path
    util_arm_split(path, dir, leaf);

    // Write the directory and leaf name to ARM memory
    err = util_arm_wildcard_prepare(dir, leaf);

    // Find the first match
    regs.r[0] = 10;
    regs.r[1] = arm_mem
                + ((char *) &arm_mem_dos.dir - (char *) &arm_mem_dos);
    regs.r[2] = arm_mem
                + ((char *) &arm_mem_dos.buffer - (char *) &arm_mem_dos);
    regs.r[3] = 1;
    regs.r[4] = 0;
    regs.r[5] = sizeof(arm_mem_dos.buffer);
    regs.r[6] = arm_mem
                + ((char *) &arm_mem_dos.leaf - (char *) &arm_mem_dos);
    if (!err) err = talk_swi(OS_GBPB, &regs, &regs);

    // Loop until no more matches
    count = 0;
    while (!err && (regs.r[4] != -1))
    {
        // Read the results buffer back
        err = talk_read(&arm_mem_dos, sizeof(arm_mem_dos), arm_mem);

        // Check that the attributes match
        if (!err && regs.r[3]
            && (attrib & FA_DIREC
                ? arm_mem_dos.buffer.type == UTIL_OBJECT_DIR
                : (arm_mem_dos.buffer.type == UTIL_OBJECT_FILE)
                  || (arm_mem_dos.buffer.type == UTIL_OBJECT_IMAGE)))
        {
            // Increase count of matching files
            count++;

            // Construct the full pathname
            sprintf(name, "%s%s", dir, arm_mem_dos.buffer.name);

            // Call the specified function
            if (func) (*func)(name);
        }

        // Find the next matching file
        if (!err) err = util_arm_wildcard_prepare(dir, leaf);
        regs.r[3] = 1;
        if (!err) err = talk_swi(OS_GBPB, &regs, &regs);
    }

    // Return the number of matching files
    return count;
}

/*
    Parameters  : active    - Should an activity indicator be displayed.
                  format    - The required format string.
                  ...       - Other arguments required for specified format.
    Returns     : void
    Description : Display a specified message.
*/
void util_status(int active, const char *format, ...)
{
    static int pos = 0;
    static int length = 0;
    const char chars[] = "|/-\\";
    va_list argptr;

    // Clear any previous message
    while (length)
    {
        printf("%c %c", 8, 8);
        length--;
    }

    // Display the error message
    va_start(argptr, format);
    length = vprintf(format, argptr);
    va_end(argptr);

    // Display activity indicator if required
    if (active)
    {
        length += printf(" %c", chars[pos]);
        pos = (pos + 1) % 4;
    }
}

/*
    Parameters  : format    - The required format string.
                  ...       - Other arguments required for specified format.
    Returns     : void
    Description : Exit with an error.
*/
void util_error(const char *format, ...)
{
    va_list argptr;

    // Display the error message
    util_status(FALSE, "");
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    fprintf(stderr, "\n");

    // Exit with a failed status code
    exit(EXIT_FAILURE);
}

/*
    Parameters  : err       - The error to be produced if abort selected.
                  msg       - The message to be displayed.
                  cancel    - Should the user be allowed to cancel.
    Returns     : int       - Should a retry be performed.
    Description : Prompt the user to either Abort, Retry, or optionally
                  Cancel.
*/
int util_retry(const char *err, const char *msg, int cancel)
{
    int ch;

    // Display a message
    util_status(FALSE, "%s: Abort, Retry%s", msg, cancel ? ", Cancel" : "");

    // Wait for a suitable key to be pressed
    ch = tolower(getch());
    while ((ch != 'a') && (ch != 'r') && (!cancel || (ch != 'c')))
    {
        ch = tolower(getch());
    }

    // Take appropriate action
    if (ch == 'a') util_error(err);
    util_status(FALSE, "");
    return ch == 'r';
}

/*
    Parameters  : msg   - The message to prompt with.
    Returns     : int   - Was "y" entered.
    Description : Prompt for a Y/N answer and return the result.
*/
int util_prompt(const char *msg)
{
    int ch;

    // Display the prompt
    util_status(FALSE, "");
    printf("%s (Y/N)?", msg);

    // Get a key press
    ch = getche();

    // Advance to the next line
    printf("\n");

    // Return the result
    return tolower(ch) == 'y';
}
