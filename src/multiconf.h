/*
    File        : multiconf.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Handling of multiple PC card configurations.

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

#ifndef multiconf_h
#define multiconf_h

// Include cpplib header files
#include "string.h"

// Include oslib header files
#include "menu.h"

// Maximum length of configuration descriptions
#define MULTICONF_NAME_LEN 64

// Part of a list of associated DOS files
struct multiconf_file
{
    multiconf_file *next;
    multiconf_file *prev;
    int id;
    string name;
    int relative;
};

// More detailed information about a configuration
struct multiconf_details
{
    string pc;
    string config;
    string partition;
    multiconf_file *files;
};

// An entry in the list of configurations
struct multiconf_list
{
    multiconf_list *next;
    multiconf_list *prev;
    int id;
    string name;
    multiconf_details *detail;
    void *handle;                       // Pointer may be used for any purpose
};
extern multiconf_list *multiconf_head;

// Possible configuration sources and destinations
#define MULTICONF_BACKUP ((multiconf_list *) 0)
#define MULTICONF_ACTIVE ((multiconf_list *) 1)

/*
    Parameters  : menu      - The toolbox ID of the menu object to build.
                  submenu   - An optional submenu for each menu entry.
                  click     - An optional object to show when each entry is
                              clicked on.
    Returns     : void
    Description : Add a menu entry for each currently defined configuration.
                  The component ID of each entry is set to be a pointer to
                  the corresponding multiconf_list structure.
*/
void multiconf_build_menu(toolbox_o menu, char *submenu, char *click);

/*
    Parameters  : menu      - The toolbox ID of the menu object to refresh.
                  select    - The configuration to tick.
    Returns     : void
    Description : Update the names of the menu items, and set a single tick.
*/
void multiconf_refresh_menu(toolbox_o menu, multiconf_list *select);

/*
    Parameters  : menu  - The toolbox ID of the menu object to destroy.
    Returns     : void
    Description : Remove menu entries previously added by multiconf_build_menu.
                  This must be called before any items are added to or removed
                  from the list of configurations.
*/
void multiconf_destroy_menu(toolbox_o menu);

/*
    Parameters  : void
    Returns     : void
    Description : Read the multiple configurations index file.
*/
void multiconf_index_read(void);

/*
    Parameters  : void
    Returns     : void
    Description : Write the multiple configurations index file.
*/
void multiconf_index_write(void);

/*
    Parameters  : conf  - The configuration to read the details for.
    Returns     : void
    Description : Read all of the details for a specified configuration.
*/
void multiconf_details_read(multiconf_list *conf);

/*
    Parameters  : conf  - The configuration to write the details for.
    Returns     : void
    Description : Write all of the details for a specified configuration.
                  This does not update the index file.
*/
void multiconf_details_write(const multiconf_list *conf);

/*
    Parameters  : ref       - An optional reference configuration.
                  config    - The configuration to process.
                  name      - Variable to receive the filename.
    Returns     : void
    Description : Construct a suitable filename for the specified configuration
                  file.
*/
void multiconf_name_config(multiconf_list *ref, multiconf_list *config,
                           char *name);

/*
    Parameters  : ref       - An optional reference configuration.
                  config    - The configuration to process.
                  file      - The associate DOS file to process.
                  name      - Variable to receive the filename.
    Returns     : void
    Description : Construct a suitable filename for the specified associated
                  DOS file.
*/
void multiconf_name_file(multiconf_list *ref, multiconf_list *config,
                         multiconf_file *file, char *name);

/*
    Parameters  : ref       - An optional reference configuration.
                  from      - The source configuration.
                  to        - The destination configuration.
                  config    - Should the configuration file be copied.
                  dos       - Should the associated DOS files be copied.
    Returns     : int       - A standard C return code, 0 for success.
    Description : Copy the specified files associated with a configuration.
*/
int multiconf_copy(multiconf_list *ref, multiconf_list *from,
                   multiconf_list *to, int config, int dos);

/*
    Parameters  : config    - The configuration to process.
                  field     - The name of the field to extract.
    Returns     : char *    - The value of the specified field, or NULL if it
                              was not found.
    Description : Extract the values of a field from the specified
                  configuration.
*/
const char *multiconf_config_extract(multiconf_list *config, const char *field);

/*
    Parameters  : config    - The configuration to process.
                  drive     - The number of the partition to find.
    Returns     : char *    - The path of the specified partition, or NULL if
                              it does not exist or was not found.
    Description : Construct the path of the specified partition.
*/
const char *multiconf_config_partition(multiconf_list *config, int drive);

#endif
