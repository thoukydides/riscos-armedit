<*
    File        : swi/tstrt.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="ARMEdit_TalkStart" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,ARMEdit_TalkStart">

<SWI NAME="ARMEdit_TalkStart" NUM="4BC41" DESC="Register a new client task">
    <SWIE REG="R0">pre-allocated ID for this task</SWIE>
    <SWIE REG="R1" MORE>flags (see below)</SWIE>
    <SWIE REG="R2" MORE>pointer to a function to be called when a message is available, or 0 for none</SWIE>
    <SWIE REG="R3" MORE>value for R12 to contain when function pointed to by R2 is called</SWIE>
    <SWIO REG="R0">a unique client handle</SWIO>
    <SWIO REG="R1" MORE>pointer to a poll word for this task</SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        An application that provides services to PC software should call this when it is starting. A message buffer is allocated and a unique handle for this task assigned. This handle should be stored and used in all other calls relating to this task.
        <P>
        The currently defined flag bits are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Bit</TH><TH>Meaning if set</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=RIGHT>0</TD><TD>Messages from the <ARMEDIT> module are required.</TD></TR>
        </TABLE>
        <P>
        All other bits should be set to 0 to allow for future expansion.
        <P>
        The poll word is initially set to zero. When there is potentially a message waiting for this task the poll word is set to a non-zero value. The poll word is cleared when either the message has been read, or no message is available for some other reason. Note that a non-zero poll word does not imply that a message will be available; another task might have read the message if it was not directed to a specific handle. The poll word must not be written to; it must only be modified by the <ARMEDIT> module.
    </SWIU>
    <SWIS><SWIL SWI="ARMEdit_TalkEnd" HREF=":swi/tend.html">, <SWIL SWI="ARMEdit_TalkTX" HREF=":swi/ttx.html">, <SWIL SWI="ARMEdit_TalkRX" HREF=":swi/trx.html">, <SWIL SWI="ARMEdit_TalkAck" HREF=":swi/tack.html">, <SWIL SWI="ARMEdit_TalkReply" HREF=":swi/trply.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
