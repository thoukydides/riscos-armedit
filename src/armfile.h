/*
    File        : armfile.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1997-2001, 2019
    Description : Filename tranlation between DOS and RISC OS formats.

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

#ifndef armfile_h
#define armfile_h

// Include cpplib header files
#include "cpp:string.h"

/*
    Parameters  : file      - The source DOS filename.
    Returns     : string    - The resulting RISC OS filename.
    Description : Perform character translation from DOS to RISC OS
                  filenames.
*/
string armfile_translate_dos_riscos(const string &file);

/*
    Parameters  : file      - The source RISC OS filename.
    Returns     : string    - The resulting DOS filename.
    Description : Perform character translation from RISC OS to DOS
                  filenames.
*/
string armfile_translate_riscos_dos(const string &file);

#endif
