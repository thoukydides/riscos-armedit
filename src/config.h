/*
    File        : config.h
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

#ifndef config_h
#define config_h

// Include oslib header files
#include "toolbox.h"
#include "types.h"

// A function to call when a quit sequence should be continued
typedef void (* config_quit_func)(void);

/*
    Parameters  : void
    Returns     : void
    Description : Edit the configuration.
*/
void config_edit(void);

/*
    Parameters  : void
    Returns     : void
    Description : Start the installer.
*/
void config_install(void);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Obtain the handle of the iconbar menu.
*/
bool config_tb_autocreated(bits event_code, toolbox_action *action,
                           toolbox_block *id_block, void *handle);

/*
    Parameters  : func  - The function to call to proceed with the quit.
    Returns     : void
    Description : Check whether it is alright for the application to quit.
                  This should not be used when Message_PreQuit is received;
                  the configuration application will respond directly.
*/
void config_prequit(config_quit_func func);

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the configuration handler.
*/
void config_initialise(void);

#endif
