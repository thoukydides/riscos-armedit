/*
    File        : oscli.c
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1995-2001, 2019
    Description : Perform RISC OS *commands from the DOS command line.

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
#include <conio.h>

// Include project header files
#include "talk.h"
#include "swi.h"
#include "hpc.h"
#include "cli.h"
#include "vdu.h"
#include "util.h"
#include "wimp.h"

#define TITLE "OSCLI 1.05 (18 April 1998) [TEST] (c) A.Thoukydides, 1995-2001\n"
#define SYNTAX "OSCLI [/?] [/I | /S | /T] [/R] [command]\n"
#define HELP TITLE "\n"\
        "Execute RISC OS *commands.\n\n"\
        SYNTAX "\n"\
        "  /?         Display this help text.\n"\
        "  /I         Interactive mode (both input and output redirected).\n"\
        "  /S         Simple mode (output is not redirected through DOS).\n"\
        "  /T         Execute the command in a RISC OS TaskWindow.\n"\
        "  /R         Raw display mode (VDU codes not processed before display).\n"\
        "  command    The *command to execute.\n"\
        "\n"\
        "If no command is specified then shell mode is entered. To exit just press\n"\
        "Return at the * prompt without entering any other characters on the line.\n"\
        "\n"\
        "The TaskWindow option requires !ARMEdit to be running.\n"

// Timeout in centiseconds
#define OSCLI_TIMEOUT 500

/* User selected options */
static int spool = TRUE;
static int trap = FALSE;
static int process = TRUE;
static int task = FALSE;

/*
    Parameters  : cmd   - The command to process.
    Returns     : int   - Standard C return code giving status of operation.
    Description : Execute the specified command using the selected options.
*/
static int oscli(const char *cmd)
{
    int success;
    const talk_error *err;

    // Process the command as required
    if (task)
    {
        long handle;
        long active;

        // Start the TaskWindow
        err = wimp_oscli_start(cmd, "ARMEdit OSCLI");
        if (!err) err = wimp_oscli_start_receive(OSCLI_TIMEOUT, &handle);

        // Keep polling the command until finished
        active = TRUE;
        while (!err && active)
        {
            long bytes;
            char buffer[1000];

            // Take special action if it is the keyboard
            if (stdin->flags & _F_TERM)
            {
                int ch;

                // Input is from the keyboard
                bytes = 0;
                while ((bytes + 1 < sizeof(buffer))
                       && ((ch = cli_getch(TRUE)) != 0))
                {
                    // Handle extended character codes
                    if (0x100 <= ch) buffer[bytes++] = 0;

                    // Add the character to the buffer
                    buffer[bytes++] = ch;
                }
            }
            else
            {
                // Input is from the specified stream
                if (feof(stdin)) bytes = 0;
                else
                {
                    buffer[0] = getc(stdin);
                    bytes = 1;
                }
            }

            // Poll the command status
            err = wimp_oscli_poll(handle, bytes, buffer);
            if (!err)
            {
                err = wimp_oscli_poll_receive(OSCLI_TIMEOUT, &active,
                                              &bytes, buffer);
            }

            // Display any output produced
            if (!err && bytes)
            {
                (*cli_output_filter_current)(stdout, buffer, bytes);
            }
        }

        // Terminate the TaskWindow
        err = wimp_oscli_end(handle);
        if (!err) err = wimp_oscli_end_receive(OSCLI_TIMEOUT);

        // Copy any error produced
        success = err ? EXIT_FAILURE : EXIT_SUCCESS;
        if (err) strcpy(cli_error, err->errmess);
    }
    else
    {
        // Use a more direct approach
        success = trap ? cli_redir(cmd, stdin, stdout)
                       : cli_simple(cmd, spool ? stdout : NULL);
    }

    // Return the status
    return success;
}

/*
    Parameters  : void
    Returns     : int   - Standard C return code giving status of operation.
    Description : Provide a command line shell that allows multiple commands
                  to be typed and processed before the software exits.
*/
static int shell(void)
{
    char cmd[256];

    if (cli_getline(stdin, stdout, cmd, sizeof(cmd))) return EXIT_FAILURE;
    while (*cmd)
    {
        // Process this command
        if (oscli(cmd))
        {
            (*cli_output_filter_current)(stdout, cli_error, strlen(cli_error));
            (*cli_output_filter_current)(stdout, "\n\r", 2);
        }

        // Read another command from the user
        if (cli_getline(stdin, stdout, cmd, sizeof(cmd))) return EXIT_FAILURE;
    }

    // If this point reached then the operation was successful
    return EXIT_SUCCESS;
}

/*
        Parameters  : argc  - The number of command line arguments.
                      argv  - The actual arguments.
        Returns     : int   - Return code.
        Description : The main program.
*/
int main(int argc, char *argv[])
{
    char cmd[256];
    const talk_error *err;
    int i;

    // Special case if no arguments
    if (argc == 1)
    {
        fprintf(stdout, TITLE SYNTAX
                        "\nPress Return without entering any text to exit.\n");
    }

    // Process the command line
    cmd[0] = 0;
    for (i = 1; i < argc; i++)
    {
        // Check if it could be an option switch
        if (!cmd[0] && (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
                case '?':
                    printf(HELP);
                    return EXIT_SUCCESS;
                    /* break; */

                case 'i':
                    trap = TRUE;
                    break;

                case 'r':
                    process = FALSE;
                    break;

                case 's':
                    spool = FALSE;
                    break;

                case 't':
                    task = TRUE;
                    break;

                default:
                    fprintf(stderr, "Unrecognised switch '%s'\n", argv[i]);
                    return EXIT_FAILURE;
                    /* break; */
            }
        }
        else
        {
            if (*cmd) strcat(cmd, " ");
            strcat(cmd, argv[i]);
        }
    }

    // Check the command line
    if ((!spool && (task || trap)) || (task && trap))
    {
        fprintf(stderr, "Invalid combination of switches\n");
        return EXIT_FAILURE;
    }

    // Prepare for use of the TaskWindow
    if (task)
    {
        // Connect to !ARMEdit server
        err = wimp_find();
        if (!err) err = wimp_find_receive(1000);
        if (err) util_error("Unable to connect to !ARMEdit (%s)\n", err->errmess);
    }

    // Set an output filter if selected
    if (process) cli_output_filter_current = vdu_output_filter;

    // Take appropriate action
    if (*cmd ? oscli(cmd) : shell())
    {
        fprintf(stderr, "%s\n", cli_error);
        return EXIT_FAILURE;
    }

    // If this point reached then completed successfully
    return EXIT_SUCCESS;
}
