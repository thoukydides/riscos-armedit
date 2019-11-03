/*
    File        : configwin.c++
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Configuration window handling for !ARMEdit.

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
#include "configwin.h"

// Include cpplib header files
#include "string.h"
#include "fstream.h"

// Include clib header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#undef _C
#undef _Z
#undef _N
#include "swis.h"

// Include oslib header files
#include "macros.h"
#include "osfile.h"
#include "window.h"
extern "C" {
#include "event.h"
}

// Include alexlib header files
#include "actionbutton_c.h"
#include "button_c.h"
#include "label_c.h"
#include "numberrange_c.h"
#include "optionbutton_c.h"
#include "popup_c.h"
#include "radiobutton_c.h"
#include "writablefield_c.h"

// Include project header files
#include "armeditmsg.h"
#include "armeditswi.h"
#include "configfile.h"
#include "go.h"
#include "multiconf.h"
#include "multiedit.h"
#include "pcpro.h"
#include "quit.h"
#include "utils.h"

// Offsets from mouse pointer to open windows at
#define CONFIGWIN_OFFSET_X (100)
#define CONFIGWIN_OFFSET_Y (-46)

// Component IDs for the gadgets
#define CONFIGWIN_MAIN_ICON_SAVE ((toolbox_c) 0x00)
#define CONFIGWIN_MAIN_ICON_CANCEL ((toolbox_c) 0x01)
#define CONFIGWIN_MAIN_ICON_FRONTEND ((toolbox_c) 0x10)
#define CONFIGWIN_MAIN_ICON_SPEED ((toolbox_c) 0x11)
#define CONFIGWIN_MAIN_ICON_PATH ((toolbox_c) 0x12)
#define CONFIGWIN_MAIN_ICON_CONFIGS ((toolbox_c) 0x13)
#define CONFIGWIN_MAIN_ICON_AUTO ((toolbox_c) 0x14)
#define CONFIGWIN_FRONTEND_ICON_CLOSE ((toolbox_c) 0x00)
#define CONFIGWIN_FRONTEND_ICON_AUTO_QUIT ((toolbox_c) 0x10)
#define CONFIGWIN_SPEED_ICON_CLOSE ((toolbox_c) 0x00)
#define CONFIGWIN_SPEED_ICON_FORE ((toolbox_c) 0x10)
#define CONFIGWIN_SPEED_ICON_FORE_VAL ((toolbox_c) 0x11)
#define CONFIGWIN_SPEED_ICON_BACK ((toolbox_c) 0x12)
#define CONFIGWIN_SPEED_ICON_BACK_VAL ((toolbox_c) 0x13)
#define CONFIGWIN_SPEED_ICON_READ ((toolbox_c) 0x14)
#define CONFIGWIN_PATH_ICON_CLOSE ((toolbox_c) 0x00)
#define CONFIGWIN_PATH_ICON_PC ((toolbox_c) 0x10)
#define CONFIGWIN_PATH_ICON_PC_DROP ((toolbox_c) 0x11)
#define CONFIGWIN_PATH_ICON_PC_ICON ((toolbox_c) 0x12)
#define CONFIGWIN_PATH_ICON_PC_LABEL ((toolbox_c) 0x13)
#define CONFIGWIN_PATH_ICON_CONFIG ((toolbox_c) 0x14)
#define CONFIGWIN_PATH_ICON_CONFIG_DROP ((toolbox_c) 0x15)
#define CONFIGWIN_PATH_ICON_CONFIG_ICON ((toolbox_c) 0x16)
#define CONFIGWIN_PATH_ICON_CONFIG_LABEL ((toolbox_c) 0x17)
#define CONFIGWIN_PATH_ICON_PARTITION ((toolbox_c) 0x18)
#define CONFIGWIN_PATH_ICON_PARTITION_DROP ((toolbox_c) 0x19)
#define CONFIGWIN_PATH_ICON_PARTITION_ICON ((toolbox_c) 0x1a)
#define CONFIGWIN_PATH_ICON_PARTITION_LABEL ((toolbox_c) 0x1b)
#define CONFIGWIN_CONFIGS_ICON_CLOSE ((toolbox_c) 0x00)
#define CONFIGWIN_CONFIGS_ICON_NAME ((toolbox_c) 0x20)
#define CONFIGWIN_CONFIGS_ICON_MENU ((toolbox_c) 0x21)
#define CONFIGWIN_CONFIGS_ICON_CONFIG ((toolbox_c) 0x30)
#define CONFIGWIN_CONFIGS_ICON_EDIT ((toolbox_c) 0x31)
#define CONFIGWIN_CONFIGS_ICON_PATHS ((toolbox_c) 0x32)
#define CONFIGWIN_CONFIGS_ICON_DOS ((toolbox_c) 0x33)
#define CONFIGWIN_CONFIG_PATHS_ICON_CLOSE ((toolbox_c) 0x00)
#define CONFIGWIN_CONFIG_PATHS_ICON_DEFAULT_FE ((toolbox_c) 0x10)
#define CONFIGWIN_CONFIG_PATHS_ICON_OTHER_FE ((toolbox_c) 0x11)
#define CONFIGWIN_CONFIG_PATHS_ICON_PC ((toolbox_c) 0x12)
#define CONFIGWIN_CONFIG_PATHS_ICON_PC_DROP ((toolbox_c) 0x13)
#define CONFIGWIN_CONFIG_PATHS_ICON_PC_ICON ((toolbox_c) 0x14)
#define CONFIGWIN_CONFIG_PATHS_ICON_PC_LABEL ((toolbox_c) 0x15)
#define CONFIGWIN_CONFIG_PATHS_ICON_CONFIG ((toolbox_c) 0x16)
#define CONFIGWIN_CONFIG_PATHS_ICON_CONFIG_DROP ((toolbox_c) 0x17)
#define CONFIGWIN_CONFIG_PATHS_ICON_CONFIG_ICON ((toolbox_c) 0x18)
#define CONFIGWIN_CONFIG_PATHS_ICON_CONFIG_LABEL ((toolbox_c) 0x19)
#define CONFIGWIN_CONFIG_PATHS_ICON_DEFAULT_PART ((toolbox_c) 0x1a)
#define CONFIGWIN_CONFIG_PATHS_ICON_OTHER_PART ((toolbox_c) 0x1b)
#define CONFIGWIN_CONFIG_PATHS_ICON_PARTITION ((toolbox_c) 0x1c)
#define CONFIGWIN_CONFIG_PATHS_ICON_PARTITION_DROP ((toolbox_c) 0x1d)
#define CONFIGWIN_CONFIG_PATHS_ICON_PARTITION_ICON ((toolbox_c) 0x1e)
#define CONFIGWIN_CONFIG_PATHS_ICON_PARTITION_LABEL ((toolbox_c) 0x1f)
#define CONFIGWIN_CONFIG_FILES_ICON_CLOSE ((toolbox_c) 0x00)
#define CONFIGWIN_CONFIG_FILES_ICON_NAME ((toolbox_c) 0x10)
#define CONFIGWIN_CONFIG_FILES_ICON_NAME_DROP ((toolbox_c) 0x11)
#define CONFIGWIN_CONFIG_FILES_ICON_NAME_ICON ((toolbox_c) 0x12)
#define CONFIGWIN_CONFIG_FILES_ICON_NAME_LABEL ((toolbox_c) 0x13)
#define CONFIGWIN_CONFIG_FILES_ICON_RELATIVE ((toolbox_c) 0x14)
#define CONFIGWIN_CONFIG_FILES_ICON_REMOVE ((toolbox_c) 0x15)
#define CONFIGWIN_CONFIG_FILES_ICON_UPDATE ((toolbox_c) 0x16)
#define CONFIGWIN_AUTO_ICON_CLOSE ((toolbox_c) 0x00)
#define CONFIGWIN_AUTO_ICON_BOOT ((toolbox_c) 0x10)
#define CONFIGWIN_AUTO_ICON_BOOT_DROP ((toolbox_c) 0x11)
#define CONFIGWIN_AUTO_ICON_BOOT_ICON ((toolbox_c) 0x12)
#define CONFIGWIN_AUTO_ICON_BOOT_LABEL ((toolbox_c) 0x13)
#define CONFIGWIN_AUTO_ICON_QUIT ((toolbox_c) 0x14)
#define CONFIGWIN_AUTO_ICON_QUIT_DROP ((toolbox_c) 0x15)
#define CONFIGWIN_AUTO_ICON_QUIT_ICON ((toolbox_c) 0x16)
#define CONFIGWIN_AUTO_ICON_QUIT_LABEL ((toolbox_c) 0x17)
#define CONFIGWIN_AUTO_ICON_LOAD ((toolbox_c) 0x18)
#define CONFIGWIN_AUTO_ICON_LOAD_DROP ((toolbox_c) 0x19)
#define CONFIGWIN_AUTO_ICON_LOAD_ICON ((toolbox_c) 0x1a)
#define CONFIGWIN_AUTO_ICON_LOAD_LABEL ((toolbox_c) 0x1b)
#define CONFIGWIN_AUTO_ICON_START ((toolbox_c) 0x1c)
#define CONFIGWIN_AUTO_ICON_START_DROP ((toolbox_c) 0x1d)
#define CONFIGWIN_AUTO_ICON_START_ICON ((toolbox_c) 0x1e)
#define CONFIGWIN_AUTO_ICON_START_LABEL ((toolbox_c) 0x1f)
#define CONFIGWIN_AUTO_ICON_EXIT ((toolbox_c) 0x20)
#define CONFIGWIN_AUTO_ICON_EXIT_DROP ((toolbox_c) 0x21)
#define CONFIGWIN_AUTO_ICON_EXIT_ICON ((toolbox_c) 0x22)
#define CONFIGWIN_AUTO_ICON_EXIT_LABEL ((toolbox_c) 0x23)

// Component IDs for the menus
#define CONFIGWIN_CONFIGS_MENU_CURRENT ((toolbox_c) 0x01)
#define CONFIGWIN_CONFIGS_MENU_COPY ((toolbox_c) 0x02)
#define CONFIGWIN_CONFIGS_MENU_DELETE ((toolbox_c) 0x03)

// The configuration status
static int configwin_win_open = FALSE;
static int configwin_win_first = TRUE;

// The toolbox object IDs
static toolbox_o configwin_main;
static toolbox_o configwin_frontend;
static toolbox_o configwin_speed;
static toolbox_o configwin_path;
static toolbox_o configwin_configs;
static toolbox_o configwin_config_paths;
static toolbox_o configwin_config_files;
static toolbox_o configwin_config_files_pane;
static toolbox_o configwin_config_menu;
static toolbox_o configwin_auto;

// Objects to represent the gadgets
static actionbutton_c configwin_main_icon_save(CONFIGWIN_MAIN_ICON_SAVE);
static actionbutton_c configwin_main_icon_cancel(CONFIGWIN_MAIN_ICON_CANCEL);
static button_c configwin_main_icon_frontend(CONFIGWIN_MAIN_ICON_FRONTEND);
static button_c configwin_main_icon_speed(CONFIGWIN_MAIN_ICON_SPEED);
static button_c configwin_main_icon_path(CONFIGWIN_MAIN_ICON_PATH);
static button_c configwin_main_icon_configs(CONFIGWIN_MAIN_ICON_CONFIGS);
static button_c configwin_main_icon_auto(CONFIGWIN_MAIN_ICON_AUTO);
static actionbutton_c configwin_frontend_icon_close(CONFIGWIN_FRONTEND_ICON_CLOSE);
static optionbutton_c configwin_frontend_icon_auto_quit(CONFIGWIN_FRONTEND_ICON_AUTO_QUIT);
static actionbutton_c configwin_speed_icon_close(CONFIGWIN_SPEED_ICON_CLOSE);
static optionbutton_c configwin_speed_icon_fore(CONFIGWIN_SPEED_ICON_FORE);
static numberrange_c configwin_speed_icon_fore_val(CONFIGWIN_SPEED_ICON_FORE_VAL);
static optionbutton_c configwin_speed_icon_back(CONFIGWIN_SPEED_ICON_BACK);
static numberrange_c configwin_speed_icon_back_val(CONFIGWIN_SPEED_ICON_BACK_VAL);
static actionbutton_c configwin_speed_icon_read(CONFIGWIN_SPEED_ICON_READ);
static actionbutton_c configwin_path_icon_close(CONFIGWIN_PATH_ICON_CLOSE);
static writablefield_c configwin_path_icon_pc(CONFIGWIN_PATH_ICON_PC);
static button_c configwin_path_icon_pc_drop(CONFIGWIN_PATH_ICON_PC_DROP);
static button_c configwin_path_icon_pc_icon(CONFIGWIN_PATH_ICON_PC_ICON);
static label_c configwin_path_icon_pc_label(CONFIGWIN_PATH_ICON_PC_LABEL);
static writablefield_c configwin_path_icon_config(CONFIGWIN_PATH_ICON_CONFIG);
static button_c configwin_path_icon_config_drop(CONFIGWIN_PATH_ICON_CONFIG_DROP);
static button_c configwin_path_icon_config_icon(CONFIGWIN_PATH_ICON_CONFIG_ICON);
static label_c configwin_path_icon_config_label(CONFIGWIN_PATH_ICON_CONFIG_LABEL);
static writablefield_c configwin_path_icon_partition(CONFIGWIN_PATH_ICON_PARTITION);
static button_c configwin_path_icon_partition_drop(CONFIGWIN_PATH_ICON_PARTITION_DROP);
static button_c configwin_path_icon_partition_icon(CONFIGWIN_PATH_ICON_PARTITION_ICON);
static label_c configwin_path_icon_partition_label(CONFIGWIN_PATH_ICON_PARTITION_LABEL);
static actionbutton_c configwin_configs_icon_close(CONFIGWIN_CONFIGS_ICON_CLOSE);
static writablefield_c configwin_configs_icon_name(CONFIGWIN_CONFIGS_ICON_NAME);
static popup_c configwin_configs_icon_menu(CONFIGWIN_CONFIGS_ICON_MENU);
static writablefield_c /*button_c*/ configwin_configs_icon_config(CONFIGWIN_CONFIGS_ICON_CONFIG);
static button_c configwin_configs_icon_edit(CONFIGWIN_CONFIGS_ICON_EDIT);
static button_c configwin_configs_icon_paths(CONFIGWIN_CONFIGS_ICON_PATHS);
static button_c configwin_configs_icon_dos(CONFIGWIN_CONFIGS_ICON_DOS);
static actionbutton_c configwin_config_paths_icon_close(CONFIGWIN_CONFIG_PATHS_ICON_CLOSE);
static radiobutton_c configwin_config_paths_icon_default_fe(CONFIGWIN_CONFIG_PATHS_ICON_DEFAULT_FE);
static radiobutton_c configwin_config_paths_icon_other_fe(CONFIGWIN_CONFIG_PATHS_ICON_OTHER_FE);
static writablefield_c configwin_config_paths_icon_pc(CONFIGWIN_CONFIG_PATHS_ICON_PC);
static button_c configwin_config_paths_icon_pc_drop(CONFIGWIN_CONFIG_PATHS_ICON_PC_DROP);
static button_c configwin_config_paths_icon_pc_icon(CONFIGWIN_CONFIG_PATHS_ICON_PC_ICON);
static label_c configwin_config_paths_icon_pc_label(CONFIGWIN_CONFIG_PATHS_ICON_PC_LABEL);
static writablefield_c configwin_config_paths_icon_config(CONFIGWIN_CONFIG_PATHS_ICON_CONFIG);
static button_c configwin_config_paths_icon_config_drop(CONFIGWIN_CONFIG_PATHS_ICON_CONFIG_DROP);
static button_c configwin_config_paths_icon_config_icon(CONFIGWIN_CONFIG_PATHS_ICON_CONFIG_ICON);
static label_c configwin_config_paths_icon_config_label(CONFIGWIN_CONFIG_PATHS_ICON_CONFIG_LABEL);
static radiobutton_c configwin_config_paths_icon_default_part(CONFIGWIN_CONFIG_PATHS_ICON_DEFAULT_PART);
static radiobutton_c configwin_config_paths_icon_other_part(CONFIGWIN_CONFIG_PATHS_ICON_OTHER_PART);
static writablefield_c configwin_config_paths_icon_partition(CONFIGWIN_CONFIG_PATHS_ICON_PARTITION);
static button_c configwin_config_paths_icon_partition_drop(CONFIGWIN_CONFIG_PATHS_ICON_PARTITION_DROP);
static button_c configwin_config_paths_icon_partition_icon(CONFIGWIN_CONFIG_PATHS_ICON_PARTITION_ICON);
static label_c configwin_config_paths_icon_partition_label(CONFIGWIN_CONFIG_PATHS_ICON_PARTITION_LABEL);
static actionbutton_c configwin_config_files_icon_close(CONFIGWIN_CONFIG_FILES_ICON_CLOSE);
static writablefield_c configwin_config_files_icon_name(CONFIGWIN_CONFIG_FILES_ICON_NAME);
static button_c configwin_config_files_icon_name_drop(CONFIGWIN_CONFIG_FILES_ICON_NAME_DROP);
static button_c configwin_config_files_icon_name_icon(CONFIGWIN_CONFIG_FILES_ICON_NAME_ICON);
static label_c configwin_config_files_icon_name_label(CONFIGWIN_CONFIG_FILES_ICON_NAME_LABEL);
static optionbutton_c configwin_config_files_icon_relative(CONFIGWIN_CONFIG_FILES_ICON_RELATIVE);
static actionbutton_c configwin_config_files_icon_remove(CONFIGWIN_CONFIG_FILES_ICON_REMOVE);
static actionbutton_c configwin_config_files_icon_update(CONFIGWIN_CONFIG_FILES_ICON_UPDATE);
static actionbutton_c configwin_auto_icon_close(CONFIGWIN_AUTO_ICON_CLOSE);
static writablefield_c configwin_auto_icon_boot(CONFIGWIN_AUTO_ICON_BOOT);
static button_c configwin_auto_icon_boot_drop(CONFIGWIN_AUTO_ICON_BOOT_DROP);
static button_c configwin_auto_icon_boot_icon(CONFIGWIN_AUTO_ICON_BOOT_ICON);
static label_c configwin_auto_icon_boot_label(CONFIGWIN_AUTO_ICON_BOOT_LABEL);
static writablefield_c configwin_auto_icon_quit(CONFIGWIN_AUTO_ICON_QUIT);
static button_c configwin_auto_icon_quit_drop(CONFIGWIN_AUTO_ICON_QUIT_DROP);
static button_c configwin_auto_icon_quit_icon(CONFIGWIN_AUTO_ICON_QUIT_ICON);
static label_c configwin_auto_icon_quit_label(CONFIGWIN_AUTO_ICON_QUIT_LABEL);
static writablefield_c configwin_auto_icon_load(CONFIGWIN_AUTO_ICON_LOAD);
static button_c configwin_auto_icon_load_drop(CONFIGWIN_AUTO_ICON_LOAD_DROP);
static button_c configwin_auto_icon_load_icon(CONFIGWIN_AUTO_ICON_LOAD_ICON);
static label_c configwin_auto_icon_load_label(CONFIGWIN_AUTO_ICON_LOAD_LABEL);
static writablefield_c configwin_auto_icon_start(CONFIGWIN_AUTO_ICON_START);
static button_c configwin_auto_icon_start_drop(CONFIGWIN_AUTO_ICON_START_DROP);
static button_c configwin_auto_icon_start_icon(CONFIGWIN_AUTO_ICON_START_ICON);
static label_c configwin_auto_icon_start_label(CONFIGWIN_AUTO_ICON_START_LABEL);
static writablefield_c configwin_auto_icon_exit(CONFIGWIN_AUTO_ICON_EXIT);
static button_c configwin_auto_icon_exit_drop(CONFIGWIN_AUTO_ICON_EXIT_DROP);
static button_c configwin_auto_icon_exit_icon(CONFIGWIN_AUTO_ICON_EXIT_ICON);
static label_c configwin_auto_icon_exit_label(CONFIGWIN_AUTO_ICON_EXIT_LABEL);

// Configuration utility details
static wimp_t configwin_edit_handle = 0;
static multiedit_details *configwin_edit = NULL;

// File list pane details
#define CONFIGWIN_FILES_HEIGHT 40
static os_box configwin_files_extent;
static string configwin_files_partition = "";
static multiconf_file *configwin_file = NULL;

/*
    Parameters  : void
    Returns     : void
    Description : Set the current configuration from all the gadgets.
*/
void configwin_read(void)
{
    // Only read the gadgets if active */
    if (configwin_win_open)
    {
        // Read the front-end window
        configfile_frontend_auto_quit = configwin_frontend_icon_auto_quit();

        // Read the speed control window
        if (configwin_speed_icon_fore())
        {
            configfile_speed_fore = configwin_speed_icon_fore_val();
        }
        else configfile_speed_fore = FALSE;
        if (configwin_speed_icon_back())
        {
            configfile_speed_back = configwin_speed_icon_back_val();
        }
        else configfile_speed_back = FALSE;

        // Read the paths window
        configfile_path_pc = configwin_path_icon_pc();
        configfile_path_config = configwin_path_icon_config();
        configfile_path_partition = configwin_path_icon_partition();

        // Read the auto window
        configfile_auto_boot = configwin_auto_icon_boot();
        configfile_auto_quit = configwin_auto_icon_quit();
        configfile_auto_load = configwin_auto_icon_load();
        configfile_auto_start = configwin_auto_icon_start();
        configfile_auto_exit = configwin_auto_icon_exit();
    }
}

/*
    Parameters  : item  - The item to redraw, or NULL to update the whole
                          window.
    Returns     : void
    Description : Force either a complete or partitial redraw of the files list
                  pane window.
*/
static void configwin_files_force_redraw(multiconf_file *item)
{
    os_box box;

    // Find the window size
    window_get_extent(0, configwin_config_files_pane, &box);

    // Should all of the window be redrawn, or just a single item
    if (item)
    {
        multiconf_file *ptr = multiedit_select->ptr->detail->files;
        int y = 0;

        // Find the item to redraw
        while (ptr && (ptr != item))
        {
            y -= CONFIGWIN_FILES_HEIGHT;
            ptr = ptr->next;
        }

        // Force a limited redraw
        if (ptr)
        {
            box.y0 = y - CONFIGWIN_FILES_HEIGHT;
            box.y1 = y;
            window_force_redraw(0, configwin_config_files_pane, &box);
        }
    }
    else
    {
        // Redraw the whole pane
        window_force_redraw(0, configwin_config_files_pane, &box);
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Check if the partition path used for configuration files has
                  changed. If it has then force a complete redraw of the paths
                  window.
*/
static void configwin_files_check_redraw(void)
{
    string str;

    // Get the new partition path
    if ((multiedit_select->ptr != MULTICONF_ACTIVE)
        && !multiedit_select->ptr->detail->partition.empty())
    {
        str = multiedit_select->ptr->detail->partition;
    }
    else str = configfile_path_partition;
    str += ".";

    // Force an update if required
    if (str != configwin_files_partition)
    {
        configwin_files_partition = str;
        configwin_files_force_redraw(NULL);
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Update any dependencies between gadgets, such as greying
                  out icons.
*/
void configwin_update(void)
{
    int other;
    char str[256];
    multiedit_details *ptr;

    // Update any external edits
    ptr = multiedit_head;
    while (ptr)
    {
        if (!ptr->edit->active)
        {
            delete ptr->edit;
            ptr->edit = NULL;
        }
        ptr = ptr->next;
    }

    // Read the main configuration details from the windows
    configwin_read();

    // Grey out the speed control icons if required
    configwin_speed_icon_fore_val.set_faded(!configwin_speed_icon_fore());
    configwin_speed_icon_back_val.set_faded(!configwin_speed_icon_back());

    // Grey out configuration buttons
    configwin_configs_icon_name.set_faded(multiedit_select
                                          == &multiedit_current);
    configwin_configs_icon_config.set_faded(configwin_edit
                                            || multiedit_select->edit);
    configwin_configs_icon_edit.set_faded((configwin_edit == multiedit_select)
                                          || multiedit_select->edit);
    configwin_configs_icon_paths.set_faded(multiedit_select
                                           == &multiedit_current);
    configwin_configs_icon_dos.set_faded(multiedit_select
                                         == &multiedit_current);

    // Hide unsuitable windows
    if ((multiedit_select == &multiedit_current)
        || !(toolbox_get_object_info(0, configwin_configs)
             & toolbox_INFO_SHOWING))
    {
        toolbox_hide_object(0, configwin_config_paths);
        toolbox_hide_object(0, configwin_config_files);
    }

    // Grey out configuration paths
    other = configwin_config_paths_icon_other_fe();
    configwin_config_paths_icon_pc.set_faded(!other);
    configwin_config_paths_icon_pc_drop.set_faded(!other);
    configwin_config_paths_icon_pc_icon.set_faded(!other);
    configwin_config_paths_icon_pc_label.set_faded(!other);
    configwin_config_paths_icon_config.set_faded(!other);
    configwin_config_paths_icon_config_drop.set_faded(!other);
    configwin_config_paths_icon_config_icon.set_faded(!other);
    configwin_config_paths_icon_config_label.set_faded(!other);
    if (!other)
    {
        configwin_config_paths_icon_pc = configfile_path_pc;
        configwin_config_paths_icon_config = configfile_path_config;
    }
    other = configwin_config_paths_icon_other_part();
    configwin_config_paths_icon_partition.set_faded(!other);
    configwin_config_paths_icon_partition_drop.set_faded(!other);
    configwin_config_paths_icon_partition_icon.set_faded(!other);
    configwin_config_paths_icon_partition_label.set_faded(!other);
    if (!other)
    {
        configwin_config_paths_icon_partition = configfile_path_partition;
    }

    // Grey out configuration files
    configwin_config_files_icon_relative.set_faded(!configwin_file);
    configwin_config_files_icon_remove.set_faded(!configwin_file);
    configwin_config_files_icon_update.set_faded((multiedit_select->ptr
                                                  == MULTICONF_ACTIVE)
                                                 || !multiedit_select->ptr->detail->files);
    configwin_config_files_icon_name = configwin_file
                                       ? configwin_file->name : string("");
    configwin_config_files_icon_relative = configwin_file
                                           ? configwin_file->relative : FALSE;

    // Update the configurations menu
    menu_set_fade(0, configwin_config_menu, CONFIGWIN_CONFIGS_MENU_DELETE,
                  multiedit_select == &multiedit_current);
    multiconf_refresh_menu(configwin_config_menu, multiedit_select->ptr);

    // Set the configuration name
    menu_get_entry_text(0, configwin_config_menu,
                        (toolbox_c) multiedit_select->ptr, str, sizeof(str));
    configwin_configs_icon_name = str;

    // Set the file list pane window extent
    if (multiedit_select != &multiedit_current)
    {
        multiconf_file *ptr = multiedit_select->ptr->detail->files;
        int y = 0;
        os_box box;
        os_box extent;

        // Count the number of files to list
        while (ptr)
        {
            y += CONFIGWIN_FILES_HEIGHT;
            ptr = ptr->next;
        }

        // Set the window extent
        box = configwin_files_extent;
        box.y0 = MIN(box.y0, box.y1 - y);
        window_get_extent(0, configwin_config_files_pane, &extent);
        if (memcmp(&extent, &box, sizeof(os_box))
            && (toolbox_get_object_info(0, configwin_config_files_pane)
                & toolbox_INFO_SHOWING))
        {
            window_set_extent(0, configwin_config_files_pane, &box);
            toolbox_show_object(0, configwin_config_files,
                                toolbox_POSITION_DEFAULT, NULL, 0, 0);
        }
    }

    // Redraw the file list window
    configwin_files_check_redraw();
}

/*
    Parameters  : void
    Returns     : void
    Description : Set all the gadgets from the current configuration.
*/
void configwin_write(void)
{
    // Update the front-end window
    configwin_frontend_icon_auto_quit = configfile_frontend_auto_quit;

    // Update the speed control window
    configwin_speed_icon_fore = BOOL(configfile_speed_fore);
    if (configfile_speed_fore)
    {
        configwin_speed_icon_fore_val = configfile_speed_fore;
    }
    configwin_speed_icon_back = BOOL(configfile_speed_back);
    if (configfile_speed_back)
    {
        configwin_speed_icon_back_val = configfile_speed_back;
    }

    // Update the paths window
    configwin_path_icon_pc = configfile_path_pc;
    configwin_path_icon_config = configfile_path_config;
    configwin_path_icon_partition = configfile_path_partition;

    // Update the auto window
    configwin_auto_icon_boot = configfile_auto_boot;
    configwin_auto_icon_quit = configfile_auto_quit;
    configwin_auto_icon_load = configfile_auto_load;
    configwin_auto_icon_start = configfile_auto_start;
    configwin_auto_icon_exit = configfile_auto_exit;

    // Update any other icons required
    configwin_update();
}

/*
    Parameters  : object    - The object to show.
    Returns     : void
    Description : Show the specified object. If the object is already
                  visible then it is just brought to the front, otherwise
                  it is positioned relative to the mouse pointer.
*/
void configwin_show(toolbox_o object)
{
    // Check if the object is already visible
    if (toolbox_get_object_info(0, object) & toolbox_INFO_SHOWING)
    {
        // Just bring it to the front
        xtoolbox_show_object(0, object, toolbox_POSITION_DEFAULT, NULL,
                             toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);
    }
    else
    {
        toolbox_position pos;
        wimp_pointer ptr;
        os_box extent;

        // Read the current pointer position
        wimp_get_pointer_info(&ptr);

        // Get the window object size
        window_get_extent(0, object, &extent);

        // Choose a suitable position
        pos.top_left.x = ptr.pos.x - extent.x1 + extent.x0
                         + CONFIGWIN_OFFSET_X;
        pos.top_left.y = ptr.pos.y + extent.y1 - extent.y0
                         + CONFIGWIN_OFFSET_Y;

        // Open the window
        xtoolbox_show_object(0, object, toolbox_POSITION_TOP_LEFT, &pos,
                             toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Update configuration details if the paths window has been
                  changed.
*/
static void configwin_config_paths_change(void)
{
    // Set the configuration from the window
    if (configwin_config_paths_icon_other_fe())
    {
        multiedit_select->ptr->detail->pc
            = configwin_config_paths_icon_pc();
        multiedit_select->ptr->detail->config
            = configwin_config_paths_icon_config();
    }
    else
    {
        multiedit_select->ptr->detail->pc = "";
        multiedit_select->ptr->detail->config = "";
    }
    if (configwin_config_paths_icon_other_part())
    {
        multiedit_select->ptr->detail->partition
            = configwin_config_paths_icon_partition();
    }
    else
    {
        multiedit_select->ptr->detail->partition = "";
    }
}

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle clicks in the main window.
*/
static bool configwin_main_click(wimp_event_no event_code, wimp_block *block,
                                 toolbox_block *id_block, void *handle)
{
    toolbox_o object;

    NOT_USED(event_code);
    NOT_USED(handle);

    // Open the required window
    switch (id_block->this_cmp)
    {
        case CONFIGWIN_MAIN_ICON_FRONTEND:
            // Open the front-end configuration window
            object = configwin_frontend;
            break;

        case CONFIGWIN_MAIN_ICON_SPEED:
            // Open the speed control configuration window
            object = configwin_speed;
            break;

        case CONFIGWIN_MAIN_ICON_PATH:
            // Open the paths configuration window
            object = configwin_path;
            break;

        case CONFIGWIN_MAIN_ICON_CONFIGS:
            // Open the multiple configurations configuration window
            object = configwin_configs;
            multiedit_backup();
            break;

        case CONFIGWIN_MAIN_ICON_AUTO:
            // Open the auto run window
            object = configwin_auto;
            break;

        default:
            // Do not care about any other clicks
            object = toolbox_NULL_OBJECT;
            break;
    }

    // Open the required window
    if (object != toolbox_NULL_OBJECT)
    {
        // Open the required window
        configwin_show(object);

        // Close the main window if the adjust button is used
        if (block->pointer.buttons == wimp_CLICK_ADJUST)
        {
            toolbox_hide_object(0, configwin_main);
        }
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : task      - The task handle of the configuration utility.
                  handle    - The configuration being edited.
    Returns     : void
    Description : The configuration utility has been started.
*/
static void configwin_edit_util_started(wimp_t task, void *handle)
{
    // Store the task handle
    configwin_edit_handle = task;

    // Recover if failed to start
    if (!task)
    {
        // No longer need to fake the old behaviour
        pcpro_fake_old = FALSE;

        // Restore the original configuration file
        multiconf_copy(((multiedit_details *) handle)->ptr, MULTICONF_BACKUP,
                       MULTICONF_ACTIVE, TRUE, FALSE);
        configwin_edit = NULL;
    }
    else configwin_edit = (multiedit_details *) handle;

    // Update the window
    configwin_update();
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle TaskCloseDown wimp message events.
*/
static bool configwin_task_close(wimp_message *message, void *handle)
{
    NOT_USED(handle);

    // Is it the configuration editor
    if (message->sender == configwin_edit_handle)
    {
        // No longer need to fake the old behaviour
        pcpro_fake_old = FALSE;

        // Copy the configuration back and restore the original
        if (configwin_edit != &multiedit_current)
        {
            multiconf_copy(configwin_edit->ptr, MULTICONF_ACTIVE,
                           configwin_edit->ptr, TRUE, FALSE);
            multiconf_copy(configwin_edit->ptr, MULTICONF_BACKUP,
                           MULTICONF_ACTIVE, TRUE, FALSE);
        }

        // Update the status
        configwin_edit_handle = 0;
        configwin_edit = NULL;

        // Update the window
        configwin_update();
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle clicks in the configurations window.
*/
static bool configwin_configs_click(wimp_event_no event_code, wimp_block *block,
                                    toolbox_block *id_block, void *handle)
{
    string path;
    char str[256];

    NOT_USED(event_code);
    NOT_USED(handle);
    NOT_USED(block);

    // Open the required window
    switch (id_block->this_cmp)
    {
        case CONFIGWIN_CONFIGS_ICON_CONFIG:
            // Set the required !PC configuration
            if ((multiedit_select->ptr != MULTICONF_ACTIVE)
                && !multiedit_select->ptr->detail->pc.empty())
            {
                path = multiedit_select->ptr->detail->pc;
            }
            else path = configfile_path_pc;
            path += ".!Boot";
            xos_cli(path.c_str());

            // Set the selected configuration if required
            if (multiedit_select != &multiedit_current)
            {
                // Store the current configuration somewhere safe
                if (multiconf_copy(multiedit_select->ptr, MULTICONF_ACTIVE,
                                   MULTICONF_BACKUP, TRUE, FALSE)) return TRUE;

                // Copy the new configuration file
                if (multiconf_copy(multiedit_select->ptr, multiedit_select->ptr,
                                   MULTICONF_ACTIVE, TRUE, FALSE)) return TRUE;
            }

            // Fake the old style behaviour if requires
            pcpro_fake_old = TRUE;

            // Start the configuration utility
            go_start_task((multiedit_select->ptr != MULTICONF_ACTIVE)
                          && !multiedit_select->ptr->detail->config.empty()
                          ? multiedit_select->ptr->detail->config.c_str()
                          : configfile_path_config.c_str(),
                          configwin_edit_util_started, multiedit_select);
            break;

        case CONFIGWIN_CONFIGS_ICON_EDIT:
            // Start an external edit on this configuration
            multiconf_name_config(NULL, multiedit_select->ptr, str);
            multiedit_select->edit = new ExtEdit(str,
                                                 multiedit_select
                                                 == &multiedit_current
                                                 ? "Current configuration"
                                                 : multiedit_select->ptr->name.c_str(),
                                                 "ARMEdit Config", TRUE);
            configwin_update();
            break;

        case CONFIGWIN_CONFIGS_ICON_PATHS:
            // Open the configuration paths window
            configwin_show(configwin_config_paths);
            break;

        case CONFIGWIN_CONFIGS_ICON_DOS:
            // Open the configuration files window
            configwin_show(configwin_config_files);
            break;
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle clicks in the files list window.
*/
static bool configwin_configs_files_click(wimp_event_no event_code,
                                          wimp_block *block,
                                          toolbox_block *id_block,
                                          void *handle)
{
    wimp_window_state state;
    multiconf_file *ptr = multiedit_select->ptr->detail->files;
    int y;

    NOT_USED(event_code);
    NOT_USED(handle);
    NOT_USED(id_block);

    // Get the window details
    state.w = block->pointer.w;
    wimp_get_window_state(&state);

    // Calculate the window relative coordinates
    y = block->pointer.pos.y + state.yscroll - state.visible.y1;

    // Find the corresponding file record
    while (ptr && (y < -CONFIGWIN_FILES_HEIGHT))
    {
        y += CONFIGWIN_FILES_HEIGHT;
        ptr = ptr->next;
    }

    // Only change the selection if over a file entry
    if (ptr)
    {
        // Update the file list and selection
        if (configwin_file) configwin_files_force_redraw(configwin_file);
        if (configwin_file == ptr) configwin_file = NULL;
        else
        {
            configwin_file = ptr;
            configwin_files_force_redraw(configwin_file);
        }

        // Perform any other updates required
        configwin_update();
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : A sub-window action button has been clicked. This normally
                  reopens the main configuration window.
*/
static bool configwin_sub_action(bits event_code, toolbox_action *action,
                                 toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);
    NOT_USED(id_block);

    // Was the button a local one
    if (action->flags & actionbutton_SELECTED_LOCAL)
    {
        // Decode the gadget IDs
        if ((id_block->this_obj == configwin_speed)
            && (id_block->this_cmp == CONFIGWIN_SPEED_ICON_READ))
        {
            // Update the icons from the current settings
            int fore, back;

            // Read the current settings
            if (!xarmedit_polling(-1, -1, &fore, &back))
            {
                // Update the icons
                configwin_speed_icon_fore = BOOL(fore);
                if (fore) configwin_speed_icon_fore_val = fore;
                configwin_speed_icon_back = BOOL(back);
                if (back) configwin_speed_icon_back_val = back;
            }

            // Set the configuration modified flag
            configfile_modified = TRUE;
        }
    }
    else
    {
        // Show the main window
        configwin_show(configwin_main);
    }

    // Update the other windows if required
    configwin_update();

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : A sub-window action button has been clicked. This normally
                  reopens the main configuration window.
*/
static bool configwin_files_action(bits event_code, toolbox_action *action,
                                   toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);
    NOT_USED(id_block);

    // Was the button a local one
    if (action->flags & actionbutton_SELECTED_LOCAL)
    {
        // Decode the gadget IDs
        if ((id_block->this_obj == configwin_config_files)
            && (id_block->this_cmp == CONFIGWIN_CONFIG_FILES_ICON_REMOVE))
        {
            // Is any file selected
            if (configwin_file)
            {
                // Remove the currently selected file from the list
                if (configwin_file->next)
                {
                    configwin_file->next->prev = configwin_file->prev;
                }
                if (configwin_file->prev)
                {
                    configwin_file->prev->next = configwin_file->next;
                }
                else
                {
                    multiedit_select->ptr->detail->files = configwin_file->next;
                }
                delete configwin_file;
                configwin_file = NULL;
                configwin_files_force_redraw(NULL);

                // Set the configuration modified flag
                configfile_modified = TRUE;
            }
        }
        else if ((id_block->this_obj == configwin_config_files)
                 && (id_block->this_cmp == CONFIGWIN_CONFIG_FILES_ICON_UPDATE))
        {
            // Update the copies of the files store with the configuration
            multiconf_copy(multiedit_select->ptr, MULTICONF_ACTIVE,
                           multiedit_select->ptr, FALSE, TRUE);

            // Set the configuration modified flag
            configfile_modified = TRUE;
        }

        // Update the other windows if required
        configwin_update();
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Handle changes to a file associated with a configuration.
*/
static void configwin_files_change(void)
{
    // Add a new file record if required
    if (!configwin_file)
    {
        multiconf_file *ptr;

        // Create a new record
        configwin_file = new multiconf_file;
        configwin_file->id = 0;
        ptr = multiedit_select->ptr->detail->files;
        while (ptr)
        {
            if (ptr->id == configwin_file->id)
            {
                configwin_file->id++;
                ptr = multiedit_select->ptr->detail->files;
            }
            else ptr = ptr->next;
        }

        configwin_file->next = multiedit_select->ptr->detail->files;
        configwin_file->prev = NULL;
        if (configwin_file->next) configwin_file->next->prev = configwin_file;
        multiedit_select->ptr->detail->files = configwin_file;
    }

    // Read the name and relative flag
    configwin_file->name = configwin_config_files_icon_name();
    configwin_file->relative = configwin_config_files_icon_relative();

    // Correct the file name if required
    // ...

    // Update the file list window
    configwin_files_force_redraw(NULL);
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Update items and modified flags when something changes.
*/
static bool configwin_modified(bits event_code, toolbox_action *action,
                               toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // Update configurations if required
    if (id_block->this_obj == configwin_config_paths)
    {
        // Configuration paths
        configwin_config_paths_change();
    }
    else if (id_block->this_obj == configwin_config_files)
    {
        // Configuration files
        configwin_files_change();
    }

    // Update any icon dependencies
    configwin_update();

    // Set the configuration modified flag
    configfile_modified = TRUE;

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : A configuration name has been changed.
*/
static bool configwin_name_changed(bits event_code, toolbox_action *action,
                                   toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);
    NOT_USED(id_block);

    // Read the new name
    multiedit_select->ptr->name = configwin_configs_icon_name();

    // Update any icon dependencies
    configwin_update();

    // Set the configuration modified flag
    configfile_modified = TRUE;

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : A menu selection has been made.
*/
static bool configwin_selection(bits event_code, toolbox_action *action,
                                toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    switch (id_block->this_cmp)
    {
        case CONFIGWIN_CONFIGS_MENU_CURRENT:
            // The current configuration
            multiedit_select = &multiedit_current;
            break;

        case CONFIGWIN_CONFIGS_MENU_COPY:
            // Copy the selected configuration
            multiconf_destroy_menu(configwin_config_menu);
            multiedit_select = multiedit_copy(multiedit_select);
            multiconf_build_menu(configwin_config_menu, NULL, NULL);
            configfile_modified = TRUE;
            break;

        case CONFIGWIN_CONFIGS_MENU_DELETE:
            // Delete the selected configuration
            multiconf_destroy_menu(configwin_config_menu);
            multiedit_delete(multiedit_select);
            multiedit_select = &multiedit_current;
            multiconf_build_menu(configwin_config_menu, NULL, NULL);
            configfile_modified = TRUE;
            break;

        default:
            // It is a configuration selection
            multiedit_select = (multiedit_details *)
                               ((multiconf_list *) id_block->this_cmp)->handle;
            if (!multiedit_select->ptr->detail->pc.empty())
            {
                configwin_config_paths_icon_other_fe = TRUE;
                configwin_config_paths_icon_pc
                    = multiedit_select->ptr->detail->pc;
                configwin_config_paths_icon_config
                    = multiedit_select->ptr->detail->config;
            }
            else configwin_config_paths_icon_default_fe = TRUE;
            if (!multiedit_select->ptr->detail->partition.empty())
            {
                configwin_config_paths_icon_other_part = TRUE;
                configwin_config_paths_icon_partition
                    = multiedit_select->ptr->detail->partition;
            }
            else configwin_config_paths_icon_default_part = TRUE;
            configwin_files_force_redraw(NULL);
            break;
    }

    // Deselect any selected file
    configwin_file = NULL;

    // Update any icon dependencies
    configwin_update();

    // Claim the event
    return TRUE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Reset the configuration windows from the current
                  configuration.
*/
static void configwin_reset(void)
{
    // Destroy any modified multiple configurations
    if (!configwin_win_first)
    {
        multiconf_destroy_menu(configwin_config_menu);
        toolbox_hide_object(0, configwin_configs);
    }

    // Clear the first time flag
    configwin_win_first = FALSE;

    // Set the required window contents
    multiedit_load();
    multiedit_select = &multiedit_current;
    multiconf_build_menu(configwin_config_menu, NULL, NULL);
    configwin_write();
}

/*
    Parameters  : void
    Returns     : void
    Description : Open the configuration window or move the window to the
                  front if it is already open.
*/
void configwin_open(void)
{
    // Extra steps if the window is not currently open
    if (!configwin_win_open)
    {
        // Update the windows from the current configuration
        configwin_reset();

        // Update the flag to mark the window as open
        configwin_win_open = TRUE;
    }

    // Open the configuration window
    configwin_show(configwin_main);
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : A main action button has been clicked, so perform a relevant
                  action.
*/
static bool configwin_action(bits event_code, toolbox_action *action,
                             toolbox_block *id_block, void *handle)
{
    wimp_message msg;

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);
    NOT_USED(id_block);

    // Action depends upon which button was clicked
    switch (id_block->this_cmp)
    {
        case CONFIGWIN_MAIN_ICON_SAVE:
            // Save the configuration
            configwin_read();
            configfile_write();
            multiedit_save();
            toolbox_hide_object(0, configwin_configs);

            // Send Message_ARMEditConfigSaved to reload configuration
            msg.size = sizeof(wimp_message);
            msg.your_ref = 0;
            msg.action = message_ARMEDIT_CONFIG_SAVED;
            wimp_send_message(wimp_USER_MESSAGE, &msg, wimp_BROADCAST);
            break;

        case CONFIGWIN_MAIN_ICON_CANCEL:
            // Update the icons from the original configuration
            configfile_read();
            configwin_reset();
            break;

        default:
            // No other action buttons should exist
            break;
    }

    // Quit the program if the adjust button not used
    if (!(action->flags & actionbutton_SELECTED_ADJUST))
    {
        quit_quit();
    }

    // Update the other windows if required
    configwin_update();

    // Claim the event
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle DataLoad wimp message events.
*/
static bool configwin_load(wimp_message *message, void *handle)
{
    toolbox_o obj;
    toolbox_c cmp;

    NOT_USED(handle);

    // Convert the details to toolbox identifiers
    window_wimp_to_toolbox(0, message->data.data_xfer.w,
                           message->data.data_xfer.i, &obj, &cmp);

    // Action depends upon the window
    if (obj == configwin_path)
    {
        // Decode the gadget ID
        switch (cmp)
        {
            case CONFIGWIN_PATH_ICON_PC:
            case CONFIGWIN_PATH_ICON_PC_DROP:
            case CONFIGWIN_PATH_ICON_PC_ICON:
            case CONFIGWIN_PATH_ICON_PC_LABEL:
                // The PC card software
                if (message->data.data_xfer.file_type
                    == osfile_TYPE_APPLICATION)
                {
                    // Set the writable field
                    configwin_path_icon_pc
                        = message->data.data_xfer.file_name;

                    // Set the modified flag
                    configwin_update();
                    configfile_modified = TRUE;
                }
                break;

            case CONFIGWIN_PATH_ICON_CONFIG:
            case CONFIGWIN_PATH_ICON_CONFIG_DROP:
            case CONFIGWIN_PATH_ICON_CONFIG_ICON:
            case CONFIGWIN_PATH_ICON_CONFIG_LABEL:
                // The PC card configuration software
                if (message->data.data_xfer.file_type
                    == osfile_TYPE_APPLICATION)
                {
                    // Set the writable field
                    configwin_path_icon_config
                        = message->data.data_xfer.file_name;

                    // Set the modified flag
                    configwin_update();
                    configfile_modified = TRUE;
                }
                break;

            case CONFIGWIN_PATH_ICON_PARTITION:
            case CONFIGWIN_PATH_ICON_PARTITION_DROP:
            case CONFIGWIN_PATH_ICON_PARTITION_ICON:
            case CONFIGWIN_PATH_ICON_PARTITION_LABEL:
                // The DOS partition
                if (message->data.data_xfer.file_type == osfile_TYPE_DOS_DISC)
                {
                    // Set the writable field
                    configwin_path_icon_partition
                        = message->data.data_xfer.file_name;

                    // Set the modified flag
                    configwin_update();
                    configfile_modified = TRUE;
                }
                break;

            default:
                // Do not care about other gadgets
                break;
        }
    }
    else if (obj == configwin_config_paths)
    {
        // Decode the gadget ID
        switch (cmp)
        {
            case CONFIGWIN_CONFIG_PATHS_ICON_PC:
            case CONFIGWIN_CONFIG_PATHS_ICON_PC_DROP:
            case CONFIGWIN_CONFIG_PATHS_ICON_PC_ICON:
            case CONFIGWIN_CONFIG_PATHS_ICON_PC_LABEL:
                // The PC card software
                if (message->data.data_xfer.file_type
                    == osfile_TYPE_APPLICATION)
                {
                    // Set the writable field
                    configwin_config_paths_icon_pc
                        = message->data.data_xfer.file_name;
                    configwin_config_paths_change();

                    // Set the modified flag
                    configwin_update();
                    configfile_modified = TRUE;
                }
                break;

            case CONFIGWIN_CONFIG_PATHS_ICON_CONFIG:
            case CONFIGWIN_CONFIG_PATHS_ICON_CONFIG_DROP:
            case CONFIGWIN_CONFIG_PATHS_ICON_CONFIG_ICON:
            case CONFIGWIN_CONFIG_PATHS_ICON_CONFIG_LABEL:
                // The PC card configuration software
                if (message->data.data_xfer.file_type
                    == osfile_TYPE_APPLICATION)
                {
                    // Set the writable field
                    configwin_config_paths_icon_config
                        = message->data.data_xfer.file_name;
                    configwin_config_paths_change();

                    // Set the modified flag
                    configwin_update();
                    configfile_modified = TRUE;
                }
                break;

            case CONFIGWIN_CONFIG_PATHS_ICON_PARTITION:
            case CONFIGWIN_CONFIG_PATHS_ICON_PARTITION_DROP:
            case CONFIGWIN_CONFIG_PATHS_ICON_PARTITION_ICON:
            case CONFIGWIN_CONFIG_PATHS_ICON_PARTITION_LABEL:
                // The DOS partition
                if (message->data.data_xfer.file_type == osfile_TYPE_DOS_DISC)
                {
                    // Set the writable field
                    configwin_config_paths_icon_partition
                        = message->data.data_xfer.file_name;
                    configwin_config_paths_change();

                    // Set the modified flag
                    configwin_update();
                    configfile_modified = TRUE;
                }
                break;

            default:
                // Do not care about other gadgets
                break;
        }
    }
    else if (obj == configwin_config_files)
    {
        // Decode the gadget ID
        switch (cmp)
        {
            case CONFIGWIN_CONFIG_FILES_ICON_NAME:
            case CONFIGWIN_CONFIG_FILES_ICON_NAME_DROP:
            case CONFIGWIN_CONFIG_FILES_ICON_NAME_ICON:
            case CONFIGWIN_CONFIG_FILES_ICON_NAME_LABEL:
                // The DOS file name
                if ((message->data.data_xfer.file_type
                     != osfile_TYPE_DIR)
                    && (message->data.data_xfer.file_type
                        != osfile_TYPE_APPLICATION))
                {
                    // Set the writable field
                    configwin_config_files_icon_name
                        = message->data.data_xfer.file_name;
                    configwin_config_files_icon_relative = FALSE;
                    configwin_files_change();

                    // Set the modified flag
                    configwin_update();
                    configfile_modified = TRUE;
                }
                break;

            default:
                // Do not care about other gadgets
                break;
        }
    }
    else if (obj == configwin_auto)
    {
        // Decode the gadget ID
        switch (cmp)
        {
            case CONFIGWIN_AUTO_ICON_BOOT:
            case CONFIGWIN_AUTO_ICON_BOOT_DROP:
            case CONFIGWIN_AUTO_ICON_BOOT_ICON:
            case CONFIGWIN_AUTO_ICON_BOOT_LABEL:
                // The PC card software
                if (message->data.data_xfer.file_type
                    == osfile_TYPE_OBEY)
                {
                    // Set the writable field
                    configwin_auto_icon_boot
                        = message->data.data_xfer.file_name;

                    // Set the modified flag
                    configwin_update();
                    configfile_modified = TRUE;
                }
                break;

            case CONFIGWIN_AUTO_ICON_QUIT:
            case CONFIGWIN_AUTO_ICON_QUIT_DROP:
            case CONFIGWIN_AUTO_ICON_QUIT_ICON:
            case CONFIGWIN_AUTO_ICON_QUIT_LABEL:
                // The PC card software
                if (message->data.data_xfer.file_type
                    == osfile_TYPE_OBEY)
                {
                    // Set the writable field
                    configwin_auto_icon_quit
                        = message->data.data_xfer.file_name;

                    // Set the modified flag
                    configwin_update();
                    configfile_modified = TRUE;
                }
                break;

            case CONFIGWIN_AUTO_ICON_LOAD:
            case CONFIGWIN_AUTO_ICON_LOAD_DROP:
            case CONFIGWIN_AUTO_ICON_LOAD_ICON:
            case CONFIGWIN_AUTO_ICON_LOAD_LABEL:
                // The PC card software
                if (message->data.data_xfer.file_type
                    == osfile_TYPE_OBEY)
                {
                    // Set the writable field
                    configwin_auto_icon_load
                        = message->data.data_xfer.file_name;

                    // Set the modified flag
                    configwin_update();
                    configfile_modified = TRUE;
                }
                break;

            case CONFIGWIN_AUTO_ICON_START:
            case CONFIGWIN_AUTO_ICON_START_DROP:
            case CONFIGWIN_AUTO_ICON_START_ICON:
            case CONFIGWIN_AUTO_ICON_START_LABEL:
                // The PC card software
                if (message->data.data_xfer.file_type
                    == osfile_TYPE_OBEY)
                {
                    // Set the writable field
                    configwin_auto_icon_start
                        = message->data.data_xfer.file_name;

                    // Set the modified flag
                    configwin_update();
                    configfile_modified = TRUE;
                }
                break;

            case CONFIGWIN_AUTO_ICON_EXIT:
            case CONFIGWIN_AUTO_ICON_EXIT_DROP:
            case CONFIGWIN_AUTO_ICON_EXIT_ICON:
            case CONFIGWIN_AUTO_ICON_EXIT_LABEL:
                // The PC card software
                if (message->data.data_xfer.file_type
                    == osfile_TYPE_OBEY)
                {
                    // Set the writable field
                    configwin_auto_icon_exit
                        = message->data.data_xfer.file_name;

                    // Set the modified flag
                    configwin_update();
                    configfile_modified = TRUE;
                }
                break;

            default:
                // Do not care about other gadgets
                break;
        }
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Perform updates required by external events.
*/
static bool configwin_refresh(wimp_event_no event_code, wimp_block *block,
                                 toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(block);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Perform a refresh
    configwin_update();

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle wimp redraw requests.
*/
static bool configwin_redraw(wimp_event_no event_code, wimp_block *block,
                             toolbox_block *id_block, void *handle)
{
    int more;
    int y;

    NOT_USED(event_code);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Loop round all the rectangles to redraw
    more = wimp_redraw_window(&block->redraw);
    while (more)
    {
        int xoff = block->redraw.box.x0 - block->redraw.xscroll;
        int yoff = block->redraw.box.y1 - block->redraw.yscroll;
        multiconf_file *ptr = multiedit_select->ptr->detail->files;

        // Redraw this rectangle
        y = 0;
        while (ptr)
        {
            // Display this filename
            y -= CONFIGWIN_FILES_HEIGHT;
            if (ptr == configwin_file)
            {
                wimp_set_colour(wimp_COLOUR_BLACK);
                os_plot(os_MOVE_TO,
                        xoff + configwin_files_extent.x0, yoff + y);
                os_plot(os_PLOT_RECTANGLE | os_PLOT_BY,
                        configwin_files_extent.x1 - configwin_files_extent.x0,
                        CONFIGWIN_FILES_HEIGHT - 1);
                wimptextop_set_colour(os_COLOUR_WHITE, os_COLOUR_BLACK);
            }
            else
            {
                wimptextop_set_colour(os_COLOUR_BLACK, os_COLOUR_WHITE);
            }
            wimptextop_paint(wimptextop_RJUSTIFY | wimptextop_GIVEN_BASELINE,
                             ptr->name.c_str(),
                             xoff + configwin_files_extent.x1 - 8,
                             yoff + y + 8);
            if (ptr->relative)
            {
                wimptextop_set_colour(os_COLOUR_MID_DARK_GREY,
                                      ptr == configwin_file
                                      ? os_COLOUR_BLACK : os_COLOUR_WHITE);
                wimptextop_paint(wimptextop_RJUSTIFY
                                 | wimptextop_GIVEN_BASELINE,
                                 configwin_files_partition.c_str(),
                                 xoff + configwin_files_extent.x1 - 8
                                 - wimptextop_string_width(ptr->name.c_str(), 0),
                                 yoff + y + 8);
            }

            // Advance to the next name
            ptr = ptr->next;
        }

        // Find the next rectangle
        more = wimp_get_rectangle(&block->redraw);
    }

    // Claim the event
    return TRUE;
}

/*
    Parameters  : handle    - The user specified handle.
    Returns     : void
    Description : This function that is called after the user has responded to
                  a prompt to quit by selecting the continue option.
*/
static void configwin_quit_selected(void *handle)
{
    NOT_USED(handle);

    // Restore the previous configuration
    configfile_read();
    configwin_reset();
}

/*
    Parameters  : handle    - The user specified handle.
                  func      - Variable to receive a pointer to the function
                              to call if the user selects the quit option.
    Returns     : char *    - Prompt for the user, or NULL if no objection.
    Description : This function is called when the software is about to quit.
                  A string is returned if there is any objection.
*/
const char *configwin_quit_check(void *handle, quit_func_quit *func)
{
    NOT_USED(handle);

    // Set the quit function
    *func = configwin_quit_selected;

    // Return the string if required
    return configfile_modified && configwin_win_open
           ? lookup_token("QuitConfig") : NULL;
}

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the configuration windows.
*/
void configwin_initialise(void)
{
    // Register a quit handler
    quit_register(configwin_quit_check);

    // Create the windows
    configwin_main = toolbox_create_object(0, (toolbox_id) "MainWin");
    configwin_frontend = toolbox_create_object(0, (toolbox_id) "FrontWin");
    configwin_speed = toolbox_create_object(0, (toolbox_id) "SpeedWin");
    configwin_path = toolbox_create_object(0, (toolbox_id) "PathWin");
    configwin_configs = toolbox_create_object(0, (toolbox_id) "ConfigWin");
    configwin_config_paths = toolbox_create_object(0,
                                                   (toolbox_id) "CfgPathWin");
    configwin_config_files = toolbox_create_object(0, (toolbox_id)
                                                   "CfgFileWin");
    configwin_auto = toolbox_create_object(0, (toolbox_id) "AutoWin");

    // Attach the gadgets
    configwin_main_icon_save.object = configwin_main;
    configwin_main_icon_cancel.object = configwin_main;
    configwin_main_icon_frontend.object = configwin_main;
    configwin_main_icon_speed.object = configwin_main;
    configwin_main_icon_path.object = configwin_main;
    configwin_main_icon_configs.object = configwin_main;
    configwin_main_icon_auto.object = configwin_main;
    configwin_frontend_icon_close.object = configwin_frontend;
    configwin_frontend_icon_auto_quit.object = configwin_frontend;
    configwin_speed_icon_close.object = configwin_speed;
    configwin_speed_icon_fore.object = configwin_speed;
    configwin_speed_icon_fore_val.object = configwin_speed;
    configwin_speed_icon_back.object = configwin_speed;
    configwin_speed_icon_back_val.object = configwin_speed;
    configwin_speed_icon_read.object = configwin_speed;
    configwin_path_icon_close.object = configwin_path;
    configwin_path_icon_pc.object = configwin_path;
    configwin_path_icon_pc_drop.object = configwin_path;
    configwin_path_icon_pc_icon.object = configwin_path;
    configwin_path_icon_pc_label.object = configwin_path;
    configwin_path_icon_config.object = configwin_path;
    configwin_path_icon_config_drop.object = configwin_path;
    configwin_path_icon_config_icon.object = configwin_path;
    configwin_path_icon_config_label.object = configwin_path;
    configwin_path_icon_partition.object = configwin_path;
    configwin_path_icon_partition_drop.object = configwin_path;
    configwin_path_icon_partition_icon.object = configwin_path;
    configwin_path_icon_partition_label.object = configwin_path;
    configwin_configs_icon_close.object = configwin_configs;
    configwin_configs_icon_name.object = configwin_configs;
    configwin_configs_icon_menu.object = configwin_configs;
    configwin_configs_icon_config.object = configwin_configs;
    configwin_configs_icon_edit.object = configwin_configs;
    configwin_configs_icon_paths.object = configwin_configs;
    configwin_configs_icon_dos.object = configwin_configs;
    configwin_config_paths_icon_close.object = configwin_config_paths;
    configwin_config_paths_icon_default_fe.object = configwin_config_paths;
    configwin_config_paths_icon_other_fe.object = configwin_config_paths;
    configwin_config_paths_icon_pc.object = configwin_config_paths;
    configwin_config_paths_icon_pc_drop.object = configwin_config_paths;
    configwin_config_paths_icon_pc_icon.object = configwin_config_paths;
    configwin_config_paths_icon_pc_label.object = configwin_config_paths;
    configwin_config_paths_icon_config.object = configwin_config_paths;
    configwin_config_paths_icon_config_drop.object = configwin_config_paths;
    configwin_config_paths_icon_config_icon.object = configwin_config_paths;
    configwin_config_paths_icon_config_label.object = configwin_config_paths;
    configwin_config_paths_icon_default_part.object = configwin_config_paths;
    configwin_config_paths_icon_other_part.object = configwin_config_paths;
    configwin_config_paths_icon_partition.object = configwin_config_paths;
    configwin_config_paths_icon_partition_drop.object = configwin_config_paths;
    configwin_config_paths_icon_partition_icon.object = configwin_config_paths;
    configwin_config_paths_icon_partition_label.object = configwin_config_paths;
    configwin_config_files_icon_close.object = configwin_config_files;
    configwin_config_files_icon_name.object = configwin_config_files;
    configwin_config_files_icon_name_drop.object = configwin_config_files;
    configwin_config_files_icon_name_icon.object = configwin_config_files;
    configwin_config_files_icon_name_label.object = configwin_config_files;
    configwin_config_files_icon_relative.object = configwin_config_files;
    configwin_config_files_icon_remove.object = configwin_config_files;
    configwin_config_files_icon_update.object = configwin_config_files;
    configwin_auto_icon_close.object = configwin_auto;
    configwin_auto_icon_boot.object = configwin_auto;
    configwin_auto_icon_boot_drop.object = configwin_auto;
    configwin_auto_icon_boot_icon.object = configwin_auto;
    configwin_auto_icon_boot_label.object = configwin_auto;
    configwin_auto_icon_quit.object = configwin_auto;
    configwin_auto_icon_quit_drop.object = configwin_auto;
    configwin_auto_icon_quit_icon.object = configwin_auto;
    configwin_auto_icon_quit_label.object = configwin_auto;
    configwin_auto_icon_load.object = configwin_auto;
    configwin_auto_icon_load_drop.object = configwin_auto;
    configwin_auto_icon_load_icon.object = configwin_auto;
    configwin_auto_icon_load_label.object = configwin_auto;
    configwin_auto_icon_start.object = configwin_auto;
    configwin_auto_icon_start_drop.object = configwin_auto;
    configwin_auto_icon_start_icon.object = configwin_auto;
    configwin_auto_icon_start_label.object = configwin_auto;
    configwin_auto_icon_exit.object = configwin_auto;
    configwin_auto_icon_exit_drop.object = configwin_auto;
    configwin_auto_icon_exit_icon.object = configwin_auto;
    configwin_auto_icon_exit_label.object = configwin_auto;

    // Prepare the file list pane
    window_get_tool_bars(window_TOOL_BAR_ITL, configwin_config_files,
                         NULL, &configwin_config_files_pane, NULL, NULL);
    window_get_extent(0, configwin_config_files_pane, &configwin_files_extent);

    // Prepare the configurations menu
    configwin_config_menu = configwin_configs_icon_menu.get_show();

    // Register event handlers
    event_register_wimp_handler(configwin_main, wimp_MOUSE_CLICK,
                                configwin_main_click, NULL);
    event_register_wimp_handler(configwin_configs, wimp_MOUSE_CLICK,
                                configwin_configs_click, NULL);
    event_register_wimp_handler(event_ANY, wimp_POINTER_LEAVING_WINDOW,
                                configwin_refresh, NULL);
    event_register_wimp_handler(event_ANY, wimp_POINTER_ENTERING_WINDOW,
                                configwin_refresh, NULL);
    event_register_wimp_handler(configwin_config_files_pane,
                                wimp_REDRAW_WINDOW_REQUEST,
                                configwin_redraw, NULL);
    event_register_wimp_handler(configwin_config_files_pane, wimp_MOUSE_CLICK,
                                configwin_configs_files_click, NULL);
    event_register_toolbox_handler(configwin_main,
                                   action_ACTION_BUTTON_SELECTED,
                                   configwin_action, NULL);
    event_register_toolbox_handler(configwin_frontend,
                                   action_ACTION_BUTTON_SELECTED,
                                   configwin_sub_action, NULL);
    event_register_toolbox_handler(configwin_speed,
                                   action_ACTION_BUTTON_SELECTED,
                                   configwin_sub_action, NULL);
    event_register_toolbox_handler(configwin_path,
                                   action_ACTION_BUTTON_SELECTED,
                                   configwin_sub_action, NULL);
    event_register_toolbox_handler(configwin_configs,
                                   action_ACTION_BUTTON_SELECTED,
                                   configwin_sub_action, NULL);
    event_register_toolbox_handler(configwin_auto,
                                   action_ACTION_BUTTON_SELECTED,
                                   configwin_sub_action, NULL);
    event_register_toolbox_handler(configwin_frontend,
                                   action_OPTION_BUTTON_STATE_CHANGED,
                                   configwin_modified, NULL);
    event_register_toolbox_handler(configwin_speed,
                                   action_OPTION_BUTTON_STATE_CHANGED,
                                   configwin_modified, NULL);
    event_register_toolbox_handler(configwin_speed,
                                   action_NUMBER_RANGE_VALUE_CHANGED,
                                   configwin_modified, NULL);
    event_register_toolbox_handler(configwin_path,
                                   action_WRITABLE_FIELD_VALUE_CHANGED,
                                   configwin_modified, NULL);
    event_register_toolbox_handler(configwin_auto,
                                   action_WRITABLE_FIELD_VALUE_CHANGED,
                                   configwin_modified, NULL);
    event_register_toolbox_handler(configwin_configs,
                                   action_OPTION_BUTTON_STATE_CHANGED,
                                   configwin_modified, NULL);
    event_register_toolbox_handler(configwin_configs,
                                   action_WRITABLE_FIELD_VALUE_CHANGED,
                                   configwin_name_changed, NULL);
    event_register_toolbox_handler(configwin_config_menu,
                                   action_MENU_SELECTION,
                                   configwin_selection, NULL);
    event_register_toolbox_handler(configwin_config_paths,
                                   action_RADIO_BUTTON_STATE_CHANGED,
                                   configwin_modified, NULL);
    event_register_toolbox_handler(configwin_config_paths,
                                   action_WRITABLE_FIELD_VALUE_CHANGED,
                                   configwin_modified, NULL);
    event_register_toolbox_handler(configwin_config_files,
                                   action_WRITABLE_FIELD_VALUE_CHANGED,
                                   configwin_modified, NULL);
    event_register_toolbox_handler(configwin_config_files,
                                   action_OPTION_BUTTON_STATE_CHANGED,
                                   configwin_modified, NULL);
    event_register_toolbox_handler(configwin_config_files,
                                   action_ACTION_BUTTON_SELECTED,
                                   configwin_files_action, NULL);
    event_register_message_handler(message_DATA_LOAD, configwin_load,
                                   NULL);
    event_register_message_handler(message_TASK_CLOSE_DOWN,
                                   configwin_task_close, NULL);

    // Set the initial icon values
    configwin_reset();
}
