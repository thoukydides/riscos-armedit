<*
    File        : hpc/read.hsc
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

<PAGE TITLE="HPC Services" SUBTITLE="HPC_ARMEDIT_READ" PARENT=":hpc/index.html" KEYWORDS="HPC,High-level Procedure Call">

<HPC NAME="HPC_ARMEDIT_READ" ID="0105" NUM="0001" DESC="Read ARM memory">
    <HPCTX OFFSET="0" SIZE="2">HPC service ID = &amp;0105</HPCTX>
    <HPCTX OFFSET="2" SIZE="2" MORE>reason code = &amp;0001</HPCTX>
    <HPCTX OFFSET="4" SIZE="4" MORE>start address of memory to read</HPCTX>
    <HPCTX OFFSET="8" SIZE="4" MORE>number of bytes to read (n &lt;= 16380)</HPCTX>
    <HPCTX OFFSET="12" SIZE="" MORE></HPCTX>
    <HPCRX OFFSET="0" SIZE="4"><A HREF=":hpc/rc.html" TITLE="Return Code">return code</A></HPCRX>
    <HPCRX OFFSET="4" SIZE="n" MORE>the contents of the specified memory</HPCRX>
    <HPCRX OFFSET="4+n" SIZE="" MORE></HPCRX>
    <HPCU>
        Read a block of ARM memory.
        <P>
        There is a risk of data corruption if 4092 &lt; n.
    </HPCU>
    <HPCS><HPCL HPC="HPC_ARMEDIT_WRITE" HREF=":hpc/write.html"></HPCS>
</HPC>

</PAGE>
