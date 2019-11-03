/*
    File        : pcpro.h
    Date        : 15-May-01
    Author      : Â© A.Thoukydides, 1997, 1998, 2019
    Description : Operations and definitions related to new versions of PCPro.
                  These include support for the multiple configuration chooser
                  and the updated configuration editor.

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

#ifndef pcpro_h
#define pcpro_h

// Include oslib header files
#include "toolbox.h"

// Message numbers
#define message_PCPRO 0x44680
#define message_PCCONFIG 0x44681

// Message_PCPro
typedef enum
{
    PCPRO_USING_CONFIGURATION
} pcpro_reason;
typedef struct
{
    pcpro_reason reason;
    char path[232];
} wimp_message_pcpro;

// Message_PCConfig
typedef enum
{
    PCPRO_PCCONFIG_STARTING,
    PCPRO_PCCONFIG_EXITING,
    PCPRO_PCCONFIG_SAVED,
    PCPRO_PCCONFIG_DELETED,
    PCPRO_PCCONFIG_CREATED,
    PCPRO_PCCONFIG_SELECTED,
    PCPRO_PCCONFIG_EDIT_SPECIFIED = 0x100,
    PCPRO_PCCONFIG_EDIT_FAILED
} pcpro_pcconfig_reason;
typedef struct
{
    pcpro_pcconfig_reason reason;
    char path[232];
} wimp_message_pcconfig;

// A flag to control the emulation of older PCConfig versions
extern int pcpro_fake_old;

/*
    Parameters  : path          - The complete path of the PC front-end
                                  software to check.
    Returns     : const char *  - The version number string, or NULL if unable
                                  to read it.
    Description : Read the version number string from the first line of the
                  !Run file.
*/
const char *pcpro_read_version(const char *path);

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the PCPro handlers.
*/
void pcpro_initialise(void);

#endif
