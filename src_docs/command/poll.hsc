<*
    File        : command/poll.hsc
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

<PAGE TITLE="*Commands" SUBTITLE="*ARMEdit_Polling" PARENT=":command/index.html" KEYWORDS="*Commands,Command Line Usage,*ARMEdit_Polling">

<CMD CMD="ARMEdit_Polling" DESC="Control multitasking speed of PC card">
<CMDS>[[-fore]&nbsp;<ARGU>polls</ARGU>] [[-back]&nbsp;<ARGU>polls</ARGU>]</CMDS>
<CMDP PARAM="-fore">foreground speed</CMDP>
<CMDP PARAM="-back" MORE>background speed</CMDP>
<CMDU>
    This command allows the multitasking speed of the PC card to be controlled. Larger values increase the performance of the PC card at the expense of slowing down the desktop. A value of 0 results in the normal behaviour.
    <P>
    Use with no parameters to display the current settings. If only a single value is specified (without switches) then both settings are updated.
    <P>
    This command should not be used with versions of the PC card software that provide more direct control over the multitasking speed.
</CMDU>
<CMDES CMD="*ARMEdit_Polling -fore 10 -back 5">
<CMDR NONE></CMDR>
</CMD>

</PAGE>
