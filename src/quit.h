/*
    File        : quit.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1997-2001, 2019
    Description : Quit and pre-quit handling.

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

#ifndef quit_h
#define quit_h

// Include oslib header files
#include "types.h"
#include "wimp.h"

/*
    Parameters  : handle    - The user specified handle.
    Returns     : void
    Description : A function that is called after the user has responded to a
                  prompt to quit by selecting the continue option. This should
                  perform an operation equivalent to marking the data as read.
                  This allows the quit to continue further if it is restarted.
*/
typedef void (* quit_func_quit)(void *handle);

/*
    Parameters  : handle    - The user specified handle.
                  func      - Variable to receive a pointer to the function
                              to call if the user selects the quit option.
    Returns     : char *    - Prompt for the user, or NULL if no objection.
    Description : A function that is called when the software is about to quit.
                  If a string is returned then the user is prompted to allow
                  the quit to be aborted or continued.
*/
typedef const char * (* quit_func_check)(void *handle, quit_func_quit *func);

/*
    Parameters  : void
    Returns     : void
    Description : Start a quit sequence. This calls any registered quit
                  handlers and offers any child tasks the opportunity to
                  object. This should normally only be used when a menu
                  option is selected; quit messages from the task manager
                  or other tasks are handled automatically.
*/
void quit_quit(void);

/*
    Parameters  : handle    - The WIMP handle of the child task.
    Returns     : void
    Description : Register a child task that should be asked before quitting.
                  There is no need to deregister the task; the task is
                  automatically unregistered when it exits.
*/
void quit_child(wimp_t handle);

/*
    Parameters  : func      - The function to register.
                  handle    - User specified data to be passed to the function.
    Returns     : void
    Description : Register a function that will be called when a quit sequence
                  has been started.
*/
void quit_register(quit_func_check func, void *handle = NULL);

/*
    Parameters  : func      - The function to deregister.
                  handle    - User specified data specified when the function
                              was registered.
    Returns     : void
    Description : Deregister a previously registered function.
*/
void quit_deregister(quit_func_check func, void *handle = NULL);

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the quit handler.
*/
void quit_initialise(void);

#endif
