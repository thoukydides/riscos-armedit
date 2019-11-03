<*
    File        : swi/trply.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="ARMEdit_TalkReply" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,ARMEdit_TalkReply">

<SWI NAME="ARMEdit_TalkReply" NUM="4BC48" DESC="Reply to a message from another client task">
    <SWIE REG="R0">client handle for this task</SWIE>
    <SWIE REG="R1" MORE>the client handle for the recipient</SWIE>
    <SWIE REG="R2" MORE>pointer to block containing the message to send</SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This is like <SWIL SWI="ARMEdit_TalkTX" HREF=":swi/ttx.html">, except that the message is stored in the destination task's message buffer. The destination task must be specified by it's client handle; it is not possible to send the message to an ID.
        <P>
        The main use of this call is to support the easy creation of RISC OS tasks acting as a server for multiple PC tasks.
    </SWIU>
    <SWIS><SWIL SWI="ARMEdit_TalkStart" HREF=":swi/tstrt.html">, <SWIL SWI="ARMEdit_TalkEnd" HREF=":swi/tend.html">, <SWIL SWI="ARMEdit_TalkTX" HREF=":swi/ttx.html">, <SWIL SWI="ARMEdit_TalkRX" HREF=":swi/trx.html">, <SWIL SWI="ARMEdit_TalkAck" HREF=":swi/tack.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
