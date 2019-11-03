<*
    File        : hpc/swi.hsc
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

<PAGE TITLE="HPC Services" SUBTITLE="HPC_ARMEDIT_SWI" PARENT=":hpc/index.html" KEYWORDS="HPC,High-level Procedure Call">

<HPC NAME="HPC_ARMEDIT_SWI" ID="0105" NUM="0000" DESC="Call a RISC OS SWI by number">
    <HPCTX OFFSET="0" SIZE="2">HPC service ID = &amp;0105</HPCTX>
    <HPCTX OFFSET="2" SIZE="2" MORE>reason code = &amp;0000</HPCTX>
    <HPCTX OFFSET="4" SIZE="4" MORE>SWI number to call</HPCTX>
    <HPCTX OFFSET="8" SIZE="4" MORE>value for R0 register on entry to the SWI</HPCTX>
    <HPCTX OFFSET="12" SIZE="4" MORE>value for R1 register on entry to the SWI</HPCTX>
    <HPCTX OFFSET="16" SIZE="4" MORE>value for R2 register on entry to the SWI</HPCTX>
    <HPCTX OFFSET="20" SIZE="4" MORE>value for R3 register on entry to the SWI</HPCTX>
    <HPCTX OFFSET="24" SIZE="4" MORE>value for R4 register on entry to the SWI</HPCTX>
    <HPCTX OFFSET="28" SIZE="4" MORE>value for R5 register on entry to the SWI</HPCTX>
    <HPCTX OFFSET="32" SIZE="4" MORE>value for R6 register on entry to the SWI</HPCTX>
    <HPCTX OFFSET="36" SIZE="4" MORE>value for R7 register on entry to the SWI</HPCTX>
    <HPCTX OFFSET="40" SIZE="4" MORE>value for R8 register on entry to the SWI</HPCTX>
    <HPCTX OFFSET="44" SIZE="4" MORE>value for R9 register on entry to the SWI</HPCTX>
    <HPCTX OFFSET="48" MORE></HPCTX>
    <HPCRX OFFSET="0" SIZE="4"><A HREF=":hpc/rc.html" TITLE="Return Code">return code</A></HPCRX>                                
    <HPCRX MORE>If the return code is &amp;0001</HPCRX>
    <HPCRX OFFSET="4" SIZE="256" INDENT MORE>RISC OS style error block returned by SWI</HPCRX>
    <HPCRX MORE>If the return code is &amp;0000</HPCRX>
    <HPCRX OFFSET="260" SIZE="4" INDENT MORE>value of R0 register on exit from the SWI</HPCRX>
    <HPCRX OFFSET="264" SIZE="4" INDENT MORE>value of R1 register on exit from the SWI</HPCRX>
    <HPCRX OFFSET="268" SIZE="4" INDENT MORE>value of R2 register on exit from the SWI</HPCRX>
    <HPCRX OFFSET="272" SIZE="4" INDENT MORE>value of R3 register on exit from the SWI</HPCRX>
    <HPCRX OFFSET="276" SIZE="4" INDENT MORE>value of R4 register on exit from the SWI</HPCRX>
    <HPCRX OFFSET="280" SIZE="4" INDENT MORE>value of R5 register on exit from the SWI</HPCRX>
    <HPCRX OFFSET="284" SIZE="4" INDENT MORE>value of R6 register on exit from the SWI</HPCRX>
    <HPCRX OFFSET="288" SIZE="4" INDENT MORE>value of R7 register on exit from the SWI</HPCRX>
    <HPCRX OFFSET="292" SIZE="4" INDENT MORE>value of R8 register on exit from the SWI</HPCRX>
    <HPCRX OFFSET="296" SIZE="4" INDENT MORE>value of R9 register on exit from the SWI</HPCRX>
    <HPCRX OFFSET="300" MORE></HPCRX>
    <HPCU>
        Calls a specified RISC OS SWI by number. The SWI is always called with the X (error returning) bit set.
        <P>
        Note that the reply packet is always the same length, regardless of whether the SWI returned an error. The error block and register values on exit do not overlap.
    </HPCU>
    <HPCS><HPCL HPC="HPC_ARMEDIT_READ" HREF=":hpc/read.html">, <HPCL HPC="HPC_ARMEDIT_WRITE" HREF=":hpc/write.html">, <HPCL HPC="HPC_ARMEDIT_ALLOC" HREF=":hpc/alloc.html">, <HPCL HPC="HPC_ARMEDIT_FREE" HREF=":hpc/free.html"></HPCS>
</HPC>

</PAGE>
