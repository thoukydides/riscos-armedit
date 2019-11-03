/*
    File        : vdu.c
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1997-2001, 2019
    Description : Process VDU control codes.

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
#include "vdu.h"

// Include system header files
#include "conio.h"
#include "dos.h"
#include "hpc.h"

// Include project header files
#include "swi.h"
#include "talk.h"

// Number of parameters required for each control code
static const int vdu_params[32] =
{
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // VDU 0 - 15
    0, 1, 2, 5, 0, 0, 1, 9, 8, 5, 0, 0, 4, 4, 0, 2  // VDU 16 - 31
};

// Colour mappings for different colour depths
static const int vdu_colour_1bpp[] = {BLACK, WHITE};
static const int vdu_colour_2bpp[] = {BLACK, RED, YELLOW, WHITE};
static const int vdu_colour_4bpp[] =
    {BLACK, RED, LIGHTGREEN, YELLOW, BLUE, LIGHTMAGENTA, LIGHTCYAN, WHITE};

// Queue of control characters waiting to be processed
static int vdu_queue_size = 0;
static char vdu_queue[12];

// Is screen output enabled
static int vdu_enable = TRUE;

// Cursor movement flags
struct vdu_movement_bits
{
    unsigned int scroll_protect : 1;
    unsigned int positive_left : 1;
    unsigned int positive_up : 1;
    unsigned int swap_axes : 1;
    unsigned int vertical_wrap : 1;
    unsigned int no_move : 1;
    unsigned int no_vdu5_wrap : 1;
};
static union
{
    unsigned int byte;
    struct vdu_movement_bits bits;
} vdu_movement = {0x41};

// The number of bits per pixel
static int vdu_bpp = 4;

// The current foreground and background colours
static int vdu_colour_fore;
static int vdu_colour_back;

// The actual screen size
static int vdu_screen_width;
static int vdu_screen_height;

// The current cursor position
static int vdu_cursor_x;
static int vdu_cursor_y;
static int vdu_pending_nl;

// The current window size and position
static int vdu_win_left;
static int vdu_win_top;
static int vdu_win_width;
static int vdu_win_height;

// Address of ARM memory allocated
#define VDU_ARM_MEM_SIZE (256)
static long vdu_arm_mem = 0;

/*
    Parameters  : void
    Returns     : void
    Description : Tidy up before the program exits.
*/
static void vdu_tidy(void)
{
    // Free the memory
    if (vdu_arm_mem)
    {
        talk_free(vdu_arm_mem);
        vdu_arm_mem = 0;
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Set the current text colours.
*/
static void vdu_colours(void)
{
    textcolor(vdu_colour_fore);
    textbackground(vdu_colour_back);
}

/*
    Parameters  : void
    Returns     : void
    Description : Place the real cursor at the required position within the
                  current window.
*/
static void vdu_cursor(void)
{
    // Place the cursor within the current window
    gotoxy(vdu_win_left + vdu_cursor_x + 1,
           vdu_win_top + vdu_cursor_y + 1);
}

/*
    Parameters  : screen    - Should the window be set to the whole screen.
    Returns     : void
    Description : Set the actual output window to either the text window or
                  the full screen.
*/
static void vdu_window(int screen)
{
    // Set the required window size
    if (screen)
    {
        // Select the whole screen
        window(1, 1, vdu_screen_width, vdu_screen_height);
    }
    else
    {
        // Select the current text window
        window(vdu_win_left + 1, vdu_win_top + 1,
               vdu_win_left + vdu_win_width,
               vdu_win_top + vdu_win_height);
    }
}

/*
    Parameters  : x         - The x component of the vector.
                  y         - The y component of the vector.
                  h         - Variable to receive the corrected horizontal
                              component of the vector.
                  v         - Variable to receive the corrected vertical
                              component of the vector.
                  screen    - Should the vector be converted to a screen
                              vector.
    Returns     : void
    Description : Correct the specified vector for the current axis directions.
                  The destination variables may be the same as the source.
*/
static void vdu_vector(int x, int y, int *h, int *v, int screen)
{
    // Swap the axes if required
    if (vdu_movement.bits.swap_axes)
    {
        *v = x;
        *h = y;
    }
    else
    {
        *h = x;
        *v = y;
    }

    // Reverse the axes if required
    if (screen)
    {
        if (vdu_movement.bits.positive_left) *h = -*h;
        if (vdu_movement.bits.positive_up) *v = -*v;
    }
    else
    {
        if (vdu_movement.bits.positive_up) *h = -*h;
        if (vdu_movement.bits.positive_left) *v = -*v;
    }
}

/*
    Parameters  : x - The x component of the tab position.
                  y - The y component of the tab position.
                  h - Variable to receive the horizontal component of the
                      cursor position.
                  v - Variable to receive the vertical component of the
                      cursor position.
    Returns     : void
    Description : Convert a tab position to a cursor position.
*/
static void vdu_tab_to_cursor(int x, int y, int *h, int *v)
{
    // Convert to a screen vector
    vdu_vector(x, y, h, v, TRUE);

    // Add offsets if required
    if (vdu_movement.bits.positive_left) *h += vdu_win_width - 1;
    if (vdu_movement.bits.positive_up) *v += vdu_win_height - 1;
}

/*
    Parameters  : h - The horizontal component of the cursor position.
                  v - The vertical component of the cursor position.
                  x - Variable to receive the x component of the cursor
                      position.
                  y - Variable to receive the y component of the cursor
                      position.
    Returns     : void
    Description : Convert a cursor position to a tab position.
*/
static void vdu_cursor_to_tab(int h, int v, int *x, int *y)
{
    // Remove offsets if required
    if (vdu_movement.bits.positive_left) h -= vdu_win_width - 1;
    if (vdu_movement.bits.positive_up) v -= vdu_win_height - 1;

    // Convert to a tab vector
    vdu_vector(h, v, x, y, FALSE);
}

/*
    Parameters  : x         - The required movement in the x direction.
                  y         - The required movement in the y direction.
                  direct    - Do not transform the movement vector before use.
                  screen    - Should the whole screen be scrolled instead of
                              just the current window.
    Returns     : void
    Description : Scroll either the current window or the whole screen.
*/
static void vdu_scroll(int x, int y, int direct, int screen)
{
    int left;
    int top;
    int right;
    int bottom;

    // Convert the axes if required
    if (!direct) vdu_vector(x, y, &x, &y, TRUE);

    // Choose the area to adjust
    if (screen)
    {
        // The whole screen should be scrolled
        left = 1;
        top = 1;
        right = vdu_screen_width;
        bottom = vdu_screen_height;
    }
    else
    {
        // The current text window should be scrolled
        left = vdu_win_left + 1;
        top = vdu_win_top + 1;
        right = vdu_win_left + vdu_win_width;
        bottom = vdu_win_top + vdu_win_height;
    }

    // Special case if scroll is larger than window
    if (((right - left) < abs(x)) || ((bottom - top) <= abs(y)))
    {
        // Just scroll everything off horizontally
        x = right - left + 1;
        y = 0;
    }
    else
    {
        // Perform the scrolling
        movetext(left + (x < 0 ? -x : 0), top + (y < 0 ? -y : 0),
                 right + (0 < x ? -x : 0), bottom + (0 < y ? -y : 0),
                 left + (0 < x ? x : 0), top + (0 < y ? y : 0));
    }

    // Clear the newly revealed areas
    if (x < 0)
    {
        // Windows scrolled to the left
        window(right + x + 1, top, right, bottom);
        clrscr();
    }
    if (0 < x)
    {
        // Window scrolled to the right
        window(left, top, left + x - 1, bottom);
        clrscr();
    }
    if (y < 0)
    {
        // Window scrolled up
        window(left, bottom + y + 1, right, bottom);
        clrscr();
    }
    if (0 < y)
    {
        // Windows scrolled down
        window(left, top, right, top + y - 1);
        clrscr();
    }

    // Clear the output window
    vdu_window(TRUE);
}

/*
    Parameters  : x     - The required movement in the x direction.
                  y     - The required movement in the y direction.
                  force - Force the move even if movement of the cursor
                          after printing is disabled.
                  prot  - Should any scroll protection be executed.
    Returns     : void
    Description : Move the cursor position, taking into account the current
                  settings.
*/
static void vdu_cursor_move(int x, int y, int force, int prot)
{
    int width;
    int height;
    int column;
    int row;

    // Check if any movement should be produced
    if (!force && vdu_movement.bits.no_move) x = y = 0;

    // Update the required movement if a new line is pending */
    if (vdu_pending_nl && (!x || !y || prot))
    {
        // Advance the cursor by one position
        x++;

        // Clear the pending flag
        vdu_pending_nl = FALSE;
    }

    // Convert the cursor position to a tab vector
    vdu_cursor_to_tab(vdu_cursor_x, vdu_cursor_y, &column, &row);

    // Calculate the correctly rotated window dimensions
    if (vdu_movement.bits.swap_axes)
    {
        width = vdu_win_height;
        height = vdu_win_width;
    }
    else
    {
        width = vdu_win_width;
        height = vdu_win_height;
    }

    // Move the cursor
    column += x;
    row += y;

    // Wrap the cursor or scroll the window as required
    while (column < 0)
    {
        // Wrap to the end of the previous line
        column += width;
        row--;
    }
    while (width <= column)
    {
        // Apply scroll protection if required
        if (vdu_movement.bits.scroll_protect && !prot && (width == column))
        {
            // Convert to a pending new line
            column--;
            vdu_pending_nl = TRUE;
        }
        else
        {
            // Wrap to the start of the next line
            column -= width;
            row++;
        }
    }
    while (row < 0)
    {
        // Should the cursor wrap or scroll
        if (vdu_movement.bits.vertical_wrap)
        {
            // Wrap to the bottom of the window
            row += height;
        }
        else
        {
            // Scroll the window down
            vdu_scroll(0, 1, FALSE, FALSE);
            row++;
        }
    }
    while (height <= row)
    {
        // Should the cursor wrap or scroll
        if (vdu_movement.bits.vertical_wrap)
        {
            // Wrap to the top of the window
            row -= height;
        }
        else
        {
            // Scroll the window up
            vdu_scroll(0, -1, FALSE, FALSE);
            row--;
        }
    }

    // Convert back to a cursor position
    vdu_tab_to_cursor(column, row, &vdu_cursor_x, &vdu_cursor_y);

    // Update the cursor on screen
    vdu_cursor();
}

/*
    Parameters  : x - The required movement in the x direction.
                  y - The required movement in the y direction.
    Returns     : void
    Description : Move the cursor position to the edge of the current window
                  in the direction specified, taking into account the current
                  settings.
*/
static void vdu_cursor_edge(int x, int y)
{
    // Convert the axes
    vdu_vector(x, y, &x, &y, TRUE);

    // Move the cursor to the edge of the window
    if (x < 0) vdu_cursor_x = 0;
    if (0 < x) vdu_cursor_x = vdu_win_width - 1;
    if (y < 0) vdu_cursor_y = 0;
    if (0 < y) vdu_cursor_y = vdu_win_height - 1;
    vdu_pending_nl = FALSE;

    // Update the cursor on screen
    vdu_cursor();
}

/*
    Parameters  : void
    Returns     : void
    Description : The DOS screen mode has changed, so update the data
                  structures as required.
*/
static void vdu_mode_changed(void)
{
    struct text_info ti;

    // Read the current screen details
    gettextinfo(&ti);

    // Store the screen size
    vdu_screen_width = ti.screenwidth;
    vdu_screen_height = ti.screenheight;

    // Store the window details
    vdu_win_left = ti.winleft - 1;
    vdu_win_top = ti.wintop - 1;
    vdu_win_width = ti.winright - ti.winleft + 1;
    vdu_win_height = ti.winbottom - ti.wintop + 1;

    // Store the cursor position
    vdu_cursor_x = ti.curx - 1;
    vdu_cursor_y = ti.cury - 1;
    vdu_pending_nl = FALSE;

    // Store the current colours
    vdu_colour_fore = ti.attribute & 0x0f;
    vdu_colour_back = (ti.attribute >> 3) & 0x0f;

    // Ensure that there is no output window
    vdu_window(TRUE);

    // Display the cursor at the correct location
    vdu_cursor();
}

/*
    Parameters  : mode  - The number of the mode, or -1 for the current one.
                  var   - The variable number.
    Returns     : long  - The value of the requested variable.
    Description : Read a RISC OS mode variable.
*/
long vdu_mode_var(long mode, int var)
{
    const talk_error *err;
    talk_swi_regs regs;

    // Attempt to read the variable value
    regs.r[0] = mode;
    regs.r[1] = var;
    err = talk_swi(OS_ReadModeVariable, &regs, &regs);
    if (err) regs.r[2] = 0;

    /* Return the value of the variable */
    return regs.r[2];
}

/*
    Parameters  : None
    Returns     : None
    Description : Initialise all features of the module.
*/
static void vdu_initialise()
{
    static int done = FALSE;

    // Initialise if not already done
    if (!done)
    {
        // Set flag to prevent multiple initialisations
        done = TRUE;

        // Register a handler to tidy up before exiting
        atexit(vdu_tidy);

        // Attempt to claim some ARM memory
        if (talk_malloc(VDU_ARM_MEM_SIZE, &vdu_arm_mem))
        {
            vdu_arm_mem = 0;
        }

        // Read the current screen details
        vdu_mode_changed();
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Process VDU23 command codes stored in the queue.
*/
static void vdu_vdu23(void)
{
    const int x[] = {1, -1, 0, 0};
    const int y[] = {0, 0, 1, -1};

    // Decode the command number
    switch (vdu_queue[1])
    {
        case 0:
            // Set the interlace and control cursor appearance
            if (vdu_queue[2] == 10)
            {
                _setcursortype((vdu_queue[3] & 0x60) == 0x20
                               ? _NOCURSOR
                               : ((vdu_queue[3] & 0x60) == 0x00
                                  ? _NORMALCURSOR : _SOLIDCURSOR));
            }
            break;

        case 1:
            // Control the appearance of the text cursor
            _setcursortype(vdu_queue[2] == 0
                           ? _NOCURSOR
                           : (vdu_queue[2] == 2
                              ? _SOLIDCURSOR : _NORMALCURSOR));
            break;

        case 7:
            // Scroll text window or screen
            vdu_scroll(x[vdu_queue[3] & 0x03], y[vdu_queue[3] & 0x03],
                       vdu_queue[3] & 0x04 ? FALSE : TRUE,
                       vdu_queue[2] ? TRUE : FALSE);
            break;

        case 16:
            // Control the movement of cursor after printing
            vdu_movement.byte = (vdu_movement.byte & vdu_queue[3])
                                ^ vdu_queue[2];
            break;

        case 17:
            // Exchange text foreground and background colours
            if (vdu_queue[2] == 5)
            {
                int t = vdu_colour_fore;
                vdu_colour_fore = vdu_colour_back;
                vdu_colour_back = t;
                vdu_colours();
            }

        default:
            // Not interested in other commands
            break;
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Process the control code that is stored in the queue.
*/
static void vdu_control(void)
{
    // Decode the control code
    switch (vdu_queue[0])
    {
        case 6:
            // Enable screen output
            vdu_enable = TRUE;
            break;

        case 7:
            // Bell
            putchar(7);
            break;

        case 8:
            // Backspace
            vdu_cursor_move(-1, 0, TRUE, FALSE);
            break;

        case 9:
            // Horizontal tab
            vdu_cursor_move(1, 0, TRUE, FALSE);
            break;

        case 10:
            // Linefeed
            vdu_cursor_move(0, 1, TRUE, FALSE);
            break;

        case 11:
            // Vertical tab
            vdu_cursor_move(0, -1, TRUE, FALSE);
            break;

        case 12:
            // Form feed/clear screen
            vdu_window(FALSE);
            clrscr();
            vdu_window(TRUE);
            vdu_tab_to_cursor(0, 0, &vdu_cursor_x, &vdu_cursor_y);
            vdu_pending_nl = FALSE;
            break;

        case 13:
            // Carriage return
            vdu_cursor_edge(-1, 0);
            break;

        case 17:
            // Set text colour
            {
                int colour;

                // Choose the closest possible colour
                if (vdu_bpp == 1) colour = vdu_colour_1bpp[vdu_queue[1] & 0x01];
                else if (vdu_bpp == 2) colour = vdu_colour_2bpp[vdu_queue[1] & 0x03];
                else colour = vdu_colour_4bpp[vdu_queue[1] & 0x07];

                // Set either the foreground or background colour
                if (vdu_queue[1] & 0x80) vdu_colour_back = colour & 0x07;
                else vdu_colour_fore = colour;
                vdu_colours();
            }
            break;

        case 20:
            // Restore default colours
            vdu_colour_fore = WHITE;
            vdu_colour_back = BLACK;
            vdu_colours();
            break;

        case 21:
             // Disable screen display
             vdu_enable = FALSE;
             break;

        case 22:
            // Change display mode
            textmode(40 <= vdu_mode_var(vdu_queue[1], 1) ? C80 : C40);
            if (25 <= vdu_mode_var(vdu_queue[1], 2)) textmode(C4350);
            vdu_bpp = 1 << vdu_mode_var(vdu_queue[1], 9);
            vdu_mode_changed();
            vdu_tab_to_cursor(0, 0, &vdu_cursor_x, &vdu_cursor_y);
            vdu_pending_nl = FALSE;
            vdu_colour_fore = WHITE;
            vdu_colour_back = BLACK;
            vdu_colours();
            clrscr();
            break;

        case 23:
            // Miscellaneous commands
            vdu_vdu23();
            break;

        case 26:
            // Restore default windows
            vdu_win_left = 0;
            vdu_win_top = 0;
            vdu_win_width = vdu_screen_width;
            vdu_win_height = vdu_screen_height;
            vdu_tab_to_cursor(0, 0, &vdu_cursor_x, &vdu_cursor_y);
            vdu_pending_nl = FALSE;
            break;

        case 28:
            // Define text window
            if ((vdu_queue[1] <= vdu_queue[3])
                && (vdu_queue[4] <= vdu_queue[2])
                && (0 <= vdu_queue[1]) && (vdu_queue[3] < vdu_screen_width)
                && (0 <= vdu_queue[4]) && (vdu_queue[2] < vdu_screen_height))
            {
                vdu_win_left = vdu_queue[1];
                vdu_win_top = vdu_queue[4];
                vdu_win_width = vdu_queue[3] - vdu_queue[1] + 1;
                vdu_win_height = vdu_queue[2] - vdu_queue[4] + 1;
                vdu_tab_to_cursor(0, 0, &vdu_cursor_x, &vdu_cursor_y);
                vdu_pending_nl = FALSE;
            }
            break;

        case 30:
            // Home text cursor
            vdu_tab_to_cursor(0, 0, &vdu_cursor_x, &vdu_cursor_y);
            vdu_pending_nl = FALSE;
            break;

        case 31:
            // Position text cursor
            if ((0 <= vdu_queue[1]) && (0 <= vdu_queue[2]))
            {
                int width = vdu_movement.bits.swap_axes
                            ? vdu_win_height : vdu_win_width;
                int height = vdu_movement.bits.swap_axes
                             ? vdu_win_width : vdu_win_height;
                if ((vdu_queue[1] < width
                     + (vdu_movement.bits.scroll_protect ? 1 : 0))
                    && (vdu_queue[2] < height))
                {
                    vdu_pending_nl = width <= vdu_queue[1];
                    vdu_tab_to_cursor(vdu_queue[1] + (vdu_pending_nl ? -1 : 0),
                                      vdu_queue[2],
                                      &vdu_cursor_x, &vdu_cursor_y);
                }
            }
            break;

        default:
            // All other codes are ignored
            break;
    }
}

/*
    Parameters  : stream    - The stream to redirect output to.
                  bytes     - Pointer to the bytes of data to output.
                  size      - The number of bytes to output.
    Returns     : void
    Description : An output stream prefilter. This emulates RISC OS treatment
                  of control codes when output is to the screen.
*/
void vdu_output_filter(FILE *stream, const char *bytes, size_t size)
{
    // Special action is taken if output is to the screen
    if (stream->flags & _F_TERM)
    {
        int old_wscroll;

        // Ensure that the module is initialised
        vdu_initialise();

        // Disable automatic window scrolling
        old_wscroll = _wscroll;
        _wscroll = FALSE;

        // Process all the characters
        while (size)
        {
            // Process this character
            if (vdu_queue_size || ((0 <= *bytes) && (*bytes < 32)))
            {
                // Add control characters to the queue
                vdu_queue[vdu_queue_size++] = *bytes;

                // Process the queue if it is long enough
                if (vdu_params[*vdu_queue] < vdu_queue_size)
                {
                   // Check if VDU output is enabled
                   if (vdu_enable || (*vdu_queue == 6)) vdu_control();

                   // Clear the VDU queue
                   vdu_queue_size = 0;
                }
            }
            else if (*bytes == 127)
            {
                // Delete character
                if (vdu_enable)
                {
                    // Backspace to the previous character
                    vdu_cursor_move(-1, 0, FALSE, FALSE);

                    // Blank out the character
                    putch(' ');
                }
            }
            else if (vdu_enable)
            {
                // Handle any pending new line and position the cursor
                vdu_cursor_move(0, 0, FALSE, TRUE);

                // Write this character at the required location
                putch(*bytes);

                // Advance the cursor if required
                vdu_cursor_move(1, 0, FALSE, FALSE);
            }

            // Advance to the next character
            size--;
            bytes++;
        }

        // Place the cursor at the required position
        vdu_cursor();

        // Restore previous window scrolling state
        _wscroll = old_wscroll;
    }
    else
    {
        // Simply copy the data to the specified stream
        if (size) fwrite(bytes, sizeof(char), size, stream);
    }
}
