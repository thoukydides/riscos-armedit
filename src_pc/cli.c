/*
    File        : cli.c
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Execute RISC OS *commands.

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
#include "cli.h"

// Include system header files
#include <ctype.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

// Include project header files
#include "armfile.h"
#include "talk.h"
#include "swi.h"
#include "hpc.h"

// Variable to contain any error message
char cli_error[256] = "";

// Address of ARM memory allocated
static long arm_mem = 0;

// RISC OS file handle
static long file_handle = 0;

// Handle for the command being executed
static long cmd_handle = 0;

/* The current output filter */
cli_output_filter *cli_output_filter_current = cli_output_filter_none;

/*
    Parameters  : stream    - The stream to redirect output to.
                  bytes     - Pointer to the bytes of data to output.
                  size      - The number of bytes to output.
    Returns     : void
    Description : An output filter that does not change the data.
*/
void cli_output_filter_none(FILE *stream, const char *bytes, size_t size)
{
    // Simply copy the data to the specified stream
    if (size) fwrite(bytes, sizeof(char), size, stream);
}

/*
    Parameters  : void
    Returns     : void
    Description : Tidy up before the program exits.
*/
static void tidy(void)
{
    // Close the RISC OS file
    if (file_handle)
    {
        talk_file_close(file_handle);
        file_handle = 0;
    }

    // Free the memory
    if (arm_mem)
    {
        talk_free(arm_mem);
        arm_mem = 0;
    }

    // Terminate command
    if (cmd_handle)
    {
        talk_oscli_end(cmd_handle);
        cmd_handle = 0;
    }
}

/*
    Parameters  : None
    Returns     : None
    Description : Initialise all features of the module.
*/
static void initialise()
{
    static int done = FALSE;

    // Initialise if not already done
    if (!done)
    {
        // Set flag to prevent multiple initialisations
        done = TRUE;

        // Register a handler to tidy up before exiting
        atexit(tidy);
    }
    else
    {
        // Tidy up if there was an error previously
        tidy();
    }
}

/*
    Parameters  : riscos    - Should the code be converted into a standard
                              RISC OS keystroke.
    Returns     : int       - The extended or RISC OS key code, or 0 if no
                              key was pressed.
    Description : Read a single key from the input buffer, combining extended
                  key codes as required. This does not wait if the buffer is
                  empty. This can optionally translate codes into their
                  RISC OS equivalent.
*/
int cli_getch(int riscos)
{
    int ch;
    const struct
    {
        int pc;
        int riscos;
    } map[] =
    {
        {0x008, 0x07f}, // Backspace
        {0x147, 0x001}, // Home
        {0x153, 0x004}, // Delete
        {0x13b, 0x181}, // F1
        {0x13c, 0x182}, // F2
        {0x13d, 0x183}, // F3
        {0x13e, 0x184}, // F4
        {0x13f, 0x185}, // F5
        {0x140, 0x186}, // F6
        {0x141, 0x187}, // F7
        {0x142, 0x188}, // F8
        {0x143, 0x189}, // F9
        {0x154, 0x191}, // Shift F1
        {0x155, 0x192}, // Shift F2
        {0x156, 0x193}, // Shift F3
        {0x157, 0x194}, // Shift F4
        {0x158, 0x195}, // Shift F5
        {0x159, 0x196}, // Shift F6
        {0x15a, 0x197}, // Shift F7
        {0x15b, 0x198}, // Shift F8
        {0x15c, 0x199}, // Shift F9
        {0x15e, 0x1a1}, // Ctrl F1
        {0x15f, 0x1a2}, // Ctrl F2
        {0x160, 0x1a3}, // Ctrl F3
        {0x161, 0x1a4}, // Ctrl F4
        {0x162, 0x1a5}, // Ctrl F5
        {0x163, 0x1a6}, // Ctrl F6
        {0x164, 0x1a7}, // Ctrl F7
        {0x165, 0x1a8}, // Ctrl F8
        {0x166, 0x1a9}, // Ctrl F9
        {0x168, 0x1b1}, // Ctrl Shift F1
        {0x169, 0x1b2}, // Ctrl Shift F2
        {0x16a, 0x1b3}, // Ctrl Shift F3
        {0x16b, 0x1b4}, // Ctrl Shift F4
        {0x16c, 0x1b5}, // Ctrl Shift F5
        {0x16d, 0x1b6}, // Ctrl Shift F6
        {0x16e, 0x1b7}, // Ctrl Shift F7
        {0x16f, 0x1b8}, // Ctrl Shift F8
        {0x170, 0x1b9}, // Ctrl Shift F9
        {0x009, 0x18a}, // Tab
        {0x14f, 0x18b}, // Copy
        {0x175, 0x1ab}, // Ctrl Copy
        {0x14b, 0x18c}, // Left
        {0x173, 0x1ac}, // Ctrl left
        {0x14d, 0x18d}, // Right
        {0x174, 0x1ad}, // Ctrl right
        {0x150, 0x18e}, // Down
        {0x148, 0x18f}, // Up
        {0x151, 0x19e}, // Page down
        {0x176, 0x1be}, // Ctrl page down
        {0x149, 0x19f}, // Page up
        {0x184, 0x1bf}, // Ctrl page up
        {0x144, 0x1ca}, // F10
        {0x145, 0x1cb}, // F11
        {0x146, 0x1cc}, // F12
        {0x15d, 0x1da}, // Shift F10
        {0x167, 0x1ea}, // Ctrl F10
        {0x171, 0x1fa}, // Ctrl Shift F10
        {0x152, 0x1cd}  // Insert
    };

    // Check if a character is available
    if (kbhit())
    {
        // Read the first byte of the character
        ch = getch();

        // Check if the character is extended
        if (!ch && kbhit()) ch = getch() | 0x100;
    }
    else ch = 0;

    if (ch && riscos)
    {
        int i;

        // Map any required key codes
        for (i = 0; i < (sizeof(map) / sizeof(map[0])); i++)
        {
            if (map[i].pc == ch)
            {
                // Map this key code
                ch = map[i].riscos;
                i = 0;
                break;
            }
        }

        // Filter out any other extended codes
        if (i && (0x100 <= ch)) ch = 0;
    }

    // Return the resulting key code
    return ch;
}

/*
    Parameters  : cmd       - The command to execute.
                  stream    - Optional stream to redirect output to. If a
                              NULL pointer is passed then no redirection is
                              used.
    Returns     : int       - Standard C return code giving status of
                              operation.
    Description : Perform a *command simply. The { > file } method of
                  redirection is used if required. No redirection of input
                  is supported.
*/
int cli_simple(const char *cmd, FILE *stream)
{
    char spool[256];
    char str[256];
    const talk_error *err;
    talk_swi_regs regs;
    long size;

    // Initialise the module
    initialise();

    // Copy the command to execute
    strcpy(str, cmd);

    // Add the redirection if required
    if (stream)
    {
        if (armfile_temporary(spool, sizeof(spool)))
        {
            sprintf(cli_error, "Unable to generate a temporary filename (%s)", armfile_error);
            return EXIT_FAILURE;
        }
        strcat(str, "{ > ");
        strcat(str, spool);
        strcat(str, " }");
    }

    // Claim some RISC OS memory to place the command into
    err = talk_malloc(strlen(str) + 1, &arm_mem);
    if (err)
    {
        sprintf(cli_error, "Unable to claim ARM memory (%s)", err->errmess);
        return EXIT_FAILURE;
    }

    // Copy the command into ARM memory
    err = talk_write(str, strlen(str) + 1, arm_mem);
    if (err)
    {
        sprintf(cli_error, "Unable to write to ARM memory (%s)", err->errmess);
        return EXIT_FAILURE;
    }

    // Execute the command
    regs.r[0] = arm_mem;
    err = talk_swi(OS_CLI, &regs, NULL);
    if (err)
    {
        strcpy(cli_error, err->errmess);
        return EXIT_FAILURE;
    }

    // Process the output if required
    if (stream)
    {
        // Open the output spool file
        err = talk_file_open(spool, -1, TRUE, &file_handle);
        if (err)
        {
            sprintf(cli_error, "Unable to read output of command (%s)",
                    err->errmess);
            return EXIT_FAILURE;
        }

        // Display the contents of the spool file
        size = TRUE;
        while (size)
        {
            err = talk_file_read(file_handle, -1, sizeof(str), str, &size);
            if (err)
            {
                sprintf(cli_error, "Unable to read output of command (%s)",
                        err->errmess);
                return EXIT_FAILURE;
            }
            if (size && cli_output_filter_current)
            {
                (*cli_output_filter_current)(stream, str, size);
            }
        }
    }

    // Tidy up before returning
    tidy();

    // If this point reached then the operation was successful
    return EXIT_SUCCESS;
}

/*
    Parameters  : cmd   - The command to execute.
                  in    - Stream to use for input. If a NULL pointer is
                          passed then the keyboard is used for input.
                  out   - Stream to use for output. If a NULL pointer is
                          passed then stdout is used.
    Returns     : int   - Standard C return code giving status of operation.
    Description : Perform a *command with full redirection. Due to the way in
                  which the command may need to be suspended, the commands
                  that may be executed are more restricted.
*/
int cli_redir(const char *cmd, FILE *in, FILE *out)
{
    const talk_error *err;
    long status = TALK_OSCLI_ACTIVE;
    long size;
    char data[256];

    // Use default streams if not specified
    if (!in) in = stdin;
    if (!out) out = stdout;

    // Start execution
    err = talk_oscli_start(cmd, &cmd_handle);
    if (err)
    {
        sprintf(cli_error, "Unable to execute command (%s)", err->errmess);
        return EXIT_FAILURE;
    }

    // Continue execution until finished
    while (status != TALK_OSCLI_FINISHED)
    {
        // Only accept input if it is required
        if (status == TALK_OSCLI_WAITING)
        {
            // Take special action if it is the keyboard
            if (in->flags & _F_TERM)
            {
                int ch;

                // Input is from the keyboard
                size = 0;
                while ((size + 1 < sizeof(data))
                       && ((ch = cli_getch(TRUE)) != 0))
                {
                    // Handle extended character codes
                    if (0x100 <= ch) data[size++] = 0;

                    // Add the character to the buffer
                    data[size++] = ch;
                }
            }
            else
            {
                // Input is from the specified stream
                if (feof(in)) size = -1;
                else
                {
                    data[0] = getc(in);
                    size = 1;
                }
            }
        }
        else size = 0;

        err = talk_oscli_poll(cmd_handle, size, data, &status, &size, data);
        if (err)
        {
            sprintf(cli_error, "Unable to continue command (%s)", err->errmess);
            return EXIT_FAILURE;
        }

        if (size && cli_output_filter_current)
        {
            (*cli_output_filter_current)(out, data, size);
        }
    }

    // Check for any error
    err = talk_oscli_end(cmd_handle);
    if (err)
    {
        strcpy(cli_error, err->errmess);
        return EXIT_FAILURE;
    }

    // Tidy up before returning
    tidy();

    // If this point reached then the operation was successful
    return EXIT_SUCCESS;
}

/*
    Parameters  : var   - The name of the variable to read, which may be
                          wildcarded (using '*' and '#').
                  data  - Pointer to a buffer to receive the variable value.
                  size  - Size of the buffer.
    Returns     : int   - Standard C return code giving status of operation.
    Description : Read the value of the first matching variable. The value is
                  always converted to a string.
*/
int cli_read_var(const char *var, char *data, size_t size)
{
    const talk_error *err;
    talk_swi_regs regs;
    int var_size;

    // Initialise the module
    initialise();

    // Claim some RISC OS memory for the variable and value
    var_size = strlen(var) + 1;
    err = talk_malloc(var_size + size, &arm_mem);
    if (err)
    {
        sprintf(cli_error, "Unable to claim ARM memory (%s)", err->errmess);
        return EXIT_FAILURE;
    }

    // Copy the variable name into ARM memory
    err = talk_write(var, var_size, arm_mem);
    if (err)
    {
        sprintf(cli_error, "Unable to write to ARM memory (%s)", err->errmess);
        return EXIT_FAILURE;
    }

    // Attempt to read the variable value
    regs.r[0] = arm_mem;
    regs.r[1] = arm_mem + var_size;
    regs.r[2] = size - 1;
    regs.r[3] = 0;
    regs.r[4] = 3;
    err = talk_swi(OS_ReadVarVal, &regs, &regs);
    if (err)
    {
        strcpy(cli_error, err->errmess);
        return EXIT_FAILURE;
    }

    // Read the value of the variable
    size = regs.r[2];
    err = talk_read(data, size, arm_mem + var_size);
    if (err)
    {
        sprintf(cli_error, "Unable to read ARM memory (%s)", err->errmess);
        return EXIT_FAILURE;
    }
    data[size] = 0;

    // Tidy up before returning
    tidy();

    // If this point reached then the operation was successful
    return EXIT_SUCCESS;
}

/*
    Parameters  : out   - Stream to use for output. If a NULL pointer is
                          passed then stdout is used.
                  num   - The number of characters to move the cursor by, This
                          may be negative to move the cursor backwards.
                  space - Should the cursor be moved using space/backspace
                          characters instead of by horizontal tabs.
    Returns     : void
    Description : Move the cursor by the specified number of places, optionally
                  overwriting existing characters with spaces.
*/
static void cli_getline_move(FILE *out, int num, int space)
{
    char ch;

    // Which direction should the cursor be moved
    if (num < 0)
    {
        ch = space ? 0x127 : 8;
        num = -num;
    }
    else ch = space ? ' ' : 9;

    // Move the cursor the required number of positions
    while (num)
    {
        (*cli_output_filter_current)(out, &ch, 1);
        num--;
    }
}

/*
    Parameters  : out   - Stream to use for output. If a NULL pointer is
                          passed then stdout is used.
                  line  - The new command line text.
                  start - The first character that has changed.
                  end   - The character after the last one that has changed.
                  from  - The current cursor position.
                  to    - The new cursor position.
    Returns     : void
    Description : Update the screen display following a change to the line
                  being edited.
*/
static void cli_getline_show(FILE *out, char *line, int start, int end,
                             int from, int to)
{
    int tail;

    // Remove silly cases
    if (start < 0) start = 0;
    if (end <= start) start = end = from;

    // Move to the start of the changed region
    cli_getline_move(out, start - from, FALSE);

    // Display the required new characters
    tail = strchr(line, 0) - line;
    if (end < tail) tail = end;
    if (tail < start) tail = start;
    if (start < tail)
    {
        (*cli_output_filter_current)(out, line + start, tail - start);
    }

    // Pad with spaces if required to the end of the changed region
    cli_getline_move(out, end - tail, TRUE);

    // Move the cursor to the require position
    cli_getline_move(out, to - end, FALSE);
}

/*
    Parameters  : in    - Stream to use for input. If a NULL pointer is
                          passed then the keyboard is used for input.
                  out   - Stream to use for output. If a NULL pointer is
                          passed then stdout is used.
                  line  - A variable to receive the input line.
                  size  - The size of the line buffer including the
                          terminating NULL character.
    Returns     : int   - Standard C return code giving status of operation.
    Description : Read a line from the specified input stream and echo it to
                  the specified output stream. A suitable prompt is displayed.
*/
int cli_getline(FILE *in, FILE *out, char *line, size_t size)
{
    typedef struct history_record_struct
    {
        history_record_struct *next;
        history_record_struct *prev;
        char *line;
    } history_record;
    static history_record *last = NULL;
    const char cursor_insert[] = {23, 1, 3, 0, 0, 0, 0, 0, 0, 0};
    const char cursor_overwrite[] = {23, 1, 2, 0, 0, 0, 0, 0, 0, 0};
    char prompt[80];

    // Use default streams if not specified
    if (!in) in = stdin;
    if (!out) out = stdout;

    // Read the user defined prompt if any
    if (cli_read_var("cli$prompt", prompt, sizeof(prompt)) || !*prompt)
    {
        strcpy(prompt, "*");
    }

    // Display the selected prompt
    (*cli_output_filter_current)(out, prompt, strlen(prompt));

    // Only use history and cursor editing if input and output both terminal
    if ((in->flags & _F_TERM) && (out->flags & _F_TERM)
        && (cli_output_filter_current != cli_output_filter_none))
    {
        history_record *history;
        int done;
        int cursor;
        int length;
        int insert;

        // Select the insert cursor
        insert = TRUE;
        (*cli_output_filter_current)(out, cursor_insert, sizeof(cursor_insert));

        // Keep reading and processing characters until return is pressed
        cursor = length = 0;
        *line = 0;
        done = FALSE;
        history = NULL;
        while (!done)
        {
            int old_cursor = cursor;
            int start = cursor;
            int end = cursor;
            int ch;

            // Read a character from the keyboard
            ch = cli_getch(FALSE);

            // Take appropriate action
            switch (ch)
            {
                case 0x000:
                    // No character was available, so just wait
                    break;

                case 0x001:
                case 0x147:
                    // Ctrl-A or Home
                    cursor = 0;
                    break;

                case 0x002:
                case 0x14b:
                    // Ctrl-B or Left
                    if (0 < cursor) cursor--;
                    break;

                case 0x004:
                case 0x153:
                    // Ctrl-D or Delete
                    if (cursor < length)
                    {
                        memmove(line + cursor, line + cursor + 1, length - cursor);
                        end = length;
                        length--;
                    }
                    break;

                case 0x005:
                case 0x14f:
                    // Ctrl-E or End
                    cursor = length;
                    break;

                case 0x006:
                case 0x14d:
                    // Ctrl-F or Right
                    if (cursor < length) cursor++;
                    break;

                case 0x173:
                    // Ctrl-Left
                    while ((0 < cursor) && (!isspace(line[cursor - 1]))) cursor--;
                    while ((0 < cursor) && (isspace(line[cursor - 1]))) cursor--;
                    break;

                case 0x174:
                    // Ctrl-Right
                    while ((cursor < length) && (!isspace(line[cursor]))) cursor++;
                    while ((cursor < length) && (isspace(line[cursor]))) cursor++;
                    break;

                case 0x008:
                    // Ctrl-H or Backspace
                    if (0 < cursor)
                    {
                        cursor--;
                        memmove(line + cursor, line + cursor + 1, length - cursor);
                        start = cursor;
                        end = length;
                        length--;
                    }
                    break;

                case 0x00a:
                case 0x00d:
                    // Ctrl-J, Ctrl-M, Enter or Return
                    done = TRUE;
                    break;

                case 0x00b:
                    // Ctrl-K
                    if (cursor < length)
                    {
                        end = length;
                        length = cursor;
                        line[length] = 0;
                    }
                    break;

                case 0x00e:
                case 0x150:
                    // Ctrl-N or Down
                    if (!history) history = last;
                    if (history)
                    {
                        history = history->next;
                        start = 0;
                        end = length;
                        strncpy(line, history->line, size);
                        line[size - 1] = 0;
                        length = strlen(line);
                        cursor = length;
                        if (end < length) end = length;
                    }
                    break;

                case 0x010:
                case 0x148:
                    // Ctrl-P or Up
                    history = history ? history->prev : last;
                    if (history)
                    {
                        start = 0;
                        end = length;
                        strncpy(line, history->line, size);
                        line[size - 1] = 0;
                        length = strlen(line);
                        cursor = length;
                        if (end < length) end = length;
                    }
                    break;

                case 0x014:
                    // Ctrl-T
                    if ((0 < cursor) && (cursor < length))
                    {
                        char t = line[cursor];
                        line[cursor] = line[cursor - 1];
                        line[cursor - 1] = t;
                        start = cursor - 1;
                        end = cursor + 1;
                    }
                    break;

                case 0x015:
                case 0x01b:
                    // Ctrl-U, Ctrl-[ or Escape
                    if (0 < length)
                    {
                        start = 0;
                        end = length;
                        length = 0;
                        cursor = 0;
                        *line = 0;
                        history = NULL;
                    }
                    break;

                case 0x152:
                    // Insert
                    insert = !insert;
                    (*cli_output_filter_current)(out, insert
                                                      ? cursor_insert
                                                      : cursor_overwrite,
                                                      insert
                                                      ? sizeof(cursor_insert)
                                                      : sizeof(cursor_overwrite));
                    break;

                default:
                    // The only other characters of interest are printable
                    if ((' ' <= ch) && (ch <= 0xff)
                        && (length + (insert || (cursor == length) ? 1 : 0) < size))
                    {
                        if (insert || (cursor == length))
                        {
                            length++;
                            memmove(line + cursor + 1, line + cursor, length - cursor);
                            end = length;
                        }
                        else end = cursor + 1;
                        line[cursor] = ch;
                        cursor++;
                    }
            }

            // Update the display with any changes
            cli_getline_show(out, line, start, end, old_cursor, cursor);
        }

        // Add this line to the end of the history list
        if (length)
        {
            history = (history_record *) malloc(sizeof(history_record));
            if (history)
            {
                history->line = (char *) malloc(length + 1);
                if (history->line)
                {
                    history->prev = last ? last : history;
                    history->next = last ? last->next : history;
                    strcpy(history->line, line);
                    last = history;
                    history->next->prev = history;
                    history->prev->next = history;
                }
                else free(history);
            }
        }

        // Move to the next display line
        (*cli_output_filter_current)(stdout, "\n\r", 2);
    }
    else
    {
        // Perform a simple line read
        fgets(line, size, in);

        // Remove any newline character at the end
        if (strchr(line, '\n')) *strchr(line, '\n') = 0;

        // Echo the line to the selected output if input not a terminal
        if (!(in->flags & _F_TERM))
        {
            (*cli_output_filter_current)(out, line, strlen(line));
        }
    }

    // If this point reached then the operation was successful
    return EXIT_SUCCESS;
}

