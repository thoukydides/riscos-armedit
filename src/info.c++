/*
    File        : info.c++
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
#include "info.h"

// Include clib header files
#include <string.h>

// Include oslib header files
#include "displayfield.h"
#include "proginfo.h"
extern "C" {
#include "event.h"
}

// Include other project header files
#include "main.h"
#include "utils.h"

// Length of time to display copyright message for
#define INFO_DELAY 5

// Quicker polling interval
#define INFO_POLL_INTERVAL 10

// The text to display
static const char info_author[] =
    "© A.Thoukydides, 1995-2001";
static const char info_others[] =
    "<Many thanks to...>"
    "Aidan Corey>"
    "Dominic Symes>"
    "Ian Harvey>"
    "Karl Davis>"
    "Matthew Bloch>"
    "Miles Sabin>"
    "Neil Bingham>"
    "Paul Gardiner>"
    "Robin Watts>"
    "Tim Tyler>"
    "Wookey>"
    "<The other testers...>"
    "Alex Card>"
    "Andreas Walter>"
    "Andrew Flegg>"
    "Andy Piper>"
    "Barry Punchard>"
    "Ben Ollivere>"
    "Bob Brand>"
    "Brian O'Keeffe>"
    "Brian Tucker>"
    "Chris Claydon>"
    "Chris Walker>"
    "Chris Ward>"
    "Colin Bennett>"
    "Colin Davies>"
    "Daniel Shimmin>"
    "Dave Ferguson>"
    "David Gommeren>"
    "David Randles>"
    "David Ruck>"
    "David Taylor>"
    "Dominic Betts>"
    "Dominic Harvey>"
    "Dominic Plunkett>"
    "Douglas McKenzie>"
    "Duncan McPherson>"
    "Ed Avis>"
    "Eduard Pfarr>"
    "Frits Polak>"
    "Frode Wells>"
    "Geoffrey Khoo>"
    "Geoffrey Nuttall>"
    "Goetz Kohlberg>"
    "Graeme Foster>"
    "Ian Cripps>"
    "James Green>"
    "James Macgill>"
    "Jason Keeler>"
    "John Armstrong>"
    "John Daniels>"
    "John Ferguson>"
    "John Harrison>"
    "John Martin>"
    "John McLaughlin>"
    "Jonathan Brady>"
    "Julian Burman>"
    "Justin Fletcher>"
    "Keith Buckler>"
    "Laurie van Someren>"
    "Luigi Di Giuseppe>"
    "Manuel Timmers>"
    "Mark Jennings>"
    "Mark Sinke>"
    "Mark Stephens>"
    "Mark Wooding>"
    "Martin Tillman>"
    "Matthew Israelsohn>"
    "Mel Sutton>"
    "Michael Carey>"
    "Mike Hobbs>"
    "Pasi Juppo>"
    "Paul Augood>"
    "Paul Skirrow>"
    "Pete Goodliffe>"
    "Peter Bell>"
    "Peter Naulls>"
    "Peter Noyce>"
    "Peter Smith>"
    "Pierre Bernhardt>"
    "Piers Williams>"
    "Rafael Hornos>"
    "Ray Dawson>"
    "Reuben Thomas>"
    "Richard Bradshaw>"
    "Richard North>"
    "Rob Cuesta>"
    "Rob Davison>"
    "Rob Warner>"
    "Robert Steindl>"
    "Roberto Casula>"
    "Rocky Grove>"
    "Ronald Kemeling>"
    "Rosalind Share>"
    "Sam Spencer>"
    "Samuel Kock>"
    "Simon Anthony>"
    "Simon Foster>"
    "Stan Carter>"
    "Stephen Wright>"
    "Steve Bradbury>"
    "Steve Cashmore>"
    "Steve Pringle>"
    "Stuart Gilson>"
    "Theo Markettos>"
    "Tim Powys-Lybbe>"
    "Tim Watson>"
    "Tony Hopstaken>"
    "Torsten Doege>"
    "<And not forgetting...>"
    "<Oscar, Flopsy & Mopsy>";

// The program information window ID
static toolbox_o info_id;

// The update status
static const char *info_ptr;
static int info_count = 0;
static int info_len = 0;

/*
    Parameters  : event_code    - The event number.
                  wimp_block    - The wimp poll block.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : Handle wimp null polls.
*/
static bool info_handler_null(wimp_event_no event_code, wimp_block *block,
                              toolbox_block *id_block, void *handle)
{
    char *ptr;
    char str[256];

    NOT_USED(event_code);
    NOT_USED(block);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Choose the string to display
    if (info_ptr)
    {
        ptr = strchr(info_ptr, '>');
        if (ptr)
        {
            if (*info_ptr == '<')
            {
                info_len++;
                strncpy(str, info_ptr + 1, info_len);
                str[info_len] = 0;
                if (info_len + 1== ptr - info_ptr)
                {
                    info_ptr = ptr + 1;
                    info_len = 0;
                }
            }
            else
            {
                strncpy(str, info_ptr, ptr - info_ptr);
                str[ptr - info_ptr] = 0;
                info_ptr = ptr + 1;
                info_len = 0;
            }
        }
        else
        {
            strcpy(str, info_author);
            info_ptr = NULL;
            info_count = INFO_DELAY;
        }
        displayfield_set_value(0, proginfo_get_window_id(0, info_id),
                               proginfo_AUTHOR, str);
    }
    else
    {
        if (!(info_count--)) info_ptr = info_others;
    }
    if (info_len) poll_interval = INFO_POLL_INTERVAL;
    else poll_interval = POLL_INTERVAL_DEFAULT;

    // Do not claim the event
    return FALSE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : ProgInfo window is about to be shown.
*/
bool info_handler_shown(bits event_code, toolbox_action *action,
                        toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(handle);

    // Install handlers
    info_id = id_block->this_obj;
    event_register_wimp_handler(event_ANY, wimp_NULL_REASON_CODE,
                                info_handler_null, NULL);

    // Set the version string
    proginfo_set_version(0, info_id, lookup_token("Version"));

    // Reset the rolling display
    info_ptr = NULL;
    info_count = INFO_DELAY;
    info_len = 0;
    poll_interval = POLL_INTERVAL_DEFAULT;
    displayfield_set_value(0, proginfo_get_window_id(0, info_id),
                           proginfo_AUTHOR, info_author);

    // Claim the event
    return TRUE;
}

/*
    Parameters  : event_code    - The event number.
                  action        - The toolbox event.
                  id_block      - The toolbox ID block.
                  handle        - An unused handle.
    Returns     : int           - Was the event claimed.
    Description : ProgInfo window is about to be hidden.
*/
bool info_handler_hidden(bits event_code, toolbox_action *action,
                         toolbox_block *id_block, void *handle)
{
    NOT_USED(event_code);
    NOT_USED(action);
    NOT_USED(id_block);
    NOT_USED(handle);

    // Deinstall handlers
    event_deregister_wimp_handler(event_ANY, wimp_NULL_REASON_CODE,
                                  info_handler_null, NULL);

    // Claim the event
    return TRUE;
}
