<*
    File        : swi/tend.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="ARMEdit_TalkEnd" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,ARMEdit_TalkEnd">

<SWI NAME="ARMEdit_TalkEnd" NUM="4BC42" DESC="Deregister a client task">
    <SWIE REG="R0">the previously assigned handle for this client task</SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        All applications that call <SWIL SWI="ARMEdit_TalkStart"> must call this before they terminate. This releases the message buffer and allows other tasks to detect whether particular services are available.
    </SWIU>     
    <SWIS><SWIL SWI="ARMEdit_TalkStart" HREF=":swi/tstrt.html">, <SWIL SWI="ARMEdit_TalkTX" HREF=":swi/ttx.html">, <SWIL SWI="ARMEdit_TalkRX" HREF=":swi/trx.html">, <SWIL SWI="ARMEdit_TalkAck" HREF=":swi/tack.html">, <SWIL SWI="ARMEdit_TalkReply" HREF=":swi/trply.html"></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
