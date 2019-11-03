<*
    File        : hpc/opoll.hsc
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

<PAGE TITLE="HPC Services" SUBTITLE="HPC_ARMEDIT_OSCLI_POLL" PARENT=":hpc/index.html" KEYWORDS="HPC,High-level Procedure Call">

<HPC NAME="HPC_ARMEDIT_OSCLI_POLL" ID="0105" NUM="0017" DESC="Continue execution of a *command">
    <HPCTX OFFSET="0" SIZE="2">HPC service ID = &amp;0105</HPCTX>
    <HPCTX OFFSET="2" SIZE="2" MORE>reason code = &amp;0017</HPCTX>
    <HPCTX OFFSET="4" SIZE="4" MORE>command handle</HPCTX>
    <HPCTX OFFSET="8" SIZE="4" MORE>number of bytes to input</HPCTX>
    <HPCTX OFFSET="12" SIZE="256" MORE>up to 256 bytes of input</HPCTX>
    <HPCTX OFFSET="268" SIZE="" MORE></HPCTX>
    <HPCRX OFFSET="0" SIZE="4"><A HREF=":hpc/rc.html" TITLE="Return Code">return code</A></HPCRX>
    <HPCRX OFFSET="4" SIZE="4" MORE>status</HPCRX>
    <HPCRX OFFSET="8" SIZE="4" MORE>number of bytes to output</HPCRX>
    <HPCRX OFFSET="12" SIZE="256" MORE>up to 256 bytes of output</HPCRX>
    <HPCRX OFFSET="268" SIZE="" MORE></HPCRX>
    <HPCU>
        Continue execution of a *command. This should only include input data if requested by the previous <HPCL HPC="HPC_ARMEDIT_OSCLI_POLL"> reply, otherwise characters may get lost.
        <P>
        The possible values for the status code are:
        <TABLE BORDER=1 ALIGN=CENTER>
            <TR><TH>Status</TH><TH>Description</TH></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>0</TD><TD>Active</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>1</TD><TD>Command has finished</TD></TR>
            <TR VALIGN=TOP><TD ALIGN=CENTER>2</TD><TD>Waiting for input</TD></TR>
        </TABLE>
    </HPCU>
    <HPCS>
    <HPCL HPC="HPC_ARMEDIT_OSCLI_START" HREF=":hpc/ostrt.html">, <HPCL HPC="HPC_ARMEDIT_OSCLI_END" HREF=":hpc/oend.html"></HPCS>
</HPC>

</PAGE>
