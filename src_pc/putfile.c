/*
    File        : putfile.c
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1995-2001, 2019
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
#include "dir.h"
#include "dos.h"

// Include project header files
#include "armfile.h"
#include "hpc.h"
#include "talk.h"
#include "util.h"
#include "wimp.h"

#define TITLE "PUTFILE 1.05 (18 April 1998) [TEST] (c) A.Thoukydides, 1995-2001\n"
#define SYNTAX "PUTFILE [/?] [/M] [/P] [/S] [/W] source [destination]\n"
#define HELP TITLE "\n"\
             "Copy files from DOS to RISC OS.\n\n"\
             SYNTAX "\n"\
             "  /?             Display this help text.\n"\
             "  /M             Enable multitasking while copying.\n"\
             "  /P             Prompts before copying each file.\n"\
             "  /S             Copy subdirectories.\n"\
             "  /W             Disable use of the \"Save as\" window.\n"\
             "  source         The wildcarded path of the DOS files to copy.\n"\
             "  destination    The path of the RISC OS files to write.\n"\
             "\n"\
             "The \"Save as\" window requires !ARMEdit to be running.\n"

// Timeout in centiseconds
#define PUTFILE_TIMEOUT 500

// Global variables
static int window = TRUE;
static int prompt = FALSE;
static char stub[MAXPATH] = "";
static char *dest = NULL;
static int files = 0;
static int dirs = 0;

/*
    Parameters  : from  - The DOS style pathname.
                  to    - Variable to receive the RISC OS pathname.
    Returns     : void
    Description : Generate a RISC OS style pathname from the DOS name given.
*/
static void generate(const char *from, char *to)
{
    // Was a destination path specified
    if (dest)
    {
        // Is just a single file being copied
        if ((files == 1) && (dirs == 0))
        {
            // Just use the specified destination
            strcpy(to, dest);
        }
        else
        {
            char path[256];

            // Translate the extension
            armfile_translate_dos_riscos(from + strlen(stub), path);

            // Add the translated extension to the specified destination
            sprintf(to, "%s.%s", dest, path);
        }
    }
    else
    {
        char path[256];
        char *ptr = path;

        // Just translate the path
        armfile_translate_dos_riscos(from, path);

        // Tidy up the result
        if (ptr[1] == ':') ptr += 2;
        if (ptr[0] == '.') ptr++;

        // Copy the result
        strcpy(to, ptr);
    }
}

/*
    Parameters  : from  - The DOS file to copy.
    Returns     : void
    Description : Copy the specified file.
*/
static void copy_file(const char *from)
{
    const talk_error *err;
    char to[256];

    // Prompt before copying if required
    if (prompt && !util_prompt(from)) return;

    // Choose a destination path
    if (window)
    {
        // Choose a temporary destination file
        err = talk_temporary(to, sizeof(to));
        if (err) util_error("Unable to choose a temporary filename (%s)\n", err->errmess);
    }
    else
    {
        // Translate the destination path
        generate(from, to);
    }

    // Copy the file
    if (armfile_copy_dos_riscos(from, to)) util_error("%s\n", armfile_error);

    // Open a SaveAs window if required
    if (window)
    {
        char suggest[256];

        // Choose a suitable name to suggest
        armfile_translate_dos_riscos(from, suggest);

        // Attempt to open the window
        err = wimp_saveas(to, suggest);
        if (!err) err = wimp_saveas_receive(PUTFILE_TIMEOUT);
        if (err) util_error("Unable to start SaveAs (%s)\n", err->errmess);
    }
}

/*
    Parameters  : from  - The DOS directory path to copy.
    Returns     : void
    Description : Recursively copy the specified directory.
*/
static void copy_dir(const char *from)
{
    char name[MAXPATH];
    char to[256];

    // Create a RISC OS directory if required
    if (!window)
    {
        // Generate RISC OS directory name
        generate(from, to);

        // Create the RISC OS directory
        armfile_create_dir(to);
    }

    // Generate path to match
    sprintf(name, "%s\\*.*", from);

    // Copy all the files in this directory
    util_wildcard(name, 0, copy_file);

    // Recurse through subdirectories
    util_wildcard(name, FA_DIREC, copy_dir);
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
    int subdir = FALSE;
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

                case 'm':
                    armfile_mtask = TRUE;
                    break;

                case 'p':
                    prompt = TRUE;
                    break;

                case 's':
                    subdir = TRUE;
                    break;

                case 'w':
                    window = FALSE;
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
    if (!first)
    {
        fprintf(stderr, "No filenames specified\n");
        return EXIT_FAILURE;
    }
    else if (2 < argc - first)
    {
        fprintf(stderr, "Too many filenames specified\n");
        return EXIT_FAILURE;
    }

    // Count the number of matching items
    files = util_wildcard(argv[first], 0, NULL);
    if (subdir) dirs = util_wildcard(argv[first], FA_DIREC, NULL);

    // Check that at least one file matches
    if (!files && !dirs) util_error("No matching files\n");

    // Prepare for use of SaveAs window if required
    if (1 < argc - first)
    {
        window = FALSE;
        dest = argv[first + 1];
    }
    if (window)
    {
        // Connect to !ARMEdit server
        err = wimp_find();
        if (!err) err = wimp_find_receive(1000);
        if (err) util_error("Unable to connect to !ARMEdit (%s)\n", err->errmess);
    }
    else
    {
        char drive[MAXDRIVE];
        char dir[MAXDIR];

        // Generate the source stub to remove
        fnsplit(argv[first], drive, dir, NULL, NULL);
        fnmerge(stub, drive, dir, NULL, NULL);
    }

    // Copy all matching files
    util_wildcard(argv[first], 0, copy_file);
    if (subdir) util_wildcard(argv[first], FA_DIREC, copy_dir);

    // If this point reached then completed successfully
    return EXIT_SUCCESS;
}
