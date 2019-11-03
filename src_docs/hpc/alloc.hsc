<*
    File        : hpc/alloc.hsc
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

<PAGE TITLE="HPC Services" SUBTITLE="HPC_ARMEDIT_ALLOC" PARENT=":hpc/index.html" KEYWORDS="HPC,High-level Procedure Call">

<HPC NAME="HPC_ARMEDIT_ALLOC" ID="0105" NUM="0003" DESC="Claim a block of ARM memory">
    <HPCTX OFFSET="0" SIZE="2">HPC service ID = &amp;0105</HPCTX>
    <HPCTX OFFSET="2" SIZE="2" MORE>reason code = &amp;0003</HPCTX>
    <HPCTX OFFSET="4" SIZE="4" MORE>number of bytes to claim</HPCTX>
    <HPCTX OFFSET="8" SIZE="" MORE></HPCTX>
    <HPCRX OFFSET="0" SIZE="4"><A HREF=":hpc/rc.html" TITLE="Return Code">return code</A></HPCRX>
    <HPCRX OFFSET="4" SIZE="4" MORE>address of the block of memory allocated</HPCRX>
    <HPCRX OFFSET="8" SIZE="" MORE></HPCRX>
    <HPCU>
        Claim a block of ARM memory. This is automatically released when the PC is shutdown or reset, but it is better to free it when no longer required using <HPCL HPC="HPC_ARMEDIT_FREE">.
    </HPCU>
    <HPCS><HPCL HPC="HPC_ARMEDIT_FREE" HREF=":hpc/free.html"></HPCS>
</HPC>

</PAGE>
