/*
    File        : speed.c++
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Handling of the speed control window.

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
#include "speed.h"

// Include alexlib header files
#include "optionbutton_c.h"
#include "numberrange_c.h"

// Include oslib header files
#include "macros.h"
#include "window.h"
extern "C" {
#include "event.h"
}

// Include project header files
#include "armeditswi.h"
#include "configfile.h"
#include "utils.h"

// Objects to represent the gadgets
static optionbutton_c speed_icon_fore(0x00);
static numberrange_c speed_icon_fore_val(0x01);
static optionbutton_c speed_icon_back(0x02);
static numberrange_c speed_icon_back_val(0x03);

/*
    Parameters  : void
    Returns     : void
    Description : Update icon dependencies.
*/
void speed_update(void)
{
    // Grey out the speed control icons if required
    speed_icon_fore_val.set_faded(!configfile_speed_fore);
    speed_icon_back_val.set_faded(!configfile_speed_back);
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Update the speed and faded icons when something changes.
*/
static bool speed_modified(bits event_code, toolbox_action *action,
                           toolbox_block *id_block, void *handle)
{
    os_error *er;

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);
    NOT_USED(id_block);

    // Read the new settings
    configfile_speed_fore = speed_icon_fore() ? speed_icon_fore_val() : 0;
    configfile_speed_back = speed_icon_back() ? speed_icon_back_val() : 0;

    // Update any icon dependencies
    speed_update();

    // Set the speed control
    er = xarmedit_polling(configfile_speed_fore, configfile_speed_back,
                          NULL, NULL);
    if (er) report_error(er);

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Update the icons just before the window is shown.
*/
static bool speed_show(bits event_code, toolbox_action *action,
                       toolbox_block *id_block, void *handle)
{
    os_error *er;

    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);
    NOT_USED(id_block);

    // Read the current speed
    er = xarmedit_polling(configfile_speed_fore, configfile_speed_back,
                          NULL, NULL);
    if (er) report_error(er);
    else
    {
        // Set the icon states
        speed_icon_fore = configfile_speed_fore;
        if (configfile_speed_fore) speed_icon_fore_val = configfile_speed_fore;
        speed_icon_back = configfile_speed_back;
        if (configfile_speed_back) speed_icon_back_val = configfile_speed_back;
    }

    // Update any icon dependencies
    speed_update();

    // Claim the event
    return TRUE;
}

/*
    Parameters  : id    - The object ID of the newly created window.
    Returns     : void
    Description : Initialise speed control window handling when the window
                  has been created.
*/
void speed_created(toolbox_o id)
{
    // Attach the gadgets
    speed_icon_fore.object = id;
    speed_icon_fore_val.object = id;
    speed_icon_back.object = id;
    speed_icon_back_val.object = id;

    // Register handlers
    event_register_toolbox_handler(id, action_WINDOW_ABOUT_TO_BE_SHOWN,
                                   speed_show, NULL);
    event_register_toolbox_handler(id, action_OPTION_BUTTON_STATE_CHANGED,
                                   speed_modified, NULL);
    event_register_toolbox_handler(id, action_NUMBER_RANGE_VALUE_CHANGED,
                                   speed_modified, NULL);
}
