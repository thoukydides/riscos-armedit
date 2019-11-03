<*
    File        : hpc/fopen.hsc
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

<PAGE TITLE="HPC Services" SUBTITLE="HPC_ARMEDIT_FOPEN" PARENT=":hpc/index.html" KEYWORDS="HPC,High-level Procedure Call">

<HPC NAME="HPC_ARMEDIT_FOPEN" ID="0105" NUM="0007" DESC="Open a RISC OS file">
    <HPCTX OFFSET="0" SIZE="2">HPC service ID = &amp;0105</HPCTX>
    <HPCTX OFFSET="2" SIZE="2" MORE>reason code = &amp;0007</HPCTX>
    <HPCTX OFFSET="4" SIZE="" MORE></HPCTX>
    <HPCRX OFFSET="0" SIZE="4"><A HREF=":hpc/rc.html" TITLE="Return Code">return code</A></HPCRX>
    <HPCRX OFFSET="4" SIZE="" MORE></HPCRX>
    <HPCU>
        Open a RISC OS file. This is automatically closed when the PC is shutdown or reset, but it is better to close it when no longer required using HPC_ARMEDIT_CLOSE. A file opened using this call should not be closed using RISC OS routines directly.
    </HPCU>
    <HPCS></HPCS>
</HPC>
        
    Data sent:
    
        Offset  Size    Description
        
        0       2       HPC service ID.
        2       2       Reason code = &0007.
        4       4       -1 to open existing file, or initial size of file to
                        create a new one.
        8       4       Non-zero to automatically delete file when closed.
        12      n       Name of file to open.

    Data returned:
    
        Offset  Size    Description
        
        0       4       Return code.
        4       4       RISC OS file handle.

</PAGE>
