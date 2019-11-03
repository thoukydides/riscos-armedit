/*
    File        : buffer.h
    Date        : 15-May-01
    Author      : © A.Thoukydides, 1997-2001, 2019
    Description : A simple first-in, first-out buffer.

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

#ifndef buffer_h
#define buffer_h

// A simple buffer class
class Buffer
{
    // The private part of the class

    char *data;
    int size;

public:

    /*
        Parameters  : void
        Returns     : -
        Description : Constructor function.
    */
    Buffer(void);

    /*
        Parameters  : -
        Returns     : -
        Description : Destructor function.
    */
    ~Buffer(void);

    /*
        Parameters  : bytes     - The number of bytes to add to the buffer.
                      buffer    - A buffer containing the bytes to add.
        Returns     : void
        Description : Add the specified number of bytes to the buffer.
    */
    void add(int bytes, const char *buffer);

    /*
        Parameters  : bytes     - The maximum number of bytes to extract from
                                  the buffer.
                      buffer    - A buffer to receive the extracted bytes.
        Returns     : int       - The actual number of bytes extracted.
        Description : Extract any bytes from the buffer, up to a maximum of
                      the limit specified.
    */
    int extract(int bytes, char *buffer);

    /*
        Parameters  : void
        Returns     : int   - Is the buffer full.
        Description : Check whether the buffer is full. A full buffer still
                      allows more data to be added, but extra memory is
                      required.
    */
    int full(void);
};

#endif
