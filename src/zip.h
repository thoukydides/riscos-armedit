/*
    File        : zip.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1997-2001, 2019
    Description : Interface to the Info-Zip UnZip utility. This provides a
                  layer of abstraction as well as providing several related
                  functions.

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

#ifndef zip_h
#define zip_h

// Include cpplib header files
#include "cpp:string.h"

// Include oslib header files
#include "os.h"

/*
    Parameters  : zip           - The leaf name of the zip file to decompress.
                  dest          - The full path of the destination directory.
    Returns     : os_error *    - An error, or NULL if successful.
    Description : Attempt to unzip all the contents of the specified archive.
*/
const os_error *zip_unzip(const string &zip, const string &dest);

#endif
