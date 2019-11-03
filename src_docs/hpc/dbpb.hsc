<*
    File        : hpc/dbpb.hsc
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

<PAGE TITLE="HPC Services" SUBTITLE="HPC_ARMEDIT_DEVICE_BPB" PARENT=":hpc/index.html" KEYWORDS="HPC,High-level Procedure Call">

<HPC NAME="HPC_ARMEDIT_DEVICE_BPB" ID="0105" NUM="0010" DESC="Construct a BIOS Parameter Block">
    <HPCTX OFFSET="0" SIZE="2">HPC service ID = &amp;0105</HPCTX>
    <HPCTX OFFSET="2" SIZE="2" MORE>reason code = &amp;0010</HPCTX>
    <HPCTX OFFSET="4" SIZE="1" MORE>unit code (drive number), or -1 for initialisation</HPCTX>
    <HPCTX OFFSET="5" SIZE="" MORE></HPCTX>
    <HPCRX OFFSET="0" SIZE="4"><A HREF=":hpc/rc.html" TITLE="Return Code">return code</A></HPCRX>
    <HPCRX OFFSET="4" SIZE="13" MORE>the BIOS parameter block</HPCRX>
    <HPCRX OFFSET="17" SIZE="" MORE></HPCRX>
    <HPCU>
        Construct a BIOS Parameter Block (BPB) for a device driver. This also informs <ARMEDIT> that the disc contents could be changed.
    </HPCU>
    <HPCS><HPCL HPC="HPC_ARMEDIT_DEVICE_INITIALISE" HREF=":hpc/dinit.html">, <HPCL HPC="HPC_ARMEDIT_DEVICE_CHANGED" HREF=":hpc/dchgd.html">, <HPCL HPC="HPC_ARMEDIT_DEVICE_READ" HREF=":hpc/dread.html">, <HPCL HPC="HPC_ARMEDIT_DEVICE_WRITE" HREF=":hpc/dwrit.html">, <HPCL HPC="HPC_ARMEDIT_DEVICE_READ_LONG" HREF=":hpc/dlrd.html">, <HPCL HPC="HPC_ARMEDIT_DEVICE_WRITE_LONG" HREF=":hpc/dlwrt.html">, <HPCL HPC="HPC_ARMEDIT_DEVICE_OPEN" HREF=":hpc/dopen.html">, <HPCL HPC="HPC_ARMEDIT_DEVICE_CLOSE" HREF=":hpc/dclse.html">, <HPCL HPC="HPC_ARMEDIT_DEVICE_REMOVABLE" HREF=":hpc/drmvb.html"></HPCS>
</HPC>

</PAGE>
