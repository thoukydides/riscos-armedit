/*
    File        : multiedit.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Editing of multiple PC card configurations.

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

#ifndef multiedit_h
#define multiedit_h

// Inlcude oslib header files
#include "toolbox.h"

// Include project header files
#include "extedit.h"
#include "multiconf.h"

// Details for a configuration
struct multiedit_details
{
    multiedit_details *next;
    multiedit_details *prev;
    ExtEdit *edit;
    multiconf_list *ptr;
};

// The current configuration
extern multiedit_details multiedit_current;

// The selected configuration
extern multiedit_details *multiedit_select;

// Head of configurations list
extern multiedit_details *multiedit_head;

/*
    Parameters  : void
    Returns     : void
    Description : Produce a backup of the current configuration. This should
                  normally be used after loading or saving the configuration,
                  but before editing is started.
*/
void multiedit_backup(void);

/*
    Parameters  : void
    Returns     : void
    Description : Save the current configurations.
*/
void multiedit_save(void);

/*
    Parameters  : void
    Returns     : void
    Description : Load the current configurations.
*/
void multiedit_load(void);

/*
    Parameters  : from              - The configuration to copy.
    Returns     : multiedit_details - The new configuration.
    Description : Create a copy of a specified configuration.
*/
multiedit_details *multiedit_copy(multiedit_details *from);

/*
    Parameters  : ptr   - The configuration to delete.
    Returns     : void
    Description : Delete the specified configuration.
*/
void multiedit_delete(multiedit_details *ptr);

#endif
