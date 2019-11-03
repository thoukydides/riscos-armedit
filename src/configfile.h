/*
    File        : configfile.h
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

#ifndef configfile_h
#define configfile_h

// Include cpplib header files
#include "cpp:string.h"

// Include oslib header files
#include "types.h"

// The configuration files directory
extern string configfile_dir;

// Has the configuration been modified
extern bool configfile_modified;

// The current configuration
extern bool configfile_frontend_auto_quit;
extern int configfile_speed_fore;
extern int configfile_speed_back;
extern string configfile_path_pc;
extern string configfile_path_config;
extern string configfile_path_partition;
extern string configfile_auto_boot;
extern string configfile_auto_quit;
extern string configfile_auto_load;
extern string configfile_auto_start;
extern string configfile_auto_exit;

// Was a valid configuration file present
extern bool configfile_present;

/*
    Parameters  : void
    Returns     : void
    Description : Attempt to read the configuration file. This also sets the
                  configfile_dir variable.
*/
void configfile_read(void);

/*
    Parameters  : void
    Returns     : void
    Description : Attempt to write the configuration file.
*/
void configfile_write(void);

#endif
