<*
    File        : command/files.hsc
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

<PAGE TITLE="*Commands" SUBTITLE="*ARMEdit_Files" PARENT=":command/index.html" KEYWORDS="*Commands,Command Line Usage,*ARMEdit_Files">

<CMD CMD="ARMEdit_Files" DESC="List currently open files">
<CMDS></CMDS>
<CMDP></CMDP>
<CMDU>
    This command displays details of the RISC OS files currently open for PC software. For each open file the file handle and filename is displayed. There is also a field that indicates whether the file will automatically be deleted when it is closed (either explicitly by the PC software, or when the PC is reset or quit).
    <P>
    Any files opened for the device driver are also listed.
</CMDU>
<CMDES CMD="*ARMEdit_Files">
<CMDR><CMDL CMD="ARMEdit_Memory" HREF=":command/mem.html"></CMDR>
</CMD>

</PAGE>
