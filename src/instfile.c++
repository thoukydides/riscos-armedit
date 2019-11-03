/*
    File        : instfile.c++
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

// Include header file for this module
#include "instfile.h"

// Include cpplib header files
#include "fstream.h"

// Include clib header files
#include <stdio.h>
#include <string.h>

// Include oslib header files
#include "osfile.h"

// Include project header files
#include "configfile.h"
#include "utils.h"

// The installation options file
#define INSTFILE_FILE ".InstallOpt"
static string instfile_file;

// Have the installation options been modified
bool instfile_modified = FALSE;

// The current installation options
string instfile_path_modify;
string instfile_path_drv_riscos;
string instfile_path_drv_dos;
string instfile_path_util_riscos;
string instfile_path_util_dos;
string instfile_path_dev_riscos;
string instfile_path_dev_dos;

// String handling macros
#define INSTFILE_WRITE(s) ((s).empty() ? "#" : (s).c_str())
#define INSTFILE_READ(s) ((s) == "#" ? string("") : (s))

/*
    Parameters  : void
    Returns     : void
    Description : Attempt to read the installation options file. This uses the
                  configfile_dir variable.
*/
void instfile_read(void)
{
    string path_modify, path_drv_riscos, path_drv_dos;
    string path_util_riscos, path_util_dos;
    string path_dev_riscos, path_dev_dos;

    // Clear the modified flag
    instfile_modified = FALSE;

    // Construct the installation options file name
    instfile_file = configfile_dir + INSTFILE_FILE;

    // Open configuration file
    ifstream file(instfile_file.c_str());
    if (!file) return;

    // Read the first section of settings from the options file
    if (file >> path_modify >> path_drv_riscos >> path_drv_dos
             >> path_util_riscos >> path_util_dos
             >> path_dev_riscos >> path_dev_dos)
    {
        instfile_path_modify = INSTFILE_READ(path_modify);
        instfile_path_drv_riscos = INSTFILE_READ(path_drv_riscos);
        instfile_path_drv_dos = INSTFILE_READ(path_drv_dos);
        instfile_path_util_riscos = INSTFILE_READ(path_util_riscos);
        instfile_path_util_dos = INSTFILE_READ(path_util_dos);
        instfile_path_dev_riscos = INSTFILE_READ(path_dev_riscos);
        instfile_path_dev_dos = INSTFILE_READ(path_dev_dos);
    }
    else return;
}

/*
    Parameters  : void
    Returns     : void
    Description : Attempt to write the installation options file.
*/
void instfile_write(void)
{
    // Open the installation options file
    ofstream file(instfile_file.c_str());

    // Write the installation options to the file
    file << INSTFILE_WRITE(instfile_path_modify) << ' '
         << INSTFILE_WRITE(instfile_path_drv_riscos) << ' '
         << INSTFILE_WRITE(instfile_path_drv_dos) << ' '
         << INSTFILE_WRITE(instfile_path_util_riscos) << ' '
         << INSTFILE_WRITE(instfile_path_util_dos) << ' '
         << INSTFILE_WRITE(instfile_path_dev_riscos) << ' '
         << INSTFILE_WRITE(instfile_path_dev_dos)
         << '\n';

    // Report an error if required
    if (!file || file.bad())
    {
        os_error er;

        lookup_token(er.errmess, sizeof(er.errmess), "InstSave");
        report_error(&er);
        return;
    }

    // Clear the modified flag if successful
    instfile_modified = FALSE;
}
