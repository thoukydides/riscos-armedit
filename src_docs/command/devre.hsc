<*
    File        : command/devre.hsc
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

<PAGE TITLE="*Commands" SUBTITLE="*ARMEdit_DevicesRelog" PARENT=":command/index.html" KEYWORDS="*Commands,Command Line Usage,*ARMEdit_DevicesRelog">

<CMD CMD="ARMEdit_DevicesRelog" DESC="Discard the device driver directory cache">
<CMDS>[-now]</CMDS>
<CMDP PARAM="-now">force immediate reset</CMDP>
<CMDU>
    Force a relog of the RISC OS directory structure. This releases most of the memory claimed to support device driver operation and closes open files.
    <P>
    Note that this may not take immediate effect - DOS must acknowledge a change of disc before any relog occurs. This normally occurs if no files are open on the device, and a simple operation (such as typing <ARG>DIR</ARG>) is performed on the drive. Note that each emulated device is treated seperately; some devices may be reset before others.
    <P>
    The <ARG>-now</ARG> switch forces an immediate reset. This should only be used if essential, and must never be used in the middle of a disc access, otherwise DOS could be confused by an inconsistent disc image.
    <P>
    It can be useful to call this from the DOS command line using the OSCLI command (supplied as part of the <ARMEDIT> suite) - perhaps it should be placed in a DOS batch file for easier use.
</CMDU>
<CMDES CMD="*ARMEdit_DevicesRelog">
<CMDR NONE></CMDR>
</CMD>

</PAGE>
