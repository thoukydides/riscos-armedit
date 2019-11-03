<*
    File        : hpc/fwrit.hsc
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

<PAGE TITLE="HPC Services" SUBTITLE="HPC_ARMEDIT_FWRITE" PARENT=":hpc/index.html" KEYWORDS="HPC,High-level Procedure Call">

<HPC NAME="HPC_ARMEDIT_FWRITE" ID="0105" NUM="000A" DESC="Write to a RISC OS file">
    <HPCTX OFFSET="0" SIZE="2">HPC service ID = &amp;0105</HPCTX>
    <HPCTX OFFSET="2" SIZE="2" MORE>reason code = &amp;000A</HPCTX>
    <HPCTX OFFSET="4" SIZE="" MORE></HPCTX>
    <HPCRX OFFSET="0" SIZE="4"><A HREF=":hpc/rc.html" TITLE="Return Code">return code</A></HPCRX>
    <HPCRX OFFSET="4" SIZE="" MORE></HPCRX>
    <HPCU>
        Write to a RISC OS file.
    </HPCU>
    <HPCS></HPCS>
</HPC>

    Data sent:
    
        Offset  Size    Description
        
        0       2       HPC service ID.
        2       2       Reason code = &000A.
        4       4       RISC OS file handle.
        8       4       Sequential file pointer for start of block, or -1 for
                        current position.
        12      4       Number of bytes to write.
                        (n <= 16368, n <= 4080 recommended)
        16      n       The data to write to the file.

    Data returned:
    
        Offset  Size    Description
        
        0       4       Return code.

</PAGE>
