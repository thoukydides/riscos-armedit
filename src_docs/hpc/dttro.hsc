<*
    File        : hpc/dttro.hsc
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

<PAGE TITLE="HPC Services" SUBTITLE="HPC_ARMEDIT_DATE_TO_RISCOS" PARENT=":hpc/index.html" KEYWORDS="HPC,High-level Procedure Call">

<HPC NAME="HPC_ARMEDIT_DATE_TO_RISCOS" ID="0105" NUM="0015" DESC="Convert a time and date from DOS to RISC OS format">
    <HPCTX OFFSET="0" SIZE="2">HPC service ID = &amp;0105</HPCTX>
    <HPCTX OFFSET="2" SIZE="2" MORE>reason code = &amp;0015</HPCTX>
    <HPCTX OFFSET="4" SIZE="2" MORE>2 byte time (hhhhhmmmmmmsssss)</HPCTX>
    <HPCTX OFFSET="6" SIZE="2" MORE>2 byte date (yyyyyyymmmmddddd)</HPCTX>
    <HPCTX OFFSET="8" SIZE="" MORE></HPCTX>
    <HPCRX OFFSET="0" SIZE="4"><A HREF=":hpc/rc.html" TITLE="Return Code">return code</A></HPCRX>
    <HPCRX OFFSET="4" SIZE="5" MORE>centiseconds since 00:00:00 on January 1 1900</HPCRX>
    <HPCRX OFFSET="9" SIZE="" MORE></HPCRX>
    <HPCU>
        Convert a time and date from the standard DOS 4 byte format to the 5 byte RISC OS equivalent.
    </HPCU>
    <HPCS><HPCL HPC="HPC_ARMEDIT_DATE_TO_DOS" HREF=":hpc/dttdo.html"></HPCS>
</HPC>

</PAGE>
