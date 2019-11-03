<*
    File        : swi/trx.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="ARMEdit_TalkRX" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,ARMEdit_TalkRX">

<SWI NAME="ARMEdit_TalkRX" NUM="4BC44" DESC="Check for any waiting messages for this client task">
    <SWIE REG="R0">client handle for this task</SWIE>
    <SWIO REG="R0">pointer to block containing waiting message, or 0 if no messages waiting</SWIO>
    <SWIO REG="R1" MORE>source ID</SWIO>
    <SWIO REG="R2" MORE>source client handle</SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        Check for any waiting messages. If none are available R0 contains 0 on exit, otherwise it contains a pointer to the first message. The message should be checked, and if it is claimed <SWIL SWI="ARMEdit_TalkAck"> or <SWIL SWI="ARMEdit_TalkReply"> should be called to prevent it being passed to other clients. Any information required from the message must be read or copied immediately, since the message could be overwritten by a new message.
        <P>
        The specified source client handle should be used for any reply; unlike the ID it uniquely identifies a particular instantiation of a client.
        <P>
        If this call is successful then it should be called again; it is possible for multiple messages to be pending for a single client.
    </SWIU>
    <SWIS><SWIL SWI="ARMEdit_TalkStart" HREF=":swi/tstrt.html">, <SWIL SWI="ARMEdit_TalkEnd" HREF=":swi/tend.html">, <SWIL SWI="ARMEdit_TalkTX" HREF=":swi/ttx.html">, <SWIL SWI="ARMEdit_TalkAck" HREF=":swi/tack.html">, <SWIL SWI="ARMEdit_TalkReply" HREF=":swi/trply.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>