/*
    File        : cli.h
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

// Only include header file once
#ifndef CLI_H
#define CLI_H

// Include system header files
#include <stdio.h>

// Variable to contain any error message
extern char cli_error[256];

#ifdef __cplusplus
extern "C" {
#endif

/*
    Parameters  : stream    - The stream to redirect output to. This
                              defaults to stdout.
                  bytes     - Pointer to the bytes of data to output.
                  size      - The number of bytes to output.
    Returns     : void
    Description : An output stream prefilter. The function may write
                  directly to the screen rather than using the specified
                  stream.
*/
typedef void cli_output_filter(FILE *stream, const char *bytes, size_t size);

/* The current output filter */
extern cli_output_filter *cli_output_filter_current;

/* An output filter that does not change the data */
cli_output_filter cli_output_filter_none;

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
int cli_getch(int riscos);

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
int cli_simple(const char *cmd, FILE *stream);

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
int cli_redir(const char *cmd, FILE *in, FILE *out);

/*
    Parameters  : var   - The name of the variable to read, which may be
                          wildcarded (using '*' and '#').
                  data  - Pointer to a buffer to receive the variable value.
                  size  - Size of the buffer.
    Returns     : int   - Standard C return code giving status of operation.
    Description : Read the value of the first matching variable. The value is
                  always converted to a string.
*/
int cli_read_var(const char *var, char *data, size_t size);

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
int cli_getline(FILE *in, FILE *out, char *line, size_t size);

#ifdef __cplusplus
}
#endif

#endif
