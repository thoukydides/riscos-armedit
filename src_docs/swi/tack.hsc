<*
    File        : swi/tack.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="ARMEdit_TalkAck" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,ARMEdit_TalkAck">

<SWI NAME="ARMEdit_TalkAck" NUM="4BC45" DESC="Claim the most recently read message">
    <SWIE REG="R0">client handle for this task</SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        After reading a message with <SWIL SWI="ARMEdit_TalkRX">, this call should be used to claim the message, and to prevent it being offered to other clients. This should be used before calling any other SWIs from this module.
        <P>
        If the message was sent by the <ARMEDIT> module then calling this SWI has no effect; other clients still receive the message.
    </SWIU>
    <SWIS><SWIL SWI="ARMEdit_TalkStart" HREF=":swi/tstrt.html">, <SWIL SWI="ARMEdit_TalkEnd" HREF=":swi/tend.html">, <SWIL SWI="ARMEdit_TalkTX" HREF=":swi/ttx.html">, <SWIL SWI="ARMEdit_TalkRX" HREF=":swi/trx.html">, <SWIL SWI="ARMEdit_TalkReply" HREF=":swi/trply.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
