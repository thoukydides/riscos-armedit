/*
    File        : buffer.c++
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

// Include header file for this module
#include "buffer.h"

// Include clib header files
#include <stdlib.h>
#include <string.h>

// The size at which to return buffer full
#define BUFFER_FULL_SIZE (2048)

/*
    Parameters  : void
    Returns     : -
    Description : Constructor function.
*/
Buffer::Buffer(void)
{
    // No data stored initially
    data = NULL;
    size = 0;
}

/*
    Parameters  : -
    Returns     : -
    Description : Destructor function.
*/
Buffer::~Buffer(void)
{
    // Release any claimed memory
    if (data) free(data);
}

/*
    Parameters  : bytes     - The number of bytes to add to the buffer.
                  buffer    - A buffer containing the bytes to add.
    Returns     : void
    Description : Add the specified number of bytes to the buffer.
*/
void Buffer::add(int bytes, const char *buffer)
{
    char *ptr;

    // Resize the buffer to include the extra data
    ptr = (char *) realloc(data, size + bytes);

    // Include the extra data if resize successful
    if (ptr)
    {
        memcpy(ptr + size, buffer, bytes);
        data = ptr;
        size += bytes;
    }
}


/*
    Parameters  : bytes     - The maximum number of bytes to extract from
                              the buffer.
                  buffer    - A buffer to receive the extracted bytes.
    Returns     : size_t    - The actual number of bytes extracted.
    Description : Extract any bytes from the buffer, up to a maximum of
                  the limit specified.
*/
int Buffer::extract(int bytes, char *buffer)
{
    int n;

    // Choose how many characters to extract
    n = size < bytes ? size : bytes;

    // Copy the required number of characters
    memcpy(buffer, data, n);

    // Remove the characters from the buffer
    memmove(data, data + n, size - n);
    size -= n;

    // Return the number of bytes extracted
    return n;
}

/*
    Parameters  : void
    Returns     : int   - Is the buffer full.
    Description : Check whether the buffer is full. A full buffer still
                  allows more data to be added, but extra memory is
                  required.
*/
int Buffer::full(void)
{
    return BUFFER_FULL_SIZE <= size;
}
