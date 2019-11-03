/*
    File        : multiconf.c++
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

// Include header file for this module
#include "multiconf.h"

// Include cpplib header files
#include "string.h"
#include "fstream.h"

// Include clib header files
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Include oslib header files
#include "osfile.h"
#include "osfscontrol.h"

// Include project header files
#include "configfile.h"
#include "scsimap.h"
#include "utils.h"

// Head of the list of configurations
multiconf_list *multiconf_head = NULL;

// The configuration files
#define MULTICONF_INDEX_FILE "CfgIndex"
#define MULTICONF_CONFIG_DIR "Cfg "
#define MULTICONF_CONFIG_FILE "Config"
#define MULTICONF_DETAILS_FILE "Info"
#define MULTICONF_FILES_FILE "Files"
static string multiconf_index_file;

// Configuration file fields
#define MULTICONF_HD_PREFIX "HD"
#define MULTICONF_HD_POSTFIX "-File-Name"
#define MULTICONF_HD_SCSI "-SCSI-ID"

// Scrap backups
#define MULTICONF_SCAP_DIR "<ARMEdit$ScrapDir>"

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
void multiconf_build_menu(toolbox_o menu, char *submenu, char *click)
{
    menu_entry_object entry;
    multiconf_list *ptr;

    // Process every configuration
    ptr = multiconf_head;
    while (ptr)
    {
        // Add this menu entry
        entry.flags = submenu
                      ? (menu_ENTRY_SUB_MENU
                         | menu_ENTRY_GENERATE_SUB_MENU_ACTION)
                      : 0;
        entry.cmp = (toolbox_c) ptr;
        entry.text = const_cast(char*, ptr->name.c_str());
        entry.text_limit = MULTICONF_NAME_LEN;
        entry.click_object_name = click;
        entry.sub_menu_object_name = submenu;
        entry.sub_menu_action = 0;
        entry.click_action = 0;
        entry.help = NULL;
        entry.help_limit = 0;

        menu_add_entry(0, menu, menu_ADD_AT_END, &entry);

        // Adavance to the next configuration
        ptr = ptr->next;
    }
}

/*
    Parameters  : menu      - The toolbox ID of the menu object to refresh.
                  select    - The configuration to tick.
    Returns     : void
    Description : Update the names of the menu items, and set a single tick.
*/
void multiconf_refresh_menu(toolbox_o menu, multiconf_list *select)
{
    static multiconf_list *old_select = NULL;
    multiconf_list *ptr;
    char str[256];

    // Update all of the text entries
    ptr = multiconf_head;
    while (ptr)
    {
        // Update the text for this entry
        if (xmenu_get_entry_text(0, menu, (toolbox_c) ptr, str,
                                 sizeof(str), NULL)
            || strcmp(str, ptr->name.c_str()))
        {
            menu_set_entry_text(0, menu, (toolbox_c) ptr, ptr->name.c_str());
        }

        // Adavance to the next configuration
        ptr = ptr->next;
    }

    // Does the tick need to be moved
    if (!menu_get_tick(0, menu, (toolbox_c) select))
    {
        // Remove the old tick
        xmenu_set_tick(0, menu, (toolbox_c) old_select, FALSE);

        // Set the tick next to the required entry
        menu_set_tick(0, menu, (toolbox_c) select, TRUE);
    }
    old_select = select;
}

/*
    Parameters  : menu  - The toolbox ID of the menu object to destroy.
    Returns     : void
    Description : Remove menu entries previously added by multiconf_build_menu.
                  This must be called before any items are added to or removed
                  from the list of configurations.
*/
void multiconf_destroy_menu(toolbox_o menu)
{
    multiconf_list *ptr;

    // Process every configuration
    ptr = multiconf_head;
    while (ptr)
    {
        // Delete this menu entry
        menu_remove_entry(0, menu, (toolbox_c) ptr);

        // Adavance to the next configuration
        ptr = ptr->next;
    }
}

/*
    Parameters  : conf  - The configuration to delete the details for.
    Returns     : void
    Description : Free the memory used by the details associated with a
                  specified configuration.
*/
static void multiconf_details_delete(multiconf_list *conf)
{
    // No action required if no details loaded
    if (conf->detail)
    {
        // Free the memory used by the file list
        while (conf->detail->files)
        {
            multiconf_file *ptr = conf->detail->files;
            conf->detail->files = ptr->next;
            delete ptr;
        }

        // Free the memory used by the details list
        delete conf->detail;
        conf->detail = NULL;
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Read the multiple configurations index file.
*/
void multiconf_index_read(void)
{
    multiconf_list *last;
    int id;
    char name[MULTICONF_NAME_LEN];
    char c;

    // First destroy any currently loaded index
    while (multiconf_head)
    {
        multiconf_list *ptr = multiconf_head;

        // Delete the details
        multiconf_details_delete(multiconf_head);

        // Delete the main entry
        multiconf_head = ptr->next;
        delete ptr;
    }

    // Construct the index filename
    multiconf_index_file = configfile_dir + "." + MULTICONF_INDEX_FILE;

    // Open the index file
    ifstream file(multiconf_index_file.c_str());
    if (!file) return;

    // Loop until all entries read
    last = NULL;
    file >> id;
    file.get(c);
    file.get(name, sizeof(name), '\n');
    while (file.get(c) && c == '\n')
    {
        multiconf_list *ptr;

        // Create a new record
        ptr = new multiconf_list;
        if (ptr)
        {
            // Fill in the details
            ptr->id = id;
            ptr->name = name;
            ptr->detail = NULL;
            ptr->handle = NULL;

            // Link the record in
            ptr->next = NULL;
            ptr->prev = last;
            if (last) last->next = ptr;
            else multiconf_head = ptr;
            last = ptr;
        }

        // Read the next line of the file
        file >> id;
        file.get(c);
        file.get(name, sizeof(name), '\n');
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Write the multiple configurations index file.
*/
void multiconf_index_write(void)
{
    multiconf_list *ptr;

    // Open the index file
    ofstream file(multiconf_index_file.c_str());

    // Write all the entries
    ptr = multiconf_head;
    while (ptr)
    {
        // Write this entry
        file << ptr->id << ' ' << ptr->name << '\n';

        // Obtain pointer to the next entry
        ptr = ptr->next;
    }
}

/*
    Parameters  : conf  - The configuration to read the details for.
    Returns     : void
    Description : Read all of the details for a specified configuration.
*/
void multiconf_details_read(multiconf_list *conf)
{
    char name[256];
    int id;
    int relative;
    multiconf_file *last;

    // Clear any previously loaded details
    multiconf_details_delete(conf);

    // Construct the name of the details file
    sprintf(name, "%s.%s%i.%s", configfile_dir.c_str(),
            MULTICONF_CONFIG_DIR, conf->id, MULTICONF_DETAILS_FILE);

    // Create a details record
    conf->detail = new multiconf_details;
    conf->detail->pc[0] = 0;
    conf->detail->config[0] = 0;
    conf->detail->partition[0] = 0;
    conf->detail->files = NULL;

    // Open the details file
    ifstream file(name);
    if (!name) return;

    // Read the details file
    getline(file, conf->detail->pc);
    getline(file, conf->detail->config);
    getline(file, conf->detail->partition);

    // Close the details file
    file.close();

    // Construct the name of the files list file
    sprintf(name, "%s.%s%i.%s", configfile_dir.c_str(),
            MULTICONF_CONFIG_DIR, conf->id, MULTICONF_FILES_FILE);

    // Open the file list file
    file.open(name);
    if (!file) return;

    // Read the details of any associated files
    last = NULL;
    while (file >> id >> relative >> name)
    {
        multiconf_file *ptr;

        // Add a new file record
        ptr = new multiconf_file;
        ptr->id = id;
        ptr->relative = relative;
        ptr->name = name;

        // Link the record in
        ptr->next = NULL;
        ptr->prev = last;
        if (last) last->next = ptr;
        else conf->detail->files = ptr;
        last = ptr;
    }
}

/*
    Parameters  : conf  - The configuration to write the details for.
    Returns     : void
    Description : Write all of the details for a specified configuration.
                  This does not update the index file.
*/
void multiconf_details_write(const multiconf_list *conf)
{
    os_error *er;
    multiconf_file *ptr;
    char name[256];

    // Only possible if details are defined
    if (!conf->detail) return;

    // Construct the name of the configuration directory
    sprintf(name, "%s.%s%i", configfile_dir.c_str(),
            MULTICONF_CONFIG_DIR, conf->id);

    // Ensure that the configuration directory exists
    er = xosfile_create_dir(name, 0);
    if (er)
    {
        report_error(er);
        return;
    }

    // Construct the name of the details file
    sprintf(name, "%s.%s%i.%s", configfile_dir.c_str(),
            MULTICONF_CONFIG_DIR, conf->id, MULTICONF_DETAILS_FILE);

    // Open the details file
    ofstream file(name);

    // Write the details file
    file << conf->detail->pc << '\n'
         << conf->detail->config << '\n'
         << conf->detail->partition << '\n';

    // Report an error if required
    if (!file || file.bad())
    {
        os_error er;

        lookup_token(er.errmess, sizeof(er.errmess), "CfgSave");
        report_error(&er);
        return;
    }

    // Close the details file
    file.close();

    // Construct the name of the files list file
    sprintf(name, "%s.%s%i.%s", configfile_dir.c_str(),
            MULTICONF_CONFIG_DIR, conf->id, MULTICONF_FILES_FILE);

    // Open the files list file
    file.open(name);

    // Write the files details
    ptr = conf->detail->files;
    while (ptr)
    {
        // Write the details for this file
        file << ptr->id << ' '
             << ptr->relative << ' '
             << ptr->name << '\n';

        // Advance to the next file
        ptr = ptr->next;
    }
}

/*
    Parameters  : ref       - An optional reference configuration.
                  config    - The configuration to process.
                  name      - Variable to receive the filename.
    Returns     : void
    Description : Construct a suitable filename for the specified configuration
                  file.
*/
void multiconf_name_config(multiconf_list *ref, multiconf_list *config,
                           char *name)
{
    // Name depends upon target type
    if (config == MULTICONF_ACTIVE)
    {
        // The current configuration
        if (ref && ref->detail && ref->detail->pc[0])
        {
            // Use the specified PC card software
            sprintf(name, "%s.%s", ref->detail->pc, MULTICONF_CONFIG_FILE);
        }
        else
        {
            // Use the default PC card software
            sprintf(name, "%s.%s", configfile_path_pc.c_str(),
                    MULTICONF_CONFIG_FILE);
        }
    }
    else if (config == MULTICONF_BACKUP)
    {
        // The backup configuration in the scrap directory
        sprintf(name, "%s.%s", MULTICONF_SCAP_DIR, MULTICONF_CONFIG_FILE);
    }
    else
    {
        // A named configuration
        sprintf(name, "%s.%s%i.%s", configfile_dir.c_str(),
                MULTICONF_CONFIG_DIR, config->id, MULTICONF_CONFIG_FILE);
    }
}

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
                         multiconf_file *file, char *name)
{
    // Name depends upon target type
    if (config == MULTICONF_ACTIVE)
    {
        // The current configuration
        if (file->relative)
        {
            if (ref && ref->detail && ref->detail->partition[0])
            {
                // Use the specified PC card partition
                sprintf(name, "%s.%s", ref->detail->partition, file->name);
            }
            else
            {
                // Use the default PC card partition
                sprintf(name, "%s.%s", configfile_path_partition.c_str(),
                        file->name);
            }
        }
        else
        {
            // An absolute path is already specified
            strcpy(name, file->name.c_str());
        }

    }
    else if (config == MULTICONF_BACKUP)
    {
        // The backup configuration in the scrap directory
        sprintf(name, "%s.%s%i", MULTICONF_SCAP_DIR, MULTICONF_FILES_FILE,
                file->id);
    }
    else
    {
        // A named configuration
        sprintf(name, "%s.%s%i.%s%i", configfile_dir.c_str(),
                MULTICONF_CONFIG_DIR, config->id, MULTICONF_FILES_FILE,
                file->id);
    }
}

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
                   multiconf_list *to, int config, int dos)
{
    char name_from[256];
    char name_to[256];
    os_error *er;

    // Choose a reference configuration if none specified
    if (!ref)
    {
        if ((from != MULTICONF_ACTIVE) && (from != MULTICONF_BACKUP))
        {
            ref = from;
        }
        else if ((to != MULTICONF_ACTIVE) && (to != MULTICONF_BACKUP))
        {
            to = from;
        }
    }

    // Ensure that the configuration details are loaded
    if (ref && !ref->detail) multiconf_details_read(ref);

    // Copy the main configuration file if required
    if (config)
    {
        // Construct the source and destination filenames
        multiconf_name_config(ref, from, name_from);
        multiconf_name_config(ref, to, name_to);

        // Attempt to copy the file
        er = xosfscontrol_copy(name_from, name_to, osfscontrol_COPY_FORCE,
                               0, 0, 0, 0, NULL);
        if (er)
        {
            report_error(er);
            return EXIT_FAILURE;
        }
    }

    // Copy the associate DOS files if required
    if (dos)
    {
        multiconf_file *ptr;

        // Check that the details are known
        if (!ref || !ref->detail) return EXIT_FAILURE;

        // Loop through all of the DOS files
        ptr = ref->detail->files;
        while (ptr)
        {
            // Construct the source and destination filenames
            multiconf_name_file(ref, from, ptr, name_from);
            multiconf_name_file(ref, to, ptr, name_to);

            // Attempt to copy the file
            er = xosfscontrol_copy(name_from, name_to, osfscontrol_COPY_FORCE,
                                   0, 0, 0, 0, NULL);
            if (er)
            {
                report_error(er);
                return EXIT_FAILURE;
            }

            // Advance to the next file
            ptr = ptr->next;
        }
    }

    // If this point reached then successful
    return EXIT_SUCCESS;
}

/*
    Parameters  : config    - The configuration to process.
                  field     - The name of the field to extract.
    Returns     : char *    - The value of the specified field, or NULL if it
                              was not found.
    Description : Extract the values of a field from the specified
                  configuration.
*/
const char *multiconf_config_extract(multiconf_list *config, const char *field)
{
    char line[256];
    char name[256];
    char c;

    // Construct the name of the configuration file
    multiconf_name_config(NULL, config, name);

    // Attempt to open the configuration file
    ifstream file(name);
    if (!file) return NULL;

    // Keep reading until either the field is found or the end reached
    file.get(line, sizeof(line), '\n');
    while (file.get(c) && c == '\n')
    {
        // Check if the required field was found
        if (!strncmp(field, line, strlen(field)))
        {
            // Skip the space character also
            return line + strlen(field) + 1;
        }

        // Read the next line of the file
        file.get(line, sizeof(line), '\n');
    }

    // Not found if this point reached
    return NULL;
}

/*
    Parameters  : config    - The configuration to process.
                  drive     - The number of the partition to find.
    Returns     : char *    - The path of the specified partition, or NULL if
                              it does not exist or was not found.
    Description : Construct the path of the specified partition.
*/
const char *multiconf_config_partition(multiconf_list *config, int drive)
{
    static char path[256];
    char field[20];
    const char *ptr;

    // First try standard partition files
    sprintf(field, "%s%i%s", MULTICONF_HD_PREFIX, drive, MULTICONF_HD_POSTFIX);
    ptr = multiconf_config_extract(config, field);
    if (ptr)
    {
        // A standard partition was specified
        strcpy(path, ptr);
        ptr = path;
    }
    else
    {
        // Try direct SCSI
        sprintf(field, "%s%i%s", MULTICONF_HD_PREFIX, drive, MULTICONF_HD_SCSI);
        ptr = multiconf_config_extract(config, field);
        if (ptr)
        {
            // A direct SCSI device was specified
            ptr = scsimap_path(atoi(ptr), 0);
        }
    }

    // Return a pointer to the partition path
    return ptr;
}
