/*
    File        : scsimap.c++
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1997-2001, 2019
    Description : Conversion between SCSI device IDs and drive numbers.

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
#include "scsimap.h"

// Include cpplib header files
#include "string.h"
#include "fstream.h"
#include "sstream.h"

// Include clib header files
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Include oslib header files
#include "os.h"

// Components of the SCSIFS path
#define SCSIMAP_PATH_PREFIX "SCSI::"
#define SCSIMAP_PATH_POSTFIX ".$"
#define SCSIMAP_PATH_UNKNOWN "SCSI::?.$"

// The command to read the configured drive mapping
#define SCSIMAP_REDIRECT "pipe:$.ARMEdit.SCSIFSMap"
#define SCSIMAP_COMMAND_PREFIX "Status "
#define SCSIMAP_COMMAND_POSTFIX " { > " SCSIMAP_REDIRECT " }"
#define SCSIMAP_COMMAND_SCSIFSMAP "SCSIFSMap"
#define SCSIMAP_COMMAND_SCSIFSLINK "SCSIFSLink"

// The mapping from SCSIFS drive number to device ID and partition number
#define SCSIMAP_MAX_DRIVE (8)
static int scsimap_map_device[SCSIMAP_MAX_DRIVE + 1];
static int scsimap_map_partition[SCSIMAP_MAX_DRIVE + 1];

/*
    Parameters  : tag       - The name of the configuration item used for the
                              SCSI drive mapping.
    Returns     : char *    - The mapping that was read.
    Description : Read the current SCSI device mapping string. The format of
                  the returned string depends upon the type of SCSI interface.
*/
static const char *scsimap_command(const char *tag)
{
    static char str[256];
    os_error *err;

    // Construct the command to perform
    sprintf(str, "%s%s%s",
            SCSIMAP_COMMAND_PREFIX, tag, SCSIMAP_COMMAND_POSTFIX);

    // Attempt to read the mapping
    err = xos_cli(str);

    // Open the pipe
    ifstream file(SCSIMAP_REDIRECT);

    // Extract the mapping if possible
    if (err) *str = 0;
    else
    {
        // Skip over the label and white space
        file.get(str, sizeof(str), ' ');
        file >> ws;

        // Read the actual mapping
        file.get(str, sizeof(str), '\n');
    }

    // Return the mapping string
    return str;
}

/*
    Parameters  : map   - The mapping to decode.
    Returns     : void
    Description : Decode the SCSIFS drive mapping as the output from SCSIFSMap
                  for a PowerROM or Power-tec SCSI interface. An example of the
                  format produced is:

                    4->3 5->3
*/
static void scsimap_read_powertec(const char *map)
{
    stringstream str;
    int device;
    int drive;
    char ch1;
    char ch2;

    // Copy the mapping to a string stream
    str << map;

    // Loop through all of the define mappings
    while ((str >> drive >> ch1 >> ch2 >> device)
           && (ch1 == '-') && (ch2 == '>'))
    {
        // Ignore if the drive number is out of range
        if ((0 <= drive) && (drive <= SCSIMAP_MAX_DRIVE))
        {
            int i;

            // Calculate the number of mappings for the partition number
            scsimap_map_partition[drive] = 0;
            for (i = 0; i < SCSIMAP_MAX_DRIVE; i++)
            {
                // Increment the partition number if already used
                if (scsimap_map_device[i] == device)
                {
                    scsimap_map_partition[drive]++;
                }
            }

            // Store this mapping
            scsimap_map_device[drive] = device;
        }
    }
}

/*
    Parameters  : map   - The mapping to decode.
    Returns     : void
    Description : Decode the SCSIFS drive mapping as the output from SCSIFSMap
                  for a Cumana SCSI interface. An example of the format
                  produced is:

                    4= 03  5= 03,1  H0= 7
*/
static void scsimap_read_cumana(const char *map)
{
    stringstream str;

    // Copy the mapping to a string stream
    str << map;

    // Loop until not possible to read more
    while (str >> ws)
    {
        int device;
        int drive;
        int partition;
        char ch;

        // Check the next character
        if (isdigit(str.peek()))
        {
            // This is a drive mapping
            str >> drive;
        }
        else
        {
            // Skip over a host adapter entry
            str >> ch >> drive;
            drive = -1;
        }

        // Read the device ID
        str >> ch >> hex >> device;

        // Check if a partition number is specified
        if (str.peek() == ',') str >> ch >> partition;
        else partition = 0;

        // Store this mapping if valid
        if (str && (0 <= drive) && (drive <= SCSIMAP_MAX_DRIVE))
        {
            scsimap_map_device[drive] = device;
            scsimap_map_partition[drive] = partition;
        }
    }
}

/*
    Parameters  : map   - The mapping to decode.
    Returns     : void
    Description : Decode the SCSIFS drive mapping as the output from SCSIFSLink
                  for a Castle Technology SCSI interface. An example of the
                  format produced is:

                    4: 03-0 5: 03-1 6: None 7: None 0: None 1: None 2: None
                    3: None
*/
static void scsimap_read_castle(const char *map)
{
    stringstream str;
    int drive;
    char ch;

    // Copy the mapping to a string stream
    str << map;

    // Loop through all of the define mappings
    while ((str >> drive >> ch >> ws) && (ch == ':'))
    {
        // Check the next character
        if (isdigit(str.peek()))
        {
            int device;
            int partition;

            // This is probably a drive mapping
            if ((str >> hex >> device >> ch >> partition) && (ch == '-')
                && (0 <= drive) && (drive <= SCSIMAP_MAX_DRIVE))
            {
                scsimap_map_device[drive] = device;
                scsimap_map_partition[drive] = partition;
            }
        }
        else
        {
            // Skip over the text
            while ((str >> ws) && !isdigit(str.peek())) str >> ch;
        }
    }
}

/*
    Parameters  : map   - The mapping to decode.
    Returns     : void
    Description : Decode the SCSIFS drive mapping as the output from SCSIFSLink
                  for a Morley SCSI interface. An example of the format
                  produced is:

                    0:-- 1:-- 2:-- 3:-- 4:3 5:3 6:-- 7:--
*/
static void scsimap_read_morley(const char *map)
{
    stringstream str;
    int drive;
    char ch;

    // Copy the mapping to a string stream
    str << map;

    // Loop through all of the define mappings
    while ((str >> drive >> ch) && (ch == ':'))
    {
        int device;

        // Check if this is a valid drive mapping
        if ((0 <= drive) && (drive <= SCSIMAP_MAX_DRIVE)
            && isdigit(str.peek()) && (str >> device))
        {
            int i;

            // Calculate the number of mappings for the partition number
            scsimap_map_partition[drive] = 0;
            for (i = 0; i < SCSIMAP_MAX_DRIVE; i++)
            {
                // Increment the partition number if already used
                if (scsimap_map_device[i] == device)
                {
                    scsimap_map_partition[drive]++;
                }
            }

            // Store this mapping
            scsimap_map_device[drive] = device;
        }
        else
        {
            // Skip over the hyphens
            while ((str >> ws) && !isdigit(str.peek())) str >> ch;
        }
    }
}

/*
    Parameters  : void
    Returns     : void
    Description : Attempt to read and decode the current drive mapping.
*/
static void scsimap_read(void)
{
    const char *map;
    int i;

    // Clear any previous mapping
    for (i = 0; i <= SCSIMAP_MAX_DRIVE; i++)
    {
        scsimap_map_device[i] = scsimap_map_partition[i] = -1;
    }

    // Read the mapping
    map = scsimap_command(SCSIMAP_COMMAND_SCSIFSMAP);

    // Attempt to recognise the format
    if (strstr(map, "->")) scsimap_read_powertec(map);
    else if (strchr(map, '=')) scsimap_read_cumana(map);
    else
    {
        // Read the mapping using a different command
        map = scsimap_command(SCSIMAP_COMMAND_SCSIFSLINK);

        // Attempt to recognise the format again
        if (strstr(map, ": ")) scsimap_read_castle(map);
        else if (strchr(map, ':')) scsimap_read_morley(map);
    }
}

/*
    Parameters  : drive     - The SCSIFS drive number (0-7).
                  partition - Variable to receive the partition number.
    Returns     : int       - The SCSI device ID, or negative if unknown.
    Description : Attempt to convert an SCSIFS drive number to the equivalent
                  SCSI device ID and partition number.
*/
int scsimap_device(int drive, int *partition)
{
    int device = -1;

    // Read the current mapping
    scsimap_read();

    // Check whether the drive number is valid
    if ((0 <= drive) && (drive <= SCSIMAP_MAX_DRIVE))
    {
        // Find the device details
        device = scsimap_map_device[drive];
        if (partition) *partition = scsimap_map_partition[drive];
    }

    // Return the mapping
    return device;
}

/*
    Parameters  : device    - The SCSI device ID.
                  partition - The partition number on the device.
    Returns     : int       - The SCSIFS drive number, or negative if unknown.
    Description : Attempt to convert a SCSI device ID and partition number to
                  the equivalent SCSIFS drive number.
*/
int scsimap_drive(int device, int partition)
{
    int drive;

    // Read the current mapping
    scsimap_read();

    // Check whether the specified device and partition are used
    drive = SCSIMAP_MAX_DRIVE;
    while ((0 <= drive)
           && ((scsimap_map_device[drive] != device)
               || (scsimap_map_partition[drive] != partition)))
    {
        drive--;
    }

    // Return the drive number
    return drive;
}

/*
    Parameters  : device    - The SCSI device ID.
                  partition - The partition number on the device.
    Returns     : char *    - The RISC OS path for the root directory of the
                              device.
    Description : Attempt to convert a SCSI device ID and partition number to
                  the equivalent RISC OS path.
*/
const char *scsimap_path(int device, int partition)
{
    static char path[256];
    int drive;

    // Attempt to map the device to a drive
    drive = scsimap_drive(device, partition);

    // Generate the path
    if (drive < 0) strcpy(path, SCSIMAP_PATH_UNKNOWN);
    else
    {
        sprintf(path, "%s%i%s",
                SCSIMAP_PATH_PREFIX, drive, SCSIMAP_PATH_POSTFIX);
    }

    // Return a pointer to the partition path
    return path;
}
