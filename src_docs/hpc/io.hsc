<*
    File        : hpc/io.hsc
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

<PAGE TITLE="HPC Services" SUBTITLE="I/O Port Access" PARENT=":hpc/index.html" KEYWORDS="HPC,High-level Procedure Call">

<HEADING>I/O Port Access</HEADING>
<PARA>
The following I/O port assignments are currently used:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Port</TH><TH>Name</TH><TH>Access type</TH></TR>
    <TR VALIGN=TOP><TD ALIGN=CENTER>&amp;2E0</TD><TD><ARG>PORT_STATUS</ARG></TD><TD>Word read</TD></TR>
    <TR VALIGN=TOP><TD ALIGN=CENTER>&amp;2E1</TD><TD><ARG>PORT_CMD</ARG></TD><TD>Word write</TD></TR>
    <TR VALIGN=TOP><TD ALIGN=CENTER>&amp;2E2</TD><TD><ARG>PORT_DATA</ARG></TD><TD>Byte or word read/write</TD></TR>
</TABLE>
<P>
The following sequence of operations should occur to perform an HPC call:
<UL>
    <LI>Read <ARG>PORT_STATUS</ARG>. The possible return codes are:
        <TABLE BORDER=1 ALIGN=CENTER>
        <TR><TH>Code</TH><TH>Description</TH></TR>
        <TR VALIGN=TOP><TD ALIGN=CENTER>&amp;454D</TD><TD>System is available and ready.</TD></TR>
        <TR VALIGN=TOP><TD ALIGN=CENTER>&amp;4D45</TD><TD>System is currently busy processing another HPC call, and cannot currently be used for another call.</TD></TR>
        </TABLE>
        <P>
        Any other code indicates that the service is not available.
    <LI>Write &amp;0000 to <ARG>PORT_CMD</ARG> to start transfer of data.
    <LI>Write the HPC packet data to <ARG>PORT_DATA</ARG> as either bytes or words.
    <LI>Write &amp;0001 to <ARG>PORT_CMD</ARG> to perform the HPC call.
    <LI>Write &amp;0002 to <ARG>PORT_CMD</ARG> to start reading the reply.
    <LI>Read the HPC reply packet from <ARG>PORT_DATA</ARG> as either bytes or words.
    <LI>Write &amp;0003 to <ARG>PORT_CMD</ARG> to reset the HPC system.
</Ul>
The port based HPC packets have the same 16384 byte limit that proper HPC
packets have.
<P>
Note that only HPC services provided by the <ARMEDIT> module can be called
using this I/O port system; other services can only be called via the normal
HPC system. However, the HPC identifier is still checked.
</PARA>

</PAGE>
