<*
    File        : swi/ctrl.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="ARMEdit_ControlPC" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,ARMEdit_ControlPC">

<SWI NAME="ARMEdit_ControlPC" NUM="4BC40" DESC="Control the PC front-end">
    <SWIE REG="R0">operation to perform:
        <TABLE ALIGN=LEFT>
            <TR ALIGN=TOP VALIGN=TOP><TD NOWRAP>0</TD><TD>Suspend full screen mode</TD></TR>
            <TR ALIGN=TOP VALIGN=TOP><TD NOWRAP>1</TD><TD>Freeze running in a window</TD></TR>
            <TR ALIGN=TOP VALIGN=TOP><TD NOWRAP>2</TD><TD>Reset the PC</TD></TR>
            <TR ALIGN=TOP VALIGN=TOP><TD NOWRAP>3</TD><TD>Quit the front-end</TD></TR>
        </TABLE>
    </SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This call allows the PC front-end to be controlled. If the PC front-end is not the current application then this will only take effect the next time the front-end is paged in.
        <P>
        Note that suspending full screen mode only starts execution in a window if enabled by the current configuration.
    </SWIU>
    <SWIS><SWIL SWI="ARMEdit_Polling" HREF=":swi/poll.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
