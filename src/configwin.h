/*
    File        : configwin.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1996-2001, 2019
    Description : Configuration window handling for !ARMEdit.

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

#ifndef configwin_h
#define configwin_h

/*
    Parameters  : void
    Returns     : void
    Description : Open the configuration window or move the window to the
                  front if it is already open.
*/
void configwin_open(void);

/*
    Parameters  : void
    Returns     : void
    Description : Initialise the configuration windows.
*/
void configwin_initialise(void);

#endif
