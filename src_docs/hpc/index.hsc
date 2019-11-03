<*
    File        : hpc/index.hsc
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

<PAGE TITLE="HPC Services" PARENT=":index.html" KEYWORDS="HPC,High-level Procedure Call">

<HEADING>HPC</HEADING>
<PARA>
The <ARMEDIT> module provides a number of High-level Procedure Call (HPC) services. To allow development to start before HPC is included in the PC front-end software, and to support earlier releases of the front-end software, a simplified interface via <A HREF=":hpc/io.html" TITLE="I/O Port Access">I/O ports</A> is also supported.
<P>
All of these services use the HPC service identifier &amp;105 which has been allocated by Aleph One.
<P>
For details of making HPC calls directly (which is potentially much more
efficient) contact Aleph One, as the information is covered by a Non
Disclosure Agreement.
<P>
Several users have found that using the complete 16384 byte buffer size can
lead to message corruption on some installations. It is strongly recommended
that where variable messages sizes can be used that these are restricted to
less than 4096 bytes. All of the code supplied with this release uses the
lower limit but support is retained for larger messages.
<P>

The following SWIs are implemented by the <ARMEDIT> module. For more details
regarding the use of the communciations SWIs see the Code documentation.
<UL>
    <LI><HPCL HPC="HPC_ARMEDIT_SWI" HREF=":hpc/swi.html">
    <LI><HPCL HPC="HPC_ARMEDIT_READ" HREF=":hpc/read.html">
    <LI><HPCL HPC="HPC_ARMEDIT_WRITE" HREF=":hpc/write.html">
    <LI><HPCL HPC="HPC_ARMEDIT_ALLOC" HREF=":hpc/alloc.html">
    <LI><HPCL HPC="HPC_ARMEDIT_FREE" HREF=":hpc/free.html">
    <LI><HPCL HPC="HPC_ARMEDIT_EXTTYPE" HREF=":hpc/extyp.html">
    <LI><HPCL HPC="HPC_ARMEDIT_TYPEEXT" HREF=":hpc/typex.html">
    <LI><HPCL HPC="HPC_ARMEDIT_FOPEN" HREF=":hpc/fopen.html">
    <LI><HPCL HPC="HPC_ARMEDIT_FCLOSE" HREF=":hpc/fclse.html">
    <LI><HPCL HPC="HPC_ARMEDIT_FREAD" HREF=":hpc/fread.html">
    <LI><HPCL HPC="HPC_ARMEDIT_FWRITE" HREF=":hpc/fwrit.html">
    <LI><HPCL HPC="HPC_ARMEDIT_TALK_START" HREF=":hpc/tstrt.html">
    <LI><HPCL HPC="HPC_ARMEDIT_TALK_END" HREF=":hpc/tend.html">
    <LI><HPCL HPC="HPC_ARMEDIT_TALK_TX" HREF=":hpc/ttx.html">
    <LI><HPCL HPC="HPC_ARMEDIT_TALK_RX" HREF=":hpc/trx.html">
    <LI><HPCL HPC="HPC_ARMEDIT_DEVICE_INITIALISE" HREF=":hpc/dinit.html">
    <LI><HPCL HPC="HPC_ARMEDIT_DEVICE_BPB" HREF=":hpc/dbpb.html">
    <LI><HPCL HPC="HPC_ARMEDIT_DEVICE_CHANGED" HREF=":hpc/dchgd.html">
    <LI><HPCL HPC="HPC_ARMEDIT_DEVICE_READ" HREF=":hpc/dread.html">
    <LI><HPCL HPC="HPC_ARMEDIT_DEVICE_WRITE" HREF=":hpc/dwrit.html">
    <LI><HPCL HPC="HPC_ARMEDIT_DATE_TO_DOS" HREF=":hpc/dttdo.html">
    <LI><HPCL HPC="HPC_ARMEDIT_DATE_TO_RISCOS" HREF=":hpc/dttro.html">
    <LI><HPCL HPC="HPC_ARMEDIT_OSCLI_START" HREF=":hpc/ostrt.html">
    <LI><HPCL HPC="HPC_ARMEDIT_OSCLI_POLL" HREF=":hpc/opoll.html">
    <LI><HPCL HPC="HPC_ARMEDIT_OSCLI_END" HREF=":hpc/oend.html">
    <LI><HPCL HPC="HPC_ARMEDIT_TALK_REPLY" HREF=":hpc/trply.html">
    <LI><HPCL HPC="HPC_ARMEDIT_FASTER" HREF=":hpc/fastr.html">
    <LI><HPCL HPC="HPC_ARMEDIT_TEMPORARY" HREF=":hpc/temp.html">
    <LI><HPCL HPC="HPC_ARMEDIT_DEVICE_READ_LONG" HREF=":hpc/dlrd.html">
    <LI><HPCL HPC="HPC_ARMEDIT_DEVICE_WRITE_LONG" HREF=":hpc/dlwrt.html">
    <LI><HPCL HPC="HPC_ARMEDIT_DEVICE_OPEN" HREF=":hpc/dopen.html">
    <LI><HPCL HPC="HPC_ARMEDIT_DEVICE_CLOSE" HREF=":hpc/dclse.html">
    <LI><HPCL HPC="HPC_ARMEDIT_DEVICE_REMOVABLE" HREF=":hpc/drmvb.html">
</UL>
<P>
<OSLIBMENTION>
</PARA>

</PAGE>
