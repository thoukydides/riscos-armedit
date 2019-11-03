/*
    File        : zip.c++
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

// Include header file for this module
#include "zip.h"

// Include cpplib header files
#include "string.h"
#include "fstream.h"

// Include clib header files
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Include oslib header files
#include "fileswitch.h"
#include "osfile.h"
#include "wimp.h"
#include "wimpspriteop.h"

// Include project header files
#include "utils.h"

// The parts of the command to execute
#define ZIP_COMMAND "Run <ARMEdit$Dir>.Configure.UnZip -o -qq "\
                    "<ARMEdit$Dir>.Install."
#define ZIP_DESTINATION " -d "
#define ZIP_REDIRECT "pipe:$.ARMEdit.UnZip"

/*
    Parameters  : dir           - The directory to create.
    Returns     : os_error *    - An error, or NULL if successful.
    Description : Attempt to create the specified directory. Any parent
                  directories are also created if required.
*/
static const os_error *zip_dir_create(const string &dir)
{
    const os_error *err = NULL;
    char *ptr;
    char str[256];

    // Copy the directory name so that it may be modified
    strcpy(str, dir.c_str());

    // Create parent directories first
    ptr = str;
    while (!err && (ptr = strchr(ptr, '.')))
    {
        *ptr = 0;
        err = xosfile_create_dir(str, 0);
        *ptr++ = '.';
    }

    // Finally create the required directory
    if (!err) err = xosfile_create_dir(dir.c_str(), 0);

    // Return the status
    return err;
}

/*
    Parameters  : dir           - The directory to check.
    Returns     : os_error *    - An error, or NULL if successful.
    Description : Ensure that the destination directory exists.
*/
static const os_error *zip_dir(const string &dir)
{
    os_error err;
    const os_error *err_ptr;
    fileswitch_object_type type;

    // Check whether the directory exists
    err_ptr = xosfile_read_no_path(dir.c_str(), &type, NULL, NULL, NULL, NULL);

    // Ensure that the destination is not a file
    if (!err_ptr && (type == fileswitch_IS_FILE))
    {
        // Create an error block
        err.errnum = 0;
        lookup_token(err.errmess, sizeof(err.errmess), "ZipDirFile",
                     dir.c_str());
        err_ptr = &err;
    }

    // Prompt the user if directory does not exist
    if (!err_ptr && (type == fileswitch_NOT_FOUND))
    {
        int create;

        // Use a suitable form of error window
        err.errnum = 0;
        if (wimp_VERSION_RO35 <= wimp_version)
        {
            // Display an error message
            lookup_token(err.errmess, sizeof(err.errmess), "ZipDirNew",
                         dir.c_str());
            create = wimp_report_error_by_category(&err,
                     wimp_ERROR_BOX_CATEGORY_QUESTION << 9, task_name,
                     AppSprite, wimpspriteop_AREA, lookup_token("ZipDirBut"))
                     == 3;
        }
        else
        {
            // Display an error message
            lookup_token(err.errmess, sizeof(err.errmess), "ZipDirOld",
                         dir.c_str());
            create = wimp_report_error(&err, wimp_ERROR_BOX_OK_ICON
                                       | wimp_ERROR_BOX_CANCEL_ICON,
                                       task_name)
                     == wimp_ERROR_BOX_SELECTED_OK;
        }

        // Attempt to create the directory if required
        if (create) err_ptr = zip_dir_create(dir);
        else
        {
            // Create an error block
            err.errnum = 0;
            lookup_token(err.errmess, sizeof(err.errmess), "ZipDirAbort");
            err_ptr = &err;
        }
    }

    // Return the status
    return err_ptr;
}

/*
    Parameters  : zip           - The leaf name of the zip file to decompress.
                  dest          - The full path of the destination directory.
    Returns     : os_error *    - An error, or NULL if successful.
    Description : Attempt to unzip all the contents of the specified archive.
*/
static const os_error *zip_extract(const string &zip, const string &dest)
{
    string str;
    os_error err;
    int code;

    // Build the command to execute
    str = ZIP_COMMAND + zip + ZIP_DESTINATION + dest
          + " { > " + ZIP_REDIRECT + " }";

    // Attempt to unzip the files
    code = system(str.c_str());

    // Open the pipe
    ifstream file(ZIP_REDIRECT);

    // Extract the error message if required
    if (code == -2)
    {
        // Unable to start the external utility
        err.errnum = 0;
        lookup_token(err.errmess, sizeof(err.errmess), "ZipRun");
    }
    else if (code != EXIT_SUCCESS)
    {
        // Recover any error message produced
        err.errnum = 0;
        file.get(err.errmess, sizeof(err.errmess), '\r');
    }

    // Return the status
    return code == EXIT_SUCCESS ? NULL : &err;
}

/*
    Parameters  : zip           - The leaf name of the zip file to decompress.
                  dest          - The full path of the destination directory.
    Returns     : os_error *    - An error, or NULL if successful.
    Description : Attempt to unzip all the contents of the specified archive.
*/
const os_error *zip_unzip(const string &zip, const string &dest)
{
    const os_error *err;

    // Check that the destination directory exists
    err = zip_dir(dest);

    // Extract the files
    if (!err) err = zip_extract(zip, dest);

    // Return the status
    return err;
}
