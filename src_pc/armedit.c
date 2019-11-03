/*
    File        : armedit.c
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Command to transfer DOS files to RISC OS.

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

// Include system header files
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Include project header files
#include "armfile.h"
#include "hpc.h"
#include "talk.h"
#include "util.h"
#include "wimp.h"

#define TITLE "ARMEDIT 1.05 (18 April 1998) [TEST] (c) A.Thoukydides, 1996-2001\n"
#define SYNTAX "ARMEDIT [/?] file [file [...]]\n"\
               "ARMEDIT [/?] /L file [line [file [line [...]]]]\n"
#define HELP TITLE "\n"\
             "Edit a file using a RISC OS editor via the External Data Editing Protocol.\n\n"\
             SYNTAX "\n"\
             "  /?             Display this help text.\n"\
             "  /L             Filenames are followed by line number to place cursor at.\n"\
             "  file           The name of the file to edit.\n"\
             "  line           Line number to position cursor at.\n"\
             "\n"\
             "This command requires !ARMEdit and a suitable RISC OS editor to be running.\n"

// Timeout in centiseconds
#define PUTFILE_TIMEOUT 500

// The line to place the cursor at initially
static int edit_line;

// Linked list of external edits
struct edit_list
{
    edit_list *next;
    edit_list *prev;
    char local[256];
    char remote[256];
    long handle;
    long flags;
};
static edit_list *edit_head = NULL;

/*
    Parameters  : path    - The path of the file to start an edit for.
    Returns     : void
    Description : Start an external edit for the specified file.
*/
static void edit_start(const char *path)
{
    edit_list *ptr;
    FILE *file;
    int line;
    int pos;
    char leaf[20];
    const char *dot;
    const talk_error *err;

    // Update the status display
    util_status(TRUE, "Starting external edit");

    // Create a new edit record
    ptr = (edit_list *) malloc(sizeof(edit_list));
    if (!ptr) util_error("Unable to allocate memory for edit record");
    strcpy(ptr->local, path);

    // Choose a RISC OS filename
    if (armfile_temporary(ptr->remote, sizeof(ptr->remote)))
    {
        util_error("Unable to choose filename for edit (%s)", armfile_error);
    }

    // Copy the file to RISC OS
    if (armfile_copy_dos_riscos(ptr->local, ptr->remote))
    {
        util_error("Unable to send file (%s)", armfile_error);
    }

    // Find the required line offset
    file = fopen(ptr->local, "rb");
    line = 1 < edit_line ? edit_line : 1;
    pos = 0;
    while (file && !feof(file) && --line)
    {
        pos++;
        while (!feof(file) && (fgetc(file) != '\n')) pos++;
    }
    if (file) fclose(file);

    // Choose a suitable leaf name for the file
    dot = strrchr(ptr->local, '\\');
    armfile_translate_dos_riscos(dot ? dot + 1 : ptr->local, leaf);

    // Start the external edit
    err = wimp_start(pos, leaf, ptr->remote);
    if (!err) err = wimp_start_receive(1000, &ptr->handle);
    if (err) util_error("Unable to start external edit (%s)", err->errmess);
    ptr->flags = 0;

    // Link in the edit record
    ptr->prev = NULL;
    ptr->next = edit_head;
    if (edit_head) edit_head->prev = ptr;
    edit_head = ptr;
}

/*
    Parameters  : argc  - The number of command line arguments.
                  argv  - The actual arguments.
    Returns     : int   - Return code.
    Description : The main program.
*/
int main(int argc, char *argv[])
{
    int i, first = 0;
    int numbers = FALSE;
    const talk_error *err;

    // Simple case is no arguments
    if (argc == 1)
    {
        fprintf(stdout, TITLE SYNTAX);
        return EXIT_SUCCESS;
    }

    // Process the command line
    for (i = 0; i < argc; i++)
    {
        if (!first && (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
                case '?':
                    printf(HELP);
                    return EXIT_SUCCESS;
                    /* break; */

                case 'l':
                    numbers = TRUE;
                    break;

                default:
                    fprintf(stderr, "Unrecognised switch '%s'\n", argv[i]);
                    return EXIT_FAILURE;
                    /* break; */
            }
        }
        else if (!first) first = i;
    }

    // Check the command line
    if (!first) util_error("No filenames specified\n");

    // Connect to !ARMEdit server
    util_status(FALSE, "Connecting to !ARMEdit");
    err = wimp_find();
    if (!err) err = wimp_find_receive(1000);
    if (err) util_error("Unable to connect to !ARMEdit (%s)\n", err->errmess);

    // Start the external edits
    i = first;
    while (i < argc)
    {
        // Choose the required line number
        edit_line = numbers && (i + 1 < argc) ? atoi(argv[i + 1]) : 1;

        // Start edits for matching files
        util_wildcard(argv[i], 0, edit_start);

        // Advance the argument pointer
        i += numbers ? 2 : 1;
    }

    // Loop until no more active edits
    while (edit_head)
    {
        edit_list *ptr;

        // Loop through all of the active edits
        ptr = edit_head;
        while (ptr)
        {
            long flags;

            // Update the status message
            util_status(TRUE, "Polling edit status");

            // Attempt to perform the poll
            err = wimp_poll(ptr->handle, ptr->flags);
            if (!err) err = wimp_poll_receive(1000, &flags);
            if (err) util_error("Unable to poll edit status (%s)", err->errmess);

            // Process the returned flags
            ptr->flags = 0;
            if (flags & WIMP_POLL_RX_FLAG_MODIFIED)
            {
                // Update the displayed status
                util_status(FALSE, "Retrieving file");

                // Retrieve the file
                if (armfile_copy_riscos_dos(ptr->remote, ptr->local))
                {
                    util_error("Unable to retrieve file (%s)", armfile_error);
                }

                // Set the flags to return
                ptr->flags |= WIMP_POLL_TX_FLAG_SAFE;
            }
            if (flags & WIMP_POLL_RX_FLAG_ABORTED)
            {
                // Unlink this record
                if (ptr->prev) ptr->prev->next = ptr->next;
                else edit_head = ptr->next;
                if (ptr->next) ptr->next->prev = ptr->prev;
            }

            // Advance to the next record
            ptr = ptr->next;
        }
    }

    // Clear any status message still displayed
    util_status(FALSE, "");

    // If this point reached then completed successfully
    return EXIT_SUCCESS;
}
