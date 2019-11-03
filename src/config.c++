/*
    File        : config.c++
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Handling of configuration.

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
#include "config.h"

// Include system header files
#include <stdlib.h>
#include <string.h>

// Include oslib header files
#include "menu.h"
#include "wimp.h"
extern "C" {
#include "event.h"
}

// Include other project header files
#include "armeditmsg.h"
#include "armeditswi.h"
#include "configfile.h"
#include "go.h"
#include "main.h"
#include "multi.h"
#include "quit.h"
#include "utils.h"

// The path of the configuration application
#define CONFIG_PATH "<ARMEdit$Dir>.Configure"

// The current configuration editor task
static wimp_t config_task = 0;
static armedit_open_reason config_status = ARMEDIT_OPEN_NONE;

// The iconbar menu handle
static toolbox_o config_menu_id;

/*
    Parameters  : void
    Returns     : void
    Description : Read or re-read the configuration.
*/
static void config_read(void)
{
    os_error *er;

    // Read the configuration file
    configfile_read();

    // Update the multiple configurations
    multi_update();

    // Set the speed control
    er = xarmedit_polling(configfile_speed_fore, configfile_speed_back,
                          NULL, NULL);
    if (er) report_error(er);
}

/*
    Parameters  : void
    Returns     : void
    Description : Update the shaded state of the relevant iconbar menu entries.
*/
static void config_menu(void)
{
    // Do not update the menu if it has not been created
    if (config_menu_id)
    {
        // Fade out unsuitable entries
        menu_set_fade(0, config_menu_id, IBAR_MENU_CONFIGURE,
                      config_status == ARMEDIT_OPEN_INSTALL);
        menu_set_fade(0, config_menu_id, IBAR_MENU_INSTALL,
                      config_status == ARMEDIT_OPEN_CONFIGURE);

        // Add ticks where required
        menu_set_tick(0, config_menu_id, IBAR_MENU_CONFIGURE,
                      config_status == ARMEDIT_OPEN_CONFIGURE);
        menu_set_tick(0, config_menu_id, IBAR_MENU_INSTALL,
                      config_status == ARMEDIT_OPEN_INSTALL);
    }
}

/*
    Parameters  : reason    - The window to open.
    Returns     : void
    Description : Open the specified configuration or installation window.
*/
static void config_open(armedit_open_reason reason)
{
    wimp_message msg;

    // Store the required status
    config_status = reason;

    // Send Message_ARMEditOpen to the configuration utility
    msg.size = sizeof(wimp_message);
    msg.your_ref = 0;
    msg.action = message_ARMEDIT_OPEN;
    ((wimp_message_armedit_open *) &msg.data)->reason = reason;
    wimp_send_message(wimp_USER_MESSAGE_RECORDED, &msg, config_task);

    // Update the iconbar menu status
    config_menu();
}

/*
    Parameters  : void
    Returns     : void
    Description : Edit the configuration.
*/
void config_edit(void)
{
    // Open the configuration window
    config_open(ARMEDIT_OPEN_CONFIGURE);
}

/*
    Parameters  : void
    Returns     : void
    Description : Start the installer.
*/
void config_install(void)
{
    // Open the installation window
    config_open(ARMEDIT_OPEN_INSTALL);
}

/*
    Parameters  : task      - The handle of the new task.
                  handle    - An unused handle.
    Returns     : void
    Description : Open the relevant window after launching the configuration
                  task.
*/
static void config_launched(wimp_t task, void *handle)
{
    NOT_USED(handle);

    // Store the task handle
    config_task = task;

    if (task)
    {
        // Send a message to open the window
        config_open(config_status);

        // Register the child to handle quit messages
        quit_child(task);
    }
    else
    {
        // Just update the status
        config_status = ARMEDIT_OPEN_NONE;
    }

    // Ensure that the menu entries are correct
    config_menu();
}

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle undelivered messages.
*/
static int config_message_acknowledge(wimp_event_no event_code,
                                      wimp_block *block,
                                      toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(handle);
    NOT_USED(id_block);

    // Split into different message types
    switch (block->message.action)
    {
        case message_ARMEDIT_OPEN:
            // Start a new copy of the configuration application
            go_start_task(CONFIG_PATH, config_launched, NULL);
            break;

        default:
            // Do not care about other messages
            break;
    }

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_ARMEditConfigSaved messages. This is used to
                  reload the configuration when it has been saved.
*/
static int config_message_saved(wimp_message *message, void *handle)
{
    NOT_USED(message);
    NOT_USED(handle);

    // Reload the configuration
    config_read();

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : message   - The wimp message.
                  handle    - An unused handle.
    Returns     : int       - Was the event claimed.
    Description : Handle Message_TaskCloseDown messages. This is used to
                  detect when the configuration task exits.
*/
static int config_message_closed(wimp_message *message, void *handle)
{
    NOT_USED(handle);

    // Check whether it is the configuration utility
    if (message->sender == config_task)
    {
        // Mark the task as not loaded
        config_task = 0;
        config_status = ARMEDIT_OPEN_NONE;

        // Update the menu
        config_menu();
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
    Description : Obtain the handle of the iconbar menu.
*/
bool config_tb_autocreated(bits event_code, toolbox_action *action,
                           toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(handle);

    if (!strcmp(action->data.created.name, "IBarMenu"))
    {
        // It is the main menu
        config_menu_id = id_block->this_obj;
    }

    // Do not claim the event - it is too generally useful
    return FALSE;
}

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the configuration handler.
*/
void config_initialise(void)
{
    // Install handlers
    event_register_wimp_handler(event_ANY, wimp_USER_MESSAGE_ACKNOWLEDGE,
                                config_message_acknowledge, NULL);
    event_register_message_handler(message_ARMEDIT_CONFIG_SAVED,
                                   config_message_saved, NULL);
    event_register_message_handler(message_TASK_CLOSE_DOWN,
                                   config_message_closed, NULL);

    // Read the configuration
    config_read();
}
