<*
    File        : swi/poll.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="ARMEdit_Polling" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,ARMEdit_Polling">

<SWI NAME="ARMEdit_Polling" NUM="4BC47" DESC="Control the multitasking speed of the PC card">
    <SWIE REG="R0">foreground speed, or -1 to read the current setting</SWIE>
    <SWIE REG="R1" MORE>background speed, or -1 to read the current setting</SWIE>
    <SWIO REG="R0">the current foreground speed</SWIO>
    <SWIO REG="R1" MORE>the current background speed</SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This call has has a similar use to <CMDL CMD="ARMEdit_Polling" HREF=":command/poll.html">.
        <P>
        Larger values increase the performance of the PC card at the expense of slowing down the desktop. A value of 0 results in the normal behaviour.
    </SWIU>
    <SWIS><SWIL SWI="ARMEdit_ControlPC" HREF=":swi/ctrl.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
