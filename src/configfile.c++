/*
    File        : configfile.c++
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Configuration file handling for !ARMEdit.

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
#include "configfile.h"

// Include cpplib header files
#include "string.h"
#include "fstream.h"

// Include clib header files
#include <stdio.h>
#include <string.h>

// Include oslib header files
#include "osfile.h"

// Include project header files
#include "utils.h"

// The configuration files directory
#define CONFIGFILE_DIR1 "<Choices$Write>.ARMEdit"
#define CONFIGFILE_DIR2 "<ARMEdit$Dir>"
#define CONFIGFILE_FILE ".Choices"
string configfile_dir;
static string configfile_file;

// Has the configuration been modified
bool configfile_modified = FALSE;

// The current configuration
#define CONFIGFILE_PATH_PC "<Diva$Dir>"
#define CONFIGFILE_PATH_CONFIG "<Diva$Dir>.^.!PCconfig"
#define CONFIGFILE_PATH_PARTITION "ADFS::4.$.Drive_C"
#define CONFIGFILE_AUTO_BOOT ""
#define CONFIGFILE_AUTO_QUIT "<ARMEdit$Dir>.PCQuit"
#define CONFIGFILE_AUTO_LOAD ""
#define CONFIGFILE_AUTO_START "<ARMEdit$Dir>.PCStart"
#define CONFIGFILE_AUTO_EXIT ""
bool configfile_frontend_auto_quit = FALSE;
int configfile_speed_fore = 0;
int configfile_speed_back = 0;
string configfile_path_pc;
string configfile_path_config;
string configfile_path_partition;
string configfile_auto_boot;
string configfile_auto_quit;
string configfile_auto_load;
string configfile_auto_start;
string configfile_auto_exit;

// Was a valid configuration file present
bool configfile_present = FALSE;

// String handling macros
#define CONFIGFILE_WRITE(s) ((s).empty() ? "#" : (s).c_str())
#define CONFIGFILE_READ(s) ((s) == "#" ? string("") : (s))

/*
    Parameters  : void
    Returns     : void
    Description : Attempt to read the configuration file. This also sets the
                  configfile_dir variable.
*/
void configfile_read(void)
{
    int dummy, frontend_auto_quit, speed_fore_val, speed_back_val;
    string path_pc, path_config, path_partition;
    string auto_boot, auto_quit;
    string auto_load, auto_start, auto_exit;

    // Clear the modified flag
    configfile_modified = FALSE;

    // Start by assuming an invalid configuration file
    configfile_present = FALSE;

    // Ensure that the directory exists
    if (xosfile_create_dir(CONFIGFILE_DIR1, 0))
    {
        configfile_dir = CONFIGFILE_DIR2;
    }
    else configfile_dir = CONFIGFILE_DIR1;

    // Construct configuration file name
    configfile_file = configfile_dir + CONFIGFILE_FILE;

    // Set default strings
    configfile_path_pc = CONFIGFILE_PATH_PC;
    configfile_path_config = CONFIGFILE_PATH_CONFIG;
    configfile_path_partition = CONFIGFILE_PATH_PARTITION;
    configfile_auto_boot = CONFIGFILE_AUTO_BOOT;
    configfile_auto_quit = CONFIGFILE_AUTO_QUIT;
    configfile_auto_load = CONFIGFILE_AUTO_LOAD;
    configfile_auto_start = CONFIGFILE_AUTO_START;
    configfile_auto_exit = CONFIGFILE_AUTO_EXIT;

    // Open configuration file
    ifstream file(configfile_file.c_str());
    if (!file) return;

    // Read the first section of settings from the configuration file
    if (file >> frontend_auto_quit
             >> dummy >> speed_fore_val
             >> dummy >> speed_back_val)
    {
        // Set the values for the icons
        configfile_frontend_auto_quit = frontend_auto_quit;
        configfile_speed_fore = speed_fore_val;
        configfile_speed_back = speed_back_val;
    }
    else return;

    // Read the second sections of settings from the configuration file
    if (file >> path_pc >> path_config >> path_partition
             >> dummy)
    {
        configfile_path_pc = CONFIGFILE_READ(path_pc);
        configfile_path_config = CONFIGFILE_READ(path_config);
        configfile_path_partition = CONFIGFILE_READ(path_partition);
    }
    else return;

    // Sufficient details have been read by this point for a valid file
    configfile_present = TRUE;

    // Read the third section of settings from the configuration file
    if (file >> auto_boot >> auto_quit)
    {
        configfile_auto_boot = CONFIGFILE_READ(auto_boot);
        configfile_auto_quit = CONFIGFILE_READ(auto_quit);
    }
    else return;

    // Read the fourth section of settings from the configuration file
    if (file >> auto_load >> auto_start >> auto_exit)
    {
        configfile_auto_load = CONFIGFILE_READ(auto_load);
        configfile_auto_start = CONFIGFILE_READ(auto_start);
        configfile_auto_exit = CONFIGFILE_READ(auto_exit);
    }
    else return;
}

/*
    Parameters  : void
    Returns     : void
    Description : Attempt to write the configuration file.
*/
void configfile_write(void)
{
    // Open the configuration file
    ofstream file(configfile_file.c_str());

    // Write the configuration to the file
    file << configfile_frontend_auto_quit
         << '\n'
         << 0 << ' ' << configfile_speed_fore
         << ' '
         << 0 << ' ' << configfile_speed_back
         << '\n'
         << CONFIGFILE_WRITE(configfile_path_pc) << ' '
         << CONFIGFILE_WRITE(configfile_path_config) << ' '
         << CONFIGFILE_WRITE(configfile_path_partition)
         << '\n'
         << 0
         << '\n'
         << CONFIGFILE_WRITE(configfile_auto_boot) << ' '
         << CONFIGFILE_WRITE(configfile_auto_quit)
         << '\n'
         << CONFIGFILE_WRITE(configfile_auto_load) << ' '
         << CONFIGFILE_WRITE(configfile_auto_start) << ' '
         << CONFIGFILE_WRITE(configfile_auto_exit)
         << '\n';

    // Report an error if required
    if (!file || file.bad())
    {
        os_error er;

        lookup_token(er.errmess, sizeof(er.errmess), "CfgSave");
        report_error(&er);
        return;
    }

    // Clear the modified flag if successful
    configfile_modified = FALSE;
}
