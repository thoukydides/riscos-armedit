/*
    File        : multi.c++
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
#include "multi.h"

// Include clib header files
#include <string.h>
#include <stdio.h>

// Include oslib header files
#include "macros.h"
#include "menu.h"
#include "osfile.h"
#include "osfscontrol.h"
extern "C" {
#include "event.h"
}

// Include other project header files
#include "configfile.h"
#include "go.h"
#include "main.h"
#include "multiconf.h"
#include "utils.h"

// The system variables used by the PC card software
#define MULTI_PC_VAR_CONFIG "Diva$ConfigFile"
#define MULTI_PC_VAR_TRUE "true"

// Toolbox object IDs
static toolbox_o multi_menu_parent_id;
static toolbox_o multi_menu_cfg_id = toolbox_NULL_OBJECT;

/*
    Parameters  : void
    Returns     : void
    Description : Read the list of configurations and update the menu.
*/
void multi_update(void)
{
    // Remove any old menu items
    if (multi_menu_cfg_id) multiconf_destroy_menu(multi_menu_cfg_id);

    // Read the list of configurations file
    multiconf_index_read();

    // Add the updated menu items
    if (multi_menu_cfg_id) multiconf_build_menu(multi_menu_cfg_id, NULL, NULL);
}

/*
    Parameters  : config    - Component ID of the configuration.
    Returns     : void
    Description : Start the PC card with the specified configuration.
*/
static void multi_start(toolbox_c config)
{
    multiconf_list *ptr = (multiconf_list *) config;

    // Set the selected configuration if required
    if (ptr != MULTICONF_ACTIVE)
    {
        char path[256];

        // Backup the existing files
        if (multiconf_copy(ptr, MULTICONF_ACTIVE, MULTICONF_BACKUP,
                           TRUE, TRUE)) return;

        // Copy the configuration file itself
        if (multiconf_copy(ptr, ptr, MULTICONF_ACTIVE, TRUE, TRUE)) return;

        // Set the system variables as required
        multiconf_name_config(ptr, MULTICONF_ACTIVE, path);
        os_set_var_val(MULTI_PC_VAR_CONFIG, (byte *) path, strlen(path), 0,
                       os_VARTYPE_LITERAL_STRING, NULL);
    }

    // Run the starting obey file
    if (!configfile_auto_start.empty())
    {
        wimp_start_task(configfile_auto_start.c_str());
    }

    // Start the required PC card software
    go_start_task((ptr != MULTICONF_ACTIVE) && ptr->detail->pc[0]
                  ? ptr->detail->pc.c_str() : configfile_path_pc.c_str(),
                  NULL, NULL);
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle menu selection toolbox events.
*/
static bool multi_menu_select(bits event_code, toolbox_action *action,
                              toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Decode the menu object ID
    if (id_block->this_obj ==  multi_menu_parent_id)
    {
        // Main iconbar menu
        switch (id_block->this_cmp)
        {
            case IBAR_MENU_START:
                // Start PC card software option selected
                multi_start((toolbox_c) MULTICONF_ACTIVE);
                break;

            default:
                // Do not care about any other entries
                break;
        }
    }
    else if (id_block->this_obj == multi_menu_cfg_id)
    {
        // Start the PC with the selected configuration
        multi_start(id_block->this_cmp);
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Attach handlers to auto-created multiple configuration
                  objects.
*/
bool multi_tb_autocreated(bits event_code, toolbox_action *action,
                          toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(handle);

    if (!strcmp(action->data.created.name, "IBarMenu"))
    {
        // It is the main menu
        multi_menu_parent_id = id_block->this_obj;
        event_register_toolbox_handler(multi_menu_parent_id,
                                       action_MENU_SELECTION,
                                       multi_menu_select, NULL);
    }
    else if (!strcmp(action->data.created.name, "CfgMenu"))
    {
        // It is the configuration menu
        multi_menu_cfg_id = id_block->this_obj;
        event_register_toolbox_handler(multi_menu_cfg_id,
                                       action_MENU_SELECTION,
                                       multi_menu_select, NULL);
        multiconf_build_menu(multi_menu_cfg_id, NULL, NULL);
    }

    // Do not claim the event - it is too generally useful
    return FALSE;
}
