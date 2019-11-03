/*
    File        : armeditmsg.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : WIMP messages used by the !ARMEdit application.

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

#ifndef armeditmsg_h
#define armeditmsg_h

// Message numbers
#define message_ARMEDIT_CONFIG_START 0x4BC40
#define message_ARMEDIT_CONFIG_END 0x4BC41
#define message_ARMEDIT_CONFIG_SAVED 0x4BC42
#define message_ARMEDIT_LAUNCHED 0x4BC43
#define message_ARMEDIT_OPEN 0x4BC44
#define message_ARMEDIT_PRE_QUIT 0x4BC45
#define message_ARMEDIT_RESTART_QUIT 0x4BC46

// Message_ARMEditLaunched
typedef struct
{
    int id;
    wimp_t handle;
} wimp_message_armedit_launched;

// Message_ARMEditOpen
typedef enum
{
    ARMEDIT_OPEN_NONE,
    ARMEDIT_OPEN_CONFIGURE,
    ARMEDIT_OPEN_INSTALL
} armedit_open_reason;
typedef struct
{
    armedit_open_reason reason;
} wimp_message_armedit_open;

#endif
