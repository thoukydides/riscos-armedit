/*
    File        : multiedit.c++
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Editing of multiple PC card configurations.

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
#include "multiedit.h"

// Include cpplib header files
#include "string.h"
#include "fstream.h"

// Include clib header files
#include <stdio.h>
#include <string.h>

// Include oslib header files
#include "osfile.h"
#include "osfscontrol.h"
#include "osgbpb.h"

// Include project header files
#include "configfile.h"

// The current configuration
multiedit_details multiedit_current =
{
    NULL,
    NULL,
    NULL,
    MULTICONF_ACTIVE
};

// The selected configuration
multiedit_details *multiedit_select = &multiedit_current;

// Head of configurations list
multiedit_details *multiedit_head = &multiedit_current;

// Configuration directory stub
#define MULTIEDIT_CONFIG_DIR "Cfg "

// Is the configuration backed up
static int multiedit_have_backup = FALSE;

/*
    Parameters  : void
    Returns     : void
    Description : Ensure that the extended descriptions match up with the
                  actual configurations.
*/
static void multiedit_refresh(void)
{
    multiedit_details *detail;
    multiconf_list *ptr;

    // Clear all pointers from the extended records
    detail = multiedit_head;
    while (detail)
    {
        // Clear the pointer and advance to the next record
        if (detail != &multiedit_current) detail->ptr = NULL;
        detail = detail->next;
    }

    // Loop round the configurations
    ptr = multiconf_head;
    while (ptr)
    {
        // Is there a corresponding extended record
        if (ptr->handle)
        {
            // Set the pointer from the extended record
            ((multiedit_details *) ptr->handle)->ptr = ptr;
        }
        else
        {
            // Create a new extended record
            detail = new multiedit_details;
            detail->next = multiedit_head;
            detail->prev = NULL;
            detail->edit = NULL;
            detail->ptr = ptr;
            if (multiedit_head) multiedit_head->prev = detail;
            ptr->handle = multiedit_head = detail;
        }

        // Advance to the next record
        ptr = ptr->next;
    }

    // Delete unlinked extended records
    detail = multiedit_head;
    while (detail)
    {
        // Has this record been used
        if (!detail->ptr)
        {
            // Unlink and delete this extended record
            if (detail->edit) delete detail->edit;
            if (detail->next) detail->next->prev = detail->prev;
            if (detail->prev) detail->prev->next = detail->next;
            else multiedit_head = detail->next;
            delete detail;

            // Start again from the head to avoid problems
            detail = multiedit_head;
        }
        else
        {
            // Advance to the next record
            detail = detail->next;
        }
    }
}

/*
    Parameters  : name  - The name of the file to check.
    Returns     : int   - Does the file exist (as a file).
    Description : Check for the existance of a specified file.
*/
static int multiedit_exist(const char *name)
{
    return osfile_read_stamped_no_path(name, NULL, NULL, NULL, NULL, NULL)
           == fileswitch_IS_FILE;
}

/*
    Parameters  : first - Is this the first time after loading a
                          configuration.
    Returns     : int   - A new unique ID.
    Description : Choose a new unique ID.
*/
static int multiedit_new_id(int first)
{
    static int id = 0;
    multiconf_list *ptr;

    if (first)
    {
        // Find the first unused ID
        id = 0;
        ptr = multiconf_head;
        while (ptr)
        {
            // Advance the ID if required
            if (id <= ptr->id) id = ptr->id + 1;

            // Advance to the next record
            ptr = ptr->next;
        }
    }
    else
    {
        // Return and advance the ID
        id = (id + 1) % 1000000;
    }

    // Return the selected ID
    return id;
}

/*
    Parameters  : void
    Returns     : void
    Description : Delete any directories not used by the current
                  configurations. This should be called after saving a
                  configuration.
*/
static void multiedit_tidy(void)
{
    multiconf_list *ptr;
    int context;
    int read;
    char buffer[256];
    char name[256];

    context = osgbpb_dir_entries(configfile_dir.c_str(),
                                 (osgbpb_string_list *) buffer,
                                 1, 0, sizeof(buffer),
                                 MULTIEDIT_CONFIG_DIR "*", &read);
    while (context != -1)
    {
        // Check this entry
        if (read)
        {
            // Check for a match
            ptr = multiconf_head;
            while (ptr && sprintf(name, MULTIEDIT_CONFIG_DIR "%i", ptr->id)
                   && strcmp(name, buffer))
            {
                // Advance to the next record
                ptr = ptr->next;
            }

            // Delete the object if no match
            if (!ptr)
            {
                // Delete the object
                sprintf(name, "%s.%s", configfile_dir.c_str(), buffer);
                osfscontrol_wipe(name,
                                 osfscontrol_WIPE_RECURSE
                                 | osfscontrol_WIPE_FORCE, 0, 0, 0, 0);

                // Need to start scanning from the start again
                context = 0;
            }
        }

        // Find the next match
        context = osgbpb_dir_entries(configfile_dir.c_str(),
                                     (osgbpb_string_list *) buffer, 1, context,
                                     sizeof(buffer), MULTIEDIT_CONFIG_DIR "*",
                                     &read);
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Produce a backup of the current configuration. This should
                  normally be used after loading or saving the configuration,
                  but before editing is started.
*/
void multiedit_backup(void)
{
    multiconf_list *ptr;
    multiconf_list record;
    int first;

    // No action required if backup already exists
    if (!multiedit_have_backup)
    {
        // Produce working copy
        first = TRUE;
        ptr = multiconf_head;
        while (ptr)
        {
            // Choose a new ID
            record.id = ptr->id;
            ptr->id = multiedit_new_id(first);
            first = FALSE;

            // Copy this configuration
            multiconf_details_write(ptr);
            multiconf_copy(ptr, &record, ptr, TRUE, TRUE);

            // Advance to the next record
            ptr = ptr->next;
        }

        // Set flag to indicate backup
        multiedit_have_backup = TRUE;
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Load the current configurations.
*/
void multiedit_load(void)
{
    multiconf_list *ptr;
    char src[256];
    char dest[256];

    // Read the configuration file
    multiconf_index_read();

    // Handle each entry
    ptr = multiconf_head;
    while (ptr)
    {
        // Check for the old format
        multiconf_name_config(NULL, ptr, dest);
        if (multiedit_exist(dest))
        {
            // Read the details
            multiconf_details_read(ptr);
        }
        else
        {
            // Convert the format
            ptr->detail = new multiconf_details;
            ptr->detail->pc[0] = 0;
            ptr->detail->config[0] = 0;
            ptr->detail->partition[0] = 0;
            ptr->detail->files = NULL;

            // Write the details
            multiconf_details_write(ptr);

            // Copy the files
            sprintf(src, "%s.Cfg%i", configfile_dir.c_str(), ptr->id);
            osfscontrol_copy(src, dest, osfscontrol_COPY_FORCE,
                             0, 0, 0, 0, NULL);

            // Check for existing files
            sprintf(src, "%s.CfgA%i", configfile_dir.c_str(), ptr->id);
            if (multiedit_exist(src))
            {
                multiconf_file *file;

                // Add an AUTOEXEC.BAT record
                file = new multiconf_file;
                file->next = NULL;
                file->prev = NULL;
                file->id = 0;
                file->name = "AUTOEXEC/BAT";
                file->relative = TRUE;
                ptr->detail->files = file;
                multiconf_name_file(ptr, ptr, file, dest);
                osfscontrol_copy(src, dest, osfscontrol_COPY_FORCE,
                                 0, 0, 0, 0, NULL);
            }
            sprintf(src, "%s.CfgC%i", configfile_dir.c_str(), ptr->id);
            if (multiedit_exist(src))
            {
                multiconf_file *file;

                // Add a CONFIG.SYS record
                file = new multiconf_file;
                file->next = ptr->detail->files;
                file->prev = NULL;
                file->id = 1;
                file->name = "CONFIG/SYS";
                file->relative = TRUE;
                ptr->detail->files = file;
                if (file->next) file->next->prev = file;
                multiconf_name_file(ptr, ptr, file, dest);
                osfscontrol_copy(src, dest, osfscontrol_COPY_FORCE,
                                 0, 0, 0, 0, NULL);
            }

            // Write the details again to include any DOS files
            multiconf_details_write(ptr);
        }

        // Fill in the remaining details
        ptr->handle = NULL;

        // Advance to the next record
        ptr = ptr->next;
    }

    // Extend the stored information
    multiedit_refresh();

    // Delete spurious directories
    multiedit_tidy();

    // Indicate that no backup exists
    multiedit_have_backup = FALSE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Save the current configurations.
*/
void multiedit_save(void)
{
    multiconf_list *ptr;

    // Write the index file
    multiconf_index_write();

    // Write all the details
    ptr = multiconf_head;
    while (ptr)
    {
        // Write the details of this record
        multiconf_details_write(ptr);

        // Advance to the next record
        ptr = ptr->next;
    }

    // Extend the stored information
    multiedit_refresh();

    // Delete spurious directories
    multiedit_tidy();

    // Indicate that no backup exists
    multiedit_have_backup = FALSE;
}

/*
    Parameters  : from              - The configuration to copy.
    Returns     : multiedit_details - The new configuration.
    Description : Create a copy of a specified configuration.
*/
multiedit_details *multiedit_copy(multiedit_details *from)
{
    multiconf_list *to;
    multiconf_file *ptr;
    multiconf_file *prev;

    // Is it from the current configuration
    if (from == &multiedit_current)
    {
        // Create a new record
        to = new multiconf_list;
        to->name = "New configuration";

        // Create the details
        to->detail = new multiconf_details;
        to->detail->pc[0] = 0;
        to->detail->config[0] = 0;
        to->detail->partition[0] = 0;
        to->detail->files = NULL;

        // Link in the new record
        to->prev = NULL;
        to->next = multiconf_head;
        if (multiconf_head) multiconf_head->prev = to;
        multiconf_head = to;
    }
    else
    {
        // Copy the record
        to = new multiconf_list;
        *to = *(from->ptr);

        // Copy the details
        to->detail = new multiconf_details;
        *(to->detail) = *(from->ptr->detail);

        // Copy the file details
        ptr = from->ptr->detail->files;
        prev = NULL;
        while (ptr)
        {
            multiconf_file *file;

            // Create a new record
            file = new multiconf_file;
            *file = *ptr;
            file->next = NULL;
            file->prev = prev;
            if (prev) prev->next = file;
            else to->detail->files = file;

            // Advance to the next record
            ptr = ptr->next;
            prev = file;
        }

        // Link in the new record
        to->prev = from->ptr;
        to->next = from->ptr->next;
        from->ptr->next = to;
        if (to->next) to->next->prev = to;
    }

    // Set other fields
    to->handle = NULL;
    to->id = multiedit_new_id(FALSE);

    // Save the details
    multiconf_details_write(to);

    // Copy the files
    multiconf_copy(to, from->ptr, to, TRUE, TRUE);

    // Extend the stored information
    multiedit_refresh();

    // Return a pointer to the new configuration
    return (multiedit_details *) to->handle;
}

/*
    Parameters  : ptr   - The configuration to delete.
    Returns     : void
    Description : Delete the specified configuration.
*/
void multiedit_delete(multiedit_details *ptr)
{
    // Delete the file details
    while (ptr->ptr->detail->files)
    {
        multiconf_file *file = ptr->ptr->detail->files;
        ptr->ptr->detail->files = file->next;
        delete file;
    }

    // Delete the details
    delete ptr->ptr->detail;

    // Unlink the record
    if (ptr->ptr->next) ptr->ptr->next->prev = ptr->ptr->prev;
    if (ptr->ptr->prev) ptr->ptr->prev->next = ptr->ptr->next;
    else multiconf_head = ptr->ptr->next;

    // Delete the record
    delete ptr->ptr;

    // Delete the unused stored information
    multiedit_refresh();
}
