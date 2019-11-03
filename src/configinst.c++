/*
    File        : configinst.c++
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Installation window handling for !ARMEdit.

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
#include "configinst.h"

// Include cpplib header files
#include "string.h"
#include "fstream.h"

// Include clib header files
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// Include oslib header files
#include "hourglass.h"
#include "osfile.h"
#include "osfscontrol.h"
#include "window.h"
#include "wimpspriteop.h"
extern "C" {
#include "event.h"
}

// Include alexlib header files
#include "actionbutton_c.h"
#include "button_c.h"
#include "draggable_c.h"
#include "label_c.h"
#include "optionbutton_c.h"
#include "radiobutton_c.h"
#include "writablefield_c.h"

// Include project header files
#include "armeditmsg.h"
#include "armfile.h"
#include "configfile.h"
#include "go.h"
#include "instfile.h"
#include "multiconf.h"
#include "quit.h"
#include "utils.h"
#include "zip.h"

// Component IDs for the gadgets
#define CONFIGINST_ICON_CONTINUE 0x00
#define CONFIGINST_ICON_CANCEL 0x01
#define CONFIGINST_ICON_HELP 0x02
#define CONFIGINST_ICON_CUSTOM 0x03
#define CONFIGINST_PC_ICON 0x10
#define CONFIGINST_PC_ICON_DROP 0x11
#define CONFIGINST_PC_ICON_ICON 0x12
#define CONFIGINST_PC_ICON_LABEL 0x13
#define CONFIGINST_CONFIG_ICON 0x14
#define CONFIGINST_CONFIG_ICON_DROP 0x15
#define CONFIGINST_CONFIG_ICON_ICON 0x16
#define CONFIGINST_CONFIG_ICON_LABEL 0x17
#define CONFIGINST_PARTITION_ICON 0x10
#define CONFIGINST_PARTITION_ICON_DROP 0x11
#define CONFIGINST_PARTITION_ICON_ICON 0x12
#define CONFIGINST_PARTITION_ICON_LABEL 0x13
#define CONFIGINST_OPT_ICON_PC_DRV 0x10
#define CONFIGINST_OPT_ICON_PC_UTIL 0x11
#define CONFIGINST_OPT_ICON_MODIFY_CONFIG 0x20
#define CONFIGINST_OPT_ICON_MODIFY_AUTOEXEC 0x21
#define CONFIGINST_OPT_ICON_DEV_RISCOS 0x30
#define CONFIGINST_OPT_ICON_DEV_DOS 0x31
#define CONFIGINST_MODIFY_ICON 0x10
#define CONFIGINST_MODIFY_ICON_DROP 0x11
#define CONFIGINST_MODIFY_ICON_ICON 0x12
#define CONFIGINST_MODIFY_ICON_LABEL 0x13
#define CONFIGINST_DRV_RISCOS_ICON 0x10
#define CONFIGINST_DRV_RISCOS_ICON_DROP 0x11
#define CONFIGINST_DRV_RISCOS_ICON_ICON 0x12
#define CONFIGINST_DRV_RISCOS_ICON_LABEL 0x13
#define CONFIGINST_DRV_RISCOS_SAME 0x20
#define CONFIGINST_DRV_DOS_ICON 0x10
#define CONFIGINST_DRV_DOS_ICON_ICON 0x12
#define CONFIGINST_DRV_DOS_ICON_LABEL 0x13
#define CONFIGINST_DRV_DOS_SAME 0x20
#define CONFIGINST_UTIL_RISCOS_ICON 0x10
#define CONFIGINST_UTIL_RISCOS_ICON_DROP 0x11
#define CONFIGINST_UTIL_RISCOS_ICON_ICON 0x12
#define CONFIGINST_UTIL_RISCOS_ICON_LABEL 0x13
#define CONFIGINST_UTIL_DOS_ICON 0x10
#define CONFIGINST_UTIL_DOS_ICON_ICON 0x12
#define CONFIGINST_UTIL_DOS_ICON_LABEL 0x13
#define CONFIGINST_DEV_RISCOS_ICON 0x10
#define CONFIGINST_DEV_RISCOS_ICON_DROP 0x11
#define CONFIGINST_DEV_RISCOS_ICON_ICON 0x12
#define CONFIGINST_DEV_RISCOS_ICON_LABEL 0x13
#define CONFIGINST_DEV_DOS_ICON 0x10
#define CONFIGINST_DEV_DOS_ICON_DROP 0x11
#define CONFIGINST_DEV_DOS_ICON_ICON 0x12
#define CONFIGINST_DEV_DOS_ICON_LABEL 0x13

// Objects to represent the gadgets
static writablefield_c configinst_pc_icon(CONFIGINST_PC_ICON);
static button_c configinst_pc_icon_drop(CONFIGINST_PC_ICON_DROP);
static button_c configinst_pc_icon_icon(CONFIGINST_PC_ICON_ICON);
static label_c configinst_pc_icon_label(CONFIGINST_PC_ICON_LABEL);
static writablefield_c configinst_config_icon(CONFIGINST_CONFIG_ICON);
static button_c configinst_config_icon_drop(CONFIGINST_CONFIG_ICON_DROP);
static button_c configinst_config_icon_icon(CONFIGINST_CONFIG_ICON_ICON);
static label_c configinst_config_icon_label(CONFIGINST_CONFIG_ICON_LABEL);
static writablefield_c configinst_partition_icon(CONFIGINST_PARTITION_ICON);
static button_c configinst_partition_icon_drop(CONFIGINST_PARTITION_ICON_DROP);
static button_c configinst_partition_icon_icon(CONFIGINST_PARTITION_ICON_ICON);
static label_c configinst_partition_icon_label(CONFIGINST_PARTITION_ICON_LABEL);
static optionbutton_c configinst_opt_icon_pc_drv(CONFIGINST_OPT_ICON_PC_DRV);
static optionbutton_c configinst_opt_icon_pc_util(CONFIGINST_OPT_ICON_PC_UTIL);
static optionbutton_c configinst_opt_icon_modify_config(CONFIGINST_OPT_ICON_MODIFY_CONFIG);
static optionbutton_c configinst_opt_icon_modify_autoexec(CONFIGINST_OPT_ICON_MODIFY_AUTOEXEC);
static optionbutton_c configinst_opt_icon_dev_riscos(CONFIGINST_OPT_ICON_DEV_RISCOS);
static optionbutton_c configinst_opt_icon_dev_dos(CONFIGINST_OPT_ICON_DEV_DOS);
static writablefield_c configinst_modify_icon(CONFIGINST_MODIFY_ICON);
static button_c configinst_modify_icon_drop(CONFIGINST_MODIFY_ICON_DROP);
static button_c configinst_modify_icon_icon(CONFIGINST_MODIFY_ICON_ICON);
static label_c configinst_modify_icon_label(CONFIGINST_MODIFY_ICON_LABEL);
static writablefield_c configinst_drv_riscos_icon(CONFIGINST_DRV_RISCOS_ICON);
static button_c configinst_drv_riscos_icon_drop(CONFIGINST_DRV_RISCOS_ICON_DROP);
static draggable_c configinst_drv_riscos_icon_icon(CONFIGINST_DRV_RISCOS_ICON_ICON);
static label_c configinst_drv_riscos_icon_label(CONFIGINST_DRV_RISCOS_ICON_LABEL);
static optionbutton_c configinst_drv_riscos_same(CONFIGINST_DRV_RISCOS_SAME);
static writablefield_c configinst_drv_dos_icon(CONFIGINST_DRV_DOS_ICON);
static button_c configinst_drv_dos_icon_icon(CONFIGINST_DRV_DOS_ICON_ICON);
static label_c configinst_drv_dos_icon_label(CONFIGINST_DRV_DOS_ICON_LABEL);
static optionbutton_c configinst_drv_dos_same(CONFIGINST_DRV_DOS_SAME);
static writablefield_c configinst_util_riscos_icon(CONFIGINST_UTIL_RISCOS_ICON);
static button_c configinst_util_riscos_icon_drop(CONFIGINST_UTIL_RISCOS_ICON_DROP);
static draggable_c configinst_util_riscos_icon_icon(CONFIGINST_UTIL_RISCOS_ICON_ICON);
static label_c configinst_util_riscos_icon_label(CONFIGINST_UTIL_RISCOS_ICON_LABEL);
static writablefield_c configinst_util_dos_icon(CONFIGINST_UTIL_DOS_ICON);
static button_c configinst_util_dos_icon_icon(CONFIGINST_UTIL_DOS_ICON_ICON);
static label_c configinst_util_dos_icon_label(CONFIGINST_UTIL_DOS_ICON_LABEL);
static writablefield_c configinst_dev_riscos_icon(CONFIGINST_DEV_RISCOS_ICON);
static button_c configinst_dev_riscos_icon_drop(CONFIGINST_DEV_RISCOS_ICON_DROP);
static draggable_c configinst_dev_riscos_icon_icon(CONFIGINST_DEV_RISCOS_ICON_ICON);
static label_c configinst_dev_riscos_icon_label(CONFIGINST_DEV_RISCOS_ICON_LABEL);
static writablefield_c configinst_dev_dos_icon(CONFIGINST_DEV_DOS_ICON);
static button_c configinst_dev_dos_icon_drop(CONFIGINST_DEV_DOS_ICON_DROP);
static draggable_c configinst_dev_dos_icon_icon(CONFIGINST_DEV_DOS_ICON_ICON);
static label_c configinst_dev_dos_icon_label(CONFIGINST_DEV_DOS_ICON_LABEL);

// The toolbox object IDs
#define CONFIGINST_WELCOME 0
#define CONFIGINST_SOFTWARE 1
#define CONFIGINST_DOS 2
#define CONFIGINST_DONE 3
#define CONFIGINST_INSTALL_OPTIONS 4
#define CONFIGINST_INSTALL_MODIFY 5
#define CONFIGINST_INSTALL_DRV_RISCOS 6
#define CONFIGINST_INSTALL_DRV_DOS 7
#define CONFIGINST_INSTALL_UTIL_RISCOS 8
#define CONFIGINST_INSTALL_UTIL_DOS 9
#define CONFIGINST_INSTALL_DEV_RISCOS 10
#define CONFIGINST_INSTALL_DEV_DOS 11
#define CONFIGINST_INSTALL_DONE 12
#define CONFIGINST_INSTALL_FAIL 13
#define CONFIGINST_MAX (CONFIGINST_INSTALL_FAIL)
#define CONFIGINST_NONE -1
static toolbox_o configinst_id[CONFIGINST_MAX + 1];
static const char *configinst_name[CONFIGINST_MAX + 1] =
{
    "WlcIntrWin",                       // CONFIGINST_WELCOME
    "WlcSWWin",                         // CONFIGINST_SOFTWARE
    "WlcDosWin",                        // CONFIGINST_DOS
    "WlcDoneWin",                       // CONFIGINST_DONE
    "InsOptWin",                        // CONFIGINST_INSTALL_OPTIONS
    "InsModify",                        // CONFIGINST_INSTALL_MODIFY
    "InsDrvRO",                         // CONFIGINST_INSTALL_DRV_RISCOS
    "InsDrvDOS",                        // CONFIGINST_INSTALL_DRV_DOS
    "InsUtilRO",                        // CONFIGINST_INSTALL_UTIL_RISCOS
    "InsUtilDOS",                       // CONFIGINST_INSTALL_UTIL_DOS
    "InsDevRO",                         // CONFIGINST_INSTALL_DEV_RISCOS
    "InsDevDOS",                        // CONFIGINST_INSTALL_DEV_DOS
    "InsDoneWin",                       // CONFIGINST_INSTALL_DONE
    "InsFailWin"                        // CONFIGINST_INSTALL_FAIL
};

// The currently open window
static int configinst_win_open = CONFIGINST_NONE;

// Zip archive filenames
#define CONFIGINST_ZIP_DRV "Device"
#define CONFIGINST_ZIP_UTIL "Utilities"
#define CONFIGINST_ZIP_DEV_RISCOS "CodeARM"
#define CONFIGINST_ZIP_DEV_DOS "CodePC"

// Default paths
#define CONFIGINST_PATH_PC_RISCOS ".Drivers.3rdParty.ARMEdit"
#define CONFIGINST_PATH_PC_DOS "C:\\Drivers\\3rdParty\\ARMEdit"
#define CONFIGINST_PATH_DEV_RISCOS "<ARMEdit$Dir>.^.Code"
#define CONFIGINST_PATH_DEV_DOS CONFIGINST_PATH_PC_RISCOS ".Code"

// The configuration files to modify
#define CONFIGINST_FILE_CONFIG_SYS "Config/Sys"
#define CONFIGINST_FILE_CONFIG_TEMP "Config/Arm"
#define CONFIGINST_FILE_CONFIG_BACK "Config/Back"
#define CONFIGINST_FILE_AUTOEXEC_BAT "Autoexec/Bat"
#define CONFIGINST_FILE_AUTOEXEC_TEMP "Autoexec/Arm"
#define CONFIGINST_FILE_AUTOEXEC_BACK "Autoexec/Bak"

// A DOS line and file end
#define CONFIGINST_DOS_EOL "\r\n"
#define CONFIGINST_DOS_EOF "\x1a"

// A default CONFIG.SYS file
#define CONFIGINST_CONFIG_PREFIX "LASTDRIVE=Z" CONFIGINST_DOS_EOL\
                                 "DEVICE="
#define CONFIGINST_CONFIG_POSTFIX CONFIGINST_DOS_EOL\
                                  CONFIGINST_DOS_EOF

// A default AUTOEXEC.BAT file
#define CONFIGINST_AUTOEXEC_PREFIX "@ECHO OFF" CONFIGINST_DOS_EOL\
                                   "PATH=C:\;C:\DOS;"
#define CONFIGINST_AUTOEXEC_POSTFIX CONFIGINST_DOS_EOL\
                                    "PROMPT $p$g"  CONFIGINST_DOS_EOL\
                                    "SET TEMP=C:\DOS" CONFIGINST_DOS_EOL\
                                    CONFIGINST_DOS_EOF

// The default drive for DOS paths
#define CONFIGINST_DOS_ROOT "C:"

// The selected options
static int configinst_opt_pc_drv;
static int configinst_opt_pc_util;
static int configinst_opt_modify_config;
static int configinst_opt_modify_autoexec;
static int configinst_opt_dev_riscos;
static int configinst_opt_dev_dos;
static int configinst_opt_riscos_same;
static int configinst_opt_dos_same;

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to an error block, or NULL if
                                  successful.
    Description : Modify the selected AUTOEXEC.BAT file to include the command
                  line utilities in the search path.
*/
static const os_error *configinst_modify_autoexec(void)
{
    char src[256];
    char temp[256];
    char back[256];
    os_error *err;
    fileswitch_object_type object_type;

    // Construct the path to the AUTOEXEC.BAT file and variants
    sprintf(src, "%s.%s", instfile_path_modify,
            CONFIGINST_FILE_AUTOEXEC_BAT);
    sprintf(temp, "%s.%s", instfile_path_modify,
            CONFIGINST_FILE_AUTOEXEC_TEMP);
    sprintf(back, "%s.%s", instfile_path_modify,
            CONFIGINST_FILE_AUTOEXEC_BACK);

    // Check whether the file exists
    err = xosfile_read_no_path(src, &object_type, NULL, NULL, NULL, NULL);

    // Different action if a new file is required
    if (err || (object_type != fileswitch_IS_FILE))
    {
        // Clear the error condition
        err = NULL;

        // Create a new AUTOEXEC.BAT file
        ofstream to(temp);
        to << CONFIGINST_AUTOEXEC_PREFIX
           << instfile_path_util_dos
           << CONFIGINST_AUTOEXEC_POSTFIX;
    }
    else
    {
        // Construct the new file under a temporary name
        ifstream from(src);
        ofstream to(temp);
        // *** modify src -> temp

        // Create a backup copy of the existing file
        from.close();
        err = xosfscontrol_copy(src, back,
                                osfscontrol_COPY_FORCE,
                                0, 0, 0, 0, NULL);
    }

    // Move the new file over the original
    if (!err)
    {
        err = xosfscontrol_copy(temp, src,
                                osfscontrol_COPY_FORCE
                                | osfscontrol_COPY_DELETE,
                                0, 0, 0, 0, NULL);
    }

    // Return any error
    return err;
}

/*
    Parameters  : void
    Returns     : os_error *    - Pointer to an error block, or NULL if
                                  successful.
    Description : Modify the selected CONFIG.SYS file to load and configure the
                  device driver.
*/
static const os_error *configinst_modify_config(void)
{
    // *** modify the file
    return NULL;
}

/*
    Parameters  : token - The token for the message to display.
    Returns     : void
    Description : Display the specified message to the user.
*/
static void configinst_message(const char *token)
{
    os_error err;

    // Construct the error block
    err.errnum = 0;
    lookup_token(err.errmess, sizeof(err.errmess), token);

    // Use a suitable form of error window
    if (wimp_VERSION_RO35 <= wimp_version)
    {
        // Display an error message
        wimp_report_error_by_category(&err,
            wimp_ERROR_BOX_OK_ICON
            | (wimp_ERROR_BOX_CATEGORY_INFO << 9), task_name,
            AppSprite, wimpspriteop_AREA, NULL);
    }
    else
    {
        // Display an error message
        wimp_report_error(&err, wimp_ERROR_BOX_OK_ICON, task_name);
    }
}

/*
    Parameters  : void
    Returns     : int   - Was the operation successful.
    Description : Perform an installation using the currently selected options.
*/
static int configinst_do_install(void)
{
    const os_error *err = NULL;

    // Turn the hourglass on
    hourglass_on();

    // Perform any required actions
    if (configinst_opt_pc_drv)
    {
        // Install the device driver into the partition
        err = zip_unzip(CONFIGINST_ZIP_DRV, instfile_path_drv_riscos);

        // Display a message if installation not complete
        if (!err && !configinst_opt_modify_config)
        {
            configinst_message("InstDevice");
        }
    }
    if (!err && configinst_opt_pc_util)
    {
        // Install command line utilities into the partition
        err = zip_unzip(CONFIGINST_ZIP_UTIL, instfile_path_util_riscos);

        // Display a message if installation not complete
        if (!err && !configinst_opt_modify_autoexec)
        {
            configinst_message("InstPath");
        }
    }
    if (!err && configinst_opt_modify_config)
    {
        // Modify CONFIG.SYS to load the device driver
        err = configinst_modify_config();
    }
    if (!err && configinst_opt_modify_autoexec)
    {
        // Modify AUTOEXEC.BAT to the end of the PATH statement
        err = configinst_modify_autoexec();
    }
    if (!err && configinst_opt_dev_riscos)
    {
        // Copy the RISC OS SWI veneers
        err = zip_unzip(CONFIGINST_ZIP_DEV_RISCOS, instfile_path_dev_riscos);
    }
    if (!err && configinst_opt_dev_dos)
    {
        // Copy the PC header and library files
        err = zip_unzip(CONFIGINST_ZIP_DEV_DOS, instfile_path_dev_dos);
    }

    // Turn the hourglass off
    hourglass_off();

    // Display any outstanding error message
    if (err)
    {
        // Use a suitable form of error window
        if (wimp_VERSION_RO35 <= wimp_version)
        {
            // Display an error message
            wimp_report_error_by_category(err,
                wimp_ERROR_BOX_CANCEL_ICON
                | (wimp_ERROR_BOX_CATEGORY_ERROR << 9), task_name,
                AppSprite, wimpspriteop_AREA, NULL);
        }
        else
        {
            wimp_report_error(err, wimp_ERROR_BOX_CANCEL_ICON, task_name);
        }
    }

    // Return the status
    return !err;
}

/*
    Parameters  : cmp   - The component that was changed.
    Returns     : void
    Description : Read the installation option buttons. The Install button is
                  disabled if no options are selected.
*/
static void configinst_read_options(toolbox_c cmp)
{
    toolbox_o object = configinst_id[CONFIGINST_INSTALL_OPTIONS];
    int any;

    // Read all the options
    configinst_opt_pc_drv = configinst_opt_icon_pc_drv();
    configinst_opt_pc_util = configinst_opt_icon_pc_util();
    configinst_opt_modify_config = configinst_opt_icon_modify_config();
    configinst_opt_modify_autoexec = configinst_opt_icon_modify_autoexec();
    configinst_opt_dev_riscos = configinst_opt_icon_dev_riscos();
    configinst_opt_dev_dos = configinst_opt_icon_dev_dos();

    // Make any other changes required
    switch (cmp)
    {
        case CONFIGINST_OPT_ICON_PC_DRV:
            // Change the CONFIG.SYS modification option to match
            /*
            configinst_opt_modify_config = configinst_opt_pc_drv;
            configinst_opt_icon_modify_config = configinst_opt_modify_config;
            */
            break;

        case CONFIGINST_OPT_ICON_PC_UTIL:
            // Change the AUTOEXEC.BAT modification option to match
            /*
            configinst_opt_modify_autoexec = configinst_opt_pc_util;
            configinst_opt_icon_modify_autoexec = configinst_opt_modify_autoexec;
            */
            break;

        default:
            // Do not care about other changes
            break;
    }

    // Check if any options are selected
    any = configinst_opt_pc_drv || configinst_opt_pc_util
          || configinst_opt_modify_config || configinst_opt_modify_autoexec
          || configinst_opt_dev_riscos || configinst_opt_dev_dos;

    // Grey out the Install button if required
    actionbutton_c(CONFIGINST_ICON_CONTINUE, object).set_faded(!any);
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Read the installation option buttons following any change.
*/
static int configinst_opt_modified(bits event_code, toolbox_action *action,
                                   toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Read the new option buttons state
    configinst_read_options(id_block->this_cmp);

    // Claim the event
    return TRUE;
}

/*
    Parameters  : path      - The path to canonicalise.
    Returns     : string    - The canonicalised equivalent.
    Description : Convert a path name to a canonical form.
*/
static string configinst_canonicalise(const string &path)
{
    char str[256];
    os_error *err;

    // Canonicalise the path
    err = xosfscontrol_canonicalise_path(path.c_str(), str, NULL, NULL,
                                         sizeof(str), NULL);

    // Return a pointer to the resulting path
    return err ? path : string(str);
}

/*
    Parameters  : path      - The DOS directory path to normalise.
    Returns     : string    - The normalised path.
    Description : Normalise the specified DOS directory path. This ensures that
                  only the root directory ends in a trailing slash.
*/
static string configinst_normalise_dos(const string &path)
{
    char str[256];

    // Copy the supplied path
    strcpy(str, path.c_str());

    // Ensure that the path starts with a drive specification
    if (isalpha(str[0]) && (str[1] == ':'))
    {
        // Check whether any modifications are required
        if (str[2] == 0)
        {
            // Add a slash to the end of the path
            str[2] = '\\';
            str[3] = 0;
        }
        else if (str[strlen(str) - 1] == '\\')
        {
            // Remove the trailing slash
            str[strlen(str) - 1] = 0;
        }
    }

    // Return the normalised path
    return string(str);
}

/*
    Parameters  : dir       - The directory path to convert.
                  root      - The root directory of the DOS partition.
    Returns     : char *    - The DOS path, or NULL if failed.
    Description : Attempt to convert the specified directory path to a DOS path
                  relative to the partition.
*/
static const char *configinst_to_dos(const string &dir, const string &root)
{
    static string path;
    char path_dir[256];
    char path_root[256];

    // Ensure that both paths are canonicalised
    strcpy(path_dir, configinst_canonicalise(dir).c_str());
    strcpy(path_root, configinst_canonicalise(root).c_str());

    // Check if the root is a prefix of the directory
    if (path_root[0] && !strncmp(path_dir, path_root, strlen(path_root)))
    {
        // The root is a prefix of the directory
        strcpy(path_root, armfile_translate_riscos_dos(path_dir + strlen(path_root)).c_str());
        path = CONFIGINST_DOS_ROOT + string(path_root);

        // Normalise the path
        path = configinst_normalise_dos(path);
    }
    else
    {
        char *ptr;

        // Check the filetype of all parents of the directory
        strcpy(path_root, path_dir);
        path = "";
        ptr = strrchr(path_root, '.');
        while (ptr)
        {
            os_error *err;
            fileswitch_object_type type;

            // Generate the next parent directory
            *ptr = 0;

            // Check if this parent is an image file
            err = xosfile_read_no_path(path_root, &type, NULL, NULL, NULL,
                                       NULL);
            if (!err && (type == fileswitch_IS_IMAGE))
            {
                // Generate the DOS filename and terminate the loop
                configinst_to_dos(path_dir, path_root);
                ptr = 0;
            }
            else ptr = strrchr(path_root, '.');
        }
    }

    // Return the resulting path
    return path.empty() ? NULL : path.c_str();
}

/*
    Parameters  : win       - The window to check.
                  gadget    - The ID of the gadget to check.
                  type      - The required file type.
                  msg       - The token for the required error message.
    Returns     : int       - Is the path acceptable.
    Description : Verify that the contents of a writable field gadget form a
                  valid path pointing to an object of the specified type.
*/
static int configinst_verify(int win, toolbox_c gadget, bits type,
                             const char *msg)
{
    toolbox_o object = configinst_id[win];
    char str[256];
    os_error *err;
    fileswitch_object_type object_type;
    bits load;
    bits file_type;
    int ok = TRUE;

    // Read the value of the field
    writablefield_get_value(0, object, gadget, str, sizeof(str));

    // Read the object type
    err = xosfile_read_stamped_no_path(str, &object_type, NULL, NULL, NULL,
                                       NULL, &file_type);
    if (!err) err = xosfile_read_no_path(str, NULL, &load, NULL, NULL, NULL);
    if (object_type == fileswitch_IS_IMAGE) file_type = (load >> 8) & 0xfff;

    // Handle the result
    if (err)
    {
        // Construct the required message
        lookup_token(str, sizeof(str), "InstErr", lookup_token(msg),
                     err->errmess);

        // Not successful if an error was produced
        ok = FALSE;
    }
    else if (type != file_type)
    {
        // Construct the required message
        lookup_token(str, sizeof(str), "InstType", lookup_token(msg));

        // Not successful if the type does not match
        ok = FALSE;
    }

    // Extra action required if not acceptable
    if (!ok)
    {
        os_error err;

        // Use a suitable form of error window
        if (wimp_VERSION_RO35 <= wimp_version)
        {
            // Display an error message
            err.errnum = 0;
            lookup_token(err.errmess, sizeof(err.errmess), "InstPathNew", str);
            ok = wimp_report_error_by_category(&err,
                     wimp_ERROR_BOX_CATEGORY_QUESTION << 9, task_name,
                     AppSprite, wimpspriteop_AREA, lookup_token("InstPathBut"))
                 == 4;
        }
        else
        {
            // Display an error message
            err.errnum = 0;
            lookup_token(err.errmess, sizeof(err.errmess), "InstPathOld", str);
            ok = wimp_report_error(&err, wimp_ERROR_BOX_OK_ICON
                                   | wimp_ERROR_BOX_CANCEL_ICON
                                   | wimp_ERROR_BOX_HIGHLIGHT_CANCEL,
                                   task_name)
                 == wimp_ERROR_BOX_SELECTED_OK;
        }

        // Set the default focus to the required field
        window_set_default_focus(0, object, gadget);
    }

    // Return the status
    return ok;
}

/*
    Parameters  : win       - The window to check.
                  gadget    - The ID of the gadget to check.
                  file      - The leafname of the required file.
    Returns     : int       - Is the path acceptable.
    Description : Verify that the contents of a writable field gadget form a
                  valid path pointing to the specified file.
*/
static int configinst_verify_file(int win, toolbox_c gadget, const char *file)
{
    toolbox_o object = configinst_id[win];
    char str[256];
    os_error *err;
    fileswitch_object_type object_type;
    int ok = TRUE;

    // Read the value of the field
    writablefield_get_value(0, object, gadget, str, sizeof(str));

    // Construct the expected object path
    sprintf(strchr(str, 0), ".%s", file);

    // Check whether the object exists
    err = xosfile_read_no_path(str, &object_type, NULL, NULL, NULL, NULL);

    // Handle the result
    if (err)
    {
        // Construct the required message
        strcpy(str, err->errmess);

        // Not successful if an error was produced
        ok = FALSE;
    }
    else if (object_type != fileswitch_IS_FILE)
    {
        // Construct the required message
        lookup_token(str, sizeof(str), "InstFile", file);

        // Not successful if the type does not match
        ok = FALSE;
    }

    // Extra action required if not acceptable
    if (!ok)
    {
        os_error err;

        // Use a suitable form of error window
        if (wimp_VERSION_RO35 <= wimp_version)
        {
            // Display an error message
            err.errnum = 0;
            lookup_token(err.errmess, sizeof(err.errmess), "InstFileNew",
                         str, file);
            ok = wimp_report_error_by_category(&err,
                     wimp_ERROR_BOX_CATEGORY_QUESTION << 9, task_name,
                     AppSprite, wimpspriteop_AREA, lookup_token("InstFileBut"))
                 == 3;
        }
        else
        {
            // Display an error message
            err.errnum = 0;
            lookup_token(err.errmess, sizeof(err.errmess), "InstFileOld",
                         str, file);
            ok = wimp_report_error(&err, wimp_ERROR_BOX_OK_ICON
                                   | wimp_ERROR_BOX_CANCEL_ICON,
                                   task_name)
                 == wimp_ERROR_BOX_SELECTED_OK;
        }

        // Set the default focus to the required field
        window_set_default_focus(0, object, gadget);
    }

    // Return the status
    return ok;
}

/*
    Parameters  : win       - The window to check.
                  gadget    - The ID of the gadget to check.
    Returns     : int       - Is the path acceptable.
    Description : Verify that the contents of a writable field gadget form a
                  valid DOS path.
*/
static int configinst_verify_dos(int win, toolbox_c gadget)
{
    toolbox_o object = configinst_id[win];
    char str[256];
    int ok = TRUE;

    // Read the value of the field
    writablefield_get_value(0, object, gadget, str, sizeof(str));

    // Perform checks
    if (!isalpha(str[0]) || (str[1] != ':'))
    {
        // Construct the required message
        lookup_token(str, sizeof(str), "InstDOSDrive");

        // Not successful if the name is unsuitable
        ok = FALSE;
    }
    else if (str[2] != '\\')
    {
        char drive[2];

        // Construct the required message
        drive[0] = toupper(str[0]);
        drive[1] = 0;
        lookup_token(str, sizeof(str), "InstDOSRoot", drive);

        // Not successful if the name is unsuitable
        ok = FALSE;
    }

    // Extra action required if not acceptable
    if (!ok)
    {
        os_error err;

        // Use a suitable form of error window
        if (wimp_VERSION_RO35 <= wimp_version)
        {
            // Display an error message
            err.errnum = 0;
            lookup_token(err.errmess, sizeof(err.errmess), "InstDOSNew", str);
            ok = wimp_report_error_by_category(&err,
                     wimp_ERROR_BOX_CATEGORY_QUESTION << 9, task_name,
                     AppSprite, wimpspriteop_AREA, lookup_token("InstDOSBut"))
                 == 4;
        }
        else
        {
            // Display an error message
            err.errnum = 0;
            lookup_token(err.errmess, sizeof(err.errmess), "InstDOSOld", str);
            ok = wimp_report_error(&err, wimp_ERROR_BOX_OK_ICON
                                   | wimp_ERROR_BOX_CANCEL_ICON
                                   | wimp_ERROR_BOX_HIGHLIGHT_CANCEL,
                                   task_name)
                 == wimp_ERROR_BOX_SELECTED_OK;
        }

        // Set the default focus to the required field
        window_set_default_focus(0, object, gadget);
    }

    // Return the status
    return ok;
}

/*
    Parameters  : win   - The window to be displayed.
    Returns     : void
    Description : Prepare the specified window before it is shown.
*/
static void configinst_pre(int win)
{
    toolbox_o object = configinst_id[win];
    const char *path;
    char str[256];

    // The action depends upon the window
    switch (win)
    {
        case CONFIGINST_SOFTWARE:
            // Set default paths for the applications
            configinst_pc_icon = configinst_canonicalise(configfile_path_pc);
            configinst_config_icon = configinst_canonicalise(configfile_path_config);
            break;

        case CONFIGINST_DOS:
            // Set a default partition path
            path = multiconf_config_partition(MULTICONF_ACTIVE, 0);
            if (!path) path = configfile_path_partition.c_str();
            configinst_partition_icon = configinst_canonicalise(path);
            break;

        case CONFIGINST_DONE:
            // Save the configuration
            configfile_write();
            break;

        case CONFIGINST_INSTALL_OPTIONS:
            // Disable the Install button if no options are selected
            configinst_read_options(toolbox_NULL_COMPONENT);
            break;

        case CONFIGINST_INSTALL_MODIFY:
            // Set the default path for the files to modify
            configinst_modify_icon = instfile_path_modify[0]
                                     ? instfile_path_modify
                                     : configinst_canonicalise(configfile_path_partition);
            break;

        case CONFIGINST_INSTALL_DRV_RISCOS:
            // Set the default destination path
            sprintf(str, "%s%s", configfile_path_partition,
                    CONFIGINST_PATH_PC_RISCOS);
            configinst_drv_riscos_icon = instfile_path_drv_riscos[0]
                                         ? instfile_path_drv_riscos
                                         : (instfile_path_util_riscos[0]
                                            ? instfile_path_util_riscos
                                            : configinst_canonicalise(str));
            configinst_drv_riscos_same = configinst_opt_pc_util;
            configinst_drv_riscos_same.set_faded(!configinst_opt_pc_util);
            break;

        case CONFIGINST_INSTALL_DRV_DOS:
            // Set the default access path
            path = configinst_to_dos(instfile_path_drv_riscos,
                                     instfile_path_modify);
            configinst_drv_dos_icon = path
                                      ? string(path)
                                      : (!instfile_path_drv_dos.empty()
                                         ? instfile_path_drv_dos
                                         : (!instfile_path_util_dos.empty()
                                            ? instfile_path_util_dos
                                              : string(CONFIGINST_PATH_PC_DOS)));
            configinst_drv_dos_same = configinst_opt_modify_autoexec
                                      && configinst_opt_riscos_same;
            configinst_drv_dos_same.set_faded(!configinst_opt_modify_autoexec);
            break;

        case CONFIGINST_INSTALL_UTIL_RISCOS:
            // Set the default destination path
            sprintf(str, "%s%s", configfile_path_partition,
                    CONFIGINST_PATH_PC_RISCOS);
            configinst_util_riscos_icon = instfile_path_util_riscos[0]
                                          ? instfile_path_util_riscos
                                          : (instfile_path_drv_riscos[0]
                                             ? instfile_path_drv_riscos
                                             : configinst_canonicalise(str));
            break;

        case CONFIGINST_INSTALL_UTIL_DOS:
            // Set the default access path
            path = configinst_to_dos(instfile_path_util_riscos,
                                     instfile_path_modify);
            configinst_util_dos_icon = path
                                       ? string(path)
                                       : (!instfile_path_util_dos.empty()
                                          && (instfile_path_drv_riscos
                                              != instfile_path_util_riscos)
                                          ? instfile_path_util_dos
                                          : (!instfile_path_drv_dos.empty()
                                             ? instfile_path_drv_dos
                                               : string(CONFIGINST_PATH_PC_DOS)));
            break;

        case CONFIGINST_INSTALL_DEV_RISCOS:
            // Set the default destination path
            configinst_dev_riscos_icon = instfile_path_dev_riscos[0]
                                         ? instfile_path_dev_riscos
                                         : configinst_canonicalise(CONFIGINST_PATH_DEV_RISCOS);
            break;

        case CONFIGINST_INSTALL_DEV_DOS:
            // Set the default destination path
            sprintf(str, "%s%s", configfile_path_partition,
                    CONFIGINST_PATH_DEV_DOS);
            configinst_dev_dos_icon = instfile_path_dev_dos[0]
                                      ? instfile_path_dev_dos
                                      : configinst_canonicalise(str);
            break;

        default:
            // No preparation required for other windows
            break;
    }
}

/*
    Parameters  : win   - The window to validate.
    Returns     : int   - Is the window acceptable.
    Description : Check whether the specified window should be displayed.
*/
static int configinst_validate(int win)
{
    int ok;

    // The check depends upon the window
    switch (win)
    {
        case CONFIGINST_INSTALL_MODIFY:
            // Modification path only required if option selected
            ok = configinst_opt_modify_config
                 || configinst_opt_modify_autoexec;
            break;

        case CONFIGINST_INSTALL_DRV_RISCOS:
            // Installation path only required if option selected
            ok = configinst_opt_pc_drv;
            break;

        case CONFIGINST_INSTALL_DRV_DOS:
            // Access path only required if option selected
            ok = configinst_opt_modify_config;
            break;

        case CONFIGINST_INSTALL_UTIL_RISCOS:
            // Installation path only required if option selected
            ok = configinst_opt_pc_util && !configinst_opt_riscos_same;
            break;

        case CONFIGINST_INSTALL_UTIL_DOS:
            // Access path only required if option selected
            ok = configinst_opt_modify_autoexec && !configinst_opt_dos_same;
            break;

        case CONFIGINST_INSTALL_DEV_RISCOS:
            // Installation path only required if option selected
            ok = configinst_opt_dev_riscos;
            break;

        case CONFIGINST_INSTALL_DEV_DOS:
            // Installation path only required if option selected
            ok = configinst_opt_dev_dos;
            break;

        case CONFIGINST_INSTALL_DONE:
            // Save the options and attempt to install the selected options
            instfile_write();
            ok = configinst_do_install();
            break;

        default:
            // All other windows are acceptable
            ok = TRUE;
            break;
    }

    // Return the status
    return ok;
}

/*
    Parameters  : win   - The window being displayed.
    Returns     : int   - The next window to display.
    Description : Check the contents of the window, and suggest which window
                  to display next if Continue is clicked. Any required update
                  of the configuration is also performed.
*/
static int configinst_post(int win)
{
    toolbox_o object = configinst_id[win];
    int next = CONFIGINST_NONE;

    // The action depends upon the window
    switch (win)
    {
        case CONFIGINST_WELCOME:
            // Just mark the configuration as modified
            configfile_modified = TRUE;
            next = win + 1;
            break;

        case CONFIGINST_SOFTWARE:
            // Read and check the paths
            if (configinst_verify(win, CONFIGINST_PC_ICON,
                                  osfile_TYPE_APPLICATION, "InstBadPC")
                && configinst_verify(win, CONFIGINST_CONFIG_ICON,
                                     osfile_TYPE_APPLICATION, "InstBadConfig"))
            {
                configfile_path_pc = configinst_pc_icon();
                configfile_path_config = configinst_config_icon();
                next = win + 1;
            }
            else next = win;
            break;

        case CONFIGINST_DOS:
            // Read and check the paths
            if (configinst_verify(win, CONFIGINST_PARTITION_ICON,
                                  osfile_TYPE_DOS_DISC, "InstBadPart"))
            {
                configfile_path_partition = configinst_partition_icon();
                next = win + 1;
            }
            else next = win;
            break;

        case CONFIGINST_DONE:
            // Perform an automatic installation
            configinst_opt_pc_drv = TRUE;
            configinst_opt_pc_util = TRUE;
            configinst_opt_modify_config = FALSE /*TRUE*/;
            configinst_opt_modify_autoexec = FALSE /*TRUE*/;
            configinst_opt_dev_riscos = FALSE;
            configinst_opt_dev_dos = FALSE;
            instfile_path_modify = configfile_path_partition;
            instfile_path_drv_riscos = instfile_path_modify
                                       + CONFIGINST_PATH_PC_RISCOS;
            instfile_path_drv_dos = CONFIGINST_PATH_PC_DOS;
            instfile_path_util_riscos = instfile_path_drv_riscos;
            instfile_path_util_dos = instfile_path_drv_dos;
            instfile_path_dev_riscos
                   = configinst_canonicalise(CONFIGINST_PATH_DEV_RISCOS);
            instfile_path_dev_dos = instfile_path_modify
                                    + CONFIGINST_PATH_DEV_DOS;
            next = CONFIGINST_INSTALL_DONE;
            break;

        case CONFIGINST_INSTALL_OPTIONS:
            // Advance to the first installation window
            instfile_modified = TRUE;
            configinst_opt_riscos_same = FALSE;
            configinst_opt_dos_same = FALSE;
            next = CONFIGINST_INSTALL_MODIFY;
            break;

        case CONFIGINST_INSTALL_MODIFY:
            // Read and check the path
            if (configinst_verify_file(win, CONFIGINST_MODIFY_ICON,
                                       CONFIGINST_FILE_CONFIG_SYS)
                && configinst_verify_file(win, CONFIGINST_MODIFY_ICON,
                                          CONFIGINST_FILE_AUTOEXEC_BAT))
            {
                instfile_path_modify = configinst_modify_icon();
                next = win + 1;
            }
            else next = win;
            break;

        case CONFIGINST_INSTALL_DRV_RISCOS:
            // Read the path
            instfile_path_drv_riscos = configinst_drv_riscos_icon();
            configinst_opt_riscos_same = configinst_drv_riscos_same();
            if (configinst_opt_riscos_same)
            {
                instfile_path_util_riscos = instfile_path_drv_riscos;
            }
            next = win + 1;
            break;

        case CONFIGINST_INSTALL_DRV_DOS:
            // Read and check the path
            if (configinst_verify_dos(win, CONFIGINST_DRV_DOS_ICON))
            {
                instfile_path_drv_dos = configinst_normalise_dos(configinst_drv_dos_icon());
                configinst_opt_dos_same = configinst_drv_dos_same();
                if (configinst_opt_dos_same)
                {
                    instfile_path_util_dos = instfile_path_drv_dos;
                }
                next = win + 1;
            }
            else next = win;
            break;

        case CONFIGINST_INSTALL_UTIL_RISCOS:
            // Read the path
            instfile_path_util_riscos = configinst_util_riscos_icon();
            next = win + 1;
            break;

        case CONFIGINST_INSTALL_UTIL_DOS:
            // Read and check the path
            if (configinst_verify_dos(win, CONFIGINST_UTIL_DOS_ICON))
            {
                instfile_path_util_dos = configinst_normalise_dos(configinst_util_dos_icon());
                next = win + 1;
            }
            else next = win;
            break;

        case CONFIGINST_INSTALL_DEV_RISCOS:
            // Read and check the path
            instfile_path_dev_riscos = configinst_dev_riscos_icon();
            next = win + 1;
            break;

        case CONFIGINST_INSTALL_DEV_DOS:
            // Read and check the path
            instfile_path_dev_dos = configinst_dev_dos_icon();
            next = win + 1;
            break;

        case CONFIGINST_INSTALL_DONE:
        case CONFIGINST_INSTALL_FAIL:
            // All finished so close the window and exit
            next = CONFIGINST_NONE;
            break;

        default:
            // All other window just advance to the next one
            next = win + 1;
            break;
    }

    // Skip windows that are not required
    while (!configinst_validate(next)) next++;

    // Return the next window to display
    return next;
}

/*
    Parameters  : win   - The window to display.
    Returns     : void
    Description : Open the specified window. Any other open windows are closed.
*/
static void configinst_show(int win)
{
    toolbox_o object = configinst_id[win];
    int centre_x;
    int centre_y;

    // The actions depends on whether a window is currently open
    if (configinst_win_open == win)
    {
        // Simply move the window to the front
        toolbox_show_object(0, object, toolbox_POSITION_DEFAULT, NULL,
                            toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);
    }
    else if (configinst_win_open == CONFIGINST_NONE)
    {
        int xeig;
        int yeig;
        int xwind;
        int ywind;

        // Read screen size
        os_read_mode_variable(os_CURRENT_MODE, os_MODEVAR_XEIG_FACTOR, &xeig);
        os_read_mode_variable(os_CURRENT_MODE, os_MODEVAR_YEIG_FACTOR, &yeig);
        os_read_mode_variable(os_CURRENT_MODE, os_MODEVAR_XWIND_LIMIT, &xwind);
        os_read_mode_variable(os_CURRENT_MODE, os_MODEVAR_YWIND_LIMIT, &ywind);

        // Calculate the centre of the screen
        centre_x = (xwind << xeig) / 2;
        centre_y = (ywind << yeig) / 2;
    }
    else
    {
        wimp_window_state state;

        // Calculate the centre of the current window
        centre_x = 0;
        centre_y = 0;

        // Obtain the actual WIMP handle for the current window
        state.w = window_get_wimp_handle(0, configinst_id[configinst_win_open]);

        // Read the window details
        wimp_get_window_state(&state);

        // Calculate the centre of the window
        centre_x = (state.visible.x0 + state.visible.x1) / 2;
        centre_y = (state.visible.y0 + state.visible.y1) / 2;

        // Close the current window
        toolbox_hide_object(0, configinst_id[configinst_win_open]);
    }

    // Open the window at the selected location if not already open
    if ((configinst_win_open != win) && (win != CONFIGINST_NONE))
    {
        toolbox_position pos;
        os_box extent;

        // Prepare the window contents for display
        configinst_pre(win);

        // Read the window size
        window_get_extent(0, object, &extent);

        // Calculate the required window position
        pos.top_left.x = centre_x - (extent.x1 - extent.x0) / 2;
        pos.top_left.y = centre_y + (extent.y1 - extent.y0) / 2;

        // Open the window at the selected position
        toolbox_show_object(0, object, toolbox_POSITION_TOP_LEFT, &pos,
                            toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);
    }

    // Store the window details
    configinst_win_open = win;
}

/*
    Parameters  : void
    Returns     : void
    Description : Open the installation window or move the window to the
                  front if it is already open.
*/
void configinst_open(void)
{
    // Open the required window
    configinst_show(configinst_win_open == CONFIGINST_NONE
                    ? (configfile_present
                       ? CONFIGINST_INSTALL_OPTIONS
                       : CONFIGINST_WELCOME)
                    : configinst_win_open);
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : An action button has been clicked, so perform a relevant
                  action.
*/
static bool configinst_action(bits event_code, toolbox_action *action,
                              toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // Action depends upon which button was clicked
    switch (id_block->this_cmp)
    {
        case CONFIGINST_ICON_CANCEL:
            // Cancel the installation process
            quit_quit();
            break;

        case CONFIGINST_ICON_CONTINUE:
            // Continue to the next page of the installation process
            configinst_show(configinst_post(configinst_win_open));
            break;

        case CONFIGINST_ICON_CUSTOM:
            // Continue to the manual installation page
            configinst_show(CONFIGINST_INSTALL_OPTIONS);
            break;

        case CONFIGINST_ICON_HELP:
            // Start the interactive help viewer
            go_start_task("Resources:$.Apps.!Help", NULL, NULL);
            break;

        default:
            // No other action buttons should exist
            break;
    }

    // Quit if no window left open
    if (configinst_win_open == CONFIGINST_NONE) quit_quit();

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : A drag operation has been completed. This sends a
                  Message_DataSave to the destination to find the path.
*/
static bool configinst_drag(bits event_code, toolbox_action *action,
                            toolbox_block *id_block, void *handle)
{
    draggable_action_drag_ended *drag = (draggable_action_drag_ended *)
                                         &(action->data);
    wimp_message msg;

    NOT_USED(event_code);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Send message to find the pathname
    msg.size = sizeof(wimp_message);
    msg.your_ref = 0;
    msg.action = message_DATA_SAVE;
    msg.data.data_xfer.w = drag->ids.wimp.w;
    msg.data.data_xfer.i = drag->ids.wimp.i;
    msg.data.data_xfer.pos = drag->pos;
    msg.data.data_xfer.est_size = 0;
    msg.data.data_xfer.file_type = 0;
    msg.data.data_xfer.file_name[0] = 0;
    wimp_send_message(wimp_USER_MESSAGE_RECORDED, &msg,
                      (wimp_t) drag->ids.wimp.w);

    // Claim event
    return TRUE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataLoad wimp message events.
*/
static bool configinst_data_load(wimp_message *message, void *handle)
{
    toolbox_o obj;
    toolbox_c cmp;

    NOT_USED(handle);

    // Convert the details to toolbox identifiers
    window_wimp_to_toolbox(0, message->data.data_xfer.w,
                           message->data.data_xfer.i, &obj, &cmp);

    // Action depends upon the window
    if (obj == configinst_id[CONFIGINST_SOFTWARE])
    {
        // Decode the gadget ID
        switch (cmp)
        {
            case CONFIGINST_PC_ICON:
            case CONFIGINST_PC_ICON_DROP:
            case CONFIGINST_PC_ICON_ICON:
            case CONFIGINST_PC_ICON_LABEL:
                // The PC card software
                if (message->data.data_xfer.file_type
                    == osfile_TYPE_APPLICATION)
                {
                    // Set the writable field
                    configinst_pc_icon = message->data.data_xfer.file_name;
                }
                break;

            case CONFIGINST_CONFIG_ICON:
            case CONFIGINST_CONFIG_ICON_DROP:
            case CONFIGINST_CONFIG_ICON_ICON:
            case CONFIGINST_CONFIG_ICON_LABEL:
                // The PC card configuration software
                if (message->data.data_xfer.file_type
                    == osfile_TYPE_APPLICATION)
                {
                    // Set the writable field
                    configinst_config_icon = message->data.data_xfer.file_name;
                }
                break;

            default:
                // Do not care about other gadgets
                break;
        }
    }
    else if (obj == configinst_id[CONFIGINST_DOS])
    {
        // Decode the gadget ID
        switch (cmp)
        {
            case CONFIGINST_PARTITION_ICON:
            case CONFIGINST_PARTITION_ICON_DROP:
            case CONFIGINST_PARTITION_ICON_ICON:
            case CONFIGINST_PARTITION_ICON_LABEL:
                // The DOS partition
                if (message->data.data_xfer.file_type == osfile_TYPE_DOS_DISC)
                {
                    // Set the writable field
                    configinst_partition_icon = message->data.data_xfer.file_name;
                }
                break;

            default:
                // Do not care about other gadgets
                break;
        }
    }
    else if (obj == configinst_id[CONFIGINST_INSTALL_MODIFY])
    {
        // Decode the gadget ID
        switch (cmp)
        {
            case CONFIGINST_MODIFY_ICON:
            case CONFIGINST_MODIFY_ICON_DROP:
            case CONFIGINST_MODIFY_ICON_ICON:
            case CONFIGINST_MODIFY_ICON_LABEL:
                // The RISC OS path to the root of the partition
                if ((message->data.data_xfer.file_type == osfile_TYPE_DOS_DISC)
                    || (message->data.data_xfer.file_type == osfile_TYPE_DIR))
                {
                    // Set the writable field
                    configinst_modify_icon = message->data.data_xfer.file_name;
                }
                else
                {
                    char *ptr;

                    // Remove any leafname
                    ptr = strrchr(message->data.data_xfer.file_name, '.');
                    if (ptr) *ptr = 0;

                    // Set the writable field
                    configinst_modify_icon = message->data.data_xfer.file_name;
                }
                break;

            default:
                // Do not care about other gadgets
                break;
        }
    }
    else if (obj == configinst_id[CONFIGINST_INSTALL_DRV_RISCOS])
    {
        // Decode the gadget ID
        switch (cmp)
        {
            case CONFIGINST_DRV_RISCOS_ICON:
            case CONFIGINST_DRV_RISCOS_ICON_DROP:
            case CONFIGINST_DRV_RISCOS_ICON_ICON:
            case CONFIGINST_DRV_RISCOS_ICON_LABEL:
                // The RISC OS path to the installation directory
                if ((message->data.data_xfer.file_type == osfile_TYPE_DOS_DISC)
                    || (message->data.data_xfer.file_type == osfile_TYPE_DIR))
                {
                    // Set the writable field
                    configinst_drv_riscos_icon = message->data.data_xfer.file_name;
                }
                break;

            default:
                // Do not care about other gadgets
                break;
        }
    }
    else if (obj == configinst_id[CONFIGINST_INSTALL_UTIL_RISCOS])
    {
        // Decode the gadget ID
        switch (cmp)
        {
            case CONFIGINST_UTIL_RISCOS_ICON:
            case CONFIGINST_UTIL_RISCOS_ICON_DROP:
            case CONFIGINST_UTIL_RISCOS_ICON_ICON:
            case CONFIGINST_UTIL_RISCOS_ICON_LABEL:
                // The RISC OS path to the installation directory
                if ((message->data.data_xfer.file_type == osfile_TYPE_DOS_DISC)
                    || (message->data.data_xfer.file_type == osfile_TYPE_DIR))
                {
                    // Set the writable field
                    configinst_util_riscos_icon = message->data.data_xfer.file_name;
                }
                break;

            default:
                // Do not care about other gadgets
                break;
        }
    }
    else if (obj == configinst_id[CONFIGINST_INSTALL_DEV_RISCOS])
    {
        // Decode the gadget ID
        switch (cmp)
        {
            case CONFIGINST_DEV_RISCOS_ICON:
            case CONFIGINST_DEV_RISCOS_ICON_DROP:
            case CONFIGINST_DEV_RISCOS_ICON_ICON:
            case CONFIGINST_DEV_RISCOS_ICON_LABEL:
                // The RISC OS path to the installation directory
                if (message->data.data_xfer.file_type == osfile_TYPE_DIR)
                {
                    // Set the writable field
                    configinst_dev_riscos_icon = message->data.data_xfer.file_name;
                }
                break;

            default:
                // Do not care about other gadgets
                break;
        }
    }
    else if (obj == configinst_id[CONFIGINST_INSTALL_DEV_DOS])
    {
        // Decode the gadget ID
        switch (cmp)
        {
            case CONFIGINST_DEV_DOS_ICON:
            case CONFIGINST_DEV_DOS_ICON_DROP:
            case CONFIGINST_DEV_DOS_ICON_ICON:
            case CONFIGINST_DEV_DOS_ICON_LABEL:
                // The RISC OS path to the installation directory
                if ((message->data.data_xfer.file_type == osfile_TYPE_DOS_DISC)
                    || (message->data.data_xfer.file_type == osfile_TYPE_DIR))
                {
                    // Set the writable field
                    configinst_dev_dos_icon = message->data.data_xfer.file_name;
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
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_DataSaveAck wimp message events.
*/
static bool configinst_data_save_ack(wimp_message *message, void *handle)
{
    toolbox_o object = configinst_id[configinst_win_open];

    NOT_USED(handle);

    // No action if an illegal size returned
    if (message->data.data_xfer.est_size != -1)
    {
        int len = strlen(message->data.data_xfer.file_name);
        if (0 < len) message->data.data_xfer.file_name[len - 1] = 0;

        // Action depends upon the open window
        switch (configinst_win_open)
        {
            case CONFIGINST_INSTALL_DRV_RISCOS:
                // The PC device driver
                configinst_drv_riscos_icon = message->data.data_xfer.file_name;
                break;

            case CONFIGINST_INSTALL_UTIL_RISCOS:
                // The PC command line utilities
                configinst_util_riscos_icon = message->data.data_xfer.file_name;
                break;

            case CONFIGINST_INSTALL_DEV_RISCOS:
                // The RISC OS software development files
                configinst_dev_riscos_icon = message->data.data_xfer.file_name;
                break;

            case CONFIGINST_INSTALL_DEV_DOS:
                // The PC software development files
                configinst_dev_dos_icon = message->data.data_xfer.file_name;
                break;

            default:
                // There should be no draggable icons in other windows
                break;
        }
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : handle    - The user specified handle.
    Returns     : void
    Description : The function that is called after the user has responded to
                  a prompt to quit by selecting the continue option.
*/
static void configinst_quit_selected(void *handle)
{
    NOT_USED(handle);

    // Restore the previous configuration
    if (configfile_modified) configfile_read();

    // Restore the previous installation options
    if (instfile_modified) instfile_read();
}

/*
    Parameters  : handle    - The user specified handle.
                  func      - Variable to receive a pointer to the function
                              to call if the user selects the quit option.
    Returns     : char *    - Prompt for the user, or NULL if no objection.
    Description : This function is called when the software is about to quit.
                  A string is returned if there is any objection.
*/
const char *configinst_quit_check(void *handle, quit_func_quit *func)
{
    NOT_USED(handle);

    // Set the quit function
    *func = configinst_quit_selected;

    // Return the string if required
    return (configfile_modified || instfile_modified)
           && (configinst_win_open != CONFIGINST_NONE)
           ? lookup_token("QuitInstall") : NULL;
}

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the installation windows.
*/
void configinst_initialise(void)
{
    int i;

    // Read the installation options if any
    instfile_read();

    // Register a quit handler
    quit_register(configinst_quit_check);

    // Prepare all the required windows
    for (i = 0; i <= CONFIGINST_MAX; i++)
    {
        // Create the window
        configinst_id[i] = toolbox_create_object(0, (toolbox_id)
                                                    configinst_name[i]);

        // Register event handlers
        event_register_toolbox_handler(configinst_id[i],
                                       action_ACTION_BUTTON_SELECTED,
                                       configinst_action, NULL);
        event_register_toolbox_handler(configinst_id[i],
                                       action_DRAGGABLE_DRAG_ENDED,
                                       configinst_drag, NULL);
    }

    // Attach the gadgets
    configinst_pc_icon.object = configinst_id[CONFIGINST_SOFTWARE];
    configinst_pc_icon_drop.object = configinst_id[CONFIGINST_SOFTWARE];
    configinst_pc_icon_icon.object = configinst_id[CONFIGINST_SOFTWARE];
    configinst_pc_icon_label.object = configinst_id[CONFIGINST_SOFTWARE];
    configinst_config_icon.object = configinst_id[CONFIGINST_SOFTWARE];
    configinst_config_icon_drop.object = configinst_id[CONFIGINST_SOFTWARE];
    configinst_config_icon_icon.object = configinst_id[CONFIGINST_SOFTWARE];
    configinst_config_icon_label.object = configinst_id[CONFIGINST_SOFTWARE];
    configinst_partition_icon.object = configinst_id[CONFIGINST_DOS];
    configinst_partition_icon_drop.object = configinst_id[CONFIGINST_DOS];
    configinst_partition_icon_icon.object = configinst_id[CONFIGINST_DOS];
    configinst_partition_icon_label.object = configinst_id[CONFIGINST_DOS];
    configinst_opt_icon_pc_drv.object = configinst_id[CONFIGINST_INSTALL_OPTIONS];
    configinst_opt_icon_pc_util.object = configinst_id[CONFIGINST_INSTALL_OPTIONS];
    configinst_opt_icon_modify_config.object = configinst_id[CONFIGINST_INSTALL_OPTIONS];
    configinst_opt_icon_modify_autoexec.object = configinst_id[CONFIGINST_INSTALL_OPTIONS];
    configinst_opt_icon_dev_riscos.object = configinst_id[CONFIGINST_INSTALL_OPTIONS];
    configinst_opt_icon_dev_dos.object = configinst_id[CONFIGINST_INSTALL_OPTIONS];
    configinst_modify_icon.object = configinst_id[CONFIGINST_INSTALL_MODIFY];
    configinst_modify_icon_drop.object = configinst_id[CONFIGINST_INSTALL_MODIFY];
    configinst_modify_icon_icon.object = configinst_id[CONFIGINST_INSTALL_MODIFY];
    configinst_modify_icon_label.object = configinst_id[CONFIGINST_INSTALL_MODIFY];
    configinst_drv_riscos_icon.object = configinst_id[CONFIGINST_INSTALL_DRV_RISCOS];
    configinst_drv_riscos_icon_drop.object = configinst_id[CONFIGINST_INSTALL_DRV_RISCOS];
    configinst_drv_riscos_icon_icon.object = configinst_id[CONFIGINST_INSTALL_DRV_RISCOS];
    configinst_drv_riscos_icon_label.object = configinst_id[CONFIGINST_INSTALL_DRV_RISCOS];
    configinst_drv_riscos_same.object = configinst_id[CONFIGINST_INSTALL_DRV_RISCOS];
    configinst_drv_dos_icon.object = configinst_id[CONFIGINST_INSTALL_DRV_DOS];
    configinst_drv_dos_icon_icon.object = configinst_id[CONFIGINST_INSTALL_DRV_DOS];
    configinst_drv_dos_icon_label.object = configinst_id[CONFIGINST_INSTALL_DRV_DOS];
    configinst_drv_dos_same.object = configinst_id[CONFIGINST_INSTALL_DRV_DOS];
    configinst_util_riscos_icon.object = configinst_id[CONFIGINST_INSTALL_UTIL_RISCOS];
    configinst_util_riscos_icon_drop.object = configinst_id[CONFIGINST_INSTALL_UTIL_RISCOS];
    configinst_util_riscos_icon_icon.object = configinst_id[CONFIGINST_INSTALL_UTIL_RISCOS];
    configinst_util_riscos_icon_label.object = configinst_id[CONFIGINST_INSTALL_UTIL_RISCOS];
    configinst_util_dos_icon.object = configinst_id[CONFIGINST_INSTALL_UTIL_DOS];
    configinst_util_dos_icon_icon.object = configinst_id[CONFIGINST_INSTALL_UTIL_DOS];
    configinst_util_dos_icon_label.object = configinst_id[CONFIGINST_INSTALL_UTIL_DOS];
    configinst_dev_riscos_icon.object = configinst_id[CONFIGINST_INSTALL_DEV_RISCOS];
    configinst_dev_riscos_icon_drop.object = configinst_id[CONFIGINST_INSTALL_DEV_RISCOS];
    configinst_dev_riscos_icon_icon.object = configinst_id[CONFIGINST_INSTALL_DEV_RISCOS];
    configinst_dev_riscos_icon_label.object = configinst_id[CONFIGINST_INSTALL_DEV_RISCOS];
    configinst_dev_dos_icon.object = configinst_id[CONFIGINST_INSTALL_DEV_DOS];
    configinst_dev_dos_icon_drop.object = configinst_id[CONFIGINST_INSTALL_DEV_DOS];
    configinst_dev_dos_icon_icon.object = configinst_id[CONFIGINST_INSTALL_DEV_DOS];
    configinst_dev_dos_icon_label.object = configinst_id[CONFIGINST_INSTALL_DEV_DOS];

    // Register WIMP message handlers
    event_register_message_handler(message_DATA_LOAD,
                                   configinst_data_load, NULL);
    event_register_message_handler(message_DATA_SAVE_ACK,
                                   configinst_data_save_ack, NULL);

    // Register toolbox message handlers
    event_register_toolbox_handler(configinst_id[CONFIGINST_INSTALL_OPTIONS],
                                   action_OPTION_BUTTON_STATE_CHANGED,
                                   configinst_opt_modified, NULL);
}
