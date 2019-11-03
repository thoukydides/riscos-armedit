/*
    File        : info.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Handling of the program information window.

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

#ifndef info_h
#define info_h

// Include oslib header files
#include "toolbox.h"

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : ProgInfo window is about to be shown.
*/
bool info_handler_shown(bits event_code, toolbox_action *action,
                        toolbox_block *id_block, void *handle);

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : ProgInfo window is about to be hidden.
*/
bool info_handler_hidden(bits event_code, toolbox_action *action,
                         toolbox_block *id_block, void *handle);

#endif
