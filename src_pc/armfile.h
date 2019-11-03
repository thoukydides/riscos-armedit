/*
    File        : armfile.h
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

// Only include header file once
#ifndef ARMFILE_H
#define ARMFILE_H

// Include system header files
#include <stdlib.h>

// Variable to contain any error message
extern char armfile_error[256];

#ifdef __cplusplus
extern "C" {
#endif

// Should multitasking be enabled during transfers (default is FALSE)
extern int armfile_mtask;

/*
    Parameters  : src   - The source DOS file.
                  dest  - The destination RISC OS file.
    Returns     : int   - Standard C return code giving status of operation.
    Description : Copy a file from DOS to RISC OS. If there is an error then
                  it is stored in the variable armfile_error and a non-zero
                  value is returned, otherwise zero is returned.
*/
int armfile_copy_dos_riscos(const char *src, const char *dest);

/*
    Parameters  : src   - The source RISC OS file.
                  dest  - The destination DOS file.
    Returns     : int   - Standard C return code giving status of operation.
    Description : Copy a file from RISC OS to DOS. If there is an error then
                  it is stored in the variable armfile_error and a non-zero
                  value is returned, otherwise zero is returned.
*/
int armfile_copy_riscos_dos(const char *src, const char *dest);

/*
    Parameters  : src   - The source DOS filename.
                  dest  - The resulting RISC OS filename.
    Returns     : void
    Description : Perform character translation from DOS to RISC OS
                  filenames.
*/
void armfile_translate_dos_riscos(const char *src, char *dest);

/*
    Parameters  : src   - The source RISC OS filename.
                  dest  - The resulting DOS filename.
    Returns     : void
    Description : Perform character translation from RISC OS to DOS
                  filenames.
*/
void armfile_translate_riscos_dos(const char *src, char *dest);

/*
    Parameters  : buf   - Buffer to receive the filename.
                  len   - Size of the buffer.
    Returns     : int   - Standard C return code giving status of operation.
    Description : Generate a filename for a temporary RISC OS file. The file
                  is deleted before exiting.
*/
int armfile_temporary(char *buf, size_t len);

/*
    Parameters  : dir   - The name of the RISC OS directory to create.
    Returns     : int   - Standard C return code giving status of operation.
    Description : Create a RISC OS directory.
*/
int armfile_create_dir(const char *dir);

#ifdef __cplusplus
}
#endif

#endif
