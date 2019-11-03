<*
    File        : command/mem.hsc
    Date        : 17-May-01
    Author      : © A.Thoukydides, 2001, 2019
    Description : Part of the ARMEdit documentation.
    
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
*>

<PAGE TITLE="*Commands" SUBTITLE="*ARMEdit_Memory" PARENT=":command/index.html" KEYWORDS="*Commands,Command Line Usage,*ARMEdit_Memory">

<CMD CMD="ARMEdit_Memory" DESC="List currently allocated memory">
<CMDS></CMDS>
<CMDP></CMDP>
<CMDU>
    This command displays details of the RISC OS memory currently being used by PC software. For each block of memory claimed the base address and size of the block is displayed.
    <P>
    The memory is currently allocated from the RMA, but a header is attached to the start of each block. Hence the address displayed is not the start of a heap block. The information in the header is used to automatically release the memory when the PC is reset or quit if the PC software fails to do so.
</CMDU>
<CMDES CMD="*ARMEdit_Memory">
<CMDR><CMDL CMD="ARMEdit_Files" HREF=":command/files.html"></CMDR>
</CMD>

</PAGE>
