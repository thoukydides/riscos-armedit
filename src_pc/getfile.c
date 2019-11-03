/*
    File        : getfile.c
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1995-2001, 2019
    Description : Command to transfer RISC OS files to DOS.

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
#include "util.h"

#define TITLE "GETFILE 1.05 (18 April 1998) [TEST] (c) A.Thoukydides, 1995-2001\n"
#define SYNTAX "GETFILE [/?] [/M] [/P] [/S] source [destination]\n"
#define HELP TITLE "\n"\
             "Copy files from RISC OS to DOS.\n\n"\
             SYNTAX "\n"\
             "  /?             Display this help text.\n"\
             "  /M             Enable multitasking while copying.\n"\
             "  /P             Prompts before copying each file.\n"\
             "  /S             Copy subdirectories.\n"\
             "  source         The wildcarded path of the RISC OS files to copy.\n"\
             "  destination    The path of the DOS files to write.\n"

// Timeout in centiseconds
#define PUTFILE_TIMEOUT 500

// Global variables
static int prompt = FALSE;
static char stub[256] = "";
static char *dest = NULL;
static int files = 0;
static int dirs = 0;

/*
    Parameters  : from  - The RISC OS style pathname.
                  to    - Variable to receive the DOS pathname.
    Returns     : void
    Description : Generate a DOS style pathname from the RISC OS name given.
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
            char path[MAXPATH];

            // Translate the extension
            armfile_translate_riscos_dos(from + strlen(stub), path);

            // Add the translated extension to the specified destination
            sprintf(to, "%s\\%s", dest, path);
        }
    }
    else
    {
        char path[MAXPATH];
        char *ptr = path;

        // Just translate the path
        armfile_translate_riscos_dos(from, path);

        // Copy the result
        strcpy(to, ptr);
    }
}

/*
    Parameters  : from  - The RISC OS file to copy.
    Returns     : void
    Description : Copy the specified file.
*/
static void copy_file(const char *from)
{
    char to[MAXPATH];

    // Prompt before copying if required
    if (prompt && !util_prompt(from)) return;

    // Translate the destination path
    generate(from, to);

    // Copy the file
    if (armfile_copy_riscos_dos(from, to)) util_error("%s\n", armfile_error);
}

/*
    Parameters  : from  - The RISC OS directory path to copy.
    Returns     : void
    Description : Recursively copy the specified directory.
*/
static void copy_dir(const char *from)
{
    char name[256];
    char to[MAXPATH];

    // Generate DOS directory name
    generate(from, to);

    // Create the DOS directory
    mkdir(to);

    // Generate path to match
    sprintf(name, "%s.*", from);

    // Copy all the files in this directory
    util_arm_wildcard(name, 0, copy_file);

    // Recurse through subdirectories
    util_arm_wildcard(name, FA_DIREC, copy_dir);
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
    files = util_arm_wildcard(argv[first], 0, NULL);
    if (subdir) dirs = util_arm_wildcard(argv[first], FA_DIREC, NULL);

    // Check that at least one file matches
    if (!files && !dirs) util_error("No matching files\n");

    // Prepate the paths
    if (1 < argc - first) dest = argv[first + 1];
    util_arm_split(argv[first], stub, NULL);

    // Copy all matching files
    util_arm_wildcard(argv[first], 0, copy_file);
    if (subdir) util_arm_wildcard(argv[first], FA_DIREC, copy_dir);

    // If this point reached then completed successfully
    return EXIT_SUCCESS;
}
