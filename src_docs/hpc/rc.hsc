<*
    File        : hpc/rc.hsc
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

<PAGE TITLE="HPC Services" SUBTITLE="Return Codes" PARENT=":hpc/index.html" KEYWORDS="HPC,High-level Procedure Call">

<HEADING>Return Codes</HEADING>
<PARA>
All of the HPC calls place a 4 byte return code at the start of the return
block. The generic values are:
<TABLE BORDER=1 ALIGN=CENTER>
    <TR><TH>Status</TH><TH>Description</TH></TR>
    <TR><TD ALIGN=CENTER>&amp;0000</TD><TD>The operation was successful.</TD></TR>
    <TR><TD ALIGN=CENTER>&amp;0001</TD><TD>The operation failed. The exact meaning of this depends upon the reason code, but in general it indicates that the rest of the return block is invalid.</TD></TR>
    <TR><TD ALIGN=CENTER>&amp;FFFF</TD><TD>Service or reason code is unknown.</TD></TR>
</TABLE>
<P>
See the individual reason codes for details of any other codes returned, and
any contents of the remainder of the return block.
</PARA>

</PAGE>
