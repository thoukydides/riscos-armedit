/*
    File        : scsimap.h
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

#ifndef scsimap_h
#define scsimap_h

/*
    Parameters  : drive     - The SCSIFS drive number (0-7).
                  partition - Variable to receive the partition number.
    Returns     : int       - The SCSI device ID, or negative if unknown.
    Description : Attempt to convert an SCSIFS drive number to the equivalent
                  SCSI device ID and partition number.
*/
int scsimap_device(int drive, int *partition);

/*
    Parameters  : device    - The SCSI device ID.
                  partition - The partition number on the device.
    Returns     : int       - The SCSIFS drive number, or negative if unknown.
    Description : Attempt to convert a SCSI device ID and partition number to
                  the equivalent SCSIFS drive number.
*/
int scsimap_drive(int device, int partition);

/*
    Parameters  : device    - The SCSI device ID.
                  partition - The partition number on the device.
    Returns     : char *    - The RISC OS path for the root directory of the
                              device.
    Description : Attempt to convert a SCSI device ID and partition number to
                  the equivalent RISC OS path.
*/
const char *scsimap_path(int device, int partition);

#endif
