/*
    File        : instfile.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1997-2001, 2019
    Description : Installation options file handling for !ARMEdit.

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

#ifndef instfile_h
#define instfile_h

// Include cpplib header files
#include "cpp:string.h"

// Include oslib header files
#include "types.h"

// Have the installation options been modified
extern bool instfile_modified;

// The current installation options
extern string instfile_path_modify;
extern string instfile_path_drv_riscos;
extern string instfile_path_drv_dos;
extern string instfile_path_util_riscos;
extern string instfile_path_util_dos;
extern string instfile_path_dev_riscos;
extern string instfile_path_dev_dos;

/*
    Parameters  : void
    Returns     : void
    Description : Attempt to read the installation options file. This uses the
                  configfile_dir variable.
*/
void instfile_read(void);

/*
    Parameters  : void
    Returns     : void
    Description : Attempt to write the installation options file.
*/
void instfile_write(void);

#endif
