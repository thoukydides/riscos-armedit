REM File        : DeviceTest
REM Date        : 09-Dec-00
REM Author      : © A.Thoukydides, 1996-2000, 2019
REM Description : Test the APIs used by the ARNEdit device driver.
REM
REM License     : ARMEdit is free software: you can redistribute it and/or
REM               modify it under the terms of the GNU General Public License
REM               as published by the Free Software Foundation, either
REM               version 3 of the License, or (at your option) any later
REM               version.
REM
REM               ARMEdit is distributed in the hope that it will be useful,
REM               but WITHOUT ANY WARRANTY; without even the implied warranty
REM               of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
REM               the GNU General Public License for more details.
REM
REM               You should have received a copy of the GNU General Public
REM               License along with ARMEdit. If not, see
REM               <http://www.gnu.org/licenses/>.

REM HPC service identifier
hpc_armedit_service% = &105

REM Result codes
hpc_result_success% = &0000
hpc_result_failed% = &0001
hpc_result_unknown% = &FFFF

REM Reason codes
hpc_armedit_device_initialise% = &000F
hpc_armedit_device_bpb% = &0010
hpc_armedit_device_changed% = &0011
hpc_armedit_device_read% = &0012
hpc_armedit_device_write% = &0013
hpc_armedit_device_read_long% = &001C
hpc_armedit_device_write_long% = &001D
hpc_armedit_device_open% = &001E
hpc_armedit_device_close% = &001F
hpc_armedit_device_removable% = &0020

REM Sector size
sector_size% = 512

REM General purpose buffers
in_header_size% = 256
DIM in_header% in_header_size%
in_buffer_size% = 4096
DIM in_buffer% in_buffer_size%
out_header_size% = 256
DIM out_header% out_header_size%
out_buffer_size% = 4096
DIM out_buffer% out_buffer_size%

REM Initialise the device driver
REM PROCDeviceInitialise(4, "-manual -long -write -limit 0 -size 1000 \adfs::4.$ adfs::4.$", devices%, message$)
REM PRINT message$; devices%; " device(s) supported"

REM Initialise the BPB
PROCDeviceBPB(-1, bpb%)
PRINT "Initialisation BPB:"
PROCShowBPB(bpb%)

REM Read the boot sector
PROCDeviceReadLong(0, 0, sector%)
PRINT "First sector - boot block:"
PROCShowBootSector(sector%)
END

REM Check if device is removable
PROCDeviceRemovable(0, removable%)
PRINT "Device 0 removable = "; removable%; " (&0000 = removable, &0200 = nonremovable)"

REM Read the first few sectors
PROCDeviceReadLong(0, 0, sector%)
PRINT "First sector - boot block:"
PROCShowSector(sector%)
FOR index% = 1 TO sectors_per_fat%
    PROCDeviceReadLong(0, index%, sector%)
    PRINT "FAT sector "; index%; ":"
    PROCShowSector(sector%)
NEXT

REM Read the first couple of sectors
END

REM Build standard header
DEFPROCHPCStart(reason%)
    !in_header% = (reason% << 16) + hpc_armedit_service%
ENDPROC

REM Check result code
DEFPROCHPCEnd
    LOCAL result$
    CASE !out_header% OF
    WHEN hpc_result_success%:
    WHEN hpc_result_failed%:
        result$ = "Operation failed"
    WHEN hpc_result_unknown%:
        result$ = "Unknown service or reason code"
    OTHERWISE
        result$ = "Unexpected result code = &" + STR$~!header%
    ENDCASE
    IF result$ <> "" THEN
        PRINT result$
        END
    ENDIF
ENDPROC

REM Display contents of BIOS Parameter Block
DEFPROCShowBPB(bpb%)
    sector_size% = (bpb%?1 << 8) + ?bpb%
    PRINT sector_size%; " bytes per sector"
    PRINT bpb%?2; " sectors per allocation unit"
    PRINT (bpb%?4 << 8) + bpb%?3; " reserved sector(s)"
    PRINT bpb%?5; " file allocation table(s)"
    PRINT (bpb%?7 << 8) + bpb%?6; " root directory entries"
    PRINT (bpb%?9 << 8) + bpb%?8; " sectors (see large sectors if 0)"
    PRINT bpb%?10; " media byte"
    sectors_per_fat% = (bpb%?12 << 8) + bpb%?11
    PRINT sectors_per_fat%; " sectors per FAT"
ENDPROC

REM Show the contents of a sector
DEFPROCShowSector(data%)
    LOCAL index%, hex$, ascii$, value%
    FOR index% = 0 TO sector_size% - 1
        IF (index% MOD 16) = 0 THEN
            PRINT ~index%;
            hex$ = ""
            ascii$ = ""
        ENDIF
        value% = data%?index%
        hex$ += " " + RIGHT$("0" + STR$~value%, 2)
        IF (32 <= value%) AND (value% < 127) THEN
            ascii$ += CHR$value%
        ELSE
            ascii$ += "."
        ENDIF
        IF (index% MOD 16) = 15 THEN
            PRINT " :"; hex$; " : "; ascii$
        ENDIF
    NEXT
ENDPROC

REM Show the contents of the boot sector
DEFPROCShowBootSector(data%)
    PRINT "Jump instruction: "; ~?data%;" "; ~data%?1; " "; ~data%?2
    PRINT "OEM name: "; CHR$data%?3; CHR$data%?4; CHR$data%?5; CHR$data%?6; CHR$data%?7; CHR$data%?8; CHR$data%?9; CHR$data%?10
    PROCShowBPB(data% + 11)
    PRINT (data%?25 << 8) + data%?24; " sectors per track"
    PRINT (data%?27 << 8) + data%?26; " heads"
    PRINT (data%?31 << 24) + (data%?30 << 16) + (data%?29 << 8) + data%?28; " hidden sectors"
    PRINT (data%?35 << 24) + (data%?34 << 16) + (data%?33 << 8) + data%?32; " large sectors"
    PRINT (data%?37 << 24) + data%?36; " physical device number"
    PRINT ~data%?38; " extended boot record signature"
    PRINT ~(data%?42 << 24) + (data%?41 << 16) + (data%?40 << 8) + data%?39; " volume serial number"
    PRINT "Volume label: "; FNReadString(data% + 43, 11)
    PRINT "File system ID: "; FNReadString(data% + 54, 7)
    PRINT "Boot sector signature: "; ~(data%?511 << 8) + data%?510
ENDPROC

REM Read a fixed length string
DEFFNReadString(ptr%, len%)
    LOCAL string$, index%
    FOR index% = 0 TO len% - 1
        string$ += CHR$(ptr%?index%)
    NEXT
= string$

DEFPROCDeviceInitialise(first%, command$, RETURN devices%, RETURN message$)
    LOCAL ptr%
    PROCHPCStart(hpc_armedit_device_initialise%)
    in_header%?4 = first%
    $in_buffer% = command$ + CHR$0
    SYS "ARMEdit_HPC", 5, in_header%, LEN(command$) + 1, in_buffer%, 8, out_header%, out_buffer_size%, out_buffer%
    PROCHPCEnd
    devices% = out_header%!4
    message$ = ""
    ptr% = out_buffer%
    WHILE CHR$?ptr% <> "$"
        message$ += CHR$?ptr%
        ptr% += 1
    ENDWHILE
ENDPROC

DEFPROCDeviceBPB(device%, RETURN bpb%)
    PROCHPCStart(hpc_armedit_device_bpb%)
    in_header%?4 = device%
    SYS "ARMEdit_HPC", 5, in_header%, 0, 0, 4, out_header%, 13, out_buffer%
    PROCHPCEnd
    bpb% = out_buffer%
ENDPROC

DEFPROCDeviceChanged
    PROCHPCStart(hpc_armedit_device_changed%)
    SYS "ARMEdit_HPC", , in_header%
    PROCHPCEnd
ENDPROC

DEFPROCDeviceRead
    PROCHPCStart(hpc_armedit_device_read%)
    SYS "ARMEdit_HPC", , in_header%
    PROCHPCEnd
ENDPROC

DEFPROCDeviceWrite
    PROCHPCStart(hpc_armedit_device_write%)
    SYS "ARMEdit_HPC", , in_header%
    PROCHPCEnd
ENDPROC

DEFPROCDeviceReadLong(device%, sector%, RETURN data%)
    PROCHPCStart(hpc_armedit_device_read_long%)
    in_header%?4 = device%
    in_header%!8 = sector%
    SYS "ARMEdit_HPC", 12, in_header%, 0, 0, 4, out_header%, sector_size%, out_buffer%
    PROCHPCEnd
    data% = out_buffer%
ENDPROC

DEFPROCDeviceWriteLong
    PROCHPCStart(hpc_armedit_device_write_long%)
    SYS "ARMEdit_HPC", , in_header%
    PROCHPCEnd
ENDPROC

DEFPROCDeviceOpen
    PROCHPCStart(hpc_armedit_device_open%)
    SYS "ARMEdit_HPC", , in_header%
    PROCHPCEnd
ENDPROC

DEFPROCDeviceClose
    PROCHPCStart(hpc_armedit_device_close%)
    SYS "ARMEdit_HPC", , in_header%
    PROCHPCEnd
ENDPROC

DEFPROCDeviceRemovable(device%, RETURN removable%)
    PROCHPCStart(hpc_armedit_device_removable%)
    in_header%?4 = device%
    SYS "ARMEdit_HPC", 5, in_header%, 0, 0, 8, out_header%, 0, 0
    PROCHPCEnd
    removable% = out_header%!4
ENDPROC

