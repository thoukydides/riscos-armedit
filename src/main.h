/*
    File        : main.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : The main !ARMEdit program.

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

#ifndef main_h
#define main_h

// Include oslib header files
#include "messagetrans.h"
#include "toolbox.h"
#include "wimp.h"

// Time between polls
#define POLL_INTERVAL_DEFAULT 100
extern int poll_interval;

// Component IDs for the menu entries
#define IBAR_MENU_INFO 0x00
#define IBAR_MENU_CONFIGURE 0x01
#define IBAR_MENU_INSTALL 0x02
#define IBAR_MENU_QUIT 0x03
#define IBAR_MENU_START 0x04
#define IBAR_MENU_SPEED 0x05
#define IBAR_MENU_RELOG 0x06
#define IBAR_MENU_HELP 0x07
#define IBAR_MENU_RELOG_NOW 0x10

#endif
