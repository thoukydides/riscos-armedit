/*
    File        : util.h
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

// Only include header file once
#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

// A function called with matching files
typedef void util_func(const char *path);

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
void util_arm_split(const char *path, char *dir, char *leaf);

/*
    Parameters  : path      - A wildcarded pathname.
                  attrib    - Required DOS file-attribute byte.
                  func      - The function to call for each match.
    Returns     : int       - The number of matches.
    Description : Call a specified function for all the files that match.
*/
int util_wildcard(const char *path, int attrib, util_func *func);

/*
    Parameters  : path      - A wildcarded pathname.
                  attrib    - The required DOS file-attribute byte. Only the
                              directory bit is used.
                  func      - The function to call for each match.
    Returns     : int       - The number of matches.
    Description : Call a specified function for all the RISC OS files that
                  match.
*/
int util_arm_wildcard(const char *path, int attrib, util_func *func);

/*
    Parameters  : active    - Should an activity indicator be displayed.
                  format    - The required format string.
                  ...       - Other arguments required for specified format.
    Returns     : void
    Description : Display a specified message.
*/
void util_status(int active, const char *format, ...);

/*
    Parameters  : format    - The required format string.
                  ...       - Other arguments required for specified format.
    Returns     : void
    Description : Exit with an error.
*/
void util_error(const char *format, ...);

/*
    Parameters  : err       - The error to be produced if abort selected.
                  msg       - The message to be displayed.
                  cancel    - Should the user be allowed to cancel.
    Returns     : int       - Should a retry be performed.
    Description : Prompt the user to either Abort, Retry, or optionally
                  Cancel.
*/
int util_retry(const char *err, const char *msg, int cancel);

/*
    Parameters  : msg   - The message to prompt with.
    Returns     : int   - Was "y" entered.
    Description : Prompt for a Y/N answer and return the result.
*/
int util_prompt(const char *msg);

#ifdef __cplusplus
}
#endif

#endif
