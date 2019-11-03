<*
    File        : hpc/trx.hsc
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

<PAGE TITLE="HPC Services" SUBTITLE="HPC_ARMEDIT_TALK_RX" PARENT=":hpc/index.html" KEYWORDS="HPC,High-level Procedure Call">

<HPC NAME="HPC_ARMEDIT_TALK_RX" ID="0105" NUM="000E" DESC="Check for any waiting messages for this client task">
    <HPCTX OFFSET="0" SIZE="2">HPC service ID = &amp;0105</HPCTX>
    <HPCTX OFFSET="2" SIZE="2" MORE>reason code = &amp;000E</HPCTX>
    <HPCTX OFFSET="4" SIZE="" MORE></HPCTX>
    <HPCRX OFFSET="0" SIZE="4"><A HREF=":hpc/rc.html" TITLE="Return Code">return code</A></HPCRX>
    <HPCRX OFFSET="4" SIZE="" MORE></HPCRX>
    <HPCU>
        Check for any waiting messages for this client task. The return code is failure if there is no message waiting. There is no acknowledge call since all messages to PC software should be addressed to a particular handle.
    </HPCU>
    <HPCS></HPCS>
</HPC>
    
    Data sent:
    
        Offset  Size    Description
        
        0       2       HPC service ID.
        2       2       Reason code = &000E.
        4       4       Client handle for this task.
    
    Data returned:
    
        Offset  Size    Description
        
        0       4       Return code.
        4       4       Source ID.
        8       4       Source client handle.
        12      1024    The waiting message.


</PAGE>
