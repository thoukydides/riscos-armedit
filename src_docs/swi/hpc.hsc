<*
    File        : swi/hpc.hsc
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

<PAGE TITLE="SWI Calls" SUBTITLE="ARMEdit_HPC" PARENT=":swi/index.html" KEYWORDS="SWI,Software Interrupt,ARMEdit_HPC">

<SWI NAME="ARMEdit_HPC" NUM="4BC46" DESC="Call an ARMEdit HPC service">
    <SWIE REG="R0">length of first input block</SWIE>
    <SWIE REG="R1" MORE>pointer to first input block</SWIE>
    <SWIE REG="R2" MORE>length of second input block</SWIE>
    <SWIE REG="R3" MORE>pointer to second input block</SWIE>
    <SWIE REG="R4" MORE>length of first output block</SWIE>
    <SWIE REG="R5" MORE>pointer to first output block</SWIE>
    <SWIE REG="R6" MORE>length of second output block</SWIE>
    <SWIE REG="R7" MORE>pointer to second output block</SWIE>
    <SWIO NONE></SWIO>
    <SWII>
    <SWIP>
    <SWIR REENTRANT="NO">
    <SWIU>
        This call may either be used to test HPC services, or to provide access to the routines from a system that does not support either of the communications systems used normally by the <ARMEDIT> system.
        <P>
        For convenience the input and output data may be split into two portions. Any length value may be zero to omit that portion. The input data must be at least two bytes long to contain a valid ID.
    </SWIU>
    <SWIS NONE></SWIS>
    <SWIV NONE></SWIV>
</SWI>

</PAGE>
