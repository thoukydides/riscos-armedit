<*
    File        : swi/ttx.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="ARMEdit_TalkTX" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,ARMEdit_TalkTX">

<SWI NAME="ARMEdit_TalkTX" NUM="4BC43" DESC="Send a message to another client task">
    <SWIE REG="R0">client handle for this task</SWIE>
    <SWIE REG="R1" MORE>either the ID or client handle for the recipient (if R2 not 0)</SWIE>
    <SWIE REG="R2" MORE>pointer to block containing the message to send, or 0 to check if the buffer already contains a message</SWIE>
    <SWIO REG="R2">pointer to message buffer, or 0 if no message is waiting to be delivered.</SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        Send a message to another task. The destination task can be specified using either it's ID or client handle. If the ID is specified then it will be offered to each matching task that polls it until it is acknowledged. A particular message is only offered to each task once. When sending messages to PC software (ID = 0) the handle should always be specified.
        <P>
        Note that the message buffer for each task can only contain a single message to send; if there is already a message waiting to be delivered then it is overwritten. When multiple clients are used, this SWI should first be called with R2 = 0 to check if a message is still waiting. Broadcast messages are never cleared automatically.
    </SWIU>
    <SWIS><SWIL SWI="ARMEdit_TalkStart" HREF=":swi/tstrt.html">, <SWIL SWI="ARMEdit_TalkEnd" HREF=":swi/tend.html">, <SWIL SWI="ARMEdit_TalkRX" HREF=":swi/trx.html">, <SWIL SWI="ARMEdit_TalkAck" HREF=":swi/tack.html">, <SWIL SWI="ARMEdit_TalkReply" HREF=":swi/trply.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>