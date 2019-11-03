;   File        : armedit.s
;   Date        : 15-May-01
;   Author      : Â© A.Thoukydides, 1995-2001, 2019
;   Description : PC utilities module.
;
;   License     : ARMEdit is free software: you can redistribute it and/or
;                 modify it under the terms of the GNU General Public License
;                 as published by the Free Software Foundation, either
;                 version 3 of the License, or (at your option) any later
;                 version.
;
;                 ARMEdit is distributed in the hope that it will be useful,
;                 but WITHOUT ANY WARRANTY; without even the implied warranty
;                 of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
;                 the GNU General Public License for more details.
;
;                 You should have received a copy of the GNU General Public
;                 License along with ARMEdit. If not, see
;                 <http://www.gnu.org/licenses/>.

; Include system header files

        GET     OS:Hdr.Buffer
        GET     OS:Hdr.FileSwitch
        GET     OS:Hdr.Macros
        GET     OS:Hdr.MessageTrans
        GET     OS:Hdr.OS
        GET     OS:Hdr.OSArgs
        GET     OS:Hdr.OSByte
        GET     OS:Hdr.OSFile
        GET     OS:Hdr.OSFind
        GET     OS:Hdr.OSFSControl
        GET     OS:Hdr.OSGBPB
        GET     OS:Hdr.OSHeap
        GET     OS:Hdr.OSModule
        GET     OS:Hdr.ResourceFS
        GET     OS:Hdr.Territory
        GET     OS:Hdr.Types

; Include my header files

        GET     AT:Hdr.macros
        GET     ^.Hdr.pc
;        GET     AT:Hdr.strongarm

; Constants

resource_directory  DefS "Resources.ARMEdit"; Directory to use in ResourceFS
swi_chunk           * &4bc40            ; SWI chunk used by this module
error_base          * &0                ; The first error number to use
emulate_port        * &2e0              ; Base address for emulated HPC port
emulate_port_status * emulate_port      ; Port address to read status
emulate_port_cmd    * emulate_port      ; Port address to write command
emulate_port_data   * emulate_port + 2  ; Port address to read/write data
emulate_code_tx     * &0000             ; Command code to start transfer
emulate_code_process * &0001            ; Command code to process HPC call
emulate_code_rx     * &0002             ; Command code to start reply
emulate_code_release * &0003            ; Command code to terminate HPC call
emulate_return_emulate * &454d          ; Return code for emulation ready
emulate_return_busy * &4d45             ; Return code for emulation busy
io_port_mask        * hdrstate_io_slots * hdrstate_io_spacing - 1
emulate_status_idle * 0                 ; Emulation is in idle state
emulate_status_rx   * 1                 ; Emulation is in receiving state
emulate_status_proc * 2                 ; Emulation is in processing state
emulate_status_tx   * 3                 ; Emulation is in transmitting state
emualte_status_err  * 4                 ; Emulation is in error state
return_success      * &0000             ; Return code for complete success
return_failure      * &0001             ; Return code for general failure
return_unknown      * &ffff             ; Return code for unknown service
oscli_status_active * 0                 ; Status for active
oscli_status_done   * 1                 ; Status for finished
oscli_status_input  * 2                 ; Status for input required
oscli_buffer_size   * 256 + 1           ; Size of buffers
swi_x_bit           * 1 << 17           ; Bit to return errors from SWIs
svc_stack_size      * 8 * 1024          ; Size of supervisor stack
UpCall_FileModified * 3                 ; UpCall reason code for file modified
String              * 256               ; A generic string
min_cluster_shift   * 2                 ; Minimum shift from sectors to clusters
max_cluster_shift   * 6                 ; Maximum shift from sectors to clusters
min_fat_sectors     * 17                ; FAT sectors to force use of FAT16
max_fat_sectors     * 256               ; Maximum supported FAT sectors
max_cylinders       * 1024              ; Maximum number of cylinders
max_sectors_per_track * 64              ; Maximum sectors per track
sector_shift        * 9                 ; Shift from bytes to sectors
bytes_per_sector    * 1 << sector_shift ; Bytes per sector
directory_shift     * 5                 ; Shift from bytes to directories
reserved_sectors    * 1                 ; Number of reserved sectors
media_byte          * &f8               ; Media descriptor byte
physical_drive      * 128               ; The physical drive number
hidden_sectors      * 0                 ; Number of hidden sectors
number_fats         * 1                 ; Number of copies of FATs
extended_signature  * &29               ; The extended boot record signature
end_boot_sector1    * &55               ; First byte of boot sector end marker
end_boot_sector2    * &AA               ; Second byte of boot sector end marker
volume_serial       * &babeface         ; The volume serial number
fat_available       * &0000             ; FAT entry is unused
fat_reserved_min    * &fff0             ; FAT entry is a reserved cluster
fat_reserved_max    * &fff7             ; FAT entry is a reserved cluster
fat_bad             * &fff7             ; FAT entry is a bad cluster
fat_eof_min         * &fff8             ; FAT entry is end of file
fat_eof_max         * &ffff             ; FAT entry is end of file
directory_unused    * &00               ; Directory entry is unused
directory_e5        * &05               ; First character is actually &e5
directory_alias     * &2e               ; Entry is an alias for current/parent
directory_erased    * &e5               ; File has been erased
attribute_read_only * &01               ; File is read only
attribute_hidden    * &02               ; File is hidden
attribute_system    * &04               ; File is hidden
attribute_volume    * &08               ; Volume label
attribute_subdirectory * &10            ; Subdirectory
attribute_archive   * &20               ; File has been modified
attribute_longname  * &0f               ; Part of a long filename
name_length         * 8                 ; Characters in filename
extension_length    * 3                 ; Characters in extension
device_flag_relog_pending * 1 << 0      ; A relog should be performed
device_flag_relog_avail * 1 << 1        ; It is possible to perform a relog
device_flag_relog_forced * 1 << 2       ; Was a relog forced
device_flag_changed * 1 << 3            ; Has directory structure changed
device_flag_anon    * 1 << 26           ; Disable automatic label generation
device_flag_manual  * 1 << 27           ; Disable automatic relogging
device_flag_write   * 1 << 28           ; Support write operations
device_flag_long    * 1 << 29           ; Generate long filenames
device_flag_raw     * 1 << 30           ; Do not canonicalise the path name
device_flag_imagefs * 1 << 31           ; Treat ImageFS as directories
device_limit        * 10                ; Default size limit
device_size         * 100               ; Default device size
client_flag_armedit * 1 << 0            ; Client flag to receive broadcast
client_flag_fn      * 1 << 31           ; Has function been called
version             DefS "1.05 (15-May-01) [TEST]"; Module version number
pipe                DefL {TRUE}        ; Should information be sent to a pipe

; Macros

        ;   Syntax      : [<label>] pipe_value <r>
        ;   Parameters  : label - An optional program label.
        ;                 r     - The register whose value to process.
        ;   Description : This is used by the pipe_string macro to process a
        ;                 single register's value.
        MACRO
$label  pipe_value $r
$label
        [       pipe :LAND: "$r" /= ""
        MacroLabels
        STMFD   r13!, {r0-r2}           ; Stack registers
        MOV     r0, $r                  ; Copy the required value
        LDR     r1, ws_pipe_output      ; Get output position
        MOV     r2, #String             ; Size of output buffer
        SWI     XOS_ConvertHex8         ; Convert the number
        MOV     r0, #' '                ; Space character
        STRB    r0, [r1], #1            ; Append the space
        STR     r1, ws_pipe_output      ; Store new output position
        LDMFD   r13!, {r0-r2}           ; Restore registers
        MacroLabelsEnd
        ]
        MEND

        ;   Syntax      : [<label>] pipe_string <tmpl> [, <r0> [, <r1> ... ]]
        ;   Parameters  : label - An optional program label.
        ;                 templ - The template text.
        ;                 r0... - Up to 10 optional parameters which are
        ;                         substituted in place of %0 to %9 in the
        ;                         template.
        ;   Description : Write a template string to the pipe. This must only
        ;                 be used in a situation which will support normal
        ;                 file operations, i.e. the processor must be in
        ;                 USR26 or SVC26 mode and there must be a valid stack.
        MACRO
$label  pipe_string $tmpl, $r0, $r1, $r2, $r3, $r4, $r5, $r6, $r7, $r8, $r9
$label
        [       pipe
        MacroLabels
        B       cont$l                  ; Skip over the template text
text$l  =       "$tmpl"                 ; The template text
        ALIGN
cont$l  STMFD   r13!, {r0}              ; Stack registers
        ADRL    r0, ws_pipe_args        ; Pointer to start of argument list
        STR     r0, ws_pipe_output      ; Store argument list pointer
        LDMFD   r13!, {r0}              ; Restore registers
        pipe_value $r0                  ; Process the 1st optional value
        pipe_value $r1                  ; Process the 2nd optional value
        pipe_value $r2                  ; Process the 3rd optional value
        pipe_value $r3                  ; Process the 4th optional value
        pipe_value $r4                  ; Process the 5th optional value
        pipe_value $r5                  ; Process the 6th optional value
        pipe_value $r6                  ; Process the 7th optional value
        pipe_value $r7                  ; Process the 8th optional value
        pipe_value $r8                  ; Process the 9th optional value
        pipe_value $r9                  ; Process the 10th optional value
        STMFD   r13!, {r0, r1, r14}     ; Stack registers
        ADR     r0, text$l              ; Get pointer to template string
        MOV     r1, #?text$l            ; Length of template string
        BL      pipe_write              ; Write the string to the pipe
        LDMFD   r13!, {r0, r1, r14}     ; Restore registers
        MacroLabelsEnd
        ]
        MEND

; Data structures

        ; Registers for SWI calls
                    ^ 0
swi_regs_r0         # Int               ; Register R0
swi_regs_r1         # Int               ; Register R1
swi_regs_r2         # Int               ; Register R2
swi_regs_r3         # Int               ; Register R3
swi_regs_r4         # Int               ; Register R4
swi_regs_r5         # Int               ; Register R5
swi_regs_r6         # Int               ; Register R6
swi_regs_r7         # Int               ; Register R7
swi_regs_r8         # Int               ; Register R8
swi_regs_r9         # Int               ; Register R9
swi_regs            * @                 ; End of the structure

        ; Contents of HPC packets for SWI calls
                    ^ 0
packet_swi_in_code  # Int               ; Service ID and reason code
packet_swi_in_swi   # Int               ; SWI number to call
packet_swi_in_regs  # swi_regs          ; Registers on entry to the SWI
packet_swi_in       * @                 ; End of the structure

                    ^ 0
packet_swi_out_code # Int               ; Return code
packet_swi_out_err  # OS_Error          ; The error block returned
packet_swi_out_regs # swi_regs          ; Registers on exit from the SWI
packet_swi_out      * @                 ; End of the structure

        ; Contents of HPC packets for memory reads and writes
                    ^ 0
packet_mem_in_code  # Int               ; Service ID and reason code
packet_mem_in_addr  # Ptr               ; Start address of ARM memory
packet_mem_in_len   # Int               ; Number of bytes to copy
packet_mem_in_data  # hpc_packet - @    ; Any data
packet_mem_in       * @                 ; End of the structure

                    ^ 0
packet_mem_out_code # Int               ; Return code
packet_mem_out_data # hpc_packet - @    ; Any data
packet_mem_out      * @                 ; End of the structure

        ; Contents of HPC packets for memory allocation
                    ^ 0
packet_alloc_in_code # Int              ; Service ID and reason code
packet_alloc_in_size # Int              ; Number of bytes of memory required
packet_alloc_in     * @                 ; End of the structure

                    ^ 0
packet_alloc_out_code # Int             ; Return code
packet_alloc_out_ptr # Ptr              ; Pointer to the memory allocated
packet_alloc_out    * @                 ; End of the structure

        ; Contents of HPC packets for memory deallocation
                    ^ 0
packet_free_in_code # Int               ; Service ID and reason code
packet_free_in_ptr  # Ptr               ; Pointer to the memory to release
packet_free_in      * @                 ; End of the structure

                    ^ 0
packet_free_out_code # Int              ; Return code
packet_free_out     * @                 ; End of the structure

        ; Contents of HPC packets for DOS extension to RISC OS filetype
                    ^ 0
packet_exttype_in_code # Int            ; Service ID and reason code
packet_exttype_in_ext # 4               ; DOS file extension
packet_exttype_in   * @                 ; End of the structure

                    ^ 0
packet_exttype_out_code # Int           ; Return code
packet_exttype_out_type # Int           ; RISC OS filetype
packet_exttype_out  * @                 ; End of the structure

        ; Contents of HPC packets for RISC OS filetype to DOS extension
                    ^ 0
packet_typeext_in_code # Int            ; Service ID and reason code
packet_typeext_in_type # Int            ; RISC OS filetype
packet_typeext_in   * @                 ; End of the structure

                    ^ 0
packet_typeext_out_code # Int           ; Return code
packet_typeext_out_ext # 4              ; DOS file extension
packet_typeext_out  * @                 ; End of the structure

        ; Contents of HPC packets for opening RISC OS files
                    ^ 0
packet_fopen_in_code # Int              ; Service ID and reason code
packet_fopen_in_size # Int              ; Initial size of the file
packet_fopen_in_del # Int               ; Should the file be deleted
packet_fopen_in_name # 256              ; Filename
packet_fopen_in     * @                 ; End of the structure

                    ^ 0
packet_fopen_out_code # Int             ; Return code
packet_fopen_out_handle # Int           ; File handle
packet_fopen_out    * @                 ; End of the structure

        ; Contents of HPC packets for closing RISC OS files
                    ^ 0
packet_fclose_in_code # Int             ; Service ID and reason code
packet_fclose_in_handle # Int           ; File handle
packet_fclose_in    * @                 ; End of the structure

                    ^ 0
packet_fclose_out_code # Int            ; Return code
packet_fclose_out   * @                 ; End of the structure

        ; Contents of HPC packets for reading from RISC OS files
                    ^ 0
packet_fread_in_code # Int              ; Service ID and reason code
packet_fread_in_handle # Int            ; File handle
packet_fread_in_ptr # Int               ; Sequential file pointer
packet_fread_in_size # Int              ; Number of bytes to read
packet_fread_in     * @                 ; End of the structure

                    ^ 0
packet_fread_out_code # Int             ; Return code
packet_fread_out_size # Int             ; Number of bytes read
packet_fread_out_data # hpc_packet - @  ; The data read
packet_fread_out    * @                 ; End of the structure

        ; Contents of HPC packets for writing to RISC OS files
                    ^ 0
packet_fwrite_in_code # Int             ; Service ID and reason code
packet_fwrite_in_handle # Int           ; File handle
packet_fwrite_in_ptr # Int              ; Sequential file pointer
packet_fwrite_in_size # Int             ; Number of bytes to write
packet_fwrite_in_data # hpc_packet - @  ; The data to write
packet_fwrite_in    * @                 ; End of the structure

                    ^ 0
packet_fwrite_out_code # Int            ; Return code
packet_fwrite_out   * @                 ; End of the structure

        ; Contents of HPC packets for starting communications clients
                    ^ 0
packet_talkstart_in_code # Int          ; Service ID and reason code
packet_talkstart_in * @                 ; End of the structure

                    ^ 0
packet_talkstart_out_code # Int         ; Return code
packet_talkstart_out_handle # Int       ; Unique client handle
packet_talkstart_out * @                ; End of the structure

        ; Contents of HPC packets for ending communications clients
                    ^ 0
packet_talkend_in_code # Int            ; Service ID and reason code
packet_talkend_in_handle # Int          ; Previously assigned handle
packet_talkend_in   * @                 ; End of the structure

                    ^ 0
packet_talkend_out_code # Int           ; Return code
packet_talkend_out  * @                 ; End of the structure

        ; Contents of HPC packets for transmitting messages
                    ^ 0
packet_talktx_in_code # Int             ; Service ID and reason code
packet_talktx_in_handle # Int           ; Client handle for this task
packet_talktx_in_dest # Int             ; ID or handle of recipient
packet_talktx_in_msg # 1024             ; Message to send
packet_talktx_in    * @                 ; End of the structure

                    ^ 0
packet_talktx_out_code # Int            ; Return code
packet_talktx_out   * @                 ; End of the structure

        ; Contents of HPC packets for receiving messages
                    ^ 0
packet_talkrx_in_code # Int             ; Service ID and reason code
packet_talkrx_in_handle # Int           ; Client handle for this task
packet_talkrx_in    * @                 ; End of the structure

                    ^ 0
packet_talkrx_out_code # Int            ; Return code
packet_talkrx_out_id # Int              ; Source client ID
packet_talkrx_out_handle # Int          ; Source client handle
packet_talkrx_out_msg # 1024        ; The waiting message
packet_talkrx_out   * @                 ; End of the structure

        ; Contents of HPC packets for device driver banner
                    ^ 0
packet_dev_init_in_code # Int           ; Service ID and reason code
packet_dev_init_in_first # Byte         ; Drive number for first unit
packet_dev_init_in_text # 256           ; Text after "=" on CONFIG.SYS line
packet_dev_init_in  * @                 ; End of the structure

                    ^ 0
packet_dev_init_out_code # Int          ; Return code
packet_dev_init_out_num # Int           ; Number of devices supported
packet_dev_init_out_msg # 256           ; Text of banner message
packet_dev_init_out * @                 ; End of the structure

        ; Contents of HPC packets for device driver BIOS parameter block
                    ^ 0
bpb_bytes_per_sector # Short            ; Bytes per sector
bpb_sectors_per_unit # Byte             ; Sectors per allocation unit
bpb_reserved_sectors # Short            ; Number of reserved sectors
bpb_fats            # Byte              ; Number of file allocation tables
bpb_root_entries    # Short             ; Number of root directory entries
bpb_sectors         # Short             ; Total number of sectors
bpb_media           # Byte              ; Media descriptor byte
bpb_sectors_per_fat # Short             ; Number of sectors per FAT
bpb                 * @                 ; End of the structure

                    ^ 0
packet_dev_bpb_in_code # Int            ; Service ID and reason code
packet_dev_bpb_in_media # Byte          ; Media descriptor byte
packet_dev_bpb_in   * @                 ; End of the structure

                    ^ 0
packet_dev_bpb_out_code # Int           ; Return code
packet_dev_bpb_out_bpb # bpb            ; BIOS parameter block
packet_dev_bpb_out  * @                 ; End of the structure

        ; Contents of HPC packets for device driver media check
                    ^ 0
packet_dev_changed_in_code # Int        ; Service ID and reason code
packet_dev_changed_in_media # Byte      ; Media descriptor byte
packet_dev_changed_in * @               ; End of the structure

                    ^ 0
packet_dev_changed_out_code # Int       ; Return code
packet_dev_changed_out_media # Int      ; Media changed code
packet_dev_changed_out * @              ; End of the structure

        ; Contents of HPC packets for device driver read sector
                    ^ 0
packet_dev_read_in_code # Int           ; Service ID and reason code
packet_dev_read_in_unit # Byte          ; Unit code (drive number)
packet_dev_read_in_sector # Short       ; Sector number to read
packet_dev_read_in  * @                 ; End of the structure

                    ^ 0
packet_dev_read_out_code # Int          ; Return code
packet_dev_read_out_data # hpc_packet - @; The contents of the sector
packet_dev_read_out * @                 ; End of the structure

        ; Contents of HPC packets for device driver write sector
                    ^ 0
packet_dev_write_in_code # Int          ; Service ID and reason code
packet_dev_write_in_unit # Byte         ; Unit code (drive number)
packet_dev_write_in_sector # Short      ; Sector number to write
packet_dev_write_in_dummy # Byte        ; Padding to word boundary
packet_dev_write_in_data # hpc_packet - @; The contents of the sector
packet_dev_write_in * @                 ; End of the structure

                    ^ 0
packet_dev_write_out_code # Int         ; Return code
packet_dev_write_out * @                ; End of the structure

        ; Contents of HPC packets for converting dates and times to DOS format
                    ^ 0
packet_date_to_dos_in_code # Int        ; Service ID and reason code
packet_date_to_dos_in_time # OS_DateAndTime; 5 byte date and time
packet_date_to_dos_in * @               ; End of the structure

                    ^ 0
packet_date_to_dos_out_code # Int       ; Return code
packet_date_to_dos_out_time # Short     ; 2 byte time (hhhhhmmmmmmsssss)
packet_date_to_dos_out_date # Short     ; 2 byte date (yyyyyyymmmmddddd)
packet_date_to_dos_out * @              ; End of the structure

        ; Contents of HPC packets for converting dates and times to RISC OS
                    ^ 0
packet_date_to_riscos_in_code # Int     ; Service ID and reason code
packet_date_to_riscos_in_time # Short   ; 2 byte time (hhhhhmmmmmmsssss)
packet_date_to_riscos_in_date # Short   ; 2 byte date (yyyyyyymmmmddddd)
packet_date_to_riscos_in * @            ; End of the structure

                    ^ 0
packet_data_to_riscos_out_code # Int    ; Return code
packet_data_to_riscos_out_time # OS_DateAndTime; 5 byte date and time
packet_data_to_riscos_out * @           ; End of the structure

        ; Contents of HPC packets for starting *commands
                    ^ 0
packet_oscli_start_in_code # Int        ; Service ID and reason code
packet_oscli_start_in_command # 256     ; Command to execute
packet_oscli_start_in * @               ; End of the structure

                    ^ 0
packet_oscli_start_out_code # Int       ; Return code
packet_oscli_start_out_handle # Int     ; Command handle
packet_oscli_start_out * @              ; End of the structure

        ; Contents of HPC packets for polling *commands
                    ^ 0
packet_oscli_poll_in_code # Int         ; Service ID and reason code
packet_oscli_poll_in_handle # Int       ; Command handle
packet_oscli_poll_in_bytes # Int        ; Number of bytes of input
packet_oscli_poll_in_data # 256         ; Up to 256 bytes of input
packet_oscli_poll_in * @                ; End of the structure

                    ^ 0
packet_oscli_poll_out_code # Int        ; Return code
packet_oscli_poll_out_status # Int      ; The status
packet_oscli_poll_out_bytes # Int       ; Number of bytes of output
packet_oscli_poll_out_data # 256        ; Up to 256 bytes of output
packet_oscli_poll_out * @               ; End of the structure

        ; Contents of HPC packets for ending *commands
                    ^ 0
packet_oscli_end_in_code # Int          ; Service ID and reason code
packet_oscli_end_in_handle # Int        ; Command handle
packet_oscli_end_in * @                 ; End of the structure

                    ^ 0
packet_oscli_end_out_code # Int         ; Return code
packet_oscli_end_out_err # OS_Error     ; The error block returned
packet_oscli_end_out * @                ; End of the structure

        ; Contents of HPC packets for replying to messages
                    ^ 0
packet_talkreply_in_code # Int          ; Service ID and reason code
packet_talkreply_in_handle # Int        ; Client handle for this task
packet_talkreply_in_dest # Int          ; Handle of recipient
packet_talkreply_in_msg # 1024          ; Message to send
packet_talkreply_in * @                 ; End of the structure

                    ^ 0
packet_talkreply_out_code # Int         ; Return code
packet_talkreply_out * @                ; End of the structure

        ; Contents of HPC packets for controlling the speed
                    ^ 0
packet_faster_in_code # Int             ; Service ID and reason code
packet_faster_in_time # Int             ; Required time in centiseconds
packet_faster_in    * @                 ; End of the structure

                    ^ 0
packet_faster_out_code # Int            ; Return code
packet_faster_out   * @                 ; End of the structure

        ; Contents of HPC packets for producing temporary filenames
                    ^ 0
packet_temporary_in_code # Int          ; Service ID and reason code
packet_temporary_in * @                 ; End of the structure

                    ^ 0
packet_temporary_out_code # Int         ; Return code
packet_temporary_out_name # 256         ; Name of the file
packet_temporary_out * @                ; End of the structure

        ; Contents of HPC packets for device driver read long sector
                    ^ 0
packet_dev_read_long_in_code # Int      ; Service ID and reason code
packet_dev_read_long_in_unit # Byte     ; Unit code (drive number)
packet_dev_read_long_in_dummy # 3 * Byte; Padding to word boundary
packet_dev_read_long_in_sector # Int    ; Sector number to read
packet_dev_read_long_in  * @            ; End of the structure

                    ^ 0
packet_dev_read_long_out_code # Int     ; Return code
packet_dev_read_long_out_data # hpc_packet - @; The contents of the sector
packet_dev_read_long_out * @            ; End of the structure

        ; Contents of HPC packets for device driver write long sector
                    ^ 0
packet_dev_write_long_in_code # Int     ; Service ID and reason code
packet_dev_write_long_in_unit # Byte    ; Unit code (drive number)
packet_dev_write_long_in_dummy # 3 * Byte; Padding to word boundary
packet_dev_write_long_in_sector # Int   ; Sector number to write
packet_dev_write_long_in_data # hpc_packet - @; The contents of the sector
packet_dev_write_long_in * @            ; End of the structure

                    ^ 0
packet_dev_write_long_out_code # Int    ; Return code
packet_dev_write_long_out * @           ; End of the structure

        ; Contents of HPC packets for device driver file open
                    ^ 0
packet_dev_open_in_code # Int           ; Service ID and reason code
packet_dev_open_in_unit # Byte          ; Unit code (drive number)
packet_dev_open_in * @                  ; End of the structure

                    ^ 0
packet_dev_open_out_code # Int          ; Return code
packet_dev_open_out * @                 ; End of the structure

        ; Contents of HPC packets for device driver file close
                    ^ 0
packet_dev_close_in_code # Int          ; Service ID and reason code
packet_dev_close_in_unit # Byte         ; Unit code (drive number)
packet_dev_close_in * @                 ; End of the structure

                    ^ 0
packet_dev_close_out_code # Int         ; Return code
packet_dev_close_out * @                ; End of the structure

        ; Contents of HPC packets for device driver removable check
                    ^ 0
packet_dev_removable_in_code # Int      ; Service ID and reason code
packet_dev_removable_in_unit # Byte     ; Unit code (drive number)
packet_dev_removable_in * @             ; End of the structure

                    ^ 0
packet_dev_removable_out_code # Int     ; Return code
packet_dev_removable_out_status # Int   ; Removable device status
packet_dev_removable_out * @            ; End of the structure

        ; Extra data stored in PC memory allocations
                    ^ 0
memory_next         # Ptr               ; Pointer to next memory allocation
memory_prev         # Ptr               ; Pointer to previous memory allocation
memory_size         # Int               ; Size of this block
memory              * @                 ; End of the structure

        ; Entries in a DOSMap conversion list
                    ^ 0
dosmap_ext          # Int               ; DOS extension encoded
dosmap_type         # Int               ; RISC OS filetype
dosmap              * @                 ; End of the structure

        ; Part of a list of files opened by the PC
                    ^ 0
files_handle        # Int               ; The file handle
files_delete        # Int               ; Should the file be deleted
files_next          # Ptr               ; Pointer to next entry in the list
files_name          # String            ; Filename
files               * @                 ; End of the structure

        ; Message passing protocol clients
                    ^ 0
client_next         # Ptr               ; The next client structure
client_handle       # Int               ; The handle of this task
client_id           # Int               ; The ID of this task
client_msg_handle   # Int               ; The handle of the task sending message
client_msg_id       # Int               ; The ID of the task sending message
client_fn           # Ptr               ; Function pointer for new message
client_r12          # Int               ; Value of r12 when function called
client_flags        # Int               ; Flags specified when client registered
client_poll_word    # Int               ; The poll word for this task
client_next_num     # Int               ; The next message to deliver
client_last         # Int               ; The last message read
client_dest         # Int               ; Destination task
client_num          # Int               ; Number of this message
client_message      # 1024              ; The message block
client              * @                 ; End of the structure

        ; Internal message passing protocol clients
client_small        * client - ?client_message + 4

        ; Device driver records
                    ^ 0
device_next         # Ptr               ; The next device record
device_path         # String            ; Root path for the device
device_canon        # String            ; Canonicalised root path
device_canon_time   # Int               ; Time taken to canonicalise path
device_canon_last   # Int               ; Monotonic time of last name check
device_flags        # Int               ; Various flags
device_open_files   # Int               ; Number of open PC files
device_sectors      # Ptr               ; Pointer to list of sectors written
device_objects      # Ptr               ; Pointer to list of known objects
device_fat          # Ptr               ; Pointer to file allocation table
device_inverse_fat  # Ptr               ; Pointer to reverse FAT lookup table
device_root         # Ptr               ; Pointer to root directory
device              * @                 ; End of the structure

        ; The contents of a boot sector
                    ^ 0
boot_jmp            # 3                 ; JMP to executable code
boot_oem_name       # 8                 ; Optional OEM name and version
boot_bpb            # bpb               ; BIOS parameter block
boot_sectors_per_track # Short          ; Number of sectors in a track
boot_heads          # Short             ; Number of heads
boot_hidden_sectors # Int               ; Number of hidden sectors
boot_large_sectors  # Int               ; Real number of total sectors
boot_physical       # Short             ; Physical device number
boot_signature      # Byte              ; Extended boot record signature
boot_serial_number  # Int               ; Volume serial number
boot_label          # 11                ; Volume label
boot_file_system    # 7                 ; File system ID
boot_bootstrap      # &1fe - @          ; Pad to sector marker
boot_end1           # Byte              ; First end of sector marker
boot_end2           # Byte              ; Second end of sector marker
boot_padding        # bytes_per_sector - @; Pad to the end of a sector
boot                * @                 ; End of the structure

        ; Device driver written sectors
                    ^ 0
device_sector_next  # Ptr               ; The next sector record
device_sector_prev  # Ptr               ; The previous sector record
device_sector_no    # Int               ; The number of this sector
device_sector_data  # bytes_per_sector  ; The actual data for this sector
device_sector       * @                 ; End of the structure

        ; Device driver file or directory objects
                    ^ 0
device_object_next  # Ptr               ; Pointer to the next object record
device_object_start # Int               ; Start cluster number
device_object_parent # Ptr              ; Pointer to parent directory object
device_object_directory # Int           ; Pointer to directory entry
device_object_handle # Int              ; File handle if open
device_object_name  * @                 ; The pathname of this object
device_object       * @                 ; End of the structure

        ; Directory entries
                    ^ 0
directory_name      # name_length       ; File name
directory_extension # extension_length  ; File extension
directory_attribute # Byte              ; File attribute byte
directory_reserved  # 10                ; Reserved bytes
directory_time      # Short             ; Time field
directory_date      # Short             ; Date field
directory_cluster   # Short             ; The starting cluster number
directory_size      # Int               ; Size of the file
directory           * @                 ; End of the structure

        ; Long filename directory entries
                    ^ 0
longdir_sequence    # Byte              ; Long filename sequence index
longdir_fivechars   # 10                ; First five characters of filename
longdir_attribute   # Byte              ; File attribute byte = 0xff
longdir_zero        # Byte              ; A byte that is always zero
longdir_checksum    # Byte              ; Checksum of the DOS 8+3 filename
longdir_sixchars    # 12                ; Another six characters from the name
longdir_zero2       # Byte * 2          ; Two bytes that are always zero
longdir_twochars    # 4                 ; Another two characters from the name
longdir             * @                 ; End of the structure

        ; *command details
                    ^ 0
oscli_next          # Ptr               ; Pointer to the next record
oscli_prev          # Ptr               ; Pointer to the previous record
oscli_ws            # Ptr               ; Pointer to module workspace
oscli_redir_in      # Int               ; File handle for input
oscli_redir_out     # Int               ; File handle for output
oscli_in_handle     # Buffer_No         ; Input buffer handle
oscli_out_handle    # Buffer_No         ; Output buffer handle
oscli_eof           # Int               ; Has end of input been reached
oscli_restart       # Ptr               ; Restart address
oscli_env           # Int * 4 * 17      ; Copy of environment
oscli_r13           # Int               ; SVC stack pointer
oscli_stack         # svc_stack_size    ; Copy of SVC stack
oscli_in            # oscli_buffer_size ; Input buffer
oscli_out           # oscli_buffer_size ; Output buffer
oscli               * @                 ; End of the structure

; Define an area to dump everything into

        AREA    |main|, PIC, CODE, DATA

; Module header

        ENTRY
base    &       0                       ; Start offset
        &       initialisation - base   ; Initialisation offset
        &       finalisation - base     ; Finalisation offset
        &       service - base          ; Service call handler offset
        &       title - base            ; Title string offset
        &       help - base             ; Help string offset
        &       commands - base         ; Help and command keyword table offset
        &       swi_chunk               ; SWI chunk base number
        &       swi_handler - base      ; SWI handler code offset
        &       swi_table - base        ; SWI decoding table offset
        &       0                       ; SWI decoding code offset

; Module title and version

title   =       "ARMEdit", 0

help    =       "ARMEdit", 9
        =       version :LEFT: 8, " ", (version :LEFT: 12) :RIGHT: 3
        =       " 20", version :RIGHT: (:LEN: version - 13)
        =       " © A.Thoukydides, 1995-2001", 0
        ALIGN

; Handle ResourceFS files

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Create and install the ResourceFS file, if required.
create_resource_files
        LocalLabels
        JSR     "r0-r5, r10, r11"       ; Stack registers
        ADR     r0, resources           ; Get pointer to resource files
        SWI     XResourceFS_RegisterFiles; Add the files to ResourceFS
        RTE VS                          ; Exit without wiping over r0
        RTS                             ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Remove any files added to ResourceFS file, and free
        ;                 the memory allocated for that purpose.
destroy_resource_files
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        ADR     r0, resources           ; Get pointer to resource files
        SWI     XResourceFS_DeregisterFiles; Remove files from ResourceFS
        RTSS                            ; Return from subroutine

        ; Stuff to go in ResourceFS for general use
resources
        LocalLabels
        &       next$l - {PC}           ; Offset to next file
        &       &FFFFFF00               ; Load address of file (text file)
        &       0                       ; Execute address of file
        &       end$l - start$l         ; Actual file size
        &       FileSwitch_AttrOwnerRead; File attributes
        =       resource_directory      ; Directory
        =       ".Messages", 0          ; Filename
        ALIGN
        &       next$l - {PC}           ; Size of data area

start$l =       "EPCA:Module cannot be killed while the PC card is active", 10
        =       "ERCL:Module cannot be killed while clients are registered", 10
        =       "ENMA:Unable to extend dynamic area", 10
        =       "ETMP:Too many parameters", 10
        =       "DBAN:ARMEdit RISC OS Support ", version
        =       " (c) A.Thoukydides, 1995-2001|M|J|J", 10
end$l
        ALIGN
next$l  &       0                       ; End of list of files
        ALIGN

; Error blocks for the different error tokens

error_number\
        DefA    error_base              ; The first error number

        ;   Syntax      : [<label>] ErrBlck <token>
        ;   Parameters  : label - An optional program label.
        ;                 token - The token string that references the error,
        ;                         or the error text if it will not be passed
        ;                         though MessageTrans.
        ;   Description : Construct an error block for the specified token,
        ;                 unsing the next sequential error number.
        MACRO
$label  ErrBlck $token
$label  &       error_number            ; The error number
        =       "$token", 0             ; Null terminated token
        ALIGN                           ; Back to word aligned
error_number\
        SETA    error_number + 1        ; Increment the error number
        MEND

        ; The actual error blocks
err_pca ErrBlck "EPCA"
err_rcl ErrBlck "ERCL"
err_nma ErrBlck "ENMA"
err_tmp ErrBlck "ETMP"

; Help and command keyword table

commands
        ; Entry for command to do display some information
        =       "ARMEdit", 0            ; Command name
        ALIGN
        &       0                       ; Offset to code
        &       0                       ; Information word
        &       0                       ; Offset to syntax message
        &       help_armedit - base     ; Offset to help text

        ; Entry for command to display information about ARM side clients
        =       "ARMEdit_Clients", 0    ; Command name
        ALIGN
        &       command_armedit_clients - base; Offset to code
        &       0                       ; Information word
        &       syntax_armedit_clients - base; Offset to syntax message
        &       help_armedit_clients - base; Offset to help text

        ; Entry for command to display information about PC devices
        =       "ARMEdit_Devices", 0    ; Command name
        ALIGN
        &       command_armedit_devices - base; Offset to code
        &       0                       ; Information word
        &       syntax_armedit_devices - base; Offset to syntax message
        &       help_armedit_devices - base; Offset to help text

        ; Entry for command to reset PC devices
        =       "ARMEdit_DevicesRelog", 0; Command name
        ALIGN
        &       command_armedit_devices_relog - base; Offset to code
        &       &00010000               ; Information word
        &       syntax_armedit_devices_relog - base; Offset to syntax message
        &       help_armedit_devices_relog - base; Offset to help text

        ; Entry for command to (re)read DOSMap mappings
        =       "ARMEdit_DOSMap", 0     ; Command name
        ALIGN
        &       command_armedit_dosmap - base; Offset to code
        &       0                       ; Information word
        &       syntax_armedit_dosmap - base; Offset to syntax message
        &       help_armedit_dosmap - base; Offset to help text

        ; Entry for command to display list of files opened by PC software
        =       "ARMEdit_Files", 0     ; Command name
        ALIGN
        &       command_armedit_files - base; Offset to code
        &       0                       ; Information word
        &       syntax_armedit_files - base; Offset to syntax message
        &       help_armedit_files - base; Offset to help text

        ; Entry for command to display PC memory usage
        =       "ARMEdit_Memory", 0     ; Command name
        ALIGN
        &       command_armedit_memory - base; Offset to code
        &       0                       ; Information word
        &       syntax_armedit_memory - base; Offset to syntax message
        &       help_armedit_memory - base; Offset to help text

        ; Entry for command to control PC card polling
        =       "ARMEdit_Polling", 0    ; Command name
        ALIGN
        &       command_armedit_polling - base; Offset to code
        &       &00040f00               ; Information word
        &       syntax_armedit_polling - base; Offset to syntax message
        &       help_armedit_polling - base; Offset to help text

        ; Entry for command to display information about PC front-end versions
        =       "ARMEdit_Version", 0    ; Command name
        ALIGN
        &       command_armedit_version - base; Offset to code
        &       0                       ; Information word
        &       syntax_armedit_version - base; Offset to syntax message
        &       help_armedit_version - base; Offset to help text

        ; End of the help and command keyword table
        =       0
        ALIGN

; Help keyword *ARMEdit

        ; Help text for the command
help_armedit
        =       "The ARMEdit module provides low level ARM side services to"
        =       " several Acorn and Aleph One PC card utilities."
        =       " Unless PCPro is being used, this module must be loaded"
        =       " before starting the PC front-end software.", 0
        ALIGN

; Command *ARMEdit_DOSMap

        ;   Parameters  : r0    - Pointer to the command tail.
        ;                 r1    - Number of parameters. This is undefined
        ;                         for a configuration keyword.
        ;                 r12   - Pointer to module private word.
        ;   Returns     : r7-r11    - Preserved.
        ;   Description : Process the command *ARMEdit_DOSMap.
command_armedit_dosmap
        LocalLabels
        JSR     "r7-r11"                ; Stack registers
        LDR     r12, [r12]              ; Get pointer to workspace
        BL      dosmap_update           ; Update the mappings
        RTE VS                          ; Return any error generated
        RTS                             ; Return from subroutine

        ; Help text for the command
help_armedit_dosmap
        =       "*ARMEdit_DOSMap updates the cached list of mappings between"
        =       " DOS extensions and RISC OS filetypes.", 13

        ; Syntax message for the command
syntax_armedit_dosmap
        =       "Syntax: *ARMEdit_DOSMap", 0
        ALIGN

; Command *ARMEdit_Memory

        ;   Parameters  : r0    - Pointer to the command tail.
        ;                 r1    - Number of parameters. This is undefined
        ;                         for a configuration keyword.
        ;                 r12   - Pointer to module private word.
        ;   Returns     : r7-r11    - Preserved.
        ;   Description : Process the command *ARMEdit_Memory.
command_armedit_memory
        LocalLabels
        JSR     "r7-r11"                ; Stack registers
        LDR     r12, [r12]              ; Get pointer to workspace
        LDR     r0, ws_pc_memory        ; Get pointer to first block
        TEQ     r0, #0                  ; Are there any blocks
        BEQ     none$l                  ; Branch if none claimed
        SWI     XOS_WriteS              ; Write a suitable header
        =       "Address  Block size", 0
        ALIGN
        SWI     XOS_NewLine             ; Write a newline character
loop$l  TEQ     r0, #0                  ; Are there any more blocks
        BEQ     done$l                  ; Exit loop if not
        BL      show$l                  ; Branch to routine to display details
        LDR     r0, [r0, #memory_next]  ; Get pointer to next memory block
        B       loop$l                  ; Loop for the next block
done$l  RTSS                            ; Return from subroutine

        ; Handle the case where there is no memory allocated
none$l  SWI     XOS_WriteS              ; Write a suitable message
        =       "There is no RISC OS memory currently claimed by PC"
        =       " software.", 0
        ALIGN
        SWI     XOS_NewLine             ; Write a newline character
        RTSS                            ; Return from subroutine

        ; Display one line of the table
show$l  JSR     "r0-r1"                 ; Stack registers
        MOV     r1, r0                  ; Copy pointer to record
        ADD     r0, r1, #memory         ; Pointer to actual block
        BL      write_hex8              ; Output the address
        SWI     XOS_WriteI + ' '        ; A space character
        LDR     r0, [r1, #memory_size]  ; Get size of this block
        BL      write_cardinal4         ; Output the size of the block
        SWI     XOS_NewLine             ; Write a newline character
        RTS                             ; Return from subroutine

        ; Help text for the command
help_armedit_memory
        =       "*ARMEdit_Memory displays details of the RISC OS memory"
        =       " currently being used by PC software.", 13

        ; Syntax message for the command
syntax_armedit_memory
        =       "Syntax: *ARMEdit_Memory", 0
        ALIGN

; Command *ARMEdit_Files

        ;   Parameters  : r0    - Pointer to the command tail.
        ;                 r1    - Number of parameters. This is undefined
        ;                         for a configuration keyword.
        ;                 r12   - Pointer to module private word.
        ;   Returns     : r7-r11    - Preserved.
        ;   Description : Process the command *ARMEdit_Files.
command_armedit_files
        LocalLabels
        JSR     "r7-r11"                ; Stack registers
        LDR     r12, [r12]              ; Get pointer to workspace
        BL      dos$l                   ; List any files for DOS clients
        BL      dev$l                   ; List any files for the device driver
        RTSS                            ; Return from subroutine

        ; List of files opened for DOS clients
dos$l   JSR     "r0"                    ; Stack registers
        LDR     r0, ws_pc_files         ; Get pointer to first record
        TEQ     r0, #0                  ; Are there any blocks
        BEQ     dosz$l                  ; Branch if none open
        SWI     XOS_WriteS              ; Write a suitable header
        =       "Handle     Delete Filename", 0
        ALIGN
        SWI     XOS_NewLine             ; Write a newline character
dosl$l  TEQ     r0, #0                  ; Are there any more records
        RTSS EQ                         ; Return from subroutine if not
        BL      doss$l                  ; Branch to routine to display details
        LDR     r0, [r0, #files_next]   ; Get pointer to next file record
        B       dosl$l                  ; Loop for the next record
dosz$l  SWI     XOS_WriteS              ; Write a suitable message
        =       "There are no RISC OS files currently open for PC software.", 0
        ALIGN
        SWI     XOS_NewLine             ; Write a newline character
        RTSS                            ; Return from subroutine

        ; Display one line of the table
doss$l  JSR     "r0-r1"                 ; Stack registers
        MOV     r1, r0                  ; Copy pointer to record
        LDR     r0, [r1, #files_handle] ; Get the file handle
        BL      write_cardinal4         ; Write the file handle
        SWI     XOS_WriteI + ' '        ; A space character
        LDR     r0, [r1, #files_delete] ; Should file be deleted
        TEQ     r0, #0                  ; Should it be deleted
        ADREQ   r0, dosn$l              ; Pointer to keep
        ADRNE   r0, dosy$l              ; Pointer to do delete
        SWI     XOS_Write0              ; Write the deletion status
        SWI     XOS_WriteI + ' '        ; A space character
        ADD     r0, r1, #files_name     ; Get pointer to the filename
        SWI     XOS_Write0              ; Write the filename
        SWI     XOS_NewLine             ; Write a newline character
        RTS                             ; Return from subroutine
dosn$l  =       "No    ", 0
dosy$l  =       "Yes   ", 0
        ALIGN

        ; List files opened for the device driver
dev$l   JSR     "r0-r2"                 ; Stack registers
        MOV     r2, #0                  ; No previous device
        LDR     r0, ws_device_head      ; Pointer to first device driver
devl$l  TEQ     r0, #0                  ; Are there any more devices
        RTSS EQ                         ; Return from subroutine if not
        LDR     r1, [r0, #device_objects]; Pointer to list of objects
devo$l  TEQ     r1, #0                  ; Is the pointer valid
        BEQ     devd$l                  ; Skip the next bit if not
        BL      devs$l                  ; Show this file if required
        LDR     r1, [r1, #device_object_next]; Pointer to the next object
        B       devo$l                  ; Loop for the next object
devd$l  LDR     r0, [r0, #device_next]  ; Pointer to the next record
        B       devl$l                  ; Loop for the next record

        ; Display the details for this file if required
devs$l  JSR     "r0, r3"                ; Stack registers
        LDR     r3, [r1, #device_object_handle]; Get the file handle
        TEQ     r3, #0                  ; Is the file open
        RTSS EQ                         ; Return from subroutine if not
        TEQ     r0, r2                  ; Is this the first for this device
        BEQ     devn$l                  ; Skip the next bit if not
        MOV     r2, r0                  ; Copy this device pointer
        SWI     XOS_NewLine             ; Start a new line
        ADD     r0, r0, #device_path    ; Pointer to the device path
        SWI     XOS_Write0              ; Write the device path
        SWI     XOS_NewLine             ; Write another newline character
devn$l  SWI     XOS_WriteS              ; Indent this line
        =       "    ", 0
        ALIGN
        ADD     r0, r1, #device_object_name; Pointer to the name
        SWI     XOS_Write0              ; Write the filename
        SWI     XOS_NewLine             ; Write a newline character
        RTSS                            ; Return from subroutine

        ; Help text for the command
help_armedit_files
        =       "*ARMEdit_Files displays details of the RISC OS files"
        =       " currently being used by PC software.", 13

        ; Syntax message for the command
syntax_armedit_files
        =       "Syntax: *ARMEdit_Files", 0
        ALIGN

; Command *ARMEdit_Clients

        ;   Parameters  : r0    - Pointer to the command tail.
        ;                 r1    - Number of parameters. This is undefined
        ;                         for a configuration keyword.
        ;                 r12   - Pointer to module private word.
        ;   Returns     : r7-r11    - Preserved.
        ;   Description : Process the command *ARMEdit_Clients.
command_armedit_clients
        LocalLabels
        JSR     "r7-r11"                ; Stack registers
        LDR     r12, [r12]              ; Get pointer to workspace
        LDR     r0, ws_clients          ; Get pointer to first record
        ADR     r1, ws_client_armedit   ; Internal client pointer
        TEQ     r0, r1                  ; Is the internal client the only one
        TEQNE   r0, #0                  ; Are there any clients if not
        BEQ     none$l                  ; Branch if none registered
        SWI     XOS_WriteS              ; Write a suitable header
        =       "ID       Handle   Pollword TX to", 0
        ALIGN
        SWI     XOS_NewLine             ; Write a newline character
loop$l  TEQ     r0, #0                  ; Are there any more records
        BEQ     done$l                  ; Exit loop if not
        BL      show$l                  ; Branch to routine to display details
        LDR     r0, [r0, #client_next]  ; Get pointer to next client record
        B       loop$l                  ; Loop for the next record
done$l  RTSS                            ; Return from subroutine

        ; Handle the case where there are no clients registered
none$l  SWI     XOS_WriteS              ; Write a dummy message
        =       "There are no clients currently registered.", 0
        ALIGN
        SWI     XOS_NewLine             ; Write a newline
        RTSS                            ; Return from subroutine

        ; Display one line of the table
show$l  JSR     "r0-r1"                 ; Stack registers
        MOV     r1, r0                  ; Copy pointer to record
        LDR     r0, [r1, #client_id]    ; Get the client ID
        BL      write_hex8              ; Output the address
        SWI     XOS_WriteI + ' '        ; A space character
        LDR     r0, [r1, #client_handle]; Get the client handle
        BL      write_hex8              ; Output the address
        SWI     XOS_WriteI + ' '        ; A space character
        ADD     r0, r1, #client_poll_word; Address of poll word
        BL      write_hex8              ; Output the address
        SWI     XOS_WriteI + ' '        ; A space character
        LDR     r0, [r1, #client_dest]  ; Destination of message
        CMP     r0, #-1                 ; Is there an output message
        BEQ     notx$l                  ; Jump if no message waiting
        BL      write_hex8              ; Output the destination
        B       tx$l                    ; Skip the next bit
notx$l  SWI     XOS_WriteS              ; Text for no output message
        =       "No msg", 0
        ALIGN
tx$l    SWI     XOS_NewLine             ; Write a newline character
        RTS                             ; Return from subroutine

        ; Help text for the command
help_armedit_clients
        =       "*ARMEdit_Clients lists the clients that have registered"
        =       " with this module for message passing.", 13

        ; Syntax message for the command
syntax_armedit_clients
        =       "Syntax: *ARMEdit_Clients", 0
        ALIGN

; Command *ARMEdit_Version

        ;   Parameters  : r0    - Pointer to the command tail.
        ;                 r1    - Number of parameters. This is undefined
        ;                         for a configuration keyword.
        ;                 r12   - Pointer to module private word.
        ;   Returns     : r7-r11    - Preserved.
        ;   Description : Process the command *ARMEdit_Version.
command_armedit_version
        LocalLabels
        JSR     "r7-r11"                ; Stack registers
        LDR     r12, [r12]              ; Get pointer to workspace
        SWI     XOS_WriteS              ; Write header text
        =       "Structure  Current    Compatible Minimum    Maximum", 0
        ALIGN
        SWI     XOS_NewLine             ; Write a new line
        SWI     XOS_WriteS              ; Write next line of text
        =       "Hardware   ", 0
        ALIGN
        LDR     r0, ws_hdr_current_ver  ; Get current hardware version
        BL      write_cardinal4         ; Show this version number
        SWI     XOS_WriteI + ' '        ; A space character
        LDR     r0, ws_hdr_min_compat_ver; Get minimum compatible version
        BL      write_cardinal4         ; Show compatible version number
        SWI     XOS_WriteI + ' '        ; A space character
        MOV     r0, #hdr1state_version  ; Get minimum known version
        BL      write_cardinal4         ; Show known version number
        SWI     XOS_WriteI + ' '        ; A space character
        MOV     r0, #hdr2state_version  ; Get maximum known version
        BL      write_cardinal4         ; Show known version number
        SWI     XOS_NewLine             ; Write a new line
        SWI     XOS_WriteS              ; Write next line of text
        =       "Front-end  ", 0
        ALIGN
        LDR     r0, ws_fe_current_ver   ; Get current front-end version
        BL      write_cardinal4         ; Show this version number
        SWI     XOS_WriteI + ' '        ; A space character
        LDR     r0, ws_fe_min_compat_ver; Get minimum compatible version
        BL      write_cardinal4         ; Show compatible version number
        SWI     XOS_WriteI + ' '        ; A space character
        MOV     r0, #fe1state_version   ; Get minimum known version
        BL      write_cardinal4         ; Show known version number
        SWI     XOS_WriteI + ' '        ; A space character
        MOV     r0, #fe3state_version   ; Get maximum known version
        BL      write_cardinal4         ; Show known version number
        SWI     XOS_NewLine             ; Write another new line
        RTSS                            ; Return from subroutine

        ; Help text for the command
help_armedit_version
        =       "*ARMEdit_Version displays the version numbers of the data"
        =       " structures used by the most recently used PC front-end.", 13

        ; Syntax message for the command
syntax_armedit_version
        =       "Syntax: *ARMEdit_Version", 0
        ALIGN

; Command *ARMEdit_Devices

        ;   Parameters  : r0    - Pointer to the command tail.
        ;                 r1    - Number of parameters. This is undefined
        ;                         for a configuration keyword.
        ;                 r12   - Pointer to module private word.
        ;   Returns     : r7-r11    - Preserved.
        ;   Description : Process the command *ARMEdit_Devices.
command_armedit_devices
        LocalLabels
        JSR     "r7-r11"                ; Stack registers
        LDR     r12, [r12]              ; Get pointer to workspace
        LDR     r0, ws_device_head      ; Pointer to first device driver
        TEQ     r0, #0                  ; Are any devices active
        BEQ     none$l                  ; Skip next bit if not
loop$l  TEQ     r0, #0                  ; Are there any more records
        BEQ     done$l                  ; Exit loop if not
        BL      show$l                  ; Branch to routine to display details
        LDR     r0, [r0, #device_next]  ; Pointer to next record
        B       loop$l                  ; Loop for next record
done$l  RTSS                            ; Return from subroutine

        ; Handle the case where there are no devices active
none$l  SWI     XOS_WriteS              ; Write a dummy message
        =       "There are no active devices.", 0
        ALIGN
        SWI     XOS_NewLine             ; Write a newline
        RTSS                            ; Return from subroutine

        ; Display one line of the table
show$l  JSR     "r0-r3, r10"            ; Stack registers
        MOV     r10, r0                 ; Copy device record pointer
        ADD     r0, r10, #device_path   ; Pointer to root path
        SWI     XOS_Write0              ; Display root directory
        LDR     r1, [r10, #device_flags]; Get the device flags
        ADR     r0, t_rp$l              ; Pointer to tag description
        BL      tag$l                   ; Show the tag description
        TST     r1, #device_flag_relog_pending; Test whether flag is set
        BL      yesno$l                 ; Display Yes or No
        ADR     r0, t_rf$l              ; Pointer to tag description
        BL      tag$l                   ; Show the tag description
        TST     r1, #device_flag_relog_forced; Test whether flag is set
        BL      yesno$l                 ; Display Yes or No
        ADR     r0, t_w$l               ; Pointer to tag description
        BL      tag$l                   ; Show the tag description
        TST     r1, #device_flag_write  ; Test whether flag is set
        BL      yesno$l                 ; Display Yes or No
        ADR     r0, t_lf$l              ; Pointer to tag description
        BL      tag$l                   ; Show the tag description
        TST     r1, #device_flag_long   ; Test whether flag is set
        BL      yesno$l                 ; Display Yes or No
        ADR     r0, t_nc$l              ; Pointer to tag description
        BL      tag$l                   ; Show the tag description
        TST     r1, #device_flag_raw    ; Test whether flag is set
        BL      yesno$l                 ; Display Yes or No
        ADR     r0, t_mn$l              ; Pointer to tag description
        BL      tag$l                   ; Show the tag description
        TST     r1, #device_flag_manual ; Test whether flag is set
        BL      yesno$l                 ; Display Yes or No
        ADR     r0, t_an$l              ; Pointer to tag description
        BL      tag$l                   ; Show the tag description
        TST     r1, #device_flag_anon   ; Test whether flag is set
        BL      yesno$l                 ; Display Yes or No
        ADR     r0, t_if$l              ; Pointer to tag description
        BL      tag$l                   ; Show the tag description
        TST     r1, #device_flag_imagefs; Test whether flag is set
        BL      yesno$l                 ; Display Yes or No
        ADR     r0, t_of$l              ; Pointer to tag description
        BL      tag$l                   ; Show the tag description
        LDR     r0, [r10, #device_open_files]; Number of open files
        BL      card4$l                 ; Display the number
        LDR     r0, [r10, #device_objects]; Pointer to list of objects
        MOV     r1, #0                  ; Initially no objects counted
        MOV     r2, #0                  ; Initially no open files counted
showl$l TEQ     r0, #0                  ; Is the pointer valid
        BEQ     showd$l                 ; Exit the loop if not
        ADD     r1, r1, #1              ; Increment the total number of objects
        LDR     r3, [r0, #device_object_handle]; Get the file handle
        TEQ     r3, #0                  ; Is the file handle valid
        ADDNE   r2, r2, #1              ; Increment number of open files
        LDR     r0, [r0, #device_object_next]; Pointer to the next record
        B       showl$l                 ; Loop for the next record
showd$l ADR     r0, t_oc$l              ; Pointer to tag description
        BL      tag$l                   ; Show the tag description
        MOV     r0, r1                  ; Copy total number of objects
        BL      card4$l                 ; Display the number
        ADR     r0, t_oa$l              ; Pointer to tag description
        BL      tag$l                   ; Show the tag description
        MOV     r0, r2                  ; Copy number of open files
        BL      card4$l                 ; Display the number
        SWI     XOS_NewLine             ; Newline after the last entry
        LDR     r0, [r10, #device_next] ; Is there another device record
        TEQ     r0, #0                  ; Is it a valid pointer
        SWINE   XOS_NewLine             ; Extra newline between entries
        RTSS                            ; Return from subroutine
t_rp$l  =       "Relog of device pending            ", 0
t_rf$l  =       "Relog of device has been forced    ", 0
t_w$l   =       "Sector write operations enabled    ", 0
t_lf$l  =       "Windows 95 long filenames enabled  ", 0
t_nc$l  =       "Path canonicalisation disabled     ", 0
t_mn$l  =       "Automatic relogging disabled       ", 0
t_an$l  =       "Anonymous volume labels            ", 0
t_if$l  =       "Image files treated as directories ", 0
t_of$l  =       "Number of DOS open files           ", 0
t_oc$l  =       "Total number of logged objects     ", 0
t_oa$l  =       "Number of RISC OS open files       ", 0

        ; Display the tag text at the start of a line
tag$l   JSR     ""                      ; Stack registers
        SWI     XOS_NewLine             ; Write a newline character
        SWI     XOS_WriteS              ; Indent this line
        =       "    ", 0
        ALIGN
        SWI     XOS_Write0              ; Display the tag for this line
        SWI     XOS_WriteS              ; Display the value separator
        =       ": ", 0
        ALIGN
        RTSS                            ; Return from subroutine

        ; Display either Yes or No
yesno$l JSR     ""                      ; Stack registers
        BEQ     yesnon$l                ; Skip to the next phrase if not set
        SWI     XOS_WriteS              ; Display Yes
        =       "Yes", 0
        ALIGN
        RTSS                            ; Return from subroutine
yesnon$l
        SWI     XOS_WriteS              ; Display No
        =       "No", 0
        ALIGN
        RTSS                            ; Return from subroutine

        ; Display a 4 byte unsigned integer
card4$l JSR     "r1-r2"                 ; Stack registers
        ADRL    r1, ws_buffer           ; Pointer to a general buffer
        MOV     r2, #String             ; Size of the buffer
        SWI     XOS_ConvertCardinal4    ; Convert the number of files
        SWI     XOS_Write0              ; Write the number
        RTSS                            ; Return from subroutine

        ; Help text for the command
help_armedit_devices
        =       "*ARMEdit_Devices lists the currently supported DOS"
        =       " devices.", 13

        ; Syntax message for the command
syntax_armedit_devices
        =       "Syntax: *ARMEdit_Devices", 0
        ALIGN

; Command *ARMEdit_DevicesRelog

        ;   Parameters  : r0    - Pointer to the command tail.
        ;                 r1    - Number of parameters. This is undefined
        ;                         for a configuration keyword.
        ;                 r12   - Pointer to module private word.
        ;   Returns     : r7-r11    - Preserved.
        ;   Description : Process the command *ARMEdit_DevicesRelog.
command_armedit_devices_relog
        LocalLabels
        JSR     "r7-r11"                ; Stack registers
        LDR     r12, [r12]              ; Get pointer to workspace
        MOV     r1, r0                  ; Copy pointer to the command tail
        ADR     r0, key$l               ; Keyword definition
        ADRL    r2, ws_buffer           ; Pointer to output buffer
        MOV     r3, #String             ; Size of output buffer
        SWI     XOS_ReadArgs            ; Process command line argument
        RTE VS                          ; Exit if error produced
        LDR     r2, [r2]                ; Was the now switch specified
        LDR     r0, ws_device_head      ; Pointer to first device driver
loop$l  TEQ     r0, #0                  ; Are there any more records
        RTSS EQ                         ; Return from subroutine
        LDR     r1, [r0, #device_flags] ; Get flags
        TST     r1, #device_flag_relog_avail; Can an update be performed
        BNE     reset$l                 ; Perform reset if possible
        TEQ     r2, #0                  ; Was the now switch specified
        BNE     now$l                   ; Force a reset if required
        ORR     r1, r1, #device_flag_relog_pending; Set pending flag
        STR     r1, [r0, #device_flags] ; Store updated flags
        B       done$l                  ; Skip the next bit
reset$l BL      device_object_reset     ; Reset the object list
        BL      device_sector_reset     ; Reset the sector list
        BL      device_prepare          ; Ensure everything is initialised
done$l  LDR     r0, [r0, #device_next]  ; Pointer to next record
        B       loop$l                  ; Loop for next record
now$l   ORR     r1, r1, #device_flag_relog_forced; Mark the reset as forced
        BIC     r1, r1, #device_flag_relog_pending; Reset is no longer pending
        BIC     r1, r1, #device_flag_changed; Structure no longer changed
        STR     r1, [r0, #device_flags] ; Write the modified flags
        B       reset$l                 ; Finally perform the reset

        ; Keyword definition
key$l   =       "now/S", 0

        ; Help text for the command
help_armedit_devices_relog
        =       "*ARMEdit_DevicesRelog resets any active devices."
        =       " This has the effect of forcing the RISC OS directory"
        =       " structure to be relogged.", 13

        ; Syntax message for the command
syntax_armedit_devices_relog
        =       "Syntax: *ARMEdit_DevicesRelog [-now]", 0
        ALIGN

; Command *ARMEdit_Polling

        ;   Parameters  : r0    - Pointer to the command tail.
        ;                 r1    - Number of parameters. This is undefined
        ;                         for a configuration keyword.
        ;                 r12   - Pointer to module private word.
        ;   Returns     : r7-r11    - Preserved.
        ;   Description : Process the command *ARMEdit_Polling.
command_armedit_polling
        LocalLabels
        JSR     "r7-r11"                ; Stack registers
        LDR     r12, [r12]              ; Get pointer to workspace
        MOV     r1, r0                  ; Copy pointer to command tail
        ADR     r0, key$l               ; Keyword definition
        ADRL    r2, ws_buffer           ; Pointer to output buffer
        MOV     r3, #String             ; Size of output buffer
        SWI     XOS_ReadArgs            ; Process command line argument
        RTE VS                          ; Exit if error produced
        LDR     r0, [r2]                ; Get pointer to first keyword
        LDR     r1, [r2, #4]            ; Get pointer to second keyword
        LDR     r2, [r2, #8]            ; Get pointer to third keyword
        TEQ     r0, #0                  ; Was the first keyword used
        TEQEQ   r1, #0                  ; Was the second keyword used
        TEQEQ   r2, #0                  ; Was the third keyword used
        BEQ     show$l                  ; Display current settings if none
        TEQ     r0, #0                  ; Was the first keyword used
        TEQNE   r1, #0                  ; Was the second keyword used
        TEQNE   r2, #0                  ; Was the third keyword used
        BNE     many$l                  ; Exit with an error if too many
        TEQ     r0, #0                  ; Is the unlabelled value specified
        BEQ     fore$l                  ; Skip next bit if not
        ADD     r0, r0, #1              ; Pointer to evaluated value
        LDRU    r3, r0, r4, r5          ; Read the value
        STR     r3, ws_activity_fore    ; Store the new foreground value
        STR     r3, ws_activity_back    ; Store the new background value
fore$l  TEQ     r1, #0                  ; Is foreground value specified
        BEQ     back$l                  ; Skip next bit if not
        ADD     r1, r1, #1              ; Pointer to evaluated value
        LDRU    r3, r1, r4, r5          ; Read the value
        STR     r3, ws_activity_fore    ; Store the new value
back$l  TEQ     r2, #0                  ; Is background value specified
        RTSS EQ                         ; Return from subroutine if not
        ADD     r2, r2, #1              ; Pointer to evaluated value
        LDRU    r3, r2, r4, r5          ; Read the value
        STR     r3, ws_activity_back    ; Store the new value
        RTSS                            ; Return from subroutine

show$l  SWI     XOS_WriteS              ; Write first line prefix
        =       "Foreground ", 0
        ALIGN
        LDR     r0, ws_activity_fore    ; Get current foreground setting
        BL      write_cardinal4         ; Display the value
        SWI     XOS_NewLine             ; Write a new line
        SWI     XOS_WriteS              ; Write first line prefix
        =       "Background ", 0
        ALIGN
        LDR     r0, ws_activity_back    ; Get current background setting
        BL      write_cardinal4         ; Display the value
        SWI     XOS_NewLine             ; Write another new line
        RTSS                            ; Return from subroutine

many$l  ADRL    r0, err_tmp             ; Pointer to error block
        ADR     r1, ws_message          ; Pointer to messages control block
        MOV     r2, #0                  ; Use internal buffer
        SWI     XMessageTrans_ErrorLookup; Lookup the error text
        MOV     r3, #0                  ; Ensure no memory moved
        RTE                             ; Return from subroutine

        ; Keyword definition
key$l   =       "/E,fore/K/E,back/E", 0

        ; Help text for the command
help_armedit_polling
        =       "*ARMEdit_Polling allows the multitasking speed of the PC"
        =       " card to be controlled. Use with no parameters to display"
        =       " the current settings.", 13

        ; Syntax message for the command
syntax_armedit_polling
        =       "Syntax: *ARMEdit_Polling [[-fore] <polls>]"
        =       " [[-back] <polls>]", 0
        ALIGN

        ;   Parameters  : r0    - Foreground speed, or -1 to read the current
        ;                         setting.
        ;                 r1    - Background speed, or -1 to read the current
        ;                         setting.
        ;   Returns     : r0    - The current foreground speed.
        ;                 r1    - The current background speed.
        ;   Description : Set the multitasking speed.
polling LocalLabels
        JSR     ""                      ; Stack registers
        CMP     r0, #-1                 ; Has a new value been specified
        STRNE   r0, ws_activity_fore    ; Store the new foreground value
        CMP     r1, #-1                 ; Has a new value been specified
        STRNE   r1, ws_activity_back    ; Store the new background value
        LDR     r0, ws_activity_fore    ; Get the foreground value
        LDR     r1, ws_activity_back    ; Get the background value
        RTS                             ; Return from subroutine

; A literal pool

        LTORG

; Initialisation entry point

        ;   Parameters  : r10       - Pointer to environment string.
        ;                 r11       - Instantiation number.
        ;                 r12       - Pointer to module private word.
        ;                 r13       - Supervisor stack pointer.
        ;   Returns     : r7-r11    - Preserved.
        ;                 r13       - Preserved.
        ;   Description : Initialisation entry point handler.
initialisation
        LocalLabels
        JSR     "r7-r11"                ; Stack registers
        MOV     r0, #OSModule_Alloc     ; Claim entry code
        LDR     r3, size$l              ; Required size
        SWI     XOS_Module              ; Claim workspace
        RTE VS                          ; Exit if error
        STR     r2, [r12]               ; Store workspace pointer
        MOV     r12, r2                 ; Copy workspace pointer
        [       pipe
        BL      pipe_open               ; Open the output pipe
        MOVVS   r11, r0                 ; Copy error pointer
        BVS     fail_pipe$l             ; Fail if unable to open pipe
        ]
        BL      dynamic_initialise      ; Initialise dynamic area manager
        MOVVS   r11, r0                 ; Copy error pointer
        BVS     fail_dynamic$l          ; Fail if unable to initialise memory
        BL      scrap_initialise        ; Initialise scrap directory
        MOVVS   r11, r0                 ; Copy error pointer
        BVS     fail_scrap$l            ; Fail if unable to set scrap directory
        BL      create_resource_files   ; Register resource files
        MOVVS   r11, r0                 ; Copy error pointer
        BVS     fail_files$l            ; Fail if unable to create messages
        BL      open_messages           ; Open messages file
        MOVVS   r11, r0                 ; Copy error pointer
        BVS     fail_messages$l         ; Fail if unable to open messages
        BL      initialise_pc           ; Initialise the PC status variables
        MOVVS   r11, r0                 ; Copy error pointer
        BVS     fail_pc$l               ; Fail if unable to initialise PC bits
        BL      dosmap_initialise       ; Initialise DOSMap handling
        MOVVS   r11, r0                 ; Copy error pointer
        BVS     fail_dosmap$l           ; Fail if unable to initialise DOSMap
        BL      memory_initialise       ; Initialise memory handling
        MOVVS   r11, r0                 ; Copy error pointer
        BVS     fail_memory$l           ; Fail if unable to initialise memory
        BL      file_initialise         ; Initialise file handling
        MOVVS   r11, r0                 ; Copy error pointer
        BVS     fail_file$l             ; Fail if unable to initialise files
        BL      talk_initialise         ; Initialise communications handling
        MOVVS   r11, r0                 ; Copy error pointer
        BVS     fail_talk$l             ; Fail if unable to initialise clients
        BL      device_initialise       ; Initialise device driver support
        MOVVS   r11, r0                 ; Copy error pointer
        BVS     fail_device$l           ; Fail if unable to initialise device
        BL      date_initialise         ; Reset time zone details
        MOVVS   r11, r0                 ; Copy error pointer
        BVS     fail_date$l             ; Fail if unable to initialise dates
        BL      oscli_initialise        ; Initialise command handler
        MOVVS   r11, r0                 ; Copy error pointer
        BVS     fail_oscli$l            ; Fail if unable to initiaslie oscli
        BL      pc_initialise           ; Initialise with PC front-end
        MOVVS   r11, r0                 ; Copy error pointer
        BVS     fail_frontend$l         ; Fail if error initialising
        RTSS                            ; Exit, pulling registers

size$l  &       ws_end - ws_start       ; Workspace size

; A literal pool

        LTORG

; Finalisation entry point

        ;   Parameters  : r10   - Fatality indication: 0 is non-fatal,
        ;                         1 is fatal.
        ;                 r11   - Instantiation number.
        ;                 r12   - Pointer to module private word.
        ;                 r13   - Supervisor stack.
        ;   Returns     : r7-r11    - Preserved.
        ;                 r13       - Preserved.
        ;   Description : Finalisation entry point handler. This is called
        ;                 before killing the module.
finalisation
        JSR     "r7-r11"                ; Stack registers
        LDR     r12, [r12]              ; Get workspace pointer
        LDR     r0, ws_pc_active        ; Get PC activity flag
        TEQ     r0, #0                  ; Is the PC front-end active
        ADRNEL  r0, err_pca             ; Pointer to error block
        BNE     error$l                 ; Produce error if active
        LDR     r0, ws_clients          ; Get pointer to list of clients
        ADR     r1, ws_client_armedit   ; Internal client pointer
        TEQ     r0, r1                  ; Is the internal client the only one
        TEQNE   r0, #0                  ; Are there any clients if not
        ADRNEL  r0, err_rcl             ; Pointer to error block
        BNE     error$l                 ; Produce error if active
        MOV     r11, #0                 ; Clear error pointer
fail_frontend$l
        BL      oscli_finalise          ; Finalise command handler
fail_oscli$l
fail_date$l
        BL      device_finalise         ; Finalise device driver support
fail_device$l
fail_talk$l
fail_file$l
fail_memory$l
        BL      dosmap_finalise         ; Release DOSMap workspace
fail_dosmap$l
fail_pc$l
        BL      close_messages          ; Close messages file
fail_messages$l
        BL      destroy_resource_files  ; Delete the messages files
fail_files$l
fail_scrap$l
        BL      dynamic_finalise        ; Remove any dynamic area
fail_dynamic$l
        [       pipe
        BL      pipe_close              ; Close the output pipe
fail_pipe$l
        ]
        MOV     r0, #OSModule_Free      ; Free entry code
        MOV     r2, r12                 ; Pointer to workspace
        SWI     XOS_Module              ; Release workspace
        TEQ     r11, #0                 ; Is there any error
        RTSS EQ                         ; Exit if not
        MOV     r0, r11                 ; Copy error pointer
        SetV                            ; Set the error flag
        RTE                             ; Return with the error

        ; Return an error
error$l ADR     r1, ws_message          ; Pointer to messages control block
        MOV     r2, #0                  ; Use internal buffer
        SWI     XMessageTrans_ErrorLookup; Lookup the error text
        MOV     r3, #0                  ; Ensure no memory moved
        RTE                             ; Return from subroutine

; PC front-end initialisation

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Attempt to register with a copy of the PC front-end
        ;                 that is already running.
pc_initialise
        LocalLabels
        JSR     "r0-r7"                 ; Stack registers
        BL      config_reset            ; Reset any loaded configuration
        SWI     XPCHelp_GetStructAddr   ; Attempt to read structure pointers
        RTSS VS                         ; Exit if failed
        TEQ     r0, #0                  ; Check if pointer is valid
        TEQNE   r1, #0                  ; Check if other pointer is valid
        RTSS EQ                         ; Exit if not
        MOV     r2, r0                  ; Copy pointer to hardware structure
        MOV     r3, r1                  ; Copy pointer to front-end strcuture
        MOV     r0, #0                  ; No extra data pointer
        MOV     r1, #Service_PCDevice   ; Service call code for PC front-end
        STR     r12, ws_buffer          ; Store private word value
        ADRL    r12, ws_buffer          ; Fake private word pointer
        BL      service                 ; Fake the service call
        RTSS                            ; Return from subroutine

; Service call handler

        ;   Parameters  : r1    - Service number.
        ;                 r12   - Pointer to module private word.
        ;                 r13   - A full, descending stack.
        ;   Returns     : r0    - May be used to pass back a result.
        ;                 r1    - Set to zero if the service is being claimed.
        ;                 r2-r8 - May be used to pass back a result.
        ;                 r13   - Preserved.
        ;   Description : Service call handler.
        LocalLabels
        &       table$l - base          ; Service table offset
service MOV     r0, r0                  ; Magic instruction for service table
        TEQ     r1, #Service_ResourceFSStarting
        TEQNE   r1, #Service_PCDevice
        MOVNES  pc, r14                 ; Reject unrecognised calls quickly
dispatch$l
        LDR     r12, [r12]              ; Workspace pointer
        TEQ     r1, #Service_ResourceFSStarting
        BEQ     resourcefs_starting$l
        TEQ     r1, #Service_PCDevice
        BEQ     pc_device$l
        MOVS    pc, r14                 ; Exit if this point reached

        ; Service call table (codes must be in ascending numerical order)
table$l &       0                       ; Default flags
        &       dispatch$l - base       ; Offset to handler code
        &       Service_ResourceFSStarting
        &       Service_PCDevice
        &       0                       ; End of list of service calls

        ; ResourceFS module is reloaded or reinitialised
resourcefs_starting$l
        JSR     "r0-r2"                 ; Stack registers
        ADRL    r0, resources           ; Pointer to resource files
        MOV     r14, pc                 ; Copy return address
        MOV     pc, r2                  ; Call ResourceFS routine
        RTSS                            ; Exit, pulling registers

        ; The PC card front-end is starting
pc_device$l
        JSR     "r0-r4"                 ; Stack registers
;        pipe_string "Service_PCDevice R0=%0 R1=%1 R2=%2 R3=%3", r0, r1, r2, r3
        LDR     r0, [r2, #hdr1_current_ver]; Get hardware structure version
        STR     r0, ws_hdr_current_ver  ; Store version number
        LDR     r1, [r2, #hdr1_min_compat_ver]; Get minimum structure version
        STR     r1, ws_hdr_min_compat_ver; Store version number
        MOV     r4, #2                  ; Start with the latest version
        CMP     r0, #hdr2state_version  ; Compare to known structure version
        MOVLT   r4, #0                  ; Is it too old
        CMP     r1, #hdr2state_version  ; Compare to known structure version
        MOVGT   r4, #0                  ; Is it too new
        TEQ     r4, #0                  ; Was a match found
        BNE     pc_device_hdr$l         ; Skip next bit if match found
        MOV     r4, #1                  ; Try original version
        CMP     r0, #hdr1state_version  ; Compare to known structure version
        MOVLT   r4, #0                  ; Is it too old
        CMP     r1, #hdr1state_version  ; Compare to known structure version
        MOVGT   r4, #0                  ; Is it too new
pc_device_hdr$l
        STR     r4, ws_hdr_ver          ; Store indexed hardware version
        LDR     r0, [r3, #fe1_current_ver]; Get front-end structure version
        STR     r0, ws_fe_current_ver   ; Store version number
        LDR     r1, [r3, #fe1_min_compat_ver]; Get minimum structure version
        STR     r1, ws_fe_min_compat_ver; Store version number
        TEQ     r4, #0                  ; Was a match found
        RTSS EQ                         ; Return if no match found
        MOV     r4, #3                  ; Start with latest version
        CMP     r0, #fe3state_version   ; Compare to known structure version
        MOVLT   r4, #0                  ; Is it too old
        CMP     r1, #fe3state_version   ; Compare to known structure version
        MOVGT   r4, #0                  ; Is it too new
        TEQ     r4, #0                  ; Was a match found
        BNE     pc_device_fe$l          ; Skip next bit if match found
        MOV     r4, #2                  ; Start with latest version
        CMP     r0, #fe2state_version   ; Compare to known structure version
        MOVLT   r4, #0                  ; Is it too old
        CMP     r1, #fe2state_version   ; Compare to known structure version
        MOVGT   r4, #0                  ; Is it too new
        TEQ     r4, #0                  ; Was a match found
        BNE     pc_device_fe$l          ; Skip next bit if match found
        MOV     r4, #1                  ; Try original version
        CMP     r0, #fe1state_version   ; Compare to known structure version
        MOVLT   r4, #0                  ; Is it too old
        CMP     r1, #fe1state_version   ; Compare to known structure version
        MOVGT   r4, #0                  ; Is it too new
        TEQ     r4, #0                  ; Was a match found
        BNE     pc_device_fe$l          ; Skip next bit if match found
        STR     r4, ws_hdr_ver          ; Clear indexed front-end version
        RTSS                            ; Return if no match found
pc_device_fe$l
        STR     r4, ws_fe_ver           ; Store indexed front-end version
        STR     r2, ws_hdr_state        ; Store pointer to hardware state
        STR     r3, ws_fe_state         ; Store pointer to front-end state
        MOV     r0, #1                  ; Value to indicate PC is active
        STR     r0, ws_pc_active        ; The PC front-end is not active
        MOV     r0, #sys_event_shutdown ; Number of shutdown event
        ADRL    r1, event_shutdown_handler; Pointer to handler function
        ADR     r2, ws_event_shutdown   ; Pointer to event block
        BL      add_event_handler       ; Add shutdown handler
        MOV     r0, #sys_event_hard_reset; Number of reset event
        ADRL    r1, event_reset_handler ; Pointer to handler function
        ADR     r2, ws_event_reset      ; Pointer to event block
        BL      add_event_handler       ; Add reset handler
        MOV     r0, #sys_event_poll_chain; Number of poll event
        ADRL    r1, event_poll_handler  ; Pointer to handler function
        ADR     r2, ws_event_poll       ; Pointer to event block
        BL      add_event_handler       ; Add poll handler
        MOV     r0, #sys_event_set_config; Number of poll event
        ADRL    r1, event_config_handler; Pointer to handler function
        ADR     r2, ws_event_config     ; Pointer to event block
        BL      add_event_handler       ; Add set configuration handler
        ADRL    r0, config_handler      ; Pointer to handler function
        ADR     r1, ws_config           ; Pointer to configuration block
        BL      add_config_handler      ; Add configuration handler
        MOV     r0, #emulate_port       ; Port to add handlers for
        ADRL    r1, port_emulate_read8_handler; Pointer to byte read handler
        ADRL    r2, port_emulate_read16_handler; Pointer to word read handler
        ADRL    r3, port_emulate_write8_handler; Pointer to byte write handler
        ADRL    r4, port_emulate_write16_handler; Pointer to word write handler
        BL      add_io_handler          ; Add I/O port handler
        LDR     r0, = sys_hpc_armedit   ; The HPC service identifier
        ADRL    r1, hpc_packet_handler  ; Pointer to HPC handler function
        ADR     r2, ws_hpc_handler      ; Pointer to HPC handler block
        BL      add_hpc_handler         ; Add HPC handler
        BL      config_reset            ; Reset any loaded configuration
        RTSS                            ; Exit, pulling registers

; SWI handler and decoding

        ;   Parameters  : r0-r8 - Passed from SWI caller.
        ;                 r11   - SWI number modulo chunk size (i.e. 0 to 63).
        ;                 r12   - Pointer to module private word.
        ;                 r13   - A full, descending stack.
        ;                 r14   - Contains flags of SWI caller, with V cleared.
        ;   Returns     : r0-r9 - Returned to SWI caller.
        ;                 r10-r12 - Corrupted.
        ;                 r13   - Preserved.
        ;   Description : Handler for SWIs belonging to this module.
swi_handler
        LocalLabels
        LDR     r12, [r12]              ; Get pointer to workspace
        CMP     r11, #(jump_end$l - jump$l) >> 2; Is it in range
        ADDLO   pc, pc, r11, LSL#2      ; Dispatch if in range
        B       unknown$l               ; Unknown SWI
jump$l  B       control_pc              ; Jump to handler for 0th SWI
        B       talk_start              ; Jump to handler for 1st SWI
        B       talk_end                ; Jump to handler for 2nd SWI
        B       talk_tx                 ; Jump to handler for 3rd SWI
        B       talk_rx                 ; Jump to handler for 4th SWI
        B       talk_ack                ; Jump to handler for 5th SWI
        B       hpc_service             ; Jump to handler for 6th SWI
        B       polling                 ; Jump to handler for 7th SWI
        B       talk_reply              ; Jump to handler for 8th SWI
jump_end$l

        ; Generate an error if the SWI doesn't exist
unknown$l
        JSR     ""                      ; Stack registers
        ADR     r0, err_unknown$l       ; Pointer to error block
        MOV     r1, #0                  ; Use global messages
        MOV     r2, #0                  ; Use an internal buffer
        ADRL    r4, title               ; Name of module
        SWI     XMessageTrans_ErrorLookup; Lookup the error text
        RTE                             ; Exit without wiping over r0

        ; The error block for an unkown SWI
err_unknown$l
        &       Error_NoSuchSWI         ; Error number same as system message
        =       "BadSWI", 0             ; Token to lookup
        ALIGN

        ; SWI decoding table
swi_table
        =       "ARMEdit", 0            ; SWI group prefix
        =       "ControlPC", 0          ; Name of 0th SWI
        =       "TalkStart", 0          ; Name of 1st SWI
        =       "TalkEnd", 0            ; Name of 2nd SWI
        =       "TalkTX", 0             ; Name of 3rd SWI
        =       "TalkRX", 0             ; Name of 4th SWI
        =       "TalkAck", 0            ; Name of 5th SWI
        =       "HPC", 0                ; Name of 6th SWI
        =       "Polling", 0            ; Name of 7th SWI
        =       "TalkReply", 0          ; Name of 8th SWI
        =       0                       ; End of decoding table
        ALIGN

; Handle the control front-end SWI

        ;   Parameters  : r0    - Operation to perform:
        ;                           0   Suspend full screen mode
        ;                           1   Freeze running in a window
        ;                           2   Reset the PC
        ;                           3   Quit the front-end
        ;   Returns     : None
        ;   Description : Control the PC front-end.
control_pc
        LocalLabels
        JSR     "r0, r1"                ; Stack registers
        MOV     r1, r0                  ; Copy required action
        ADR     r0, ws_control_cb_block ; Pointer to callback descriptor
        STR     r1, [r0, #callback_r0]  ; Store required action
        ADR     r1, callback$l          ; Address of handler function
        STR     r1, [r0, #callback_fn]  ; Store pointer to function
        STR     r12, [r0, #callback_r12]; Store private word value
        SWI     XPC_SetCallback         ; Request a callback when paged in
        RTS                             ; Return from subroutine

        ; The callback handler
callback$l
        JSR     ""                      ; Stack registers
        CMP     r0, #(jump_end$l - jump$l) >> 2; Is it in range
        ADDLO   pc, pc, r0, LSL#2       ; Dispatch if in range
        B       unknown$l               ; Unknown operation
jump$l  B       suspend$l               ; Jump to handler for 0th code
        B       freeze$l                ; Jump to handler for 1st code
        B       reset$l                 ; Jump to handler for 2nd code
        B       quit$l                  ; Jump to handler for 3rd code
jump_end$l

        ; Ignore unknown codes
unknown$l
        RTS                             ; Return from subroutine

        ; Suspend full screen mode
suspend$l
        BL      suspend_pc              ; Suspend the PC
        RTS                             ; Return from subroutine

        ; Freeze running in a window
freeze$l
        BL      suspend_pc              ; Suspend the PC
        BL      freeze_pc               ; Freeze the PC
        RTS                             ; Return from subroutine

        ; Reset the PC
reset$l BL      reset_pc                ; Reset the PC front-end
        RTS                             ; Return from subroutine

        ; Quit the front-end
quit$l  BL      suspend_pc              ; Suspend the PC
        BL      quit_pc                 ; Request that the PC quits
        RTS                             ; Return from subroutine

; Handle messages file

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Open the messages file.
open_messages
        LocalLabels
        JSR     "r0-r4"                 ; Stack registers
        MOV     r0, #0                  ; Null pointer
        STR     r0, ws_message_ptr      ; Reset message block pointer
        ADR     r0, var$l               ; Pointer to variable name
        MOV     r2, #1:SHL:31           ; Check for existence
        MOV     r3, #0                  ; First call
        MOV     r4, #0                  ; Don't want expansion
        SWI     XOS_ReadVarVal          ; Check if variable exists
        TEQ     r2, #0                  ; Does it exist (ignore any error)
        BNE     exist$l                 ; Skip next section if it exists
        ADR     r0, var$l               ; Pointer to variable name
        ADR     r1, def$l               ; Pointer to variable value
        MOV     r2, #?def$l - 1         ; Length of string
        MOV     r3, #0                  ; First call
        MOV     r4, #OS_VartypeLiteralString; A simple string
        SWI     XOS_SetVarVal           ; Set the variable
        RTE VS                          ; Exit without wiping over r0
exist$l ADR     r1, file$l              ; Pointer to filename
        SWI     XMessageTrans_FileInfo  ; Get information about file
        RTE VS                          ; Exit without wiping over r0
        TST     r0, #1                  ; Is file held in memory
        BNE     mem$l                   ; No need to allocate memory if it is
        MOV     r0, #OSModule_Alloc     ; Claim entry code
        MOV     r3, r2                  ; Required size
        SWI     XOS_Module              ; Claim workspace
        RTE VS                          ; Exit without wiping over r0
        STR     r2, ws_message_ptr      ; Store memory pointer
mem$l   ADR     r0, ws_message          ; File descriptor
        ADR     r1, file$l              ; Pointer to filename
        LDR     r2, ws_message_ptr      ; Pointer to buffer
        SWI     XMessageTrans_OpenFile  ; Open the messages file
        RTE VS                          ; Exit without wiping over r0
        RTS                             ; Return from subroutine

        ; Details of how to access messages file
def$l   =       "Resources:$.":CC:resource_directory:CC:".", 0; Default path
var$l   =       "ARMEdit$Path", 0       ; System variable to use
file$l  =       "ARMEdit:Messages", 0   ; Name to access messages file as
        ALIGN

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Close the messages file, and free any memory used.
close_messages
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        ADR     r0, ws_message          ; File descriptor
        SWI     XMessageTrans_CloseFile ; Close the messages file
        LDR     r2, ws_message_ptr      ; Pointer to workspace
        TEQ     r2, #0                  ; Was any workspace used
        RTS EQ                          ; Exit if no workspace used
        MOV     r0, #OSModule_Free      ; Free entry code
        SWI     XOS_Module              ; Release workspace
        RTSS                            ; Return from subroutine

; Low level interface to the PC front-end

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Initialise the status of the PC when the module
        ;                 starts.
initialise_pc
        LocalLabels
        JSR     "r0"                    ; Stack registers
        MOV     r0, #0                  ; Value to clear flags with
        STR     r0, ws_fe_state         ; No pointer to front-end state
        STR     r0, ws_hdr_state        ; No pointer to hardware state
        STR     r0, ws_fe_ver           ; No current front-end version
        STR     r0, ws_hdr_ver          ; No current hardware version
        STR     r0, ws_pc_active        ; The PC front-end is not active
        STR     r0, ws_hdr_current_ver  ; Hardware structure version number
        STR     r0, ws_hdr_min_compat_ver; Hardware structure version number
        STR     r0, ws_fe_current_ver   ; Front-end structure version number
        STR     r0, ws_fe_min_compat_ver; Front-end structure version number
        STR     r0, ws_control_cb_block + callback_tag; Reset callback flag
        STR     r0, ws_activity_fore    ; Clear foreground poll rate
        STR     r0, ws_activity_back    ; Clear background poll rate
        STR     r0, ws_activity_last    ; Clear last activity timeout value
        SWI     XOS_ReadMonotonicTime   ; Read the current time
        STR     r0, ws_activity_delay   ; Resume multitasking immediately
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Number of event to add handler for.
        ;                 r1    - Pointer to the event handler function.
        ;                 r2    - Pointer to the event handler record to use.
        ;   Returns     : None
        ;   Description : Add a handler for the specified event, using the
        ;                 supplied event handler record.
add_event_handler
        LocalLabels
        JSR     "r1, r3-r4"             ; Stack registers
        LDR     r3, ws_hdr_ver          ; Get hardware structure version index
        SUBS    r3, r3, #1              ; Remove offset
        RTSS MI                         ; Exit if unsuitable
        CMP     r3, #(table_end$l - table$l) >> 2; Is it in range
        RTSS HS                         ; Exit if unsuitable
        ADR     r4, table$l             ; Pointer to start of table
        LDR     r4, [r4, r3, LSL#2]     ; Get offset to event lists
        LDR     r3, ws_hdr_state        ; Get pointer to hardware state
        ADD     r3, r3, r4              ; Pointer to event lists
        STR     r1, [r2, #call_evt_list_fn]; Store pointer to handler function
        LDR     r1, [r3, r0, ASL#2]     ; Get pointer to current head of list
        STR     r1, [r2, #call_evt_list_next]; Store pointer to previous head
        STR     r12, [r2, #call_evt_list_r12]; Store private word value
        STR     r2, [r3, r0, ASL#2]     ; Store pointer to this record
        RTSS                            ; Return from subroutine

        ; Offsets to event lists for different hardware structure versions
table$l &       hdr1_event_list         ; Offset for 1st hardware version
        &       hdr2_event_list         ; Offset for 2nd hardware version
table_end$l

        ;   Parameters  : r0    - Pointer to the unknown configuration handler
        ;                         function.
        ;                 r1    - Pointer to the unknown configuration handler
        ;                         record to use.
        ;   Returns     : None
        ;   Description : Add a handler for unknown configuration entries,
        ;                 using the supplied configuration hander record.
add_config_handler
        LocalLabels
        JSR     "r0, r2-r3"             ; Stack registers
        LDR     r2, ws_hdr_ver          ; Get hardware structure version index
        SUBS    r2, r2, #1              ; Remove offset
        RTSS MI                         ; Exit if unsuitable
        CMP     r2, #(table_end$l - table$l) >> 2; Is it in range
        RTSS HS                         ; Exit if unsuitable
        ADR     r3, table$l             ; Pointer to start of table
        LDR     r3, [r3, r2, LSL#2]     ; Get offset to configuration list
        LDR     r2, ws_hdr_state        ; Get pointer to hardware state
        STR     r0, [r1, #call_cfg_list_fn]; Store pointer to handler function
        LDR     r0, [r2, r3]            ; Get pointer to current head of list
        STR     r0, [r1, #call_cfg_list_next]; Store pointer to previous head
        STR     r12, [r1, #call_cfg_list_r12]; Store private word value
        STR     r1, [r2, r3]            ; Store pointer to this record
        RTSS                            ; Return from subroutine

        ; Offsets to event lists for different hardware structure versions
table$l &       hdr1_configure_list     ; Offset for 1st hardware version
        &       hdr2_configure_list     ; Offset for 2nd hardware version
table_end$l

        ;   Parameters  : r0    - Address of port to add handler for.
        ;                 r1    - Pointer to function to handle byte reads.
        ;                 r2    - Pointer to function to handle word reads.
        ;                 r3    - Pointer to function to handle byte writes.
        ;                 r4    - Pointer to function to handle word writes.
        ;   Returns     : None
        ;   Description : Set the functions to handler port reads and writes.
add_io_handler
        LocalLabels
        JSR     "r0, r5-r6"             ; Stack registers
        LDR     r5, = io_port_mask      ; Mask for port address
        AND     r0, r0, r5              ; Get port address in range
        ASSERT  hdrstate_io_spacing = 4 ; Check that the spacing is correct
        MOV     r0, r0, ASR#2           ; Convert to an array index
        MOV     r5, #handler            ; Size of each array entry
        MUL     r0, r5, r0              ; Offset into array
        LDR     r5, ws_hdr_ver          ; Get hardware structure version index
        SUBS    r5, r5, #1              ; Remove offset
        RTSS MI                         ; Exit if unsuitable
        CMP     r5, #(table_end$l - table$l) >> 2; Is it in range
        RTSS HS                         ; Exit if unsuitable
        ADR     r6, table$l             ; Pointer to start of table
        LDR     r6, [r6, r5, LSL#2]     ; Get offset to I/O handlers array
        LDR     r5, ws_hdr_state        ; Get pointer to hardware state
        ADD     r5, r5, r6              ; Pointer to array of I/O handlers
        ADD     r0, r5, r0              ; Pointer to handler entry to modify
        STR     r1, [r0, #handler_read8]; Store pointer to byte read handler
        STR     r2, [r0, #handler_read16]; Store pointer to word read handler
        STR     r3, [r0, #handler_write8]; Store pointer to byte write handler
        STR     r4, [r0, #handler_write16]; Store pointer to word write handler
        STR     r12, [r0, #handler_r12] ; Store private word value
        RTSS                            ; Return from subroutine

        ; Offsets to I/O handler array for different hardware structure versions
table$l &       hdr1_io_handlers        ; Offset for 1st hardware version
        &       hdr2_io_handlers        ; Offset for 2nd hardware version
table_end$l

        ;   Parameters  : r0    - HPC service identifier to add handler for.
        ;                 r1    - Pointer to the HPC handler function.
        ;                 r2    - Pointer to the HPC handler record to use.
        ;   Returns     : None
        ;   Description : Add a handler for the specified HPC ID, using the
        ;                 supplied HPC handler record.
add_hpc_handler
        LocalLabels
        JSR     "r0-r1"                 ; Stack registers
        STR     r0, [r2, #hpc_handler_id]; Store HPC service identifier
        STR     r1, [r2, #hpc_handler_fn]; Store pointer to HPC handler
        MOV     r1, #0                  ; Flags are reserved
        STR     r1, [r2, #hpc_handler_flags]; Store flags
        STR     r1, [r2, #hpc_handler_request]; Clear request flag
        STR     r12, [r2, #hpc_handler_r12]; Store private word value
        HPC_Hash r0                     ; Convert identifier to array index
        LDR     r1, ws_hdr_ver          ; Get hardware structure version index
        SUBS    r1, r1, #2              ; Remove offset
        RTSS MI                         ; Exit if unsuitable
        CMP     r1, #(table_end$l - table$l) >> 2; Is it in range
        RTSS HS                         ; Exit if unsuitable
        ADR     r3, table$l             ; Pointer to start of table
        LDR     r3, [r3, r1, LSL#2]     ; Get offset to HPC handlers array
        LDR     r1, ws_hdr_state        ; Get pointer to hardware state
        ADD     r1, r1, r3              ; Pointer to array of HPC handlers
        ADD     r0, r1, r0, LSL#2       ; Pointer to correct array entry
        LDR     r1, [r0]                ; Get pointer to current head of list
        STR     r1, [r2, #hpc_handler_next]; Store pointer to previous head
        STR     r2, [r0]                ; Make this record new head of list
        RTSS                            ; Return from subroutine

        ; Offsets to HPC handler array for different hardware structure versions
table$l &       hdr2_hpc_handlers       ; Offset for 2nd hardware version
table_end$l

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Request that the PC is suspended.
suspend_pc
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        LDR     r0, ws_fe_ver           ; Get front-end structure version index
        SUBS    r0, r0, #1              ; Remove offset
        RTSS MI                         ; Exit if unsuitable
        CMP     r0, #(table_end$l - table$l) >> 2; Is it in range
        RTSS HS                         ; Exit if unsuitable
        ADR     r1, table$l             ; Pointer to start of table
        LDR     r1, [r1, r0, LSL#2]     ; Get offset to suspend flag
        LDR     r0, ws_fe_state         ; Get pointer to front-end state
        MOV     r2, #1                  ; Value to set flag to
        STR     r2, [r0, r1]            ; Set flag as required
        RTSS                            ; Return from subroutine

        ; Offsets to suspend flag for different front-end structure versions
table$l &       fe1_suspend_request     ; Offset for 1st front-end version
        &       fe2_suspend_request     ; Offset for 2nd front-end version
        &       fe3_suspend_request     ; Offset for 3rd front-end version
table_end$l

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Request that the PC frozen in a window.
freeze_pc
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        LDR     r0, ws_fe_ver           ; Get front-end structure version index
        SUBS    r0, r0, #2              ; Remove offset
        RTSS MI                         ; Exit if unsuitable
        CMP     r0, #(table_end$l - table$l) >> 2; Is it in range
        RTSS HS                         ; Exit if unsuitable
        ADR     r1, table$l             ; Pointer to start of table
        LDR     r1, [r1, r0, LSL#2]     ; Get offset to freeze flag
        LDR     r0, ws_fe_state         ; Get pointer to front-end state
        MOV     r2, #1                  ; Value to set flag to
        STR     r2, [r0, r1]            ; Set flag as required
        RTSS                            ; Return from subroutine

        ; Offsets to freeze flag for different front-end structure versions
table$l &       fe2_freeze_request      ; Offset for 2nd front-end version
        &       fe3_freeze_request      ; Offset for 3rd front-end version
table_end$l

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Request that the PC is reset next time is resumes
        ;                 execution.
reset_pc
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        LDR     r0, ws_fe_ver           ; Get front-end structure version index
        SUBS    r0, r0, #1              ; Remove offset
        RTSS MI                         ; Exit if unsuitable
        CMP     r0, #(table_end$l - table$l) >> 2; Is it in range
        RTSS HS                         ; Exit if unsuitable
        ADR     r1, table$l             ; Pointer to start of table
        LDR     r1, [r1, r0, LSL#2]     ; Get offset to reset flag
        LDR     r0, ws_fe_state         ; Get pointer to front-end state
        MOV     r2, #1                  ; Value to set flag to
        STR     r2, [r0, r1]            ; Set flag as required
        RTSS                            ; Return from subroutine

        ; Offsets to reset flag for different front-end structure versions
table$l &       fe1_reset_request       ; Offset for 1st front-end version
        &       fe2_reset_request       ; Offset for 2nd front-end version
        &       fe3_reset_request       ; Offset for 3rd front-end version
table_end$l

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Request that the front-end quits.
quit_pc
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        LDR     r0, ws_fe_ver           ; Get front-end structure version index
        SUBS    r0, r0, #1              ; Remove offset
        RTSS MI                         ; Exit if unsuitable
        CMP     r0, #(table_end$l - table$l) >> 2; Is it in range
        RTSS HS                         ; Exit if unsuitable
        ADR     r1, table$l             ; Pointer to start of table
        LDR     r1, [r1, r0, LSL#2]     ; Get offset to quit flag
        LDR     r0, ws_fe_state         ; Get pointer to front-end state
        MOV     r2, #1                  ; Value to set flag to
        STR     r2, [r0, r1]            ; Set flag as required
        RTSS                            ; Return from subroutine

        ; Offsets to quit flag for different front-end structure versions
table$l &       fe1_quit_request        ; Offset for 1st front-end version
        &       fe2_quit_request        ; Offset for 2nd front-end version
        &       fe3_quit_request        ; Offset for 3rd front-end version
table_end$l

; A literal pool

        LTORG

; Unknown configuration handlers

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Clear any previously processed configuration lines.
config_reset
        LocalLabels
        JSR     "r0"                    ; Stack registers
        MOV     r0, #0                  ; String terminator
        STRB    r0, ws_device_prefix    ; Clear the device configuration prefix
        STRB    r0, ws_device_suffix    ; Clear the device configuration suffix
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to string buffer containing a line
        ;                         from the configuration file. This is null
        ;                         terminated, but may have a CR and/or LF at
        ;                         the end, or end in a comment (with a #
        ;                         character).
        ;   Returns     : r0    - 1 if the line was processed, or 0 if not
        ;                         recognised.
        ;   Description : Handle lines from the PC configuration file.
config_handler
        LocalLabels
        JSR     "r4"                    ; Stack registers
        ADRL    r1, ws_buffer           ; Pointer to destination buffer
        ADD     r2, r1, #String         ; Pointer to end of destination buffer
copy$l  LDRB    r3, [r0], #1            ; Read source character
        STRB    r3, [r1], #1            ; Store in destination buffer
        TEQ     r3, #0                  ; Is it a null terminator
        TEQNE   r3, #10                 ; Is it a newline
        TEQNE   r3, #13                 ; Is it a carriage return
        TEQNE   r3, #'#'                ; Is it a comment
        TEQNE   r1, r2                  ; Has the buffer been filled
        BNE     copy$l                  ; Keep looping until finished
        SUB     r1, r1, #1              ; Back to the terminator
        ADRL    r2, ws_buffer           ; Pointer to start of buffer
end$l   TEQ     r1, r2                  ; Has the start been reached
        BEQ     fail$l                  ; No line to process if it has
        LDRB    r0, [r1, #-1]!          ; Read the next character
        TEQ     r0, #' '                ; Is it a space
        BEQ     end$l                   ; Keep looping while spaces found
        MOV     r0, #0                  ; Null terminator
        STRB    r0, [r1, #1]            ; Terminate the string
start$l LDRB    r0, [r2], #1            ; Read character from the buffer
        TEQ     r0, #' '                ; Is it a space
        BEQ     start$l                 ; Skip over all spaces
        SUB     r2, r2, #1              ; Pointer to start of keyword
        MOV     r1, r2                  ; Copy pointer to keyword
key$l   LDRB    r0, [r1]                ; Read character from keyword
        Upper   r0                      ; Convert to upper case
        STRB    r0, [r1], #1            ; Store modified character
        TEQ     r0, #0                  ; Is it a null terminator
        TEQNE   r0, #' '                ; Is it a space character
        BNE     key$l                   ; Keep looping until keyword processed
        SUB     r1, r1, #1              ; Back to the terminator
        TEQ     r0, #0                  ; Has the terminator been reached
        BEQ     done$l                  ; No further processing if it has
        MOV     r0, #0                  ; String terminator
        STRB    r0, [r1], #1            ; Terminate the keyword
arg$l   LDRB    r0, [r1], #1            ; Read next character of arguments
        TEQ     r0, #' '                ; Is it a space
        BEQ     arg$l                   ; Keep looping while spaces found
        SUB     r1, r1, #1              ; Back to the first non-space character
done$l  MOV     r0, r2                  ; Copy pointer to keyword
        ADR     r2, prefix$l            ; Pointer to prefix string
pref$l  LDRB    r3, [r2], #1            ; Read character from prefix
        TEQ     r3, #0                  ; Is it the terminator
        BEQ     ok$l                    ; Keyword matches if it is
        LDRB    r4, [r0], #1            ; Read character from keyword
        TEQ     r3, r4                  ; Does the keyword match the prefix
        BEQ     pref$l                  ; Loop until finished or failed
fail$l  MOV     r0, #0                  ; Mark the line as unrecognised
        RTSS                            ; Return from subroutine
ok$l    ; r0 = pointer to keyword (after prefix)
        ; r1 = pointer to arguments
        MOV     r0, #1                  ; Claim the line if recognised
        RTSS                            ; Return from subroutine
spc$l   JSR     "r1"                    ; Stack registers
spcl$l  LDRB    r1, [r0]                ; Read the next character
        TEQ     r1, #' '                ; Is it a space
        ADDEQ   r0, r0, #1              ; Skip this character if it is
        BEQ     spcl$l                  ; Keep looping while spaces
        RTSS                            ; Return from subroutine

        ; Standard prefix string
prefix$l
        =       "ARMEDIT-", 0
        ALIGN

        [       {FALSE}

        ; Supported keywords
keys$l  &       device_prefix$l
        =       "DEVICEPREFIX", 0
        ALIGN
        &       device_suffix$l
        =       "DEVICESUFFIX", 0
        ALIGN
        &       0                       ; End of list marker

        ; ARMEditDevicePrefix
device_prefix$l
        JSR     "r1"                    ; Stack registers
        ADR     r1, ws_device_prefix    ; Pointer to destination buffer
        BL      copy_args$l             ; Copy the value
        RTSS                            ; Return from subroutine

        ; ARMEditDeviceSuffix
device_suffix$l
        JSR     "r1"                    ; Stack registers
        ADR     r1, ws_device_suffix    ; Pointer to destination buffer
        BL      copy_args$l             ; Copy the value
        RTSS                            ; Return from subroutine

        ; Copy configuration arguments
copy_args$l
        JSR     "r0-r2"                 ; Stack registers
copy_argsl$l
        LDRB    r2, [r0], #1            ; Read source character
        STRB    r2, [r1], #1            ; Write destination character
        TEQ     r2, #0                  ; Has the terminator been reached
        BNE     copy$l                  ; Loop while not finished
        RTSS                            ; Return from subroutine

        ]

; PC event handlers

        ;   Parameters  : None
        ;   Returns     : r0    - Always 0.
        ;   Description : Set the selected configuration after all lines have
        ;                 been processed.
event_config_handler
        LocalLabels
        JSR     ""                      ; Stack registers
        MOV     r0, #0                  ; r0 must be zero on exit
        RTSS                            ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : r0    - Always 0.
        ;   Description : Clear flags when the PC front-end quits.
event_shutdown_handler
        LocalLabels
        JSR     ""                      ; Stack registers
        BL      memory_reset            ; Ensure all memory released
        BL      file_reset              ; Ensure all files closed
        BL      device_reset            ; Ensure device handling is reset
        BL      talk_reset              ; Ensure PC clients are deregistered
        BL      oscli_reset             ; Ensure commands are terminated
        MOV     r0, #1                  ; Code for shutdown
        BL      talk_armedit            ; Broadcast message
        MOV     r0, #0                  ; Value to clear flags with
        STR     r0, ws_fe_state         ; No pointer to front-end state
        STR     r0, ws_hdr_state        ; No pointer to hardware state
        STR     r0, ws_fe_ver           ; No current front-end version
        STR     r0, ws_hdr_ver          ; No current hardware version
        STR     r0, ws_pc_active        ; The PC front-end is not active
        MOV     r0, #0                  ; r0 must be zero on exit
        RTSS                            ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : r0    - Always 0.
        ;   Description : Reset state when PC is reset.
event_reset_handler
        LocalLabels
        JSR     ""                      ; Stack registers
        BL      date_initialise         ; Reset time zone details
        BL      memory_reset            ; Ensure all memory released
        BL      file_reset              ; Ensure all files closed
        BL      device_reset            ; Ensure device handling is reset
        BL      talk_reset              ; Ensure PC clients are deregistered
        BL      oscli_reset             ; Ensure commands are terminated
        MOV     r0, #0                  ; Code for reset
        BL      talk_armedit            ; Broadcast message
        MOV     r0, #0                  ; Value to reset position with
        STR     r0, ws_emulate_ptr      ; Reset packet data pointer position
        MOV     r0, #emulate_status_idle; Default state of emulation
        STR     r0, ws_emulate_status   ; Reset emulation status
        MOV     r0, #0                  ; r0 must be zero on exit
        RTSS                            ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : r0    - Always 0.
        ;   Description : Called at regular intervals (typically every 20-30ms)
        ;                 while the emulation is running.
event_poll_handler
        LocalLabels
        JSR     ""                      ; Stack registers
        LDR     r0, ws_fe_ver           ; Get front-end structure version index
        SUBS    r0, r0, #1              ; Remove offset
        RTSS MI                         ; Exit if unsuitable
        CMP     r0, #(table_end$l - table$l) >> 4; Is it in range
        RTSS HS                         ; Exit if unsuitable
        ADR     r1, table$l             ; Pointer to start of table
        ADD     r1, r1, r0, LSL#4       ; Pointer to table entries
        LDR     r0, ws_fe_state         ; Get pointer to front-end state
        LDR     r2, [r1], #4            ; Get offset to windowed flag
        LDR     r2, [r0, r2]            ; Get multitasking mode flag
        TEQ     r2, #0                  ; Is the PC multitasking
        BEQ     done$l                  ; Exit if not
        LDR     r2, [r1], #4            ; Get offset to has input focus flag
        LDR     r2, [r0, r2]            ; Get input focus flag
        TEQ     r2, #0                  ; Has the PC got the input focus
        LDREQ   r2, ws_activity_back    ; Activity value for background
        LDRNE   r2, ws_activity_fore    ; Activity value for foreground
        LDR     r1, [r1]                ; Get offset to activity timeout
        ADD     r0, r0, r1              ; Pointer to activity timeout
        LDR     r1, [r0]                ; Get current activity timeout
        CMP     r2, #1                  ; Is the reset value sensible
        BLT     skip$l                  ; Skip the next bit if not
        CMP     r1, r2                  ; Is the current value higher
        MOVGT   r1, r2                  ; Clip the value if required
        BGT     skip$l                  ; Skip the next bit if so
        LDR     r3, ws_activity_last    ; Get the last activity timeout value
        CMP     r3, #1                  ; Was last value a final one
        BGT     skip$l                  ; Skip the next bit if not
        CMP     r1, r3                  ; Has the value been reduced
        BLT     skip$l                  ; Skip the next bit if it has
        MOV     r1, r2                  ; Increase the current value
skip$l  BL      faster$l                ; Check faster control
        STR     r1, [r0]                ; Store updated activity timeout
        STR     r1, ws_activity_last    ; Store for next comparison
done$l  MOV     r0, #0                  ; r0 must be zero on exit
        RTSS                            ; Return from subroutine

faster$l
        JSR     "r0, r2"                ; Stack registers
        LDR     r2, ws_activity_delay   ; Get multitasking time
        SWI     XOS_ReadMonotonicTime   ; Read the current time
        RTSS VS                         ; Exit if error produced
        CMP     r0, r2                  ; Has the time been reached
        STRPL   r0, ws_activity_delay   ; Store new time if it has
        RTSS PL                         ; Exit if no action
        MOV     r0, #2                 ; Value to override with
        CMP     r0, r1                  ; Is it larger
        MOVGT   r1, r0                  ; Use if it is
        RTSS                            ; Return from subroutine

        ; Offsets for different front-end structure versions
table$l &       fe1_win_running         ; Offsets for 1st front-end version
        &       fe1_has_input_focus
        &       fe1_activity
        &       0
        &       fe2_win_running         ; Offsets for 2nd front-end version
        &       fe2_has_input_focus
        &       fe2_activity
        &       0
        &       fe3_win_running         ; Offsets for 3rd front-end version
        &       fe3_has_input_focus
        &       fe3_activity
        &       0
table_end$l

; PC I/O port handlers

        ;   Parameters  : r0    - Port from which the PC is making a byte read.
        ;   Returns     : r0    - Bits 0 to 7 are passed back to the PC.
        ;   Description : Handler for byte reads from an I/O port.
port_emulate_read8_handler
        LocalLabels
        JSR     "r1"                    ; Stack registers
        LDR     r1, = io_port_mask      ; Mask for port address
        AND     r0, r0, r1              ; Get port address in range
        TEQ     r0, #emulate_port_status; Is it the status port
        BEQ     status$l                ; Jump to handler if it is
        LDR     r1, = emulate_port_data ; Address of the data port
        TEQ     r0, r1                  ; Is it the data port
        BEQ     data$l                  ; Jump to handler if it is
        MOV     r0, #0                  ; Return zero for other ports
        RTSS                            ; Return from subroutine

        ; Handle byte status reads
status$l
        BL      emulate_hpc_status      ; Jump to status read routine
        RTSS                            ; Return from subroutine

        ; Handle byte data reads
data$l  BL      emulate_hpc_read        ; Jump to data read routine
        RTSS

        ;   Parameters  : r0    - Port from which the PC is making a word read.
        ;   Returns     : r0    - Bits 0 to 15 are passed back to the PC.
        ;   Description : Handler for word reads from an I/O port.
port_emulate_read16_handler
        LocalLabels
        JSR     "r1"                    ; Stack registers
        LDR     r1, = io_port_mask      ; Mask for port address
        AND     r0, r0, r1              ; Get port address in range
        TEQ     r0, #emulate_port_status; Is it the status port
        BEQ     status$l                ; Jump to handler if it is
        LDR     r1, = emulate_port_data ; Address of the data port
        TEQ     r0, r1                  ; Is it the data port
        BEQ     data$l                  ; Jump to handler if it is
        MOV     r0, #0                  ; Return zero for other ports
        RTSS                            ; Return from subroutine

        ; Handle word status reads
status$l
        BL      emulate_hpc_status      ; Jump to status read routine
        RTSS                            ; Return from subroutine

        ; Handle word data reads
data$l  BL      emulate_hpc_read        ; Jump to data read routine
        MOV     r1, r0                  ; Copy least significant byte
        BL      emulate_hpc_read        ; Jump to data read routine
        ORR     r0, r1, r0, LSL#8       ; Combine the two bytes
        RTSS

        ;   Parameters  : r0    - Port to which the PC has written a byte.
        ;                 r1    - Bits 0 to 7 are data written by the PC.
        ;   Returns     : None
        ;   Description : Handler for byte writes to an I/O port.
port_emulate_write8_handler
        LocalLabels
        JSR     "r0, r2"                ; Stack registers
        LDR     r2, = io_port_mask      ; Mask for port address
        AND     r0, r0, r2              ; Get port address in range
        TEQ     r0, #emulate_port_cmd   ; Is it the command port
        BEQ     cmd$l                   ; Jump to handler if it
        LDR     r2, = emulate_port_data ; Address of the data port
        TEQ     r0, r2                  ; Is it the data port
        BEQ     data$l                  ; Jump to handler if it
        RTSS                            ; Return from subroutine

        ; Handle byte command writes
cmd$l   MOV     r0, r1                  ; Copy command
        BL      emulate_hpc_command     ; Jump to command write routine
        RTSS                            ; Return from subroutine

        ; Handle byte data writes
data$l  MOV     r0, r1                  ; Copy the value
        BL      emulate_hpc_write       ; Jump to data write routine
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Port to which the PC has written a word.
        ;                 r1    - Bits 0 to 15 are data written by the PC.
        ;   Returns     : None
        ;   Description : Handler for word writes to an I/O port.
port_emulate_write16_handler
        LocalLabels
        JSR     "r0, r2"                ; Stack registers
        LDR     r2, = io_port_mask      ; Mask for port address
        AND     r0, r0, r2              ; Get port address in range
        TEQ     r0, #emulate_port_cmd   ; Is it the command port
        BEQ     cmd$l                   ; Jump to handler if it
        LDR     r2, = emulate_port_data ; Address of the data port
        TEQ     r0, r2                  ; Is it the data port
        BEQ     data$l                  ; Jump to handler if it
        RTSS                            ; Return from subroutine

        ; Handle word command writes
cmd$l   MOV     r0, r1                  ; Copy command
        BL      emulate_hpc_command     ; Jump to command write routine
        RTSS                            ; Return from subroutine

        ; Handle word data writes
data$l  AND     r0, r1, #&ff            ; Copy least significant byte
        BL      emulate_hpc_write       ; Jump to data write routine
        MOV     r0, r1, LSR#8           ; Copy most significant byte
        BL      emulate_hpc_write       ; Jump to data write routine
        RTSS                            ; Return from subroutine

; A literal pool

        LTORG

; Emulated HPC handling

        ;   Parameters  : None
        ;   Returns     : r0    - The current status of the emulation.
        ;   Description : Check the current state of the HPC emulation.
emulate_hpc_status
        LocalLabels
        JSR     ""                      ; Stack registers
        LDR     r0, ws_emulate_status   ; Get the current state of emulation
        TEQ     r0, #emulate_status_idle; Is the emulation idle
        LDREQ   r0, = emulate_return_emulate; Code for emulation available
        LDRNE   r0, = emulate_return_busy; Otherwise it is busy
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - The command to process.
        ;   Returns     : None
        ;   Description : Process an HPC emulation command.
emulate_hpc_command
        LocalLabels
        JSR     "r0"                    ; Stack registers
        TEQ     r0, #emulate_code_tx    ; Is it the command to start receive
        BEQ     tx$l                    ; Jump to start reception code
        TEQ     r0, #emulate_code_process; Is it the command to do HPC call
        BEQ     process$l               ; Jump to process HPC call code
        TEQ     r0, #emulate_code_rx    ; Is it the command to start transmit
        BEQ     rx$l                    ; Jump to start transmission code
        TEQ     r0, #emulate_code_release; Is it the command to end HPC call
        BEQ     release$l               ; Jump to reset state code
        RTSS                            ; Return from subroutine

        ; Start receiving an HPC packet
tx$l    LDR     r0, ws_emulate_status   ; Get the current emulation status
        TEQ     r0, #emulate_status_idle; Is the emulation idle
        BNE     error$l                 ; Enter error state if not
        MOV     r0, #0                  ; Value to reset position with
        STR     r0, ws_emulate_ptr      ; Reset packet data pointer position
        MOV     r0, #emulate_status_rx  ; Value to indicate receive state
        STR     r0, ws_emulate_status   ; Store the new status
        RTSS                            ; Return from subroutine

        ; Process the packet just received
process$l
        LDR     r0, ws_emulate_status   ; Get the current emulation status
        TEQ     r0, #emulate_status_rx  ; Was the emulation receiving
        BNE     error$l                 ; Enter error state if not
        LDR     r1, ws_emulate_ptr      ; Get current buffer pointer
        CMP     r1, #2                  ; Has the minimum message been received
        BLS     error$l                 ; Enter error state if not
        MOV     r0, #emulate_status_proc; Value to indicate processing state
        STR     r0, ws_emulate_status   ; Store the new status
        ADRL    r0, ws_message_buffer   ; Get pointer to message buffer
        BL      hpc_packet_handler      ; Handle this HPC data packet
        RTSS                            ; Return from subroutine

        ; Start transmitting the reply to a message
rx$l    LDR     r0, ws_emulate_status   ; Get the current emulation status
        TEQ     r0, #emulate_status_proc; Has command been processed
        BNE     error$l                 ; Enter error state if not
        MOV     r0, #0                  ; Value to reset position with
        STR     r0, ws_emulate_ptr      ; Reset packet data pointer position
        MOV     r0, #emulate_status_tx  ; Value to indicate transmit state
        STR     r0, ws_emulate_status   ; Store the new status
        RTSS                            ; Return from subroutine

        ; Reset the emulation after an HPC call is complete
release$l
        MOV     r0, #0                  ; Value to reset position with
        STR     r0, ws_emulate_ptr      ; Reset packet data pointer position
        MOV     r0, #emulate_status_idle; Value to indicate idle state
        STR     r0, ws_emulate_status   ; Store the new status
        RTSS                            ; Return from subroutine

        ; Place the emulation in error state and return
error$l MOV     r0, #emualte_status_err ; Value to indicate error state
        STR     r0, ws_emulate_status   ; Store the new status
        RTSS                            ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : r0    - The value read.
        ;   Description : Return the next byte of the reply
emulate_hpc_read
        LocalLabels
        JSR     "r1, r2"                ; Stack registers
        LDR     r1, ws_emulate_status   ; Get the current status
        TEQ     r1, #emulate_status_tx  ; Is emulation transmitting
        BNE     error$l                 ; Enter error state if not
        LDR     r1, ws_emulate_ptr      ; Get current buffer pointer
        CMP     r1, #hpc_packet         ; Compare it to the maximum size
        BHS     error$l                 ; Enter error state if already done
        ADRL    r2, ws_message_buffer   ; Get pointer to message buffer
        LDRB    r0, [r2, r1]            ; Read this byte
        ADD     r1, r1, #1              ; Increment number of bytes transmitted
        STR     r1, ws_emulate_ptr      ; Store new buffer pointer
        RTSS                            ; Return from subroutine

        ; Place the emulation in error state and return
error$l MOV     r0, #emualte_status_err ; Value to indicate error state
        STR     r0, ws_emulate_status   ; Store the new status
        MOV     r0, #0                  ; Dummy value to return
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - The next value to write.
        ;   Returns     : None
        ;   Description : Process the next byte of the received packet.
emulate_hpc_write
        LocalLabels
        JSR     "r1, r2"                ; Stack registers
        LDR     r1, ws_emulate_status   ; Get the current status
        TEQ     r1, #emulate_status_rx  ; Is emulation receiving
        BNE     error$l                 ; Enter error state if not
        LDR     r1, ws_emulate_ptr      ; Get current buffer pointer
        CMP     r1, #hpc_packet         ; Compare it to the maximum size
        BHS     error$l                 ; Enter error state if already full
        ADRL    r2, ws_message_buffer   ; Get pointer to message buffer
        STRB    r0, [r2, r1]            ; Store this byte
        ADD     r1, r1, #1              ; Increment number of bytes received
        STR     r1, ws_emulate_ptr      ; Store new buffer pointer
        RTSS                            ; Return from subroutine

        ; Place the emulation in error state and return
error$l MOV     r1, #emualte_status_err ; Value to indicate error state
        STR     r1, ws_emulate_status   ; Store the new status
        RTSS                            ; Return from subroutine

; A literal pool

        LTORG

; HPC handler

        ;   Parameters  : r0    - Length of first input block.
        ;                 r1    - Pointer to first input block.
        ;                 r2    - Length of second input block.
        ;                 r3    - Pointer to second input block.
        ;                 r4    - Length of first output block.
        ;                 r5    - Pointer to first output block.
        ;                 r6    - Length of second output block.
        ;                 r7    - Pointer to second output block.
        ;   Returns     : None
        ;   Description : Calls an HPC service directly.
hpc_service
        LocalLabels
        JSR     "r0, r1, r8"            ; Stack registers
        pipe_string "HPC in  %0 @ %1, %2 @ %3", r0, r1, r2, r3
        pipe_string "HPC out %0 @ %1, %2 @ %3", r4, r5, r6, r7
        MOV     r8, #0                  ; Value to clear flag with
        STR     r8, ws_dynamic_enable   ; Disable use of dynamic area
        ADRL    r8, ws_message_buffer   ; Get pointer to message buffer
        BL      in$l                    ; Copy the first input block
        MOV     r0, r2                  ; Copy length of second block
        MOV     r1, r3                  ; Copy pointer to second block
        BL      in$l                    ; Copy the second input block
        STR     r8, ws_emulate_ptr      ; Store packet data pointer position
        MOV     r0, #emulate_status_proc; Value to indicate processing state
        STR     r0, ws_emulate_status   ; Store the new status
        ADRL    r0, ws_message_buffer   ; Get pointer to message buffer
        BL      hpc_packet_handler      ; Perform the call
        ADRL    r8, ws_message_buffer   ; Get pointer to message buffer
        MOV     r0, r4                  ; Copy length of first block
        MOV     r1, r5                  ; Copy pointer to first block
        BL      out$l                   ; Copy the first output block
        MOV     r0, r6                  ; Copy length of second block
        MOV     r1, r7                  ; Copy pointer to second block
        BL      out$l                   ; Copy the second output block
        MOV     r0, #0                  ; Value to reset position with
        STR     r0, ws_emulate_ptr      ; Reset packet data pointer position
        MOV     r0, #emulate_status_idle; Default state of emulation
        STR     r0, ws_emulate_status   ; Reset emulation status
        MOV     r0, #1                  ; Value to set flag with
        STR     r0, ws_dynamic_enable   ; Reenable use of dynamic area
        RTSS                            ; Return from subroutine

in$l    JSR     "r0-r2"                 ; Stack registers
in_l$l  SUBS    r0, r0, #1              ; Decrement number of bytes
        RTSS MI                         ; Exit if finished
        LDRB    r2, [r1], #1            ; Read source byte
        STRB    r2, [r8], #1            ; Write destination byte
        B       in_l$l                  ; Loop for the next byte

out$l   JSR     "r0-r2"                 ; Stack registers
out_l$l SUBS    r0, r0, #1              ; Decrement number of bytes
        RTSS MI                         ; Exit if finished
        LDRB    r2, [r8], #1            ; Read source byte
        STRB    r2, [r1], #1            ; Write destination byte
        B       out_l$l                 ; Loop for the next byte

        ;   Parameters  : r0        - Pointer to HPC packet for input and
        ;                             output data.
        ;   Returns     : r4-r11    - Preserved.
        ;   Description : Called when an HPC call is made.
hpc_packet_handler
        LocalLabels
        JSR     "r0-r11"                ; Stack registers
        STR     r0, ws_packet_ptr       ; Store pointer to the HPC packet
        LDR     r1, [r0]                ; Get the first word of the packet
        LDR     r2, = &ffff             ; Load mask for service identifier
        AND     r2, r1, r2              ; Get the HPC service identifier
        LDR     r3, = sys_hpc_armedit   ; This HPC service identifier
        TEQ     r2, r3                  ; Is it the required service
        BNE     unknown$l               ; Return with unknown reason code if not
        MOV     r1, r1, LSR#16          ; Get reason code
        CMP     r1, #(jump_end$l - jump$l) >> 2; Is it in range
        ADDLO   pc, pc, r1, LSL#2       ; Dispatch if in range
        B       unknown$l               ; Unknown reason code
jump$l  B       swi$l                   ; Jump to handler for reason code &00
        B       read$l                  ; Jump to handler for reason code &01
        B       write$l                 ; Jump to handler for reason code &02
        B       alloc$l                 ; Jump to handler for reason code &03
        B       free$l                  ; Jump to handler for reason code &04
        B       exttype$l               ; Jump to handler for reason code &05
        B       typeext$l               ; Jump to handler for reason code &06
        B       fopen$l                 ; Jump to handler for reason code &07
        B       fclose$l                ; Jump to handler for reason code &08
        B       fread$l                 ; Jump to handler for reason code &09
        B       fwrite$l                ; Jump to handler for reason code &0a
        B       talkstart$l             ; Jump to handler for reason code &0b
        B       talkend$l               ; Jump to handler for reason code &0c
        B       talktx$l                ; Jump to handler for reason code &0d
        B       talkrx$l                ; Jump to handler for reason code &0e
        B       dev_init$l              ; Jump to handler for reason code &0f
        B       dev_bpb$l               ; Jump to handler for reason code &10
        B       dev_changed$l           ; Jump to handler for reason code &11
        B       dev_read$l              ; Jump to handler for reason code &12
        B       dev_write$l             ; Jump to handler for reason code &13
        B       date_to_dos$l           ; Jump to handler for reason code &14
        B       date_to_riscos$l        ; Jump to handler for reason code &15
        B       oscli_start$l           ; Jump to handler for reason code &16
        B       oscli_poll$l            ; Jump to handler for reason code &17
        B       oscli_end$l             ; Jump to handler for reason code &18
        B       talkreply$l             ; Jump to handler for reason code &19
        B       faster$l                ; Jump to handler for reason code &1A
        B       temporary$l             ; Jump to handler for reason code &1B
        B       dev_read_long$l         ; Jump to handler for reason code &1C
        B       dev_write_long$l        ; Jump to handler for reason code &1D
        B       dev_open$l              ; Jump to handler for reason code &1E
        B       dev_close$l             ; Jump to handler for reason code &1F
        B       dev_removable$l         ; Jump to handler for reason code &20
jump_end$l

        ; Handle unknown service identifiers of reason codes
unknown$l
        LDR     r0, ws_packet_ptr       ; Get pointer to packet buffer
        LDR     r1, = return_unknown    ; Return code for unknown service
        STR     r1, [r0]                ; Store in first word of block
        RTSS                            ; Return from subroutine

        ; Handler for SWI reason code
swi$l   LDR     r10, [r0, #packet_swi_in_swi]; Read required SWI number
        ORR     r10, r10, #swi_x_bit    ; Ensure it returns an error
        LDR     r2, swi_code$l          ; Get the base SWI instruction
        ORR     r1, r10, r2             ; Combine with SWI number
        STR     r1, ws_swi              ; Store in workspace
        LDR     r1, swi_code$l + 4      ; Get return instruction
        STR     r1, ws_swi + 4          ; Store in workspace
        ADD     r11, r0, #packet_swi_in_regs; Pointer to register dump
        MOV     r0, #0                  ; Reason code to read features
        SWI     XOS_PlatformFeatures    ; Check the host platform features
        BVS     swi_old$l               ; Assume old machine if error produced
        TST     r0, #OS_PlatformCodeNeedsSynchronisation; Self-modifying alright
        BNE     swi_strong_arm$l        ; Special code if not
swi_old$l
        ClearFlags                      ; Ensure error flag is not set
        LDMIA   r11, {r0-r9}            ; Load required register values
        ADR     r10, ws_swi             ; Address to jump to
        MOV     r11, pc                 ; Prepare return address
        MOV     pc, r10                 ; Call the SWI
swi_done$l
        LDR     r10, ws_packet_ptr      ; Get the packet pointer
        ADD     r11, r10, #packet_swi_out_regs; Pointer to register dump
        STMIA   r11, {r0-r9}            ; Store exit registers
        BVS     swi_error$l             ; Handle errors
        MOV     r0, #return_success     ; Reason code for success
        STR     r0, [r10, #packet_swi_out_code]; Set reason code for success
        RTSS                            ; Return from subroutine
swi_error$l
        ADD     r1, r10, #packet_swi_out_err; Pointer to start of output error
        ADD     r2, r1, #OS_Error       ; End of bytes to copy
swi_error_loop$l
        LDRB    r3, [r0], #1            ; Read byte of error block
        STRB    r3, [r1], #1            ; Write byte of error block
        CMP     r1, r2                  ; Is that the end of the block
        BLT     swi_error_loop$l        ; Loop for next byte
        MOV     r0, #return_failure     ; Reason code for error
        STR     r0, [r10, #packet_swi_out_code]; Set reason code for error
        RTSS                            ; Return from subroutine
swi_code$l
        SWI     0                       ; Base SWI instruction
        MOV     pc, r11                 ; Return to module code
swi_strong_arm$l
        LDMIA   r11, {r0-r9}            ; Load required register values
        SWI     XOS_CallASWI            ; Call the SWI
        B       swi_done$l              ; Handle SWI results as before

        ; Handler for memory reads
read$l  LDR     r1, [r0, #packet_mem_in_len]; Read length of data
        LDR     r0, [r0, #packet_mem_in_addr]; Read start address
        ADD     r1, r0, r1              ; End address of data
        SWI     XOS_ValidateAddress     ; Check that the memory is valid
        BCS     mem_fail$l              ; Jump to error handler if required
        LDR     r2, ws_packet_ptr       ; Get the packet pointer
        MOV     r3, #return_success     ; Reason code for success
        STR     r3, [r2, #packet_mem_out_code]; Set reason code for success
        ADD     r2, r2, #packet_mem_out_data; Pointer to output buffer
read_loop$l
        CMP     r0, r1                  ; Is the copy finished
        RTSS HS                         ; Return from subroutine if it is
        LDRB    r3, [r0], #1            ; Read a byte
        STRB    r3, [r2], #1            ; Write the byte
        B       read_loop$l             ; Loop for the next byte

        ; Handler for memory writes
write$l LDR     r1, [r0, #packet_mem_in_len]; Read length of data
        LDR     r0, [r0, #packet_mem_in_addr]; Read start address
        ADD     r1, r0, r1              ; End address of data
        SWI     XOS_ValidateAddress     ; Check that the memory is valid
        BCS     mem_fail$l              ; Jump to error handler if required
        LDR     r2, ws_packet_ptr       ; Get the packet pointer
        MOV     r3, #return_success     ; Reason code for success
        STR     r3, [r2, #packet_mem_in_code]; Set reason code for success
        ADD     r2, r2, #packet_mem_in_data; Pointer to output buffer
write_loop$l
        CMP     r0, r1                  ; Is the copy finished
        RTSS HS                         ; Return from subroutine if it is
        LDRB    r3, [r2], #1            ; Read a byte
        STRB    r3, [r0], #1            ; Write the byte
        B       write_loop$l            ; Loop for the next byte

        ; Handle failed memory operations
mem_fail$l
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_failure     ; Reason code for error
        STR     r1, [r0, #packet_mem_out_code]; Set reason code for error
        RTSS                            ; Return from subroutine

        ; Allocate memory for PC card usage
alloc$l LDR     r3, [r0, #packet_alloc_in_size]; Get required size
        ADD     r3, r3, #memory         ; Add overhead
        MOV     r0, #OSModule_Alloc     ; Reason code to allocate memory
        SWI     XOS_Module              ; Allocate the memory
        BVS     alloc_fail$l            ; Jump to error handler if required
        TEQ     r2, #0                  ; Check that the pointer is valid
        BEQ     alloc_fail$l            ; Handle failure
        LDR     r0, ws_pc_memory        ; Previous first entry
        STR     r2, ws_pc_memory        ; Make this the new first entry
        STR     r0, [r2, #memory_next]  ; Make the original item the next
        SUB     r3, r3, #memory         ; Back to original size
        STR     r3, [r2, #memory_size]  ; Store size of the block
        MOV     r1, #0                  ; Value for null pointer
        STR     r1, [r2, #memory_prev]  ; Clear previous pointer
        TEQ     r0, #0                  ; Is there another block
        STRNE   r2, [r0, #memory_prev]  ; Make this the previous of the next
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        ADD     r2, r2, #memory         ; Skip housekeeping information
        STR     r2, [r0, #packet_alloc_out_ptr]; Store memory pointer
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_alloc_out_code]; Set return code for success
        RTSS                            ; Return from subroutine
alloc_fail$l
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_failure     ; Return code for error
        STR     r1, [r0, #packet_alloc_out_code]; Set return code for error
        RTSS                            ; Return from subroutine

        ; Deallocate memory previously claimed by the PC card
free$l  LDR     r2, [r0, #packet_free_in_ptr]; Get pointer to block
        SUB     r2, r2, #memory         ; Skip housekeeping information
        LDR     r0, [r2, #memory_prev]  ; Get previous allocation pointer
        LDR     r1, [r2, #memory_next]  ; Get next allocation pointer
        TEQ     r0, #0                  ; Is there a previous allocation
        STREQ   r1, ws_pc_memory        ; Make next head of list if not
        STRNE   r1, [r0, #memory_next]  ; Otherwise make it next of previous
        TEQ     r1, #0                  ; Is there a next allocation
        STRNE   r0, [r1, #memory_prev]  ; If so then store previous in next
        MOV     r0, #OSModule_Free      ; Reason code to free memory
        SWI     XOS_Module              ; Free the memory
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_alloc_out_code]; Set return code for success
        RTSS                            ; Return from subroutine

        ; Convert a DOS extension into a RISC OS filetype
exttype$l
        MOV     r1, r0                  ; Copy pointer to packet
        LDR     r0, [r1, #packet_exttype_in_ext]; Get the extension
        BL      dosmap_ext_to_type      ; Perform the conversion
        STR     r0, [r1, #packet_exttype_out_type]; Store the filetype
        MOV     r0, #return_success     ; Return code for success
        STR     r0, [r1, #packet_exttype_out_code]; Set return code for success
        RTSS                            ; Return from subroutine

        ; Convert a RISC OS filetype into an extension
typeext$l
        MOV     r1, r0                  ; Copy pointer to packet
        LDR     r0, [r1, #packet_typeext_in_type]; Get the filetype
        BL      dosmap_type_to_ext      ; Perform the conversion
        STR     r0, [r1, #packet_typeext_out_ext]; Store the extension
        MOV     r0, #return_success     ; Return code for success
        STR     r0, [r1, #packet_typeext_out_code]; Set return code for success
        RTSS                            ; Return from subroutine

        ; Open a RISC OS file
fopen$l ADD     r1, r0, #packet_fopen_in_name; Pointer to filename
        LDR     r5, [r0, #packet_fopen_in_size]; Get initial size requested
        CMP     r5, #-1                 ; Should existing file be opened
        BEQ     fopen_exist$l           ; Open an existing file
        MOV     r0, #OSFile_CreateStamped; Reason code to create stamped file
        LDR     r2, = OSFile_TypeDOS    ; File type for a DOS file
        MOV     r4, #0                  ; Start address of file
        SWI     XOS_File                ; Create the file
        BVS     fopen_fail$l            ; Handle any error produced
        MOV     r0, #OSFind_Openout :OR: OSFind_NoPath; Reason code for new
        B       fopen_open$l
fopen_exist$l
        MOV     r0, #OSFind_Openup \
                     :OR: OSFind_NoPath \
                     :OR: OSFind_ErrorIfAbsent
fopen_open$l
        MOV     r2, #0                  ; No path
        SWI     XOS_Find                ; Open the file
        BVS     fopen_fail$l            ; Handle any error produced
        MOV     r6, r0                  ; Copy file handle
        MOV     r0, #OSModule_Alloc     ; Reason code to allocate memory
        MOV     r3, #files              ; Size of block to claim
        BL      dynamic_osmodule        ; Claim memory for file record
        BVS     fopen_exit$l            ; Bare minimum to do if failed
        LDR     r0, ws_pc_files         ; Previous head of list
        STR     r0, [r2, #files_next]   ; It now comes after this one
        STR     r2, ws_pc_files         ; This record become head of the list
        STR     r6, [r2, #files_handle] ; Store the file handle
        LDR     r0, ws_packet_ptr       ; Restore pointer to the packet
        LDR     r1, [r0, #packet_fopen_in_del]; Should file be deleted
        STR     r1, [r2, #files_delete] ; Store whether to delete file
        ADD     r2, r2, #files_name     ; Pointer to buffer for filename
        MOV     r0, #OSArgs_ReadPath    ; Reason code to canonicalise name
        MOV     r1, r6                  ; Copy the file handle
        MOV     r5, #256                ; Size of the buffer
        SWI     XOS_Args                ; Read the filename into record
fopen_exit$l
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        STR     r6, [r0, #packet_fopen_out_handle]; Store the file handle
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_fopen_out_code]; Set return code for success
        RTSS                            ; Return from subroutine

        ; Handle failed file open
fopen_fail$l
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_failure     ; Return code for error
        STR     r1, [r0, #packet_fopen_out_code]; Set return code for error
        RTSS                            ; Return from subroutine

        ; Close a RISC OS file
fclose$l
        LDR     r0, [r0, #packet_fclose_in_handle]; Get the file handle
        BL      file_find               ; Find the corresponding record
        TEQ     r0, #0                  ; Was the file found
        BEQ     fclose_fail$l           ; Return error code if not
        BL      file_close              ; Close the file
        BVS     fclose_fail$l           ; Handle any error returned
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_fclose_out_code]; Set return code for success
        RTSS                            ; Return from subroutine

        ; Handle failed file close
fclose_fail$l
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_failure     ; Return code for error
        STR     r1, [r0, #packet_fclose_out_code]; Set return code for error
        RTSS                            ; Return from subroutine

        ; Read from a RISC OS file
fread$l LDR     r1, [r0, #packet_fread_in_handle]; Get the file handle
        ADD     r2, r0, #packet_fread_out_data; Pointer to output buffer
        LDR     r3, [r0, #packet_fread_in_size]; Number of bytes to read
        MOV     r5, r3                  ; Copy number of bytes to read
        LDR     r4, [r0, #packet_fread_in_ptr]; Get required file pointer
        CMP     r4, #-1                 ; Should current pointer be used
        MOVEQ   r0, #OSGBPB_Read        ; Reason code for current pointer
        MOVNE   r0, #OSGBPB_ReadAt      ; Reason code to read at specified
        SWI     XOS_GBPB                ; Read from the file
        BVS     fread_fail$l            ; Handle any error produced
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        SUB     r1, r5, r3              ; Number of bytes read
        STR     r1, [r0, #packet_fread_out_size]; Store number of bytes read
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_fread_out_code]; Set return code for success
        RTSS                            ; Return from subroutine

        ; Handle failed file reads
fread_fail$l
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_failure     ; Return code for error
        STR     r1, [r0, #packet_fread_out_code]; Set return code for error
        RTSS                            ; Return from subroutine

        ; Write to a RISC OS file
fwrite$l
        LDR     r1, [r0, #packet_fwrite_in_handle]; Get the file handle
        ADD     r2, r0, #packet_fwrite_in_data; Pointer to input buffer
        LDR     r3, [r0, #packet_fwrite_in_size]; Number of bytes to write
        LDR     r4, [r0, #packet_fwrite_in_ptr]; Get required file pointer
        CMP     r4, #-1                 ; Should current pointer be used
        MOVEQ   r0, #OSGBPB_Write       ; Reason code for current pointer
        MOVNE   r0, #OSGBPB_WriteAt     ; Reason code to write at specified
        SWI     XOS_GBPB                ; Write to the file
        BVS     fwrite_fail$l           ; Handle any error produced
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_fwrite_out_code]; Set return code for success
        RTSS                            ; Return from subroutine

        ; Handle failed file writes
fwrite_fail$l
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_failure     ; Return code for error
        STR     r1, [r0, #packet_fwrite_out_code]; Set return code for error
        RTSS                            ; Return from subroutine

        ; Register a new client task
talkstart$l
        MOV     r0, #0                  ; PC clients have ID 0
        MOV     r1, #0                  ; PC clients have flags value 0
        MOV     r2, #0                  ; No function to call for PC clients
        BL      talk_start              ; Initialise this client
        BVS     talkstart_fail$l        ; Handle any error produced
        MOV     r1, r0                  ; Copy client handle
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        STR     r1, [r0, #packet_talkstart_out_handle]; Store client handle
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_talkstart_out_code]; Set success return code
        RTSS                            ; Return from subroutine

        ; Handle failed task registration
talkstart_fail$l
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_failure     ; Return code for error
        STR     r1, [r0, #packet_talkstart_out_code]; Set error return code
        RTSS                            ; Return from subroutine

        ; Deregister a client task
talkend$l
        LDR     r0, [r0, #packet_talkend_in_handle]; Get client handle
        BL      talk_end                ; End this client
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_talkend_out_code]; Set success return code
        RTSS                            ; Return from subroutine

        ; Send a message to another client task
talktx$l
        LDR     r1, [r0, #packet_talktx_in_dest]; Destination ID or handle
        ADD     r2, r0, #packet_talktx_in_msg; Pointer to the message to send
        LDR     r0, [r0, #packet_talktx_in_handle]; Client handle for the task
        BL      talk_tx                 ; Send the message
        BVS     talktx_fail$l           ; Handle any error produced
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_talktx_out_code]; Set success return code
        RTSS                            ; Return from subroutine

        ; Handle failed message send
talktx_fail$l
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_failure     ; Return code for error
        STR     r1, [r0, #packet_talktx_out_code]; Set error return code
        RTSS                            ; Return from subroutine

        ; Check for any waiting messages for this client task
talkrx$l
        LDR     r0, [r0, #packet_talkrx_in_handle]; Client handle
        BL      talk_rx                 ; Check for a message
        BVS     talkrx_fail$l           ; Handle any error produced
        TEQ     r0, #0                  ; Check if a message was found
        BEQ     talkrx_fail$l           ; Handle lack of waiting message
        MOV     r3, r0                  ; Copy message pointer
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        STR     r1, [r0, #packet_talkrx_out_id]; Store source ID
        STR     r2, [r0, #packet_talkrx_out_handle]; Store source handle
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_talkrx_out_code]; Set success return code
        ADD     r2, r0, #packet_talkrx_out_msg; Pointer to destination buffer
        MOV     r1, #?client_message    ; Number of bytes to copy
talkrx_copy$l
        SUBS    r1, r1, #1              ; Decrement number of bytes to copy
        RTSS MI                         ; Exit if done
        LDRB    r0, [r3, r1]            ; Read byte of message
        STRB    r0, [r2, r1]            ; Write byte of message
        B       talkrx_copy$l           ; Copy the next byte of the message

        ; Handle failed message receive
talkrx_fail$l
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_failure     ; Return code for error
        STR     r1, [r0, #packet_talkrx_out_code]; Set error return code
        RTSS                            ; Return from subroutine

        ; Handle device driver initialisation requests
dev_init$l
        LDRB    r1, [r0, #packet_dev_init_in_first]; Get first device number
        LDR     r2, ws_device_first     ; Get previous first device number
        STR     r1, ws_device_first     ; Store this as the first device number
        CMP     r1, r2                  ; Check if rebooted
        BLLS    device_reset            ; Reset the device driver if it is
        ADD     r0, r0, #packet_dev_init_in_text; Pointer to configuration text
        BL      device_start            ; Initialise the device handler
        MOV     r1, r0                  ; Copy the number of devices
        LDR     r0, ws_packet_ptr       ; Restore pointer to buffer
        STR     r1, [r0, #packet_dev_init_out_num]; Set number of drives
        ADD     r2, r0, #packet_dev_init_out_msg; Pointer to buffer
        ADR     r0, ws_message          ; Pointer to messages control block
        ADR     r1, dev_init_token$l    ; Pointer to token to lookup
        MOV     r3, #256                ; Size of buffer
        SWI     XMessageTrans_Lookup    ; Lookup the message text
        BVS     dev_init_fail$l         ; Handle any error produced
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_dev_init_out_code]; Set code for success
        ADD     r0, r0, #packet_dev_init_out_msg; Pointer to buffer
        MOV     r1, r0                  ; Copy pointer to buffer
dev_init_loop$l
        LDRB    r2, [r0], #1            ; Read a character from buffer
        TEQ     r2, #0                  ; Is it the terminator
        BEQ     dev_init_done$l         ; Exit loop if it is
        TEQ     r2, #'|'                ; Is it an escape character
        BNE     dev_init_simple$l       ; If not then simply write it back
        LDRB    r2, [r0], #1            ; Read next character
        SUB     r2, r2, #'A' - 1        ; Treat as a CTRL-character
dev_init_simple$l
        STRB    r2, [r1], #1            ; Write the modified character
        B       dev_init_loop$l         ; Loop for next character
dev_init_done$l
        MOV     r2, #'$'                ; New terminator for string
        STRB    r2, [r1]                ; Terminate the string suitably
        RTSS                            ; Return from subroutine

        ; Handle failed token lookups
dev_init_fail$l
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_failure     ; Return code for error
        STR     r1, [r0, #packet_dev_init_out_code]; Set return code for error
        RTSS                            ; Return from subroutine

        ; Token to use for banner text lookup
dev_init_token$l
        =       "DBAN", 0
        ALIGN

        ; Handle device driver create BIOS Parameter Block requests
dev_bpb$l
        ADD     r1, r0, #packet_dev_bpb_out_bpb; Pointer to buffer for BPB
        LDRB    r0, [r0, #packet_dev_bpb_in_media]; Get unit code (drive number)
        BL      device_build_bpb        ; Create the parameter block
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_success     ; Reason code for success
        STR     r1, [r0, #packet_dev_bpb_out_code]; Set return code for success
        RTSS                            ; Return from subroutine

        ; Handle device driver media check requests
dev_changed$l
        LDRB    r0, [r0, #packet_dev_changed_in_media]; Get unit code
        BL      device_changed          ; Check if the device has changed
        MOV     r1, r0                  ; Copy changed code
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        STR     r1, [r0, #packet_dev_changed_out_media]; Set changed code
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_dev_changed_out_code]; Set success return code
        RTSS                            ; Return from subroutine

        ; Handle device driver read sector requests
dev_read$l
        LDRB    r1, [r0, #packet_dev_read_in_sector]; LSB of sector number
        LDRB    r2, [r0, #packet_dev_read_in_sector+1]; MSB of sector number
        ADD     r2, r1, r2, LSL#8       ; Combine to get actual sector number
        ADD     r1, r0, #packet_dev_read_out_data; Pointer to output buffer
        LDRB    r0, [r0, #packet_dev_read_in_unit]; Read unit code
        BL      device_read_sector      ; Read the sector
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOVVC   r1, #return_success     ; Return code for success
        MOVVS   r1, #return_failure     ; Return code for failure
        STR     r1, [r0, #packet_dev_read_out_code]; Set success return code
        RTSS                            ; Return from subroutine

        ; Handle device driver write sector requests
dev_write$l
        LDRB    r1, [r0, #packet_dev_write_in_sector]; LSB of sector number
        LDRB    r2, [r0, #packet_dev_write_in_sector+1]; MSB of sector number
        ADD     r2, r1, r2, LSL#8       ; Combine to get actual sector number
        ADD     r1, r0, #packet_dev_write_in_data; Pointer to input buffer
        LDRB    r0, [r0, #packet_dev_write_in_unit]; Read unit code
        BL      device_write_sector     ; Write the sector
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOVVC   r1, #return_success     ; Return code for success
        MOVVS   r1, #return_failure     ; Return code for failure
        STR     r1, [r0, #packet_dev_write_out_code]; Set success return code
        RTSS                            ; Return from subroutine

        ; Handle date and time conversions to DOS format
date_to_dos$l
        LDR     r1, [r0, #packet_date_to_dos_in_time]; Get low word of time
        LDRB    r0, [r0, #packet_date_to_dos_in_time + 4]; Get high byte of time
        BL      date_to_dos             ; Perform the conversion
        MOV     r2, r0                  ; Copy 2 byte time
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        STRB    r2, [r0, #packet_date_to_dos_out_time]; Store LSB of time
        MOV     r2, r2, LSR#8           ; Obtain MSB of time
        STRB    r2, [r0, #packet_date_to_dos_out_time + 1]; Store MSB of time
        STRB    r1, [r0, #packet_date_to_dos_out_date]; Store LSB of date
        MOV     r1, r1, LSR#8           ; Obtain MSB of date
        STRB    r1, [r0, #packet_date_to_dos_out_date + 1]; Store MSB of date
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_date_to_dos_out_code]; Set success return code
        RTSS                            ; Return from subroutine

        ; Handle date and time conversions to RISC OS format
date_to_riscos$l
        LDRB    r2, [r0, #packet_date_to_riscos_in_time]; Get LSB of time
        LDRB    r1, [r0, #packet_date_to_riscos_in_time + 1]; Get MSB of time
        ADD     r2, r2, r1, LSL#8       ; Combine 2 byte time
        LDRB    r1, [r0, #packet_date_to_riscos_in_date]; Get LSB of date
        LDRB    r0, [r0, #packet_date_to_riscos_in_date + 1]; Get MSB of date
        ADD     r1, r1, r0, LSL#8       ; Combine 2 byte date
        MOV     r0, r2                  ; Copy date field
        BL      date_to_riscos          ; Perform the conversion
        MOV     r2, r0                  ; Copy high byte of date and time
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        STR     r1, [r0, #packet_data_to_riscos_out_time]; Store low word
        STRB    r2, [r0, #packet_data_to_riscos_out_time + 4]; Store high byte
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_data_to_riscos_out_code]; Set success code
        RTSS                            ; Return from subroutine

        ; Start executing a *command
oscli_start$l
        ADD     r0, r0, #packet_oscli_start_in_command; Pointer to command
        BL      oscli_start             ; Prepare for executing the command
        MOV     r1, r0                  ; Copy command handle
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        STR     r1, [r0, #packet_oscli_start_out_handle]; Store command handle
        MOVVC   r1, #return_success     ; Return code for success
        MOVVS   r1, #return_failure     ; Return code for failure
        STR     r1, [r0, #packet_oscli_start_out_code]; Set success return code
        RTSS                            ; Return from subroutine

        ; Continue executing a *command
oscli_poll$l
        LDR     r1, [r0, #packet_oscli_poll_in_bytes]; Get size of input
        ADD     r2, r0, #packet_oscli_poll_in_data; Pointer to input data
        ADD     r3, r0, #packet_oscli_poll_out_data; Pointer to output data
        LDR     r0, [r0, #packet_oscli_poll_in_handle]; Get command handle
        BL      oscli_poll              ; Execute the command a bit more
        MOV     r2, r0                  ; Copy the status
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        STR     r2, [r0, #packet_oscli_poll_out_status]; Store the status
        STR     r1, [r0, #packet_oscli_poll_out_bytes]; Store size of output
        MOVVC   r1, #return_success     ; Return code for success
        MOVVS   r1, #return_failure     ; Return code for failure
        STR     r1, [r0, #packet_oscli_poll_out_code]; Set success return code
        RTSS                            ; Return from subroutine

        ; End executing a *command
oscli_end$l
        ADD     r1, r0, #packet_oscli_end_out_err; Pointer to error buffer
        LDR     r0, [r0, #packet_oscli_end_in_handle]; Get command handle
        BL      oscli_end               ; Terminate the command
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOVVC   r1, #return_success     ; Return code for success
        MOVVS   r1, #return_failure     ; Return code for failure
        STR     r1, [r0, #packet_oscli_end_out_code]; Set success return code
        RTSS                            ; Return from subroutine

        ; Reply to a message from another client task
talkreply$l
        LDR     r1, [r0, #packet_talkreply_in_dest]; Destination handle
        ADD     r2, r0, #packet_talkreply_in_msg; Pointer to the message to send
        LDR     r0, [r0, #packet_talkreply_in_handle]; Client handle for task
        BL      talk_reply              ; Send the message
        BVS     talkreply_fail$l        ; Handle any error produced
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_talkreply_out_code]; Set success return code
        RTSS                            ; Return from subroutine

        ; Handle failed message send
talkreply_fail$l
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_failure     ; Return code for error
        STR     r1, [r0, #packet_talkreply_out_code]; Set error return code
        RTSS                            ; Return from subroutine

        ; Control multitasking
faster$l
        LDR     r1, [r0, #packet_faster_in_time]; Centiseconds to delay for
        SWI     XOS_ReadMonotonicTime   ; Read the current time
        BVS     faster_fail$l           ; Fail if error returned
        ADD     r0, r0, r1              ; Time to resume normal operation
        TEQ     r1, #0                  ; Is it a cancel instruction
        BEQ     faster_done$l           ; Skip next bit if it is
        LDR     r1, ws_activity_delay   ; Get current time
        CMP     r0, r1                  ; Check direction of change
        MOVMI   r0, r1                  ; Keep old time if longer
faster_done$l
        STR     r0, ws_activity_delay   ; Store the updated time
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_faster_out_code]; Set success return code
        RTSS                            ; Return from subroutine

        ; Handle failed time reading
faster_fail$l
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_failure     ; Return code for error
        STR     r1, [r0, #packet_faster_out_code]; Set error return code
        RTSS                            ; Return from subroutine

        ; Generate a temporary filename
temporary$l
        MOV     r1, #return_success     ; Return code for success
        STR     r1, [r0, #packet_temporary_out_code]; Set success return code
        ADD     r0, r0, #packet_temporary_out_name; Destination buffer
        BL      scrap_name              ; Generate a filename
        BVS     temporary_fail$l        ; Fail if an error returned
        RTSS                            ; Return from subroutine

        ; Handle failed generation
temporary_fail$l
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOV     r1, #return_failure     ; Return code for error
        STR     r1, [r0, #packet_temporary_out_code]; Set error return code
        RTSS                            ; Return from subroutine

        ; Handle device driver read sector requests
dev_read_long$l
        LDR     r2, [r0, #packet_dev_read_long_in_sector]; Read sector number
        ADD     r1, r0, #packet_dev_read_long_out_data; Pointer to output buffer
        LDRB    r0, [r0, #packet_dev_read_long_in_unit]; Read unit code
        BL      device_read_sector      ; Read the sector
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOVVC   r1, #return_success     ; Return code for success
        MOVVS   r1, #return_failure     ; Return code for failure
        STR     r1, [r0, #packet_dev_read_long_out_code]; Set return code
        RTSS                            ; Return from subroutine

        ; Handle device driver write sector requests
dev_write_long$l
        LDR     r2, [r0, #packet_dev_write_long_in_sector]; Read sector number
        ADD     r1, r0, #packet_dev_write_long_in_data; Pointer to input buffer
        LDRB    r0, [r0, #packet_dev_write_long_in_unit]; Read unit code
        BL      device_write_sector     ; Write the sector
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOVVC   r1, #return_success     ; Return code for success
        MOVVS   r1, #return_failure     ; Return code for failure
        STR     r1, [r0, #packet_dev_write_long_out_code]; Set return code
        RTSS                            ; Return from subroutine

        ; Handle device driver file open
dev_open$l
        LDRB    r0, [r0, #packet_dev_open_in_unit]; Read unit code
        BL      device_open             ; Handle the file open
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOVVC   r1, #return_success     ; Return code for success
        MOVVS   r1, #return_failure     ; Return code for failure
        STR     r1, [r0, #packet_dev_open_out_code]; Set return code
        RTSS                            ; Return from subroutine

        ; Handle device driver file close
dev_close$l
        LDRB    r0, [r0, #packet_dev_close_in_unit]; Read unit code
        BL      device_close            ; Handle the file close
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOVVC   r1, #return_success     ; Return code for success
        MOVVS   r1, #return_failure     ; Return code for failure
        STR     r1, [r0, #packet_dev_close_out_code]; Set return code
        RTSS                            ; Return from subroutine

        ; Handle device driver removable checks
dev_removable$l
        LDRB    r0, [r0, #packet_dev_removable_in_unit]; Read unit code
        BL      device_removable        ; Handle the file close
        MOV     r2, r0                  ; Copy the removable media result
        LDR     r0, ws_packet_ptr       ; Get the packet pointer
        MOVVC   r1, #return_success     ; Return code for success
        MOVVS   r1, #return_failure     ; Return code for failure
        STR     r1, [r0, #packet_dev_removable_out_code]; Set return code
        STR     r2, [r0, #packet_dev_removable_out_status]; Set the status
        RTSS                            ; Return from subroutine

; A literal pool

        LTORG

; Read and process DOSMap extension to filetype mappings

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Initialise the DOSMap extension handling.
dosmap_initialise
        LocalLabels
        JSR     "r0"                    ; Stack registers
        MOV     r0, #0                  ; Value to clear pointers with
        STR     r0, ws_dosmap_start     ; Clear start pointer
        STR     r0, ws_dosmap_end       ; Clear end pointer
        BL      dosmap_update           ; Read the DOSMap mappings
        RTE VS                          ; Return any error produced
        RTS                             ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Finalise the DOSMap extension handling.
dosmap_finalise
        LocalLabels
        JSR     "r0, r2"                ; Stack registers
        LDR     r2, ws_dosmap_start     ; Get pointer to list
        MOV     r0, #OSModule_Free      ; Reason code to free memory
        TEQ     r2, #0                  ; Is there any memory to free
        BLNE    dynamic_osmodule        ; Release the memory if required
        MOV     r0, #0                  ; Value to clear pointers with
        STR     r0, ws_dosmap_start     ; Clear start pointer
        STR     r0, ws_dosmap_end       ; Clear end pointer
        RTS                             ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Update the cached DOSMap extension mappings.
dosmap_update
        LocalLabels
        JSR     "r0-r5"                 ; Stack registers
        LDR     r2, ws_dosmap_start     ; Get pointer to list
        MOV     r0, #OSModule_Free      ; Reason code to free memory
        TEQ     r2, #0                  ; Is there any memory to free
        BLNE    dynamic_osmodule        ; Release the memory if required
        MOV     r0, #0                  ; Value to clear pointers with
        STR     r0, ws_dosmap_start     ; Clear start pointer
        STR     r0, ws_dosmap_end       ; Clear end pointer
        ADR     r0, cmd$l               ; Pointer to command to execute
        SWI     XOS_CLI                 ; Read the mapping
        RTE VS                          ; Return if error produced
        MOV     r0, #OSFind_Openin:OR:OSFind_NoPath; Reason code to read file
        ADRL    r1, file$l              ; Pointer to filename
        SWI     XOS_Find                ; Open the file for input
        RTE VS                          ; Return if error produced
        TEQ     r0, #0                  ; Check file handle
        RTS EQ                          ; Exit if file not opened
        MOV     r1, r0                  ; Copy the file handle
        MOV     r0, #OSArgs_ReadExt     ; Reason code to read size
        SWI     XOS_Args                ; Read file size
        SUBS    r2, r2, #87             ; Subtract header and footer size
        BMI     none$l                  ; Skip next bit if no mappings
        MOV     r3, #29                 ; Length of each line
        DivRem  r4, r2, r3, r5          ; Calculate number of entries
        MOV     r2, #dosmap             ; Size of each entry in the list
        MUL     r3, r2, r4              ; Amount of memory required
        MOV     r0, #OSModule_Alloc     ; Reason code to allocate memory
        BL      dynamic_osmodule        ; Allocate the memory required
        STR     r2, ws_dosmap_start     ; Store pointer to start of memory
        ADD     r4, r2, r3              ; Pointer to end of memory
        STR     r4, ws_dosmap_end       ; Store pointer to end of memory
        MOV     r3, r2                  ; Copy list start pointer
        MOV     r2, #63                 ; File position for first extension
loop$l  CMP     r3, r4                  ; Have all entries been processed
        BHS     done$l                  ; Exit loop if the have
        MOV     r0, #OSArgs_SetPtr      ; Reason code to set file pointer
        SWI     XOS_Args                ; Set the file pointer
        ADD     r2, r2, #19             ; Update file pointer position
        MOV     r5, #0                  ; Start with a blank extension
        SWI     XOS_BGet                ; Read first character of extension
        TEQ     r0, #' '                ; Is it a space character
        BEQ     skip$l                  ; Skip the rest of the extension if so
        ORR     r5, r5, r0              ; Include it in the value
        SWI     XOS_BGet                ; Read second character of extension
        TEQ     r0, #' '                ; Is it a space character
        BEQ     skip$l                  ; Skip the rest of the extension if so
        ORR     r5, r5, r0, LSL#8       ; Include it in the value
        SWI     XOS_BGet                ; Read third character of extension
        TEQ     r0, #' '                ; Is it a space character
        BEQ     skip$l                  ; Skip the rest of the extension if so
        ORR     r5, r5, r0, LSL#16      ; Include it in the value
skip$l  STR     r5, [r3, #dosmap_ext]   ; Store the file extension
        MOV     r0, #OSArgs_SetPtr      ; Reason code to set file pointer
        SWI     XOS_Args                ; Set the file pointer
        ADD     r2, r2, #10             ; Update file pointer position
        SWI     XOS_BGet                ; Get most significant nibble
        CMP     r0, #'A'                ; Is it a digit or character
        SUBLT   r0, r0, #'0'            ; Convert a digit to number
        SUBGE   r0, r0, #'A' - 10       ; Convert a character to number
        MOV     r5, r0                  ; Copy the most significant nibble
        SWI     XOS_BGet                ; Get middle nibble
        CMP     r0, #'A'                ; Is it a digit or character
        SUBLT   r0, r0, #'0'            ; Convert a digit to number
        SUBGE   r0, r0, #'A' - 10       ; Convert a character to number
        ORR     r5, r0, r5, LSL#4       ; Copy the middle nibble
        SWI     XOS_BGet                ; Get least significant nibble
        CMP     r0, #'A'                ; Is it a digit or character
        SUBLT   r0, r0, #'0'            ; Convert a digit to number
        SUBGE   r0, r0, #'A' - 10       ; Convert a character to number
        ORR     r5, r0, r5, LSL#4       ; Copy the least significant nibble
        STR     r5, [r3, #dosmap_type]  ; Store the filetype
        ADD     r3, r3, #dosmap         ; Advance the list pointer
        B       loop$l                  ; Loop for the next entry
none$l  MOV     r0, #0                  ; Value to clear pointers with
        STR     r0, ws_dosmap_start     ; Clear start pointer
        STR     r0, ws_dosmap_end       ; Clear end pointer
done$l  MOV     r0, #OSFind_Close       ; Reason code to close the file
        SWI     XOS_Find                ; Close the file
        MOV     r0, #OSFile_Delete      ; Reason code to delete file
        ADR     r1, file$l              ; Pointer to the filename
        SWI     XOS_File                ; Delete the file
        RTE VS                          ; Return an error if unable to delete
        RTS                             ; Return from subroutine

        ; The command to read the DOSMap mappings
cmd$l   =   "DOSMap { > <ARMEdit$ScrapDir>.DOSMap }", 0

        ; The file to read the information from
file$l  =   "<ARMEdit$ScrapDir>.DOSMap", 0
        ALIGN

        ;   Parameters  : r0    - DOS extension (low byte is first character).
        ;   Returns     : r0    - Corresponding RISC OS filetype.
        ;   Description : Convert a DOS extension (0 to 3 characters) into a
        ;                 RISC OS filetype.
dosmap_ext_to_type
        LocalLabels
        JSR     "r1-r3"                 ; Stack registers
        LDR     r1, ws_dosmap_start     ; Pointer to start of list
        LDR     r2, ws_dosmap_end       ; Pointer to end of list
loop$l  CMP     r1, r2                  ; Have all entries been checked
        BHS     done$l                  ; Exit loop if the have
        LDR     r3, [r1, #dosmap_ext]   ; Get the file extension
        TEQ     r0, r3                  ; Does the extension match
        ADDNE   r1, r1, #dosmap         ; Advance the list pointer
        BNE     loop$l                  ; Loop for the next entry
        LDR     r0, [r1, #dosmap_type]  ; Read the filetype
        RTS                             ; Return from subroutine
done$l  LDR     r0, = OSFile_TypeDOS    ; Default filetype is DOS
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - The RISC OS filetype.
        ;   Returns     : r0    - Corresponding DOS extension (low byte is
        ;                         first character).
        ;   Description : Convert a RISC OS filetype into a DOS extension.
dosmap_type_to_ext
        LocalLabels
        JSR     "r1-r3"                 ; Stack registers
        LDR     r1, ws_dosmap_start     ; Pointer to start of list
        LDR     r2, ws_dosmap_end       ; Pointer to end of list
loop$l  CMP     r1, r2                  ; Have all entries been checked
        BHS     done$l                  ; Exit loop if the have
        LDR     r3, [r1, #dosmap_type]  ; Get the filetype
        TEQ     r0, r3                  ; Does the extension match
        ADDNE   r1, r1, #dosmap         ; Advance the list pointer
        BNE     loop$l                  ; Loop for the next entry
        LDR     r0, [r1, #dosmap_ext]   ; Read the extension
        RTS                             ; Return from subroutine
done$l  MOV     r0, #0                  ; Default extension is none
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - The character to convert.
        ;   Returns     : r0    - The converted 16 bit character; the low byte
        ;                         should be stored first in directory entries.
        ;   Description : Convert a character from a RISC OS filename to a
        ;                 suitable Windows 95 equivalent.
dosmap_char_riscos_to_win95
        LocalLabels
        JSR     ""                      ; Stack registers
        ; From  0-31 127-143 146-159 \ : * | " 144 145 160 ? / . < >
        ; To       _       _       _ _ _ _ _ _ 145 146  32 # . \ $ ^
        CMP     r0, #' '                ; Is it a control character
        MOVLT   r0, #'_'                ; Replace control characters
        TEQ     r0, #'\'                ; Is it an illegal Windows 95 character
        TEQNE   r0, #':'                ; Is it an illegal Windows 95 character
        TEQNE   r0, #'*'                ; Is it an illegal Windows 95 character
        TEQNE   r0, #'|'                ; Is it an illegal Windows 95 character
        TEQNE   r0, #'"'                ; Is it an illegal Windows 95 character
        MOVEQ   r0, #'_'                ; Replace illegal characters
        TEQ     r0, #160                ; Is it a hard space
        MOVEQ   r0, #' '                ; Replace by a standard space
        TEQ     r0, #''                ; Is it a single close quote
        MOVEQ   r0, #146                ; Replace with correct quote
        TEQ     r0, #''                ; Is it a single open quote
        MOVEQ   r0, #145                ; Replace with correct quote
        TEQ     r0, #'?'                ; Is it a question mark
        MOVEQ   r0, #'#'                ; Replace with a hash character
        TEQ     r0, #'.'                ; Is it a period
        MOVEQ   r0, #'\'                ; Replace with a backslash
        TEQ     r0, #'/'                ; Is it a slash
        MOVEQ   r0, #'.'                ; Replace with a period
        TEQ     r0, #'<'                ; Is it a less than symbol
        MOVEQ   r0, #'$'                ; Replace with a dollar sign
        TEQ     r0, #'>'                ; Is it a greater than symbol
        MOVEQ   r0, #'^'                ; Replace with a hat sign
        CMP     r0, #127                ; Is it a standard character
        RTSS LT                         ; Return from subroutine if standard
        CMP     r0, #160                ; Is it a mapped top bit set character
        RTSS GE                         ; Return from subroutine if mapped
        TEQ     r0, #145                ; Is it a single open quote
        TEQNE   r0, #146                ; Is it a single close quote
        MOVNE   r0, #'_'                ; Other characters are replaced
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - The 16 bit character to convert.
        ;   Returns     : r0    - The converted character.
        ;   Description : Convert a character from a Windows 95 filename to a
        ;                 suitable RISC OS equivalent.
dosmap_char_win95_to_riscos
        LocalLabels
        JSR     "r1-r3"                 ; Stack registers
        ; From  0-31 127-144 147-159 / : * | " & @ % 145 146  32 # . \ $ ^
        ; To       _       _       _ _ _ _ _ _ _ _ _ 144 145 160 ? / . < >
        CMP     r0, #' '                ; Is it a control character
        MOVLT   r0, #'_'                ; Replace control characters
        TEQ     r0, #'/'                ; Is it an illegal RISC OS character
        TEQNE   r0, #':'                ; Is it an illegal RISC OS character
        TEQNE   r0, #'*'                ; Is it an illegal RISC OS character
        TEQNE   r0, #'|'                ; Is it an illegal RISC OS character
        TEQNE   r0, #'"'                ; Is it an illegal RISC OS character
        TEQNE   r0, #'&'                ; Is it an illegal RISC OS character
        TEQNE   r0, #'@'                ; Is it an illegal RISC OS character
        TEQNE   r0, #'%'                ; Is it an illegal RISC OS character
        MOVEQ   r0, #'_'                ; Replace illegal characters
        TEQ     r0, #' '                ; Is it a standard space
        MOVEQ   r0, #160                ; Replace by a hard space
        TEQ     r0, #145                ; Is it a single open quote
        MOVEQ   r0, #''                ; Replace with correct quote
        TEQ     r0, #146                ; Is it a single close quote
        MOVEQ   r0, #''                ; Replace with correct quote
        TEQ     r0, #'#'                ; Is it a question mark
        MOVEQ   r0, #'?'                ; Replace with a hash character
        TEQ     r0, #'.'                ; Is it a period
        MOVEQ   r0, #'/'                ; Replace with a backslash
        TEQ     r0, #'\'                ; Is it a slash
        MOVEQ   r0, #'.'                ; Replace with a period
        TEQ     r0, #'$'                ; Is it a less than symbol
        MOVEQ   r0, #'<'                ; Replace with a dollar sign
        TEQ     r0, #'^'                ; Is it a greater than symbol
        MOVEQ   r0, #'>'                ; Replace with a hat sign
        CMP     r0, #127                ; Is it a standard character
        RTSS LT                         ; Return from subroutine if standard
        CMP     r0, #160                ; Is it a mapped top bit set character
        RTSS GE                         ; Return from subroutine if mapped
        TEQ     r0, #''                ; Is it a single open quote
        TEQNE   r0, #''                ; Is it a single close quote
        MOVNE   r0, #'_'                ; Other characters are replaced
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - The 16 bit character to convert.
        ;   Returns     : r0    - The converted character, or 0 to skip.
        ;   Description : Convert a character from a Windows 95 filename to a
        ;                 suitable DOS equivalent.
dosmap_char_win95_to_dos
        LocalLabels
        JSR     "r1-r3"                 ; Stack registers
        BL      dosmap_win95_upper      ; Convert character to upper case
        CMP     r0, #' '                ; Is it a space or control character
        MOVLE   r0, #0                  ; Skip control characters
        CMP     r0, #256                ; Is the character out of normal range
        MOVGE   r0, #'_'                ; Replace the character if it is
        TEQ     r0, #';'                ; Is it an illegal DOS character
        TEQNE   r0, #'='                ; Is it an illegal DOS character
        TEQNE   r0, #'+'                ; Is it an illegal DOS character
        TEQNE   r0, #','                ; Is it an illegal DOS character
        TEQNE   r0, #'['                ; Is it an illegal DOS character
        TEQNE   r0, #']'                ; Is it an illegal DOS character
        MOVEQ   r0, #'_'                ; Replace illegal characters
        CMP     r0, #128                ; Is it a top bit set character
        RTSS LT                         ; All finished if not top bit set
        MOV     r1, #128                ; Number of values to check
        ADR     r2, dosmap_char_map_dos_win95; Pointer to mapping table
loop$l  SUBS    r1, r1, #1              ; Decrement number of characters left
        MOVMI   r0, #'_'                ; Replace character if not found
        RTSS MI                         ; Return from subroutine if failed
        LDRB    r3, [r2, r1]            ; Read character from the table
        TEQ     r0, r3                  ; Does the character match
        BNE     loop$l                  ; Loop for next character if not
        ADD     r0, r1, #128            ; Convert index to character
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - The 16 bit character to convert.
        ;   Returns     : r0    - The converted character, or 0 to skip.
        ;   Description : Convert a character from a Windows 95 filename to a
        ;                 suitable DOS equivalent suitable for earlier versions
        ;                 of DOS.
dosmap_char_win95_to_dos_strict
        LocalLabels
        JSR     ""                      ; Stack registers
        BL      dosmap_char_win95_to_dos; Perform a standard conversion first
        CMP     r0, #127                ; Is it a special character
        MOVGE   r0, #'_'                ; Replace all special characters
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - The character to convert.
        ;   Returns     : r0    - The converted 16 bit character; the low byte
        ;                         should be stored first in directory entries.
        ;                         This should be converted to upper case if it
        ;                         is the first character of the filename.
        ;   Description : Convert a character from a DOS filename to a suitable
        ;                 Windows 95 equivalent.
dosmap_char_dos_to_win95
        LocalLabels
        JSR     "r1-r2"                 ; Stack registers
        AND     r0, r0, #&ff            ; Ensure that only the lower byte is set
        CMP     r0, #' '                ; Is it a control character
        MOVLT   r0, #'_'                ; Replace control characters
        SUBS    r1, r0, #128            ; Is the top bit set
        ADR     r2, dosmap_char_map_dos_win95; Pointer to mapping table
        LDRGEB  r0, [r2, r1]            ; Lookup the character in the table
        BL      dosmap_win95_lower      ; Convert to lower case
        RTSS                            ; Return from subroutine

        ; Conversion between DOS and Windows 95 top bit set characters
dosmap_char_map_dos_win95
        =       "ÇüéâäàåçêëèïîìÄÅ"      ; Mapping for codes 128 to 143
        =       "ÉæÆôöòûùÿÖÜø£Ø×_"      ; Mapping for codes 144 to 159
        =       "áíóúñÑªº¿®¬½¼¡«»"      ; Mapping for codes 160 to 175
        =       "_____ÁÂÀ©____¢¥_"      ; Mapping for codes 176 to 191
        =       "______ãÃ_______¤"      ; Mapping for codes 192 to 207
        =       "ðÐÊËÈ¹ÍÎÏ____¦Ì_"      ; Mapping for codes 208 to 223
        =       "ÓßÔÒõÕµÞþÚÛÙýÝ__"      ; Mapping for codes 224 to 239
        =       "-±=¾¶§÷_°_·¹³²__"      ; Mapping for codes 240 to 255
        ALIGN

        ;   Parameters  : r0    - The 16 bit character to convert.
        ;   Returns     : r0    - The 16 bit character result.
        ;   Description : Convert a Windows 95 character to lower case.
dosmap_win95_lower
        LocalLabels
        JSR     ""                      ; Stack registers
        CMP     r0, #'A'                ; Is it at least an 'A'
        RTSS LT                         ; Return if not
        CMP     r0, #'Z'                ; Is it at most a 'Z'
        BLE     lower$l                 ; Convert A-Z to lower case
        CMP     r0, #'À'                ; Is it at least an 'À'
        RTSS LT                         ; Return if not
        CMP     r0, #'Þ'                ; Is it at most a 'Þ'
        RTSS GT                         ; Return if it is higher
        TEQ     r0, #'×'                ; Is it a '×' symbol
        RTSS EQ                         ; Return if it is
lower$l ADD     r0, r0, #'a' - 'A'      ; Convert the character to lower case
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - The 16 bit character to convert.
        ;   Returns     : r0    - The 16 bit character result.
        ;   Description : Convert a Windows 95 character to upper case.
dosmap_win95_upper
        LocalLabels
        JSR     ""                      ; Stack registers
        CMP     r0, #'a'                ; Is it at least an 'a'
        RTSS LT                         ; Return if not
        CMP     r0, #'z'                ; Is it at most a 'z'
        BLE     upper$l                 ; Convert a-z to upper case
        CMP     r0, #'à'                ; Is it at least an 'à'
        RTSS LT                         ; Return if not
        CMP     r0, #'þ'                ; Is it at most a 'þ'
        RTSS GT                         ; Return if it is higher
        TEQ     r0, #'÷'                ; Is it a '÷' symbol
        RTSS EQ                         ; Return if it is
upper$l SUB     r0, r0, #'a' - 'A'      ; Convert the character to upper case
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to a RISC OS style leafname,
        ;                         terminated by a control character.
        ;                 r1    - Filetype of the RISC OS file, or -1 to
        ;                         disable the automatic addition of an
        ;                         extension.
        ;                 r2    - Pointer to buffer to receive the Windows 95
        ;                         style filename, terminated by two null bytes.
        ;                 r3    - Size of the destination buffer.
        ;   Returns     : None
        ;   Description : Convert a RISC OS leafname into a Windows 95 style
        ;                 name.
dosmap_name_riscos_to_win95
        LocalLabels
        JSR     ""                      ; Stack registers
        BL      trans$l                 ; Translate the name
        BL      test$l                  ; Test if all upper case
        BLEQ    low$l                   ; Convert to lower case if it is
        RTSS                            ; Return from subroutine
trans$l JSR     "r0-r5"                 ; Stack registers
        BIC     r3, r3, #1              ; Ensure that buffer size is even
        ADD     r3, r2, r3              ; Pointer to character after buffer
        SUBS    r3, r3, #2              ; Pointer to last character in buffer
        RTSS MI                         ; Exit if buffer too small
        MOV     r4, r0                  ; Copy pointer to source buffer
pre$l   LDRB    r0, [r4], #1            ; Read character from source name
        TEQ     r0, #160                ; Is it a hard space
        TEQNE   r0, #'/'                ; Is it an extension separator
        BEQ     pre$l                   ; Skip spaces and extension separators
        SUB     r4, r4, #1              ; Back to the first valid character
        MOV     r5, r2                  ; Current end of output name
loop$l  LDRB    r0, [r4], #1            ; Read character from source buffer
        CMP     r0, #' '                ; Is character a terminator
        BLE     tail$l                  ; Perform tail processing if it is
        TEQ     r2, r3                  ; Has end of buffer been reached
        BEQ     tail$l                  ; Perform tail processing if it is
        BL      dosmap_char_riscos_to_win95; Translate the character
        TEQ     r0, #'.'                ; Is character an extension separator
        MOVEQ   r1, #-1                 ; Do not add extension if it is
        TEQNE   r0, #' '                ; Is character a space
        ADDNE   r5, r2, #2              ; Accept character if it is not
        STRB    r0, [r2], #1            ; Write low byte of character
        MOV     r0, r0, LSR#8           ; Obtain high byte of character
        STRB    r0, [r2], #1            ; Write high byte of charactert
        B       loop$l                  ; Loop for the next character
tail$l  MOV     r2, r5                  ; Return to the last proper character
        TEQ     r2, r3                  ; Has end of buffer been reached
        BEQ     done$l                  ; Just terminate name if end of buffer
        CMP     r1, #-1                 ; Has a filetype been specified
        BEQ     done$l                  ; All finished if no filetype
        MOV     r0, r1                  ; Copy the required filetype
        BL      dosmap_type_to_ext      ; Convert to an extension
        MOVS    r1, r0                  ; Copy the selected extension
        BEQ     done$l                  ; All finished if no extension
        MOV     r0, #'.'                ; Extension separator
        STRB    r0, [r2], #1            ; Write low byte of separator
        MOV     r0, r0, LSR#8           ; Obtain the high byte
        STRB    r0, [r2], #1            ; Write high byte of separator
ext$l   TEQ     r2, r3                  ; Has end of buffer been reached
        BEQ     done$l                  ; Exit loop if end reached
        AND     r0, r1, #&ff            ; Extract the next character
        TEQ     r0, #0                  ; Has the extension end been reached
        BEQ     done$l                  ; All finished if end reached
        MOV     r1, r1, LSR#8           ; Prepare for the following character
        BL      dosmap_char_riscos_to_win95; Convert the character
        BL      dosmap_win95_lower      ; Convert to lower case
        STRB    r0, [r2], #1            ; Write low byte of character
        MOV     r0, r0, LSR#8           ; Obtain the high byte
        STRB    r0, [r2], #1            ; Write high byte of character
        B       ext$l                   ; Loop for next character of extension
done$l  MOV     r0, #0                  ; Terminator character
        STRB    r0, [r2], #1            ; Write low byte of terminator
        STRB    r0, [r2]                ; Write high byte of terminator
        RTSS                            ; Return from subroutine
test$l  JSR     "r0-r2"                 ; Stack registers
testl$l LDRB    r0, [r2], #1            ; Read low byte of character
        LDRB    r1, [r2], #1            ; Read high byte of character
        ORRS    r0, r0, r1, LSL#8       ; Combine to form a single character
        RTS EQ                          ; Return from subroutine if the end
        MOV     r1, r0                  ; Copy the character
        BL      dosmap_win95_upper      ; Convert character to upper case
        TEQ     r0, r1                  ; Was the character already upper case
        RTS NE                          ; Return if character was not upper case
        B       testl$l                 ; Loop for the next character
low$l   JSR     "r0-r3"                 ; Stack registers
        MOV     r3, r2                  ; Copy the initial pointer
lowl$l  LDRB    r0, [r2]                ; Read low byte of character
        LDRB    r1, [r2, #1]            ; Read high byte of character
        ORRS    r0, r0, r1, LSL#8       ; Combine to form a single character
        RTS EQ                          ; Return from subroutine if the end
        TEQ     r2, r3                  ; Is this the first character
        BLNE    dosmap_win95_lower      ; Convert character to lower case
        STRB    r0, [r2], #1            ; Store modified low byte of character
        MOV     r0, r0, LSR#8           ; Obtain high byte of the character
        STRB    r0, [r2], #1            ; Store modified high byte of character
        B       lowl$l                  ; Loop for the next character

        ;   Parameters  : r0    - Pointer to a Windows 95 style leafname,
        ;                         terminated by two null bytes.
        ;                 r1    - Pointer to buffer to receive the RISC OS
        ;                         style filename, terminated by a single
        ;                         null character.
        ;                 r2    - Size of the destination buffer.
        ;   Returns     : None
        ;   Description : Convert a Windows 95 style leafname into a RISC OS
        ;                 name.
dosmap_name_win95_to_riscos
        LocalLabels
        JSR     "r0-r4"                 ; Stack registers
        MOV     r3, r0                  ; Copy pointer to source buffer
        ADD     r2, r1, r2              ; Pointer to character after buffer
        SUBS    r2, r2, #1              ; Pointer to last character in buffer
        RTSS MI                         ; Exit if no space in destination buffer
loop$l  TEQ     r1, r2                  ; Has the end of buffer been reached
        BEQ     done$l                  ; Just write a terminator if full
        LDRB    r0, [r3], #1            ; Read low byte of character
        LDRB    r4, [r3], #1            ; Read high byte of character
        ORRS    r0, r0, r4, LSL#8       ; Combine to form a single character
        BEQ     done$l                  ; Write a simple terminator if required
        BL      dosmap_char_win95_to_riscos; Translate the character
        STRB    r0, [r1], #1            ; Store the character to the buffer
        B       loop$l                  ; Loop for the next character
done$l  MOV     r0, #0                  ; Terminator character
        STRB    r0, [r1]                ; Write terminator to destination buffer
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to a Windows 95 style leafname,
        ;                         terminated by two null bytes.
        ;                 r1    - Pointer to buffer to receive the DOS style
        ;                         filename, padded to 8 + 3 characters using
        ;                         spaces.
        ;   Returns     : None
        ;   Description : Convert a Windows 95 style leafname into a DOS name.
dosmap_name_win95_to_dos
        LocalLabels
        JSR     "r0-r6"                 ; Stack registers
        MOV     r2, r0                  ; Copy pointer to source name
        MOV     r6, r0                  ; Make another copy of the source name
        ADD     r3, r1, #8              ; Pointer to output extension
        ADD     r4, r3, #3              ; Pointer to character after extension
name$l  LDRB    r0, [r2], #1            ; Read low byte of next character
        LDRB    r5, [r2], #1            ; Read high byte of the character
        ORRS    r0, r0, r5, LSL#8       ; Combine to form a single character
        BEQ     padex$l                 ; Extension complete if a terminator
        TEQ     r0, #'.'                ; Is it an extension separator
        BEQ     pad$l                   ; Pad to the extension if it is
        TEQ     r1, r3                  ; Has end of output buffer been reached
        BEQ     name$l                  ; Do not add this character if it has
        BL      dosmap_char_win95_to_dos; Translate this character
        TEQ     r0, #0                  ; Should the character be skipped
        STRNEB  r0, [r1], #1            ; Store the translated character
        B       name$l                  ; Loop for the next character
pad$l   TEQ     r1, r3                  ; Is the padding complete
        BEQ     ext$l                   ; Exit loop when padding complete
        MOV     r0, #' '                ; Character to pad with
        STRB    r0, [r1], #1            ; Store the padding character
        B       pad$l                   ; Loop for the next character
ext$l   LDRB    r0, [r2], #1            ; Read low byte of next character
        LDRB    r5, [r2], #1            ; Read high byte of the character
        ORRS    r0, r0, r5, LSL#8       ; Combine to form a single character
        BEQ     padex$l                 ; Extension complete if a terminator
        TEQ     r0, #'.'                ; Is it an extension separator
        MOVEQ   r1, r3                  ; Return to the start of the extension
        BEQ     ext$l                   ; Loop for next character
        TEQ     r1, r4                  ; Has end of output buffer been reached
        BEQ     ext$l                   ; Do not add this character if it has
        BL      dosmap_char_win95_to_dos; Translate this character
        TEQ     r0, #0                  ; Should the character be skipped
        STRNEB  r0, [r1], #1            ; Store the translated character
        B       ext$l                   ; Loop for the next character
padex$l TEQ     r1, r4                  ; Is the padding complete
        BEQ     inc$l                   ; Check if name needs incrementing
        MOV     r0, #' '                ; Character to pad with
        STRB    r0, [r1], #1            ; Store the padding character
        B       padex$l                 ; Loop for the next character
inc$l   SUB     r0, r3, #8              ; Pointer to start of filename
        SUB     r13, r13, #28           ; Reserve space on the stack
        MOV     r1, r13                 ; Copy the buffer pointer
        BL      dosmap_name_dos_to_win95; Translate the name back
        MOV     r0, r6                  ; Pointer to the original filename
        BL      cmp$l                   ; Compare the filenames
        BEQ     done$l                  ; Skip increment if the same
        SUB     r0, r3, #8              ; Pointer to start of filename
        BL      dosmap_name_dos_increment; Add a ~1 to the end of the filename
done$l  ADD     r13, r13, #28           ; Restore the stack pointer
        RTSS                            ; Return from subroutine
cmp$l   JSR     "r0-r4"                 ; Stack registers
        MOV     r2, r0                  ; Copy pointer to the original name
cmpl$l  LDRB    r0, [r2], #1            ; Read low byte of character
        LDRB    r4, [r2], #1            ; Read high byte of character
        ORR     r0, r0, r4, LSL#8       ; Combine to form a single character
        BL      dosmap_win95_upper      ; Convert to upper case
        MOV     r3, r0                  ; Copy the first character
        LDRB    r0, [r1], #1            ; Read low byte of character
        LDRB    r4, [r1], #1            ; Read high byte of character
        ORR     r0, r0, r4, LSL#8       ; Combine to form a single character
        BL      dosmap_win95_upper      ; Convert to upper case
        TEQ     r0, r3                  ; Do the characters match
        RTS NE                          ; Exit if different
        TEQ     r0, #0                  ; Has the end of both names been reached
        BNE     cmpl$l                  ; Loop if not the end
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - Pointer to a Windows 95 style leafname,
        ;                         terminated by two null bytes.
        ;                 r1    - Pointer to buffer to receive the DOS style
        ;                         filename, padded to 8 + 3 characters using
        ;                         spaces.
        ;   Returns     : None
        ;   Description : Convert a Windows 95 style leafname into a DOS name,
        ;                 using only characters supported by old versions of
        ;                 DOS.
dosmap_name_win95_to_dos_strict
        LocalLabels
        JSR     "r0-r5"                 ; Stack registers
        MOV     r2, r0                  ; Copy pointer to source name
        ADD     r3, r1, #8              ; Pointer to output extension
        ADD     r4, r3, #3              ; Pointer to character after extension
name$l  LDRB    r0, [r2], #1            ; Read low byte of next character
        LDRB    r5, [r2], #1            ; Read high byte of the character
        ORRS    r0, r0, r5, LSL#8       ; Combine to form a single character
        BEQ     padex$l                 ; Extension complete if a terminator
        TEQ     r0, #'.'                ; Is it an extension separator
        BEQ     pad$l                   ; Pad to the extension if it is
        TEQ     r1, r3                  ; Has end of output buffer been reached
        BEQ     name$l                  ; Do not add this character if it has
        BL      dosmap_char_win95_to_dos_strict; Translate this character
        TEQ     r0, #0                  ; Should the character be skipped
        STRNEB  r0, [r1], #1            ; Store the translated character
        B       name$l                  ; Loop for the next character
pad$l   TEQ     r1, r3                  ; Is the padding complete
        BEQ     ext$l                   ; Exit loop when padding complete
        MOV     r0, #' '                ; Character to pad with
        STRB    r0, [r1], #1            ; Store the padding character
        B       pad$l                   ; Loop for the next character
ext$l   LDRB    r0, [r2], #1            ; Read low byte of next character
        LDRB    r5, [r2], #1            ; Read high byte of the character
        ORRS    r0, r0, r5, LSL#8       ; Combine to form a single character
        BEQ     padex$l                 ; Extension complete if a terminator
        TEQ     r0, #'.'                ; Is it an extension separator
        MOVEQ   r1, r3                  ; Return to the start of the extension
        BEQ     ext$l                   ; Loop for next character
        TEQ     r1, r4                  ; Has end of output buffer been reached
        BEQ     ext$l                   ; Do not add this character if it has
        BL      dosmap_char_win95_to_dos_strict; Translate this character
        TEQ     r0, #0                  ; Should the character be skipped
        STRNEB  r0, [r1], #1            ; Store the translated character
        B       ext$l                   ; Loop for the next character
padex$l TEQ     r1, r4                  ; Is the padding complete
        RTSS EQ                         ; Return from subroutine when finished
        MOV     r0, #' '                ; Character to pad with
        STRB    r0, [r1], #1            ; Store the padding character
        B       padex$l                 ; Loop for the next character

        ;   Parameters  : r0    - Pointer to a DOS style filename, padded to
        ;                         8 + 3 characters using spaces.
        ;                 r1    - Pointer to buffer to receive the Windows 95
        ;                         style filename, terminated by two null bytes.
        ;   Returns     : None
        ;   Description : Convert a DOS style leafname into a Windows 95 name.
dosmap_name_dos_to_win95
        LocalLabels
        JSR     "r0-r3"                 ; Stack registers
        MOV     r2, r0                  ; Copy source buffer pointer
        MOV     r3, #0                  ; Start from the beginning
loop$l  TEQ     r3, #11                 ; Have all characters been processed
        BEQ     done$l                  ; Exit loop if they have
        LDRB    r0, [r2, r3]            ; Read character from source name
        TEQ     r0, #' '                ; Is this a padding character
        BNE     ok$l                    ; Accept all other characters
        CMP     r3, #8                  ; Has the extension been reached
        BGE     done$l                  ; All finished if extension padded
        MOV     r3, #8                  ; Advance to separator
        B       loop$l                  ; Loop for the next character
ok$l    TEQ     r3, #8                  ; Is an extension separator required
        BLEQ    ext$l                   ; Add an extension separator
        ADD     r3, r3, #1              ; Increment number of characters
        BL      dosmap_char_dos_to_win95; Convert to a Windows 95 character
        TEQ     r3, #1                  ; Is it the first character
        BLEQ    dosmap_win95_upper      ; Convert to upper case if it is
        STRB    r0, [r1], #1            ; Store low byte of converted character
        MOV     r0, r0, LSR#8           ; Obtain the high byte of the character
        STRB    r0, [r1], #1            ; Store high byte of converted character
        B       loop$l                  ; Loop for the next character
done$l  MOV     r0, #0                  ; Terminator character
        STRB    r0, [r1], #1            ; Store low byte of terminator
        STRB    r0, [r1], #1            ; Store high byte of terminator
        RTSS                            ; Return from subroutine
ext$l   JSR     "r0"                    ; Stack registers
        MOV     r0, #'.'                ; Extension separator
        STRB    r0, [r1], #1            ; Store low byte of the separator
        MOV     r0, r0, LSR#8           ; Obtain the high byte of the separator
        STRB    r0, [r1], #1            ; Store high byte of the separator
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to a Windows 95 style leafname,
        ;                         terminated by two null bytes.
        ;   Returns     : flags - Z flag set (EQ) if it is a simple DOS name.
        ;   Description : Check if the specified filename needs to be stored
        ;                 as a long filename, or whether it can be written as
        ;                 a standard DOS file.
dosmap_name_win95_is_long
        LocalLabels
        JSR     "r0-r4"                 ; Stack registers
        SUB     r13, r13, #40           ; Reserve some space on the stack
        MOV     r1, r13                 ; Copy pointer to buffer
        BL      dosmap_name_win95_to_dos; Convert it to the DOS equivalent name
        MOV     r2, r0                  ; Copy pointer to the original name
        MOV     r0, r1                  ; Copy pointer to the DOS version
        ADD     r1, r0, #11             ; Pointer to a new buffer
        BL      dosmap_name_dos_to_win95; Convert the name back
loop$l  LDRB    r3, [r1], #1            ; Read low byte of character
        LDRB    r0, [r1], #1            ; Read high byte of character
        ORR     r3, r3, r0, LSL#8       ; Combine to form a single character
        LDRB    r4, [r2], #1            ; Read low byte of character
        LDRB    r0, [r2], #1            ; Read high byte of character
        ORR     r4, r4, r0, LSL#8       ; Combine to form a single character
        TEQ     r3, r4                  ; Do the characters match
        BNE     done$l                  ; All done if no match
        TEQ     r3, #0                  ; Is it the end of the filename
        BNE     loop$l                  ; Loop until finished
done$l  ADD     r13, r13, #40           ; Restore the stack pointer
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - Pointer to a DOS style filename, padded to
        ;                         8 + 3 characters using spaces.
        ;   Returns     : flags - Z flag set (EQ) if name matches one of the
        ;                         reserved DOS names.
        ;   Description : There are quite a few filenames that are reserved
        ;                 by DOS, corresponding to system devices. This checks
        ;                 if a filename matches any of these names.
dosmap_name_dos_is_special
        LocalLabels
        JSR     "r0-r4"                 ; Stack registers
        LDRB    r1, [r0, #8]            ; Read first character of extension
        TEQ     r1, #' '                ; Is extension character a space
        RTS NE                          ; Not a match if there is an extension
        ADR     r1, match$l             ; Pointer to list of reserved names
name$l  LDRB    r2, [r1], #1            ; Read next character
        CMP     r2, #' '                ; Is it the start of a name
        RTS LT                          ; No match if end of list reached
        BNE     name$l                  ; Loop until the start found
        MOV     r2, r0                  ; Start from the first character
loop$l  LDRB    r3, [r1], #1            ; Read character from name to match
        LDRB    r4, [r2], #1            ; Read character from name to check
        TEQ     r3, r4                  ; Do the characters match
        BNE     name$l                  ; Try the next name if not
        TEQ     r3, #' '                ; Is it the end of the name
        BNE     loop$l                  ; Loop for next character if not
        RTS                             ; Return from subroutine
match$l = " CLOCK$ CON AUX COM1 COM2 COM3 COM4 LPT1 LPT2 LPT3 LPT4 NUL PRN ", 0
        ALIGN

        ;   Parameters  : r0    - Pointer to the first DOS style name, padded
        ;                         to 8 + 3 characters using spaces.
        ;                 r1    - Pointer to the second DOS style name, also
        ;                         padded to 8 + 3 characters using spaces.
        ;   Returns     : flags - Z flag set (EQ) if names match.
        ;   Description : Check if two DOS filenames are the same.
dosmap_name_dos_compare
        LocalLabels
        JSR     "r0-r4"                 ; Stack registers
        ADD     r2, r0, #name_length + extension_length; First character after
loop$l  TEQ     r0, r2                  ; Has end been reached
        RTS EQ                          ; Return if all checked
        LDRB    r3, [r0], #1            ; Read character from first name
        LDRB    r4, [r1], #1            ; Read character from second name
        TEQ     r3, r4                  ; Are the characters the same
        RTS NE                          ; Return if characters different
        B       loop$l                  ; Loop for the next character

        ;   Parameters  : r0    - Pointer to a DOS style filename, padded to
        ;                         8 + 3 characters using spaces.
        ;   Returns     : None
        ;   Description : Increment the specified filename in-place. This can
        ;                 be used as many times as required to generate a
        ;                 unique filename.
dosmap_name_dos_increment
        LocalLabels
        JSR     "r0-r3"                 ; Stack registers
        ADD     r1, r0, #name_length    ; Pointer to after the last character
end$l   CMP     r1, r0                  ; Are there any characters
        BLT     new$l                   ; Construct a name
        LDRB    r2, [r1, #-1]!          ; Read the next character
        CMP     r2, #' '                ; Is it a padding character
        BLE     end$l                   ; Loop until first character found
        MOV     r2, r1                  ; Copy pointer to last used character
check$l CMP     r2, r0                  ; Has the start been passed
        BLT     new$l                   ; Add an extension if it has
        LDRB    r3, [r2]                ; Read the next character
        TEQ     r3, #'~'                ; Has a tilde been found
        BEQ     ok$l                    ; All done if correct
        CMP     r3, #'0'                ; Is it at least a '0'
        BLT     new$l                   ; Add a new extension if not
        CMP     r3, #'9'                ; Is it at most a '9'
        BGT     new$l                   ; Add a new extension if not
        SUB     r2, r2, #1              ; Advance to previous character
        B       check$l                 ; Loop until tilde found
ok$l    MOV     r2, r1                  ; Copy pointer to last used character
inc$l   LDRB    r3, [r2]                ; Read the current character
        TEQ     r3, #'~'                ; Is it the tilde
        BEQ     add$l                   ; Add another digit if it is
        TEQ     r3, #'9'                ; Is there a carry
        ADDNE   r3, r3, #1              ; Increment digit if no carry
        MOVEQ   r3, #'0'                ; Wrap digit if carry to next digit
        STRB    r3, [r2], #-1           ; Store the modified digit
        BEQ     inc$l                   ; Perform the carry if required
        RTSS                            ; Return from subroutine
add$l   SUB     r3, r1, r0              ; Check length of existing name
        CMP     r3, #7                  ; Can the name be extended
        BLT     ext$l                   ; Extend the name if possible
        SUB     r1, r2, #2              ; Pointer to store new characters at
new$l   SUB     r2, r1, r0              ; Check length of existing name
        SUBS    r2, r2, #5              ; Number of characters to overwrite
        MOVMI   r2, #0                  ; Do not add unnecessary characters
        SUB     r1, r1, r2              ; Overwrite the required number
        CMP     r1, r0                  ; Compare to start of name
        SUBLT   r1, r0, #1              ; Do not go below first character
        MOV     r2, #'~'                ; First character of addition
        STRB    r2, [r1, #1]            ; Store the first new character
        MOV     r2, #'1'                ; Second character of addition
        STRB    r2, [r1, #2]            ; Store the second new character
        RTSS                            ; Return from subroutine
ext$l   LDRB    r3, [r1]                ; Read current character
        TEQ     r3, #'~'                ; Is it the tilde
        MOVEQ   r3, #'1'                ; Modify the first digit
        STRB    r3, [r1, #1]            ; Store the digit one place further
        RTSS EQ                         ; Return from subroutine if finished
        SUB     r1, r1, #1              ; Advance to the next digit
        B       ext$l                   ; Loop for the next digit

        ;   Parameters  : r0    - Pointer to a DOS style filename, padded to
        ;                         8 + 3 characters using spaces.
        ;   Returns     : r0    - The checksum for the supplied filename.
        ;   Description : Calculate the checksum for the specified DOS style
        ;                 filename.
dosmap_name_dos_checksum
        LocalLabels
        JSR     "r1-r3"                 ; Stack registers
        MOV     r1, r0                  ; Copy pointer to the name
        ADD     r2, r1, #11             ; Pointer to first character after name
        MOV     r0, #0                  ; Set the checksum to zero initially
loop$l  MOVS    r0, r0, LSR#1           ; Shift the checksum so far
        ORRCS   r0, r0, #1<<7           ; Wrap the first bit round to the end
        LDRB    r3, [r1], #1            ; Read next character from name
        ADD     r0, r0, r3              ; Include this character in the sum
        AND     r0, r0, #&ff            ; Restrict the sum to a single byte
        TEQ     r1, r2                  ; Has the last character been processed
        BNE     loop$l                  ; Loop until all characters included
        RTSS                            ; Return from subroutine

; PC memory manipulation

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Initialise memory allocations by the PC.
memory_initialise
        LocalLabels
        JSR     "r0"                    ; Stack registers
        MOV     r0, #0                  ; Null pointer for no blocks
        STR     r0, ws_pc_memory        ; No allocations by the PC so far
        RTS                             ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Release any PC memory claimed
memory_reset
        LocalLabels
        JSR     "r0-r3"                 ; Stack registers
        LDR     r3, ws_pc_memory        ; Get pointer to first block
memory_reset_loop$l
        TEQ     r3, #0                  ; Are there any more
        BEQ     memory_reset_done$l     ; Exit loop if there are not
        MOV     r2, r3                  ; Copy pointer
        LDR     r3, [r3, #memory_next]  ; Get pointer to next block
        MOV     r0, #OSModule_Free      ; Reason code to release memory
        SWI     XOS_Module              ; Release this block of memory
        B       memory_reset_loop$l     ; Loop for next block
memory_reset_done$l
        MOV     r0, #0                  ; Null pointer to store in workspace
        STR     r0, ws_pc_memory        ; Clear pointer to memory list
        RTS                             ; Return from subroutine

; PC file manipulation

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Initialise file handling by the PC.
file_initialise
        LocalLabels
        JSR     "r0"                    ; Stack registers
        MOV     r0, #0                  ; Null pointer for no files
        STR     r0, ws_pc_files         ; No files opened by the PC so far
        RTS                             ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Ensure that all files opened by the PC are closed.
file_reset
        LocalLabels
        JSR     "r0-r1"                 ; Stack registers
        ADR     r0, ws_pc_files         ; Get pointer to pointer to first record
loop$l  LDR     r1, [r0]                ; Get value of pointer
        TEQ     r1, #0                  ; Are there any more record
        RTS EQ                          ; Return if not
        BL      file_close              ; Close this file
        B       loop$l                  ; Loop for the next file

        ;   Parameters  : r0    - File handle.
        ;   Returns     : r0    - Pointer to pointer to this record.
        ;   Description : Find the record that contains details about this
        ;                 file.
file_find
        LocalLabels
        JSR     "r1-r3"                 ; Stack registers
        MOV     r3, r0                  ; Copy file handle
        ADR     r0, ws_pc_files         ; Pointer to start from
loop$l  LDR     r1, [r0]                ; Get the value of this pointer
        TEQ     r1, #0                  ; Is the pointer valid
        MOVEQ   r0, #0                  ; Clear main pointer if not
        RTS EQ                          ; Return if not valid
        LDR     r2, [r1, #files_handle] ; Get the file handle
        TEQ     r3, r2                  ; Does it match
        RTS EQ                          ; Return if it does
        ADD     r0, r1, #files_next     ; Pointer to the next pointer
        B       loop$l                  ; Loop for the next file

        ;   Parameters  : r0    - Pointer to the pointer to this record.
        ;   Returns     : None
        ;   Description : Close the specified file, delete it if required, and
        ;                 free the memory used by its record.
file_close
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        BL      close$l                 ; Close the file
        LDR     r2, [r0]                ; Get pointer to actual record
        LDR     r1, [r2, #files_next]   ; Get pointer to next record
        STR     r1, [r0]                ; Unlink this record from the list
        MOV     r0, #OSModule_Free      ; Reason code to release memory
        BL      dynamic_osmodule        ; Free the memory
        RTSS                            ; Return from subroutine

        ; Actually close and optionally delete the file
close$l JSR     "r0-r5"                 ; Stack registers
        LDR     r2, [r0]                ; Get pointer to the actual record
        LDR     r1, [r2, #files_handle] ; Get the file handle
        MOV     r0, #OSFind_Close       ; Reason code to close file
        SWI     XOS_Find                ; Close the file
        LDR     r0, [r2, #files_delete] ; Get whether the file should be deleted
        TEQ     r0, #0                  ; Should it be deleted
        RTS EQ                          ; Return if not
        MOV     r0, #OSFile_Delete      ; Reason code to delete file
        ADD     r1, r2, #files_name     ; The file name
        SWI     XOS_File                ; Delete the file
        RTS                             ; Return from subroutine

; Communications support

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Initialise communications protocol management.
talk_initialise
        LocalLabels
        JSR     "r0"                    ; Stack registers
        MOV     r0, #0                  ; Value to initialise data with
        STR     r0, ws_client_handle    ; The initial client handle
        STR     r0, ws_client_next      ; The first message number
        ADR     r0, ws_client_armedit   ; The internal client
        STR     r0, ws_clients          ; No communications clients so far
        MOV     r1, #0                  ; Only the internal client present
        STR     r1, [r0, #client_next]  ; Terminate the list of clients
        STR     r1, [r0, #client_fn]    ; No function to call
        STR     r1, [r0, #client_flags] ; Default flags
        STR     r1, [r0, #client_poll_word]; Clear the initial poll word
        STR     r1, [r0, #client_next_num]; Store number of next message
        STR     r1, [r0, #client_num]   ; Set initial message number
        MOV     r1, #1                  ; The internal ID
        STR     r1, [r0, #client_handle]; Set the internal handle
        STR     r1, [r0, #client_id]    ; Set the internal ID to 1
        MOV     r1, #-1                 ; Initial destination (none)
        STR     r1, [r0, #client_dest]  ; Set initial destination
        RTS                             ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Deregister any PC side clients.
talk_reset
        LocalLabels
        JSR     "r0-r1"                 ; Stack registers
        LDR     r1, ws_clients          ; Get pointer to first record
loop$l  TEQ     r1, #0                  ; Is the pointer valid
        RTS EQ                          ; Exit if not
        LDR     r2, [r1, #client_id]    ; Read the client ID
        LDR     r0, [r1, #client_handle]; Read the client handle
        LDR     r1, [r1, #client_next]  ; Pointer to next record
        TEQ     r2, #0                  ; Is it a PC client
        BLEQ    talk_end                ; Deregister it if it is
        B       loop$l                  ; Loop for next record

        ;   Parameters  : r0    - Pre-allocated ID for task.
        ;                 r1    - Flags.
        ;                 r2    - Pointer to optional function to call.
        ;                 r3    - Value for R12 when function called.
        ;   Returns     : r0    - Unique client handle.
        ;                 r1    - Poll word for this task.
        ;   Description : Register a new task.
talk_start
        LocalLabels
        JSR     "r2-r7"                 ; Stack registers
        MOV     r4, r0                  ; Copy task ID
        MOV     r5, r1                  ; Copy flags
        MOV     r6, r2                  ; Copy function pointer
        MOV     r7, r3                  ; Copy required r12 value
        MOV     r0, #OSModule_Alloc     ; Reason code to allocate memory
        LDR     r3, = client            ; Size of block to claim
        BL      dynamic_osmodule        ; Claim memory for file record
        RTE VS                          ; Exit if unable to allocate memory
        LDR     r0, ws_clients          ; Previous head of list
        STR     r0, [r2, #client_next]  ; It now comes after this one
        STR     r2, ws_clients          ; This record becomes head of the list
        STR     r4, [r2, #client_id]    ; Store the client ID
        BIC     r5, r5, #client_flag_fn ; Clear function called flag
        STR     r5, [r2, #client_flags] ; Store the flags
        STR     r6, [r2, #client_fn]    ; Store the function pointer
        STR     r7, [r2, #client_r12]   ; Store the required r12 value
        LDR     r0, ws_client_next      ; Get the next message number
        STR     r0, [r2, #client_next_num]; Store number of next message
        STR     r0, [r2, #client_num]   ; Set initial message number
        MOV     r0, #0                  ; Value to clear poll word with
        STR     r0, [r2, #client_poll_word]; Set initial poll word value
        MOV     r0, #-1                 ; Initial destination (none)
        STR     r0, [r2, #client_dest]  ; Set initial destination
        LDR     r0, ws_client_handle    ; Get the previous unique handle
        ADD     r0, r0, #1              ; Increment the handle
        ORR     r0, r0, #1 << 31        ; Ensure the top bit is set
        CMP     r0, #-1                 ; Is it the special value
        MOVEQ   r0, #1 << 31            ; The wrap-around value
        STR     r0, ws_client_handle    ; Store the new value
        STR     r0, [r2, #client_handle]; Store the unique handle
        ADD     r1, r2, #client_poll_word; Pointer to poll word for task
        BL      talk_update             ; Update any poll words
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - Previously assigned handle for task.
        ;   Returns     : None
        ;   Description : Deregister a task.
talk_end
        LocalLabels
        JSR     "r0-r3"                 ; Stack registers
        ADR     r1, ws_clients          ; Pointer to first pointer
loop$l  LDR     r2, [r1]                ; Get value of pointer
        TEQ     r2, #0                  ; Is it a valid pointer
        RTS EQ                          ; Exit if not found
        LDR     r3, [r2, #client_handle]; Read the client handle
        TEQ     r0, r3                  ; Is it the required handle
        BEQ     found$l                 ; Exit loop if it is
        ADD     r1, r2, #client_next    ; Get next pointer to record pointer
        B       loop$l                  ; Loop for next record
found$l LDR     r0, [r2, #client_next]  ; Get pointer to next record
        STR     r0, [r1]                ; Unlink this record
        MOV     r0, #OSModule_Free      ; Reason code to release memory
        BL      dynamic_osmodule        ; Free the memory
        BL      talk_update             ; Update any poll words
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - Message number to find.
        ;   Returns     : r0    - Pointer to client record containing the
        ;                         message, or 0 if not found.
        ;   Description : Attempt to find the specified message.
talk_find
        LocalLabels
        JSR     "r1-r2"                 ; Stack registers
        MOV     r1, r0                  ; Copy the required message number
        LDR     r0, ws_clients          ; Pointer to the first record
loop$l  TEQ     r0, #0                  ; Check if it is a valid pointer
        RTS EQ                          ; Exit if not
        LDR     r2, [r0, #client_dest]  ; Get message destination
        CMP     r2, #-1                 ; Is it a valid message
        BEQ     skip$l                  ; Skip the next bit if not
        LDR     r2, [r0, #client_num]   ; Get the number of this message
        TEQ     r1, r2                  ; Is it the required message number
        RTS EQ                          ; Exit if it is
skip$l  LDR     r0, [r0, #client_next]  ; Pointer to the next record
        B       loop$l                  ; Check the next client

        ;   Parameters  : r0    - Pointer to the client record.
        ;                 r1    - Message number to check.
        ;   Returns     : r0    - Non-zero if the message is suitable.
        ;   Description : Check if a particular message number is valid for a
        ;                 client.
talk_check
        LocalLabels
        JSR     "r1-r2"                 ; Stack registers
        MOV     r2, r0                  ; Copy the client record pointer
        MOV     r0, r1                  ; Copy the message number
        BL      talk_find               ; Find the message details
        TEQ     r0, #0                  ; Was the message found
        BEQ     fail$l                  ; Exit if not
        LDR     r0, [r0, #client_dest]  ; Get the destination of this message
        TST     r0, #1 << 31            ; Check the top bit of the destination
        BEQ     id$l                    ; Skip to ID handler if bit is clear
        LDR     r1, [r2, #client_handle]; Read the client handle
        TEQ     r0, r1                  ; Does it match
        BNE     fail$l                  ; Return failure if not
        B       ok$l                    ; Otherwise it is suitable
id$l    TEQ     r0, #1                  ; Is it a broadcast message
        BEQ     broadcast$l             ; Special case if it is
        LDR     r1, [r2, #client_id]    ; Read the client ID
        TEQ     r0, r1                  ; Does it match
        BNE     fail$l                  ; Skip next bit if not
        B       ok$l                    ; Otherwise it is suitable
broadcast$l
        LDR     r0, [r2, #client_flags] ; Get the flags
        TST     r0, #client_flag_armedit; Check if it requires broadcast
        BEQ     fail$l                  ; Fail if it does not
ok$l    MOV     r0, #1                  ; Value to indicate success
        RTS                             ; Return from subroutine
fail$l  MOV     r0, #0                  ; Value to indicate unsuitable
        RTS                             ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Update poll words, and delete any irrelevant
        ;                 messages.
talk_update
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        LDR     r2, ws_clients          ; Pointer to the first record
loop$l  TEQ     r2, #0                  ; Is it a valid pointer
        BEQ     check$l                 ; Check for functions if done
        MOV     r0, #0                  ; No message poll word
        STR     r0, [r2, #client_poll_word]; Store blank poll word
        LDR     r0, ws_client_next      ; Get next message number
        LDR     r1, [r2, #client_next_num]; Get message number
        TEQ     r0, r1                  ; Have all messages been delivered
        BEQ     next$l                  ; Next client if they have
        MOV     r0, #1                  ; There could be a message
        STR     r0, [r2, #client_poll_word]; Store set poll word
        MOV     r0, r2                  ; Copy client record pointer
        BL      talk_check              ; Check the current message
        TEQ     r0, #0                  ; Is the message valid
        BNE     next$l                  ; Client done if message alright
        ADD     r1, r1, #1              ; Increment message number
        STR     r1, [r2, #client_next_num]; Store new message number
        LDR     r0, [r2, #client_flags] ; Get the flags
        BIC     r0, r0, #client_flag_fn ; Clear function delivered flag
        STR     r0, [r2, #client_flags] ; Store the modified flags
        B       loop$l                  ; Check this message number
next$l  LDR     r2, [r2, #client_next]  ; Get pointer to next record
        B       loop$l                  ; Loop for the next client
check$l LDR     r0, ws_clients          ; Back to the first client
try$l   TEQ     r0, #0                  ; Is the pointer valid
        RTS EQ                          ; Exit if all checked
        LDR     r1, [r0, #client_poll_word]; Get the poll word
        TEQ     r1, #0                  ; Is there a message waiting
        BEQ     not$l                   ; Skip next bit if not
        LDR     r1, [r0, #client_fn]    ; Check if there is a function to call
        TEQ     r1, #0                  ; Is it a valid pointer
        BEQ     not$l                   ; Skip next bit if not
        LDR     r1, [r0, #client_flags] ; Get the flags
        TST     r1, #client_flag_fn     ; Has the function been called
        BNE     not$l                   ; Skip next bit if it has
        ADR     r0, callback$l          ; Function to callback
        MOV     r1, r12                 ; Value of r12 to be called with
        SWI     XOS_AddCallBack         ; Add a transient callback
        RTS                             ; Exit if callback added
not$l   LDR     r0, [r0, #client_next]  ; Pointer to next client record
        B       try$l                   ; Loop for the next client

        ; Callback routine to call functions if required
callback$l
        JSR     "r0-r2"                 ; Stack registers
start$l LDR     r0, ws_clients          ; Pointer to the first record
again$l TEQ     r0, #0                  ; Is the pointer valid
        RTS EQ                          ; Exit if all checked
        LDR     r1, [r0, #client_poll_word]; Get the poll word
        TEQ     r1, #0                  ; Is there a message waiting
        BEQ     skip$l                  ; Skip next bit if not
        LDR     r1, [r0, #client_fn]    ; Check if there is a function to call
        TEQ     r1, #0                  ; Is it a valid pointer
        BEQ     skip$l                  ; Skip next bit if not
        LDR     r1, [r0, #client_flags] ; Get the flags
        TST     r1, #client_flag_fn     ; Has the function been called
        BNE     skip$l                  ; Skip next bit if it has
        ORR     r1, r1, #client_flag_fn ; Set the function called flag
        STR     r1, [r0, #client_flags] ; Store the modified flags
        STMFD   r13!, {r12}             ; Store private word value
        LDR     r12, [r0, #client_r12]  ; Get the required r12 value
        MOV     r14, pc                 ; Copy return address
        LDR     pc, [r0, #client_fn]    ; Call the function
        LDMFD   r13!, {r12}             ; Restore private word value
        B       start$l                 ; Back to the start of the check
skip$l  LDR     r0, [r0, #client_next]  ; Pointer to next client record
        B       again$l                 ; Loop for next character

        ;   Parameters  : r0    - Client handle for task.
        ;                 r1    - Either the ID or client handle of destination.
        ;                 r2    - Pointer to block containing the message, or
        ;                         0 to check if the buffer already contains a
        ;                         message.
        ;   Returns     : r2    - Pointer to message buffer, or 0 if no message
        ;                         waiting to be delivered.
        ;   Description : Send a message to another task.
talk_tx LocalLabels
        JSR     "r0-r1, r3-r4"          ; Stack registers
        LDR     r3, ws_clients          ; Pointer to first client record
loop$l  TEQ     r3, #0                  ; Is it a valid pointer
        MOVEQ   r2, #0                  ; Clear pointer if not found
        RTS EQ                          ; Exit if not found
        LDR     r4, [r3, #client_handle]; Get handle for this client
        TEQ     r0, r4                  ; Does the handle match
        BEQ     found$l                 ; Exit loop if it does
        LDR     r3, [r3, #client_next]  ; Pointer to next client record
        B       loop$l                  ; Loop for next record
found$l TEQ     r2, #0                  ; Is there a message to send
        BEQ     skip$l                  ; Skip the next bit if not
        MOV     r0, r3                  ; Store message in senders buffer
        BL      talk_send               ; Send the message
skip$l  LDR     r2, [r3, #client_dest]  ; Get the destination
        CMP     r2, #-1                 ; Is it valid
        MOVEQ   r2, #0                  ; Clear pointer if not
        ADDNE   r2, r3, #client_message ; Pointer to message buffer
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - Client handle for task.
        ;                 r1    - Client handle of the destination.
        ;                 r2    - Pointer to block containing the message.
        ;   Returns     : None
        ;   Description : Reply to a message from another task.
talk_reply
        LocalLabels
        JSR     "r0, r3-r5"             ; Stack registers
        LDR     r3, ws_clients          ; Pointer to first client record
loop1$l TEQ     r3, #0                  ; Is it a valid pointer
        RTS EQ                          ; Exit if not
        LDR     r4, [r3, #client_handle]; Get handle for this client
        TEQ     r0, r4                  ; Does the handle match
        BEQ     found1$l                ; Exit loop if done
        LDR     r3, [r3, #client_next]  ; Pointer to next client record
        B       loop1$l                 ; Loop for next record
found1$l
        MOV     r0, r3                  ; Copy client record pointer
        LDR     r3, ws_clients          ; Pointer to first client record
loop2$l TEQ     r3, #0                  ; Is it a valid pointer
        RTS EQ                          ; Exit if not
        LDR     r4, [r3, #client_handle]; Get handle for this client
        TEQ     r1, r4                  ; Does the handle match
        BEQ     found2$l                ; Exit loop if done
        LDR     r3, [r3, #client_next]  ; Pointer to next client record
        B       loop2$l                 ; Loop for next record
found2$l
        BL      talk_send               ; Stack registers
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - Pointer to client record of the sender.
        ;                 r1    - Either the ID or client handle of destination.
        ;                 r2    - Pointer to block containing the message.
        ;                 r3    - Pointer to client record to contain the
        ;                         message.
        ;   Returns     : None
        ;   Description : Send a message to another task, using the specified
        ;                 message buffer.
talk_send
        LocalLabels
        JSR     "r0-r1, r3"             ; Stack registers
        STR     r1, [r3, #client_dest]  ; Set the destination
        LDR     r1, ws_client_next      ; Get the next message number
        STR     r1, [r3, #client_num]   ; Store this message number
        ADD     r1, r1, #1              ; Increment the message number
        STR     r1, ws_client_next      ; Store the next message number
        LDR     r1, [r0, #client_handle]; Get client handle of sender
        STR     r1, [r3, #client_msg_handle]; Set the sender handle
        LDR     r1, [r0, #client_id]    ; Get client ID of sender
        STR     r1, [r3, #client_msg_id]; Set the sender ID
        CMP     r2, #-1                 ; Is it a valid message pointer
        BEQ     done$l                  ; Do not copy message for internal
        ADD     r3, r3, #client_message ; Pointer to message buffer
        MOV     r1, #?client_message    ; Number of bytes to copy
copy$l  SUBS    r1, r1, #1              ; Decrement number of bytes to copy
        BMI     done$l                  ; Exit loop if done
        LDRB    r0, [r2, r1]            ; Read byte of message
        STRB    r0, [r3, r1]            ; Write byte of message
        B       copy$l                  ; Copy the next byte of the message
done$l  BL      talk_update             ; Update all messages and poll words
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - Message type.
        ;   Returns     : None
        ;   Description : Broadcast an internal message
talk_armedit
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        STR     r0, ws_client_armedit + client_message; Store message
        MOV     r0, #1                  ; Internal client handle
        MOV     r1, #1                  ; Destination is broadcast
        MOV     r2, #-1                 ; Dummy message block
        BL      talk_tx                 ; Send the message
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - Client handle for task.
        ;   Returns     : r0    - Pointer to block containing message.
        ;                 r1    - Source ID.
        ;                 r2    - Source client handle.
        ;   Description : Check for a waiting message.
talk_rx LocalLabels
        JSR     ""                      ; Stack registers
        LDR     r1, ws_clients          ; Pointer to first client record
loop$l  TEQ     r1, #0                  ; Is it a valid pointer
        BEQ     fail$l                  ; Exit if not found
        LDR     r2, [r1, #client_handle]; Get handle for this client
        TEQ     r0, r2                  ; Does the handle match
        BEQ     found$l                 ; Exit loop if it does
        LDR     r1, [r1, #client_next]  ; Pointer to next client record
        B       loop$l                  ; Loop for next record
found$l LDR     r0, [r1, #client_poll_word]; Check the poll word
        TEQ     r0, #0                  ; Is there a message to deliver
        RTS EQ                          ; Exit if not
        LDR     r0, [r1, #client_next_num]; Get the message number
        BL      talk_find               ; Find the message
        TEQ     r0, #0                  ; Was the message found
        RTE EQ                          ; Exit if not
        LDR     r2, [r1, #client_next_num]; Get the message number
        STR     r2, [r1, #client_last]  ; Store the message number
        ADD     r2, r2, #1              ; Increment the message number
        STR     r2, [r1, #client_next_num]; Store the new message number
        LDR     r2, [r1, #client_flags] ; Get the client flags
        BIC     r2, r2, #client_flag_fn ; Clear the function called flag
        STR     r2, [r1, #client_flags] ; Store the new flags
        LDR     r1, [r0, #client_dest]  ; Get the destination
        TST     r1, #1 << 31            ; Check the top bit of the destination
        MOVNE   r1, #-1                 ; Value for message delivered
        STRNE   r1, [r0, #client_dest]  ; Delete message if aimed at a handle
        LDR     r1, [r0, #client_msg_id]; Read the source ID
        LDR     r2, [r0, #client_msg_handle]; Read the source handle
        ADD     r0, r0, #client_message ; Pointer to the actual message
        BL      talk_update             ; Update all messages and poll words
        RTS                             ; Return from subroutine
fail$l  MOV     r0, #0                  ; Value to indicate failure
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - Client handle for task.
        ;   Returns     : None
        ;   Description : Claim the most recently read message.
talk_ack
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        LDR     r1, ws_clients          ; Pointer to first client record
loop$l  TEQ     r1, #0                  ; Is it a valid pointer
        RTS EQ                          ; Exit if not found
        LDR     r2, [r1, #client_handle]; Get handle for this client
        TEQ     r0, r2                  ; Does the handle match
        BEQ     found$l                 ; Exit loop if it does
        LDR     r1, [r1, #client_next]  ; Pointer to next client record
        B       loop$l                  ; Loop for next record
found$l LDR     r0, [r1, #client_last]  ; Number of last message delivered
        BL      talk_find               ; Find the message if it exists
        TEQ     r0, #0                  ; Does the message exist
        RTS EQ                          ; Exit if it does not exist
        MOV     r1, #-1                 ; Value to delete message with
        STR     r1, [r0, #client_dest]  ; Delete the message
        BL      talk_update             ; Update all messages and poll words
        RTS                             ; Return from subroutine

; A literal pool

        LTORG

; Scrap directory manipulation

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Set a system variable pointing to the scrap directory
        ;                 and ensure it exists.
scrap_initialise
        LocalLabels
        JSR     "r0-r4"                 ; Stack registers
        ADRL    r0, ws_temporary_name   ; Temporary name destination
        ADR     r1, temp$l              ; Initial temporary name
loop$l  LDRB    r2, [r1], #1            ; Read character from source
        STRB    r2, [r0], #1            ; Write character to destination
        TEQ     r2, #0                  ; Check for terminator
        BNE     loop$l                  ; Loop if not reached
        ADR     r0, var$l               ; Pointer to variable name
        MOV     r2, #1:SHL:31           ; Check for existence
        MOV     r3, #0                  ; First call
        MOV     r4, #0                  ; Don't want expansion
        SWI     XOS_ReadVarVal          ; Check if variable exists
        TEQ     r2, #0                  ; Does it exist (ignore any error)
        BNE     exist$l                 ; Skip next section if it exists
        ADR     r0, var$l               ; Pointer to variable name
        ADR     r1, def$l               ; Pointer to variable value
        MOV     r2, #?def$l - 1         ; Length of string
        MOV     r3, #0                  ; First call
        MOV     r4, #OS_VartypeString   ; A simple GSTrans'd string
        SWI     XOS_SetVarVal           ; Set the variable
        RTE VS                          ; Return any error produced
exist$l MOV     r0, #OSFile_CreateDir   ; Reason code to create directory
        ADR     r1, dir$l               ; Pointer to directory to create
        MOV     r4, #0                  ; Default number of entries
        SWI     XOS_File                ; Create the directory
        RTE VS                          ; Return any error produced
        RTS                             ; Return from subroutine

        ; Details of how to access scrap directory
def$l   =       "<Wimp$ScrapDir>.ARMEdit", 0; Default directory
var$l   =       "ARMEdit$ScrapDir", 0   ; System variable to use
dir$l   =       "<ARMEdit$ScrapDir>", 0 ; How to access the directory
temp$l  =       "<ARMEdit$ScrapDir>.Temp_aaaaa", 0; First temporary file
        ALIGN

        ;   Parameters  : r0    - Pointer to a buffer for the filename.
        ;   Returns     : None
        ;   Description : Generate a unique filename for a temporary file.
scrap_name
        LocalLabels
        JSR     "r0-r5"                 ; Stack registers
        MOV     r2, r0                  ; Copy pointer to destination buffer
        MOV     r0, #OSFSControl_CanonicalisePath; Reason code to canonicalise
        ADRL    r1, ws_temporary_name   ; Pointer to original name
        MOV     r3, #0                  ; No path variable
        MOV     r4, #0                  ; No path string
        MOV     r5, #String             ; Default buffer size
        SWI     XOS_FSControl           ; Canonicalise the filename
        BL      inc$l                   ; Increment the filename
        RTE VS                          ; Return any error
        RTSS                            ; Return from subroutine
inc$l   JSR     "r0-r1"                 ; Stack registers
        ADRL    r1, ws_temporary_name   ; Pointer to original name
end$l   LDRB    r0, [r1, #1]!           ; Read character from name
        TEQ     r0, #0                  ; Is it the terminator
        BNE     end$l                   ; Loop for next character if not
loop$l  LDRB    r0, [r1, #-1]!          ; Read character from name
        CMP     r0, #'z'                ; Can it be incremented
        ADDLT   r0, r0, #1              ; Do so, if it can
        MOVGE   r0, #'a'                ; Wrap round otherwise
        STRB    r0, [r1]                ; Store the new character
        BGE     loop$l                  ; Loop for next character if required
        RTSS                            ; Return from subroutine

; Display of numbers

        ; Parameters    : r0    - Value to output.
        ; Returns       : None
        ; Description   : Output the specified value as 8 hexadecimal digits.
write_hex8
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        ADRL    r1, ws_buffer           ; Pointer to a general buffer
        MOV     r2, #String             ; Size of the buffer
        SWI     XOS_ConvertHex8         ; Convert the number
        SWI     XOS_Write0              ; Write the number
        RTS                             ; Return from subroutine

        ; Parameters    : r0    - Value to output.
        ; Returns       : None
        ; Description   : Output the specified 4 byte value as a right
        ;                 justified decimal value.
write_cardinal4
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        ADRL    r1, ws_buffer           ; Pointer to a general buffer
        MOV     r2, #11                 ; Size of the buffer
        SWI     XOS_ConvertCardinal4    ; Convert the number
loop$l  SUBS    r2, r2, #1              ; Decrement number of padding characters
        BLS     done$l                  ; Exit loop if padding done
        SWI     XOS_WriteI + ' '        ; Pad with a space
        B       loop$l                  ; Loop for another character
done$l  SWI     XOS_Write0              ; Write the number
        RTS                             ; Return from subroutine

; Device driver support

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Initialise device driver handling when the module
        ;                 is loaded.
device_initialise
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        MOV     r0, #0                  ; Value to clear record with
        STR     r0, ws_device_head      ; Clear the head of list pointer
        MOV     r0, #UpCallV            ; Vector number to claim
        ADRL    r1, device_upcallv      ; Pointer to vector handler
        MOV     r2, r12                 ; Value to be passed in R12 to routine
        SWI     XOS_Claim               ; Claim the upcallv vector
        RTSS                            ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Tidy up when the module is about to exit.
device_finalise
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        BL      device_reset            ; Reset the device driver
        MOV     r0, #UpCallV            ; Vector number to release
        ADRL    r1, device_upcallv      ; Pointer to vector handler
        MOV     r2, r12                 ; Value passed in R2 when claimed
        SWI     XOS_Release             ; Release the upcallv vector
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to configuration text.
        ;   Returns     : r0    - Number of drives supported.
        ;   Description : Initialise the device drivers when the PC driver is
        ;                 loaded.
device_start
        LocalLabels
        JSR     "r1-r6"                 ; Stack registers
        MOV     r1, r0                  ; Copy pointer to configuration text
        LDR     r0, ws_device_head      ; Get pointer to first record
        TEQ     r0, #0                  ; Check if already started
        BNE     bad$l                   ; Skip next bit if already working
        ADRL    r0, key$l               ; Pointer to keyword definition
        ADRL    r2, ws_buffer           ; Pointer to output buffer
        MOV     r3, #String             ; Size of output buffer
        SWI     XOS_ReadArgs            ; Scan the command line arguments
        BVS     bad$l                   ; Finish if failed
        ADD     r2, r2, #4              ; Skip the device name
        LDR     r0, [r2], #4            ; Pointer to the size limit
        MOV     r1, #device_limit       ; Default size limit
        TEQ     r0, #0                  ; Was a limit specified
        BEQ     skip$l                  ; Skip the next bit if not
        ADD     r0, r0, #1              ; Pointer to evaluated value
        LDRU    r1, r0, r3, r4          ; Read the value
skip$l  MOV     r1, r1, LSL#20          ; Convert the size to bytes
        STR     r1, ws_device_limit     ; Store the size limit
        MOV     r6, #0                  ; Default device flags
        LDR     r0, [r2], #4            ; Should long filenames be generated
        TEQ     r0, #0                  ; Test if the flag is present
        ORRNE   r6, r6, #device_flag_long; Set the long filename flag if it is
        LDR     r0, [r2], #4            ; Should write operations be supported
        TEQ     r0, #0                  ; Test if the flag is present
        ORRNE   r6, r6, #device_flag_write; Set the write enable flag if it is
        LDR     r0, [r2], #4            ; Should automatic relogging be disabled
        TEQ     r0, #0                  ; Test if the flag is present
        ORRNE   r6, r6, #device_flag_manual; Set the manual relog flag if it is
        LDR     r0, [r2], #4            ; Should automatic labels be disabled
        TEQ     r0, #0                  ; Test if the flag is present
        ORRNE   r6, r6, #device_flag_anon; Set the anonymous flag if it is
        ORRNE   r6, r6, #device_flag_manual; Also set the manual relog flag
        LDR     r1, [r2], #4            ; Pointer to the device size
        MOV     r0, #device_size        ; Default device size
        TEQ     r1, #0                  ; Was a size specified
        BEQ     skips$l                 ; Skip the next bit if not
        ADD     r1, r1, #1              ; Pointer to evaluated value
        LDRU    r0, r1, r3, r4          ; Read the value
skips$l BL      size$l                  ; Fill in all the size details
        MOV     r1, #0                  ; No devices initialised
        ADR     r5, ws_device_head      ; Pointer to head of list pointer
        LDR     r0, [r2], #4            ; Pointer to first device name
        BL      add$l                   ; Create a new device
        LDR     r0, [r2], #4            ; Pointer to second device name
        BL      add$l                   ; Create a new device
        LDR     r0, [r2], #4            ; Pointer to third device name
        BL      add$l                   ; Create a new device
        LDR     r0, [r2]                ; Pointer to fourth device name
        BL      add$l                   ; Create a new device
        MOV     r0, r1                  ; Copy number of devices
        RTSS                            ; Return from subroutine
bad$l   MOV     r0, #0                  ; Do not support any devices
        RTSS                            ; Return from subroutine
add$l   JSR     "r0, r2-r4"             ; Stack registers
        TEQ     r0, #0                  ; Was a path specified
        BEQ     done$l                  ; Exit if no path specified
        MOV     r4, r0                  ; Copy pointer to the path
        BL      mem$l                   ; Allocate the memory for this device
        TEQ     r2, #0                  ; Check if memory allocated
        BEQ     done$l                  ; Finish if failed
        ADD     r1, r1, #1              ; Increment number of devices
        MOV     r3, r6                  ; Initial flags to use
start$l LDRB    r0, [r4], #1            ; Read first character of device name
        TEQ     r0, #'\'                ; Is it a special character
        ORREQ   r3, r3, #device_flag_imagefs; Include image file flag if present
        BEQ     start$l                 ; Loop back to find the actual start
        TEQ     r0, #'#'                ; Is it a special character
        ORREQ   r3, r3, #device_flag_raw; Include disable canonicalise flag
        BEQ     start$l                 ; Loop back to find the actual start
        STR     r3, [r2, #device_flags] ; Store initial flags
        SUB     r4, r4, #1              ; Back to the start of the path name
        TST     r3, #device_flag_raw    ; Should the path be canonicalised
        BLEQ    canon$l                 ; Canonicalise the path name
        BLNE    copy$l                  ; Perform plain copy otherwise
        MOV     r0, #0                  ; No PC files open initially
        STR     r0, [r2, #device_open_files]; Clear the number of files
        MOV     r0, r2                  ; Copy pointer to record
        BL      device_prepare          ; Prepare this device
        STR     r2, [r5]                ; Add to the list when finished
        ADD     r5, r2, #device_next    ; Update previous next pointer
        MOV     r0, #0                  ; Value to terminate list with
        STR     r0, [r5]                ; Terminate device list
done$l  RTSS                            ; Return from subroutine
mem$l   JSR     "r0, r3-r4"             ; Stack registers
        MOV     r0, #OSModule_Alloc     ; Reason code to allocate memory
        LDR     r3, = device            ; Size of block required
        BL      dynamic_osmodule        ; Attempt to allocate the memory
        MOVVS   r2, #0                  ; Pointer not valid if failed
        TEQ     r2, #0                  ; Check if memory allocated
        BEQ     memd$l                  ; Return if failed to allocate memory
        MOV     r4, r2                  ; Copy the device record pointer
        LDR     r3, ws_device_sectors_per_fat; Number of sectors in the FAT
        MOV     r3, r3, LSL#sector_shift; Size of FAT in bytes
        BL      dynamic_osmodule        ; Attempt to allocate the memory
        MOVVS   r2, #0                  ; Pointer not valid if failed
        TEQ     r2, #0                  ; Check if memory allocated
        BEQ     memff$l                 ; Return if failed to allocate memory
        STR     r2, [r4, #device_fat]   ; Store pointer to the FAT
        LDR     r3, ws_device_total_clusters; Total number of clusters
        ADD     r3, r3, #2              ; Number of FAT entries
        MOV     r3, r3, LSL#2           ; Required size of inverse FAT
        BL      dynamic_osmodule        ; Attempt to allocate the memory
        MOVVS   r2, #0                  ; Pointer not valid if failed
        TEQ     r2, #0                  ; Check if memory allocated
        BEQ     memfi$l                 ; Return if failed to allocate memory
        STR     r2, [r4, #device_inverse_fat]; Store pointer to inverse FAT
        LDR     r3, ws_device_root_directory_sectors; Root directory sectors
        MOV     r3, r3, LSL#sector_shift; Size of root directory
        BL      dynamic_osmodule        ; Attempt to allocate the memory
        MOVVS   r2, #0                  ; Pointer not valid if failed
        TEQ     r2, #0                  ; Check if memory allocated
        BEQ     memfr$l                 ; Return if failed to allocate memory
        STR     r2, [r4, #device_root]  ; Store pointer to root directory
        MOV     r2, r4                  ; Restore the device record pointer
        B       memd$l                  ; All finished so return
memfr$l MOV     r0, #OSModule_Free      ; Reason code to release memory
        LDR     r2, [r4, #device_inverse_fat]; Get pointer to inverse FAT
        BL      dynamic_osmodule        ; Free this block
memfi$l MOV     r0, #OSModule_Free      ; Reason code to release memory
        LDR     r2, [r4, #device_fat]   ; Get pointer to FAT
        BL      dynamic_osmodule        ; Free this block
memff$l MOV     r0, #OSModule_Free      ; Reason code to release memory
        MOV     r2, r4                  ; Restore the device record pointer
        BL      dynamic_osmodule        ; Free this block
        MOV     r2, #0                  ; Mark the failure
memd$l  RTSS                            ; Return from subroutine
canon$l JSR     "r0-r5"                 ; Stack registers
        MOV     r0, #OSFSControl_CanonicalisePath; Reason code to canonicalise
        MOV     r1, r4                  ; Pointer to pathname
        ADD     r2, r2, #device_path    ; Pointer to output buffer
        MOV     r3, #0                  ; Do not use a path variable
        MOV     r4, #0                  ; Do not use a path string
        MOV     r5, #String             ; Size of buffer
        SWI     XOS_FSControl           ; Canonicalise the path
        SUB     r2, r2, #device_path    ; Restore the device record pointer
        MOV     r4, r1                  ; Restore the source path pointer
        BLVS    copy$l                  ; Perform a simple copy if failed
        RTSS                            ; Return from subroutine
copy$l  JSR     "r0, r2, r4"            ; Stack registers
        ADD     r2, r2, #device_path    ; Pointer to output buffer
copyl$l LDRB    r0, [r4], #1            ; Read source byte
        CMP     r0, #' '                ; Is it a terminator
        MOVLE   r0, #0                  ; Use a null terminator
        STRB    r0, [r2], #1            ; Store this character
        BGT     copyl$l                 ; Loop if not finished
        RTSS                            ; Return from subroutine
size$l  JSR     "r0-r3"                 ; Stack registers
        MOV     r0, r0, LSL#(20 - sector_shift); Convert size to sectors
        MOV     r1, #min_cluster_shift - 1; Try the minimum cluster shift first
sizez$l ADD     r1, r1, #1              ; Increment the cluster shift
        MOV     r2, r0, LSR r1          ; Calculate number of clusters
        MOV     r2, r2, LSR#(sector_shift - 1); Number of sectors per FAT
        CMP     r1, #max_cluster_shift  ; Can the shift be increased further
        BHS     sized$l                 ; No point checking further if not
        CMP     r2, #max_fat_sectors    ; Are there too many FAT sectors
        BHI     sizez$l                 ; Loop until small enough
        CMP     r2, #min_fat_sectors << 1; Can the FAT size be reduced further
        BLO     sized$l                 ; Exit the loop if FAT is small enough
        MOV     r3, #4                  ; Value to shift
        MOV     r3, r3, LSL r1          ; Maximum sectors per FAT for this size
        CMP     r2, r3                  ; Is the size suitable
        BHI     sizez$l                 ; Try again if too large
sized$l CMP     r2, #min_fat_sectors    ; Are there enough sectors per FAT
        MOVLO   r2, #min_fat_sectors    ; Use at least the minimum number
        CMP     r2, #max_fat_sectors    ; Are there too many sectors per FAT
        MOVHI   r2, #max_fat_sectors    ; Use at most the maximum number
        MOV     r0, r2, LSL#(sector_shift - 1); Number of FAT entries
        LDR     r3, = fat_reserved_min - 1; Maximum allowed cluster number
        CMP     r0, r3                  ; Compare to maximum allowed
        MOVHS   r0, r3                  ; Place an upper limit on the clusters
        SUB     r0, r0, #2              ; Remove the initial wasted entries
        STR     r0, ws_device_total_clusters; Store number of clusters
        STR     r1, ws_device_cluster_shift; Store sectors per cluster shift
        STR     r2, ws_device_sectors_per_fat; Store number of sectors per FAT
        MOV     r0, #1                  ; Value to shift
        MOV     r0, r0, LSL r1          ; Double number of root sectors
        MOV     r0, r0, LSR#1           ; Number of root directory sectors
        STR     r0, ws_device_root_directory_sectors; Store root sectors
        LDR     r0, ws_device_sectors_per_fat; Number of sectors per FAT
        MOV     r1, #number_fats        ; Number of copies of FAT
        MUL     r0, r1, r0              ; Number of sectors used for FATs
        ADD     r0, r0, #reserved_sectors; Sectors before root directory
        LDR     r1, ws_device_root_directory_sectors; Sectors in root directory
        ADD     r0, r0, r1              ; Calculate offset to first data sector
        STR     r0, ws_device_skip_sectors; Store offset to first data sector
        LDR     r1, ws_device_total_clusters; Get total number of clusters
        LDR     r2, ws_device_cluster_shift; Get sectors per cluster shift
        ADD     r0, r0, r1, LSL r2      ; Calculate total number of sectors
        STR     r0, ws_device_total_sectors; Store total number of sectors
        MOV     r0, #0                  ; Value to clear boot sector with
        ADR     r1, ws_device_boot      ; Pointer to start of boot sector
        ADD     r2, r1, #boot           ; Number of bytes to clear
sizec$l STRB    r0, [r1], #1            ; Clear the next byte
        TEQ     r1, r2                  ; Have all bytes been cleared
        BNE     sizec$l                 ; Keep looping until finished
        ADRL    r1, jmp$l               ; Pointer to jump instruction
        ADR     r2, ws_device_boot + boot_jmp; Pointer to destination buffer
        ADD     r3, r1, #?jmp$l + ?oem$l; End of source data
sizej$l LDRB    r0, [r1], #1            ; Read byte from the source
        STRB    r0, [r2], #1            ; Store buffer in destination
        TEQ     r1, r3                  ; Has the end been reached
        BNE     sizej$l                 ; Keep looping until jump copied
        ADRL    r1, ws_device_boot + boot_bpb; Pointer to BIOS parameter block
        MOV     r0, #bytes_per_sector   ; Number of bytes per sector
        STRB    r0, [r1, #bpb_bytes_per_sector]; Store the LSB
        MOV     r0, r0, LSR#8           ; Obtain the most significant byte
        STRB    r0, [r1, #bpb_bytes_per_sector + 1]; Store the MSB
        LDR     r0, ws_device_cluster_shift; Shift from sectors to cluster
        MOV     r2, #1                  ; Calculate for single cluster
        MOV     r0, r2, LSL r0          ; Number of sectors per allocation unit
        STRB    r0, [r1, #bpb_sectors_per_unit]; Store the value
        MOV     r0, #reserved_sectors   ; Number of reserved sectors
        STRB    r0, [r1, #bpb_reserved_sectors]; Store the LSB
        MOV     r0, r0, LSR#8           ; Obtain the most significant byte
        STRB    r0, [r1, #bpb_reserved_sectors + 1]; Store the MSB
        MOV     r0, #number_fats        ; Only a single FAT
        STRB    r0, [r1, #bpb_fats]     ; Store number of file allocation tables
        LDR     r0, ws_device_root_directory_sectors; Sectors for root directory
        MOV     r0, r0, LSL#sector_shift; Bytes per root directory
        MOV     r0, r0, LSR#directory_shift; Number of root directory entries
        STRB    r0, [r1, #bpb_root_entries]; Store the LSB
        MOV     r0, r0, LSR#8           ; Obtain the most significant byte
        STRB    r0, [r1, #bpb_root_entries + 1]; Store the MSB
        LDR     r0, ws_device_total_sectors; Total number of sectors
        CMP     r0, #1 << 16            ; Is it too large for a word value
        MOVHS   r0, #0                  ; Use a value of 0 if too large
        STRB    r0, [r1, #bpb_sectors]  ; Store the LSB
        MOV     r0, r0, LSR#8           ; Obtain the most significant byte
        STRB    r0, [r1, #bpb_sectors + 1]; Store the MSB
        MOV     r0, #media_byte         ; Media descriptor byte
        STRB    r0, [r1, #bpb_media]    ; Store the media descriptor byte
        LDR     r0, ws_device_sectors_per_fat; Number of sectors per FAT
        STRB    r0, [r1, #bpb_sectors_per_fat]; Store the LSB
        MOV     r0, r0, LSR#8           ; Obtain the most significant byte
        STRB    r0, [r1, #bpb_sectors_per_fat + 1]; Store the MSB
        LDR     r2, ws_device_total_sectors; Total number of sectors
        MOV     r1, #max_cylinders      ; Maximum number of cylinders
        DivRem  r3, r2, r1, r0          ; Calculate sectors per cylinder
        TEQ     r1, #0                  ; Was there any remainder
        ADDNE   r3, r3, #1              ; Add an extra sector if required
        MOV     r0, #max_sectors_per_track; Assume maximum sectors per track
        DivRem  r1, r3, r0, r2          ; Calculate number of heads
        TEQ     r3, #0                  ; Was there any remainder
        ADDNE   r1, r1, #1              ; Add an extra head if required
        STRB    r0, ws_device_boot + boot_sectors_per_track; Store LSB
        MOV     r0, r0, LSR#8           ; Obtain the most significant byte
        STRB    r0, ws_device_boot + boot_sectors_per_track + 1; Store MSB
        STRB    r1, ws_device_boot + boot_heads; Store the LSB
        MOV     r1, r1, LSR#8           ; Obtain the most significant byte
        STRB    r1, ws_device_boot + boot_heads + 1; Store the MSB
        MOV     r0, #hidden_sectors     ; Number of hidden sectors
        STRB    r0, ws_device_boot + boot_hidden_sectors; Store the LSB
        MOV     r0, r0, LSR#8           ; Obtain the next byte
        STRB    r0, ws_device_boot + boot_hidden_sectors + 1; Store next byte
        MOV     r0, r0, LSR#8           ; Obtain the next byte
        STRB    r0, ws_device_boot + boot_hidden_sectors + 2; Store next byte
        MOV     r0, r0, LSR#8           ; Obtain the most significant byte
        STRB    r0, ws_device_boot + boot_hidden_sectors + 3; Store the MSB
        LDR     r0, ws_device_total_sectors; Total number of sectors
        STRB    r0, ws_device_boot + boot_large_sectors; Store the LSB
        MOV     r0, r0, LSR#8           ; Obtain the next byte
        STRB    r0, ws_device_boot + boot_large_sectors + 1; Store the next byte
        MOV     r0, r0, LSR#8           ; Obtain the next byte
        STRB    r0, ws_device_boot + boot_large_sectors + 2; Store the next byte
        MOV     r0, r0, LSR#8           ; Obtain the most significant byte
        STRB    r0, ws_device_boot + boot_large_sectors + 3; Store the MSB
        MOV     r0, #physical_drive     ; The physical drive number
        STRB    r0, ws_device_boot + boot_physical; Store the LSB
        MOV     r0, r0, LSR#8           ; Obtain the most significant byte
        STRB    r0, ws_device_boot + boot_physical + 1; Store the MSB
        MOV     r0, #extended_signature ; Identify the extended boot record
        STRB    r0, ws_device_boot + boot_signature; Store the boot signature
        LDR     r0, = volume_serial     ; The volume serial number
        STRB    r0, ws_device_boot + boot_serial_number; Store the LSB
        MOV     r0, r0, LSR#8           ; Obtain the next byte
        STRB    r0, ws_device_boot + boot_serial_number + 1; Store the next byte
        MOV     r0, r0, LSR#8           ; Obtain the next byte
        STRB    r0, ws_device_boot + boot_serial_number + 2; Store the next byte
        MOV     r0, r0, LSR#8           ; Obtain the most significant byte
        STRB    r0, ws_device_boot + boot_serial_number + 3; Store the MSB
        ADR     r1, label$l             ; Pointer to disc label
        ADRL    r2, ws_device_boot + boot_label; Pointer to destination buffer
        ADD     r3, r1, #?label$l + ?fs$l; End of source data
sizef$l LDRB    r0, [r1], #1            ; Read byte from the source
        STRB    r0, [r2], #1            ; Store buffer in destination
        TEQ     r1, r3                  ; Has the end been reached
        BNE     sizef$l                 ; Keep looping until label copied
        MOV     r0, #end_boot_sector1   ; First end of sector marker
        STRB    r0, ws_device_boot + boot_end1; Store the first marker
        MOV     r0, #end_boot_sector2   ; Second end of sector marker
        STRB    r0, ws_device_boot + boot_end2; Store the second marker
        RTSS                            ; Return from subroutine

        ; Keyword definition
key$l   =       "/A,limit/K/E,long/S,write/S,manual/S,anon/S,size/K/E,/A,,,", 0

        ; Parts of the device boot sector
jmp$l   =       0, 0, 0                 ; No jump to bootstrap
oem$l   =       "ARMEdit1"              ; OEM name and version
label$l =       "ARMEditDisc"           ; Volume label
fs$l    =       "FAT16  "               ; File system ID
        ALIGN

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - Pointer to destination buffer.
        ;   Returns     : r1    - Pointer to buffer, or 0 if not performed.
        ;   Description : Canonicalise the device path to the specified buffer.
        ;                 If another canonicalisation was performed recently
        ;                 then this just clears the buffer pointer and returns.
device_canonicalise
        LocalLabels
        JSR     "r2-r3"                 ; Stack registers
        LDR     r2, [r0, #device_canon_last]; Time of last canonicalisation
        LDR     r3, [r0, #device_canon_time]; Time to perform canonicalisation
        ADD     r3, r2, r3, ASL#2       ; Earliest time for next one
        BL      time$l                  ; Read the current time
        CMP     r2, r3                  ; Has sufficient time past
        MOVMI   r1, #0                  ; Clear pointer if too soon
        RTSS MI                         ; Return from subroutine if too soon
        MOV     r3, r2                  ; Copy the time before canonicalisation
        BL      canon$l                 ; Try to canonicalise the device again
        BL      time$l                  ; Read time after canonicalisation
        STR     r2, [r0, #device_canon_last]; Store last canonicalisation time
        SUB     r2, r2, r3              ; Calculate time taken
        STR     r2, [r0, #device_canon_time]; Store time taken
        RTSS                            ; Return from subroutine
canon$l JSR     "r0-r5"                 ; Stack registers
        MOV     r2, r1                  ; Copy destination buffer pointer
        ADD     r1, r0, #device_path    ; Pointer to the device path
        MOV     r3, #0                  ; No path variable
        MOV     r4, #0                  ; No path string
        MOV     r5, #String             ; Size of destination buffer
        MOV     r0, #OSFSControl_CanonicalisePath; Reason code to canonicalise
        SWI     XOS_FSControl           ; Canonicalise the path
        MOV     r0, #0                  ; Terminator character
        STRVSB  r0, [r2]                ; Blank result if error produced
        RTSS                            ; Return from subroutine
time$l  JSR     "r0"                    ; Stack registers
        SWI     XOS_ReadMonotonicTime   ; Read the number of centiseconds
        MOV     r2, r0                  ; Copy the monotonic time since reset
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the device record.
        ;   Returns     : None
        ;   Description : Update the boot sector with the correct volume label
        ;                 for the specified device.
device_boot_update
        LocalLabels
        JSR     "r1-r2"                 ; Stack registers
        LDR     r1, [r0, #device_flags] ; Get the flags for this device
        TST     r1, #device_flag_anon   ; Should anonymouse name be used
        BLNE    anon$l                  ; Use default name if it should
        BLEQ    can$l                   ; Attempt to canonicalise the path
        BL      find$l                  ; Find the required part of the name
        BL      str$l                   ; Convert and store the disc name
        RTSS                            ; Return from subroutine
anon$l  JSR     "r0, r2-r3"             ; Stack registers
        ADR     r2, def$l               ; Default volume label
        ADD     r1, r0, #device_canon   ; Pointer to canonicalised path
        MOV     r3, r1                  ; Copy pointer to destination
anonl$l LDRB    r0, [r2], #1            ; Read character from source
        STRB    r0, [r3], #1            ; Write to destination
        TEQ     r0, #0                  ; Is it a terminator
        BNE     anonl$l                 ; Loop until finished
        RTSS                            ; Return from subroutine
can$l   JSR     "r0, r2"                ; Stack registers
        ADD     r1, r0, #device_canon   ; Pointer to canonicalised path
        LDRB    r2, [r1]                ; Read first character of path
        TEQ     r2, #0                  ; Is path already canonicalised
        RTSS NE                         ; No further action if already done
        BL      device_canonicalise     ; Attempt to canonicalise the path
        TEQ     r1, #0                  ; Was it successful
        BEQ     canf$l                  ; Revert to the raw path if not
        LDRB    r2, [r1]                ; Read first character of path
        TEQ     r2, #0                  ; Is path successfully canonicalised
        RTSS NE                         ; No further action if successful
canf$l  ADD     r1, r0, #device_path    ; Pointer to the original device path
        RTSS                            ; Return from subroutine
str$l   JSR     "r0-r1, r3-r5"          ; Stack registers
        ADRL    r3, ws_device_boot + boot_label; Pointer to destination buffer
        ADD     r4, r3, #?boot_label    ; First character after destination
        LDR     r5, [r0, #device_flags] ; Get the flags for this device
strl$l  TEQ     r3, r4                  ; Has end of buffer been reached
        RTSS EQ                         ; Return from subroutine if finished
        TEQ     r1, r2                  ; Has end of source been reached
        BEQ     strd$l                  ; Pad to the end if it has
        LDRB    r0, [r1], #1            ; Read next source character
        TEQ     r0, #0                  ; Is it a terminator
        BEQ     strd$l                  ; Pad to the end if it is
        BL      dosmap_char_riscos_to_win95; Convert to a unicode character
        TST     r5, #device_flag_long   ; Are long filenames enabled
        BLNE    dosmap_char_win95_to_dos; Perform Windows 95 conversion
        BLEQ    dosmap_char_win95_to_dos_strict; Perform strict conversion
        TEQ     r0, #0                  ; Should the character be skipped
        STRNEB  r0, [r3], #1            ; Store the converted character
        B       strl$l                  ; Loop for the next character
strd$l  MOV     r0, #' '                ; Pad with space to the end
strp$l  TEQ     r3, r4                  ; Has end of buffer been reached
        RTSS EQ                         ; Return from subroutine if finished
        STRB    r0, [r3], #1            ; Store this character
        B       strp$l                  ; Loop until padded to the end
find$l  JSR     "r0, r3-r4"             ; Stack registers
        MOV     r2, r1                  ; Start from the start of the path
        MOV     r3, #0                  ; Initially no colon found
findc$l LDRB    r0, [r2], #1            ; Read next character from path
        TEQ     r0, #0                  ; Is it the terminator
        BEQ     finde$l                 ; Exit loop if finished check
        TEQ     r0, #':'                ; Is it a colon
        MOVEQ   r3, r2                  ; Copy pointer if it is
        B       findc$l                 ; Loop until finished
finde$l TEQ     r3, #0                  ; Was a colon found
        BEQ     findf$l                 ; Use complete path if not
        MOV     r4, r3                  ; Start from after the last colon
findd$l LDRB    r0, [r4], #1            ; Read next character from the path
        TEQ     r0, #0                  ; Is it a terminator
        TEQNE   r0, #'.'                ; Is it a period
        BNE     findd$l                 ; Loop until finished
        SUB     r4, r4, #1              ; Back to the final character
        MOV     r2, r3                  ; Copy pointer to start of check
findn$l TEQ     r2, r4                  ; Have all suitable characters failed
        BEQ     findf$l                 ; Use simple algorithm if they have
        LDRB    r0, [r2], #1            ; Read next character
        CMP     r0, #'0'                ; Compare to the first digit
        BLT     findo$l                 ; Accept if less than a digit
        CMP     r0, #'9'                ; Compare to the last digit
        BGT     findo$l                 ; Accept if more than a digit
        B       findn$l                 ; Loop until all checked
findf$l MOV     r2, #0                  ; Last resort is complete path specified
        RTSS                            ; Return from subroutine
findo$l MOV     r1, r3                  ; Copy pointer to start of name
        MOV     r2, r4                  ; Copy pointer to end of name
        RTSS                            ; Return from subroutine
def$l   =       "ARMEditDisc", 0        ; Default volume label
        ALIGN

; A literal pool

        LTORG

        ;   Parameters  : r0    - Reason code.
        ;   Returns     : Depends upon reason code.
        ;   Description : UpCallV vector handler.
device_upcallv
        LocalLabels
        TEQ     r0, #UpCall_FileModified; Is a file being modified
        MOVNES  pc, r14                 ; Pass on to previous if not
        JSR     "r0"                    ; Stack registers
        LDR     r0, ws_device_head      ; Pointer to first device record
        TEQ     r0, #0                  ; Are any devices active
        RTSS EQ                         ; No further checks if not
        LDR     r0, = UpCallFind_Close  ; Closing file
        TEQ     r9, r0                  ; Does the reason code match
        BLEQ    hand$l                  ; Check file details by file handle
        TEQ     r9, #UpCallFile_Save    ; Saving memory to a file
        TEQNE   r9, #UpCallFile_SetArgs ; Writing catalogue information
        TEQNE   r9, #UpCallFile_SetLoadAddr; Writing load address only
        TEQNE   r9, #UpCallFile_SetExecAddr; Writing execution address only
        TEQNE   r9, #UpCallFile_SetAttr ; Writing attributes only
        TEQNE   r9, #UpCallFile_Delete  ; Deleting file
        TEQNE   r9, #UpCallFile_Create  ; Creating empty file
        TEQNE   r9, #UpCallFile_CreateDir; Creating directory
        LDR     r0, = UpCallFind_CreateOpenup; Creating and opening for update
        TEQNE   r9, r0                  ; Does the reason code match
        LDR     r0, = UpCallFind_Openup ; Opening for update
        TEQNE   r9, r0                  ; Does the reason code match
        TEQNE   r9, #UpCallFSControl_Rename; Renaming file
        LDR     r0, = UpCallFSControl_SetAttrString; Setting attributes
        TEQNE   r9, r0                  ; Does the reason code match
        BLEQ    name$l                  ; Check file details by object name
        RTSS                            ; Pass on to previous
hand$l  JSR     "r0-r2, r5"             ; Stack registers
        SUB     r13, r13, #String       ; Claim space on the stack
        MOV     r0, #OSArgs_ReadPath    ; Reason code to convert to a path
        MOV     r2, r13                 ; Pointer to buffer on the stack
        MOV     r5, #String             ; Size of the string buffer
        SWI     XOS_Args                ; Read the file path
        MOV     r0, r13                 ; Copy the buffer pointer
        BLVC    full$l                  ; Check the full filename
        ADD     r13, r13, #String       ; Restore stack pointer
        RTSS                            ; Return from subroutine
name$l  JSR     "r0-r4"                 ; Stack registers
        SUB     r13, r13, #String       ; Claim space on the stack
        MOV     r4, r1                  ; Copy filename pointer
        MOV     r0, #OSFSControl_ReadFSName; Reason code to convert to name
        AND     r1, r8, #&ff            ; Extract the filesystem number
        MOV     r2, r13                 ; Pointer to buffer on the stack
        MOV     r3, #String             ; Size of the string buffer
        SWI     XOS_FSControl           ; Convert number to filesystem name
        BVS     namef$l                 ; Fail if an error was produced
        MOV     r1, r13                 ; Copy the buffer pointer
        ADD     r2, r1, #String         ; Pointer to end of buffer
        LDRB    r0, [r1]                ; Check the first character of result
        TEQ     r0, #0                  ; Is it a terminator
        BEQ     namef$l                 ; Fail if the result is blank
namec$l LDRB    r0, [r1, #1]!           ; Read next character from name
        TEQ     r0, #0                  ; Is it a terminator
        BNE     namec$l                 ; Loop for the next character
        MOV     r0, #':'                ; A colon character
        STRB    r0, [r1], #1            ; Store the colon in the buffer
namep$l TEQ     r1, r2                  ; Is the buffer full
        BEQ     namef$l                 ; Fail if buffer too small
        LDRB    r0, [r4], #1            ; Read character from source path
        STRB    r0, [r1], #1            ; Store character in destination buffer
        TEQ     r0, #0                  ; Is it a terminator
        BNE     namep$l                 ; Loop until finished or failed
        MOV     r0, r13                 ; Copy the buffer pointer
        BL      full$l                  ; Check the full filename
namef$l ADD     r13, r13, #String       ; Restore stack pointer
        RTSS                            ; Return from subroutine
full$l  JSR     "r0-r5"                 ; Stack registers
        MOV     r1, r0                  ; Copy the pathname pointer
        LDR     r0, ws_device_head      ; Pointer to first device driver
fulll$l TEQ     r0, #0                  ; Is the device record valid
        RTSS EQ                         ; Return from subroutine when finished
        LDR     r2, [r0, #device_flags] ; Get the flags for this device
        TST     r2, #device_flag_manual ; Is automatic relogging disabled
        BNE     fulln$l                 ; Skip this device if it is
        LDR     r2, [r0, #device_objects]; Pointer to the first logged object
        TEQ     r2, #0                  ; Are there any logged objects
        BEQ     fulln$l                 ; No action required if no objects
        LDRB    r2, [r0, #device_canon] ; First character of canonicalised path
        TEQ     r2, #0                  ; Has path been canonicalised
        BEQ     fulln$l                 ; No action if not canonicalised
        ADD     r2, r0, #device_canon   ; Root path for this device
        MOV     r3, r1                  ; Copy the pathname pointer again
fullp$l LDRB    r4, [r2], #1            ; Read character from device path
        LDRB    r5, [r3], #1            ; Read character from
        TEQ     r4, #0                  ; Is it a terminator
        TEQNE   r5, #0                  ; Is other one a terminator
        BEQ     fullm$l                 ; Accept as a match if either finished
        Lower   r4                      ; Convert first character to lower case
        Lower   r5                      ; Convert second one to lower case
        TEQ     r4, r5                  ; Do the characters match
        BEQ     fullp$l                 ; Check the next characters if they do
        B       fulln$l                 ; No action if paths do not match
fullm$l LDR     r2, [r0, #device_flags] ; Get the flags for this device
        ORR     r2, r2, #device_flag_relog_pending; Set the pending flag
        STR     r2, [r0, #device_flags] ; Store the updated flags
fulln$l LDR     r0, [r0, #device_next]  ; Pointer to the next device record
        B       fulll$l                 ; Loop for the next record

        ;   Parameters  : r0    - Unit code (drive number), or -1 for
        ;                         initialisation.
        ;   Returns     : r0    - Changed code:
        ;                           -1  Disc has been changed.
        ;                           0   Don't know if disc has been changed.
        ;                           1   Disc has not been changed.
        ;   Description : Check if the device has been changed
device_changed
        LocalLabels
        JSR     "r1-r2"                 ; Stack registers
        pipe_string "device_changed: unit %0", r0
        BL      device_find             ; Find the device record
        TEQ     r0, #0                  ; Was the record found
        RTSS EQ                         ; Exit if not found
        BL      device_object_close     ; Close any open files
        LDR     r1, [r0, #device_flags] ; Get device flags
        MOV     r2, #-1                 ; Assume device has been changed
        TST     r1, #device_flag_relog_forced; Was a relog forced
        BNE     done$l                  ; Device changed if relog forced
        MOV     r2, #0                  ; Assume device may have changed
        TST     r1, #device_flag_relog_pending; Is a relog required
        TSTEQ   r1, #device_flag_changed; Has directory structure changed
        BNE     done$l                  ; Device could have changed
        TST     r1, #device_flag_manual ; Is automatic relogging disabled
        BNE     man$l                   ; If manual relogging then skip next bit
        BL      canon$l                 ; Attempt to canonicalise the path
        BEQ     man$l                   ; Skip the next bit if failed
        BL      cmp$l                   ; Compare the device paths
        ORRNE   r1, r1, #device_flag_relog_pending; Trigger a relog if changed
        BNE     done$l                  ; Device could have changed
man$l   MOV     r2, #1                  ; Otherwise disc has not changed
done$l  STR     r1, [r0, #device_flags] ; Store the modified flags
        MOV     r0, r2                  ; Copy changed code
        pipe_string "device_changed: -> %0", r0
        RTSS                            ; Return from subroutine
canon$l JSR     "r1"                    ; Stack registers
        ADRL    r1, ws_buffer           ; Pointer to destination buffer
        BL      device_canonicalise     ; Attempt to canonicalise the path
        TEQ     r1, #0                  ; Check whether operation performed
        RTS                             ; Return from subroutine
cmp$l   JSR     "r0-r3"                 ; Stack registers
        ADD     r2, r0, #device_canon   ; Pointer to previous result
        ADRL    r3, ws_buffer           ; Pointer to new result
cmpl$l  LDRB    r0, [r2], #1            ; Read character from previous result
        LDRB    r1, [r3], #1            ; Read character from new result
        TEQ     r0, r1                  ; Do the characters match
        RTS NE                          ; Return if different
        TEQ     r0, #0                  ; Is it a terminator
        BNE     cmpl$l                  ; Loop until finished
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - Unit code (drive number).
        ;   Returns     : None
        ;   Description : Increment the reference count of open files.
device_open
        LocalLabels
        JSR     "r0-r1"                 ; Stack registers
        pipe_string "device_open: unit %0", r0
        BL      device_find             ; Find the device record
        TEQ     r0, #0                  ; Was the record found
        RTSS EQ                         ; Exit if not found
        LDR     r1, [r0, #device_open_files]; Read previous number of open files
        ADD     r1, r1, #1              ; Increment the number of files
        STR     r1, [r0, #device_open_files]; Store the updated number of files
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Unit code (drive number).
        ;   Returns     : None
        ;   Description : Decrement the reference count of open files.
device_close
        LocalLabels
        JSR     ""                      ; Stack registers
        pipe_string "device_close: unit %0", r0
        BL      device_find             ; Find the device record
        TEQ     r0, #0                  ; Was the record found
        RTSS EQ                         ; Exit if not found
        LDR     r1, [r0, #device_open_files]; Read previous number of open files
        SUBS    r1, r1, #1              ; Decrement the number of files
        STRPL   r1, [r0, #device_open_files]; Store the updated number of files
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Unit code (drive number).
        ;   Returns     : r0    - Removable device status:
        ;                           &0000   Removable.
        ;                           &2000   Nonremovable.
        ;   Description : Decrement the reference count of open files.
device_removable
        LocalLabels
        JSR     ""                      ; Stack registers
        pipe_string "device_removable: unit %0", r0
        MOV     r0, #0                  ; All devices are removable
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the device record.
        ;   Returns     : None
        ;   Description : Prepare a device record.
device_prepare
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        MOV     r1, #0                  ; Value to clear pointers with
        STR     r1, [r0, #device_sectors]; Clear sector list pointer
        STR     r1, [r0, #device_objects]; Clear object list pointer
        STRB    r1, [r0, #device_canon] ; Clear the canonicalised filename
        STR     r1, [r0, #device_canon_time]; Assume path canonicalisation quick
        BL      time$l                  ; Read the time
        STR     r1, [r0, #device_canon_last]; Make this the last update
        BL      device_fat_initialise   ; Initialise the FAT entries
        LDR     r1, [r0, #device_root]  ; Pointer to root directory
        MOV     r2, #directory_unused   ; Unused entry value
        STRB    r2, [r1]                ; Initialise root directory
        RTSS                            ; Return from subroutine
time$l  JSR     "r0"                    ; Stack registers
        SWI     XOS_ReadMonotonicTime   ; Read the number of centiseconds
        MOV     r1, r0                  ; Copy the monotonic time since reset
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the device record.
        ;   Returns     : None
        ;   Description : Initialise the FAT and inverse FAT.
device_fat_initialise
        LocalLabels
        JSR     "r0-r3"                 ; Stack registers
        LDR     r1, ws_device_total_clusters; Total number of clusters
        ADD     r1, r1, #2              ; Number of FAT entries
        LDR     r2, = fat_eof_min       ; Mark end of chain
        MOV     r3, #0                  ; Value to clear inverse FAT entry with
loop$l  SUBS    r1, r1, #1              ; Decrement number of clusters
        BMI     done$l                  ; Exit loop when finished
        BL      device_fat_write        ; Clear this entry of the FAT
        MOV     r2, r1                  ; Copy current cluster number to link
        B       loop$l                  ; Loop for next cluster
done$l  LDR     r2, [r0, #device_fat]   ; Pointer to start of FAT
        LDR     r1, = media_byte :OR: &ffffff00; Media descriptor byte
        STR     r1, [r2]                ; Store at start of FAT
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - Cluster number.
        ;   Returns     : r2    - The value read from the FAT.
        ;                 r3    - The value read from the inverse FAT.
        ;   Description : Read an entry from the FAT for this device.
device_fat_read
        LocalLabels
        JSR     "r4"                    ; Stack registers
        LDR     r4, [r0, #device_fat]   ; Pointer to start of FAT
        ADD     r4, r4, r1, LSL#1       ; Add offset to required entry
        LDRB    r2, [r4]                ; Extract least significant byte
        LDRB    r4, [r4, #1]            ; Extract most significant byte
        ORR     r2, r2, r4, LSL#8       ; Combine the two bytes
        LDR     r4, [r0, #device_inverse_fat]; Pointer to inverse FAT
        LDR     r3, [r4, r1, LSL#2]     ; Extract inverse FAT entry
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - Cluster number.
        ;                 r2    - Value to write to the FAT.
        ;                 r3    - Value to write to the inverse FAT.
        ;   Returns     : None
        ;   Description : Write an entry in the FAT for this device.
device_fat_write
        LocalLabels
        JSR     "r2, r4"                ; Stack registers
        LDR     r4, [r0, #device_fat]   ; Pointer to start of FAT
        ADD     r4, r4, r1, LSL#1       ; Add offset to required entry
        STRB    r2, [r4]                ; Store least significant byte
        MOV     r2, r2, LSR#8           ; Extact most significant byte
        STRB    r2, [r4, #1]            ; Store most significant byte
        LDR     r4, [r0, #device_inverse_fat]; Pointer to inverse FAT
        STR     r3, [r4, r1, LSL#2]     ; Store inverse FAT entry
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - Number of clusters required.
        ;                 r2    - Required inverse FAT entry.
        ;   Returns     : r1    - Number of first cluster, or 0 if failed.
        ;   Description : Attempt to allocate a cluster chain of the requested
        ;                 length. If not possible then no clusters are
        ;                 reserved.
device_fat_allocate
        LocalLabels
        [       {FALSE}
        JSR     "r0, r2-r5"             ; Stack registers
        MOV     r4, r1                  ; Copy required number of clusters
        MOV     r3, r2                  ; Copy inverse FAT entry
        MOV     r5, #0                  ; Initialise number available
        MOV     r1, #0                  ; Start from the beginning
count$l BL      next$l                  ; Find the next available cluster
        TEQ     r1, #0                  ; Was a free cluster found
        BEQ     done$l                  ; Exit loop if not
        ADD     r5, r5, #1              ; Increment counter
        B       count$l                 ; Loop for next free cluster
done$l  CMP     r5, r4                  ; Are sufficient clusters available
        RTSS LO                         ; Exit if insufficient
        LDR     r2, = fat_eof_min       ; Mark first cluster as the end
loop$l  SUBS    r4, r4, #1              ; Decrement number of clusters
        RTSS MI                         ; Exit if finished
        BL      next$l                  ; Find the next sector
        BL      device_fat_write        ; Chain this cluster
        MOV     r2, r1                  ; Copy cluster number
        B       loop$l                  ; Loop for the next cluster
next$l  JSR     "r0, r2-r3"             ; Stack registers
        CMP     r1, #2                  ; Is it a special cluster number
        MOVLO   r1, #1                  ; Skip to just before start
nextl$l ADD     r1, r1, #1              ; Advance to next cluster number
        LDR     r2, ws_device_total_clusters; Total number of clusters
        ADD     r2, r2, #2              ; Number of FAT entries
        CMP     r1, r2                  ; Have all clusters been checked
        BHS     nextf$l                 ; Exit loop if failed
        BL      device_fat_read         ; Read this FAT entry
        TEQ     r2, #fat_available      ; Is the cluster available
        BNE     nextl$l                 ; Loop for next cluster if not
        RTSS                            ; Return from subroutine
nextf$l MOV     r1, #0                  ; Indicate failure
        RTSS                            ; Return from subroutine
        |
        JSR     "r0, r2-r6"             ; Stack registers
        MOV     r4, r1                  ; Copy required number of clusters
        MOV     r5, r2                  ; Copy inverse FAT entry
        LDR     r6, ws_device_total_clusters; Total number of clusters
        ADD     r6, r6, #2              ; End of FAT
        MOV     r1, #1                  ; Start from first normal entry
find$l  ADD     r1, r1, #1              ; Advance cluster number
        BL      device_fat_read         ; Read the FAT entry
        TEQ     r3, #0                  ; Is the entry used
        BEQ     found$l                 ; Exit loop if not
        CMP     r1, r6                  ; Have all clusters been checked
        BLO     find$l                  ; Loop if not
fail$l  MOV     r1, #0                  ; Indicate failure
        RTSS                            ; Return from subroutine
found$l ADD     r2, r1, r4              ; Calculate last cluster required
        SUB     r2, r2, #1              ; Back to actual last used
        CMP     r2, r6                  ; Is it a valid cluster
        BHS     fail$l                  ; Fail if not
        MOV     r4, r1                  ; Copy start cluster
        MOV     r1, r2                  ; Copy end cluster
        LDR     r2, = fat_eof_min       ; Mark end of cluster chain
        MOV     r3, r5                  ; Copy inverse FAT entry
loop$l  BL      device_fat_write        ; Write this FAT entry
        MOV     r2, r1                  ; Copy cluster number
        SUB     r1, r1, #1              ; Back to previous cluster
        CMP     r1, r4                  ; Have all clusters been written
        BHS     loop$l                  ; Loop if not
        MOV     r1, r4                  ; Restore initial cluster number
        RTSS                            ; Return from subroutine
        ]

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - Cluster number.
        ;   Returns     : r1    - Pointer to the object record, or 0 if none.
        ;                 r2    - Cluster offset from start of object.
        ;   Description : Attempt to find the object and offset that the
        ;                 specified cluster corresponds to.
device_fat_decode
        LocalLabels
        JSR     "r0, r3-r5"             ; Stack registers
        MOV     r5, r1                  ; Copy initial cluster number
        BL      device_fat_read         ; Read the FAT entry
        TEQ     r2, #fat_available      ; Is the cluster used
        BEQ     none$l                  ; Skip next bit if not
        TEQ     r3, #0                  ; Is the object record valid
        BEQ     none$l                  ; Skip next bit if not
        MOV     r4, #0                  ; Cluster offset starts from zero
        LDR     r1, [r3, #device_object_start]; Start cluster number
loop$l  TEQ     r1, r5                  ; Does the cluster number match
        BEQ     done$l                  ; Exit loop if finished
        BL      device_fat_read         ; Read the FAT entry
        TEQ     r2, #fat_available      ; Is the entry used
        BEQ     none$l                  ; No match if not
        MOV     r1, r2                  ; Copy cluster number
        ADD     r4, r4, #1              ; Increment cluster offset
        B       loop$l                  ; Loop for the next cluster
done$l  TEQ     r3, #0                  ; Check the object record pointer again
        BEQ     none$l                  ; Not found if invalid
        MOV     r1, r3                  ; Copy object record pointer
        MOV     r2, r4                  ; Copy cluster offset
        RTSS                            ; Return from subroutine
none$l  MOV     r1, #0                  ; Clear object record pointer
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the device record.
        ;   Returns     : None
        ;   Description : Initialise the root directory for the device.
device_root_create
        LocalLabels
        JSR     "r0-r3"                 ; Stack registers
        BL      device_boot_update      ; Ensure that the boot sector is valid
        LDRB    r1, [r0, #device_canon] ; Read character of canonicalised path
        TEQ     r1, #0                  ; Is the character valid
        RTSS EQ                         ; No action if not valid
        LDR     r1, [r0, #device_flags] ; Get current flags
        ORR     r1, r1, #device_flag_changed; Set structure changed flags
        STR     r1, [r0, #device_flags] ; Store modified flags
        MOV     r1, #0                  ; Directory is the root
        MOV     r2, #0                  ; Start from the first entry
        BL      device_dir_entry        ; Obtain pointer to first entry
        BL      vol$l                   ; Write the volume label
        ADD     r2, r2, #1              ; Advance to the next entry
        BL      device_dir_build        ; Add all the objects
        RTSS                            ; Return from subroutine
vol$l   JSR     "r0-r3"                 ; Stack registers
        BL      device_object_zero      ; Clear any initial garbage data
        ADRL    r2, ws_device_boot + boot_label; Pointer to volume label
        MOV     r1, #name_length + extension_length; Number of characters
voll$l  SUBS    r1, r1, #1              ; Decrement number of characters
        BMI     vold$l                  ; Exit loop if finished
        LDRB    r0, [r2, r1]            ; Read character of volume label
        STRB    r0, [r3, r1]            ; Store character in directory entry
        B       voll$l                  ; Loop for next character
vold$l  MOV     r0, #attribute_volume   ; Attribute for volume label
        STRB    r0, [r3, #directory_attribute]; Store attribute
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - Pointer to the object record.
        ;   Returns     : None
        ;   Description : Initialise the specified subdirectory for the device.
device_subdir_create
        LocalLabels
        JSR     "r0-r4"                 ; Stack registers
        LDR     r2, [r0, #device_flags] ; Get current flags
        ORR     r2, r2, #device_flag_changed; Set structure changed flags
        STR     r2, [r0, #device_flags] ; Store modified flags
        BL      alloc$l                 ; Allocate cluster chain
        RTS VS                          ; Exit if error produced
        MOV     r2, #0                  ; Start from the first entry
        BL      device_dir_entry        ; Obtain pointer to first entry
        BL      dir$l                   ; Create first special directory
        MOV     r4, #'.'                ; Special directory character
        STRB    r4, [r3, #directory_name]; Store as the first character
        LDR     r4, [r1, #device_object_start]; Get cluster number
        STRB    r4, [r3, #directory_cluster]; Store LSB of initial cluster
        MOV     r4, r4, LSR#8           ; Shift to get other byte
        STRB    r4, [r3, #directory_cluster + 1]; Store MSB of initial cluster
        ADD     r2, r2, #1              ; Advance to the next entry
        BL      device_dir_entry        ; Obtain pointer to second entry
        BL      dir$l                   ; Create second special directory
        MOV     r4, #'.'                ; Special directory character
        STRB    r4, [r3, #directory_name]; Store as the first character
        STRB    r4, [r3, #directory_name + 1]; Store as the second character
        LDR     r4, [r1, #device_object_parent]; Get parent object pointer
        TEQ     r4, #0                  ; Is there a parent record
        LDRNE   r4, [r4, #device_object_start]; Get parent initial cluster
        STRB    r4, [r3, #directory_cluster]; Store LSB of initial cluster
        MOV     r4, r4, LSR#8           ; Shift to get other byte
        STRB    r4, [r3, #directory_cluster + 1]; Store MSB of initial cluster
        ADD     r2, r2, #1              ; Advance to the next entry
        BL      device_dir_build        ; Add all the objects
        RTSS                            ; Return from the subroutine
dir$l   JSR     "r0-r3"                 ; Stack registers
        BL      device_object_zero      ; Start by clearing the entry
        MOV     r0, #' '                ; Value to clear name with
        MOV     r1, #name_length + extension_length; Number of characters
dirl$l  SUBS    r1, r1, #1              ; Decrement number of characters
        BMI     dird$l                  ; Exit loop when finished
        STRB    r0, [r3, r1]            ; Clear next character
        B       dirl$l                  ; Loop for next character
dird$l  MOV     r0, #attribute_subdirectory; Attribute for subdirectory
        STRB    r0, [r3, #directory_attribute]; Store attribute
        RTSS                            ; Return from subroutine
alloc$l JSR     "r0-r2"                 ; Stack registers
        LDR     r1, [r1, #device_object_start]; Initial cluster number
        BL      device_sector_chain     ; Allocate the sector buffers
        TEQ     r2, #0                  ; Were the buffers allocated
        RTS NE                          ; Return from subroutine
        SetV                            ; Set error flag
        RTS                             ; Return if failed

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - Pointer to the directory object record, or
        ;                         0 for the root directory.
        ;                 r2    - Zero based index for the first directory
        ;                         entry to use.
        ;   Returns     : None
        ;   Description : Build the specified directory. Any memory allocation
        ;                 required should already have been performed.
device_dir_build
        LocalLabels
        JSR     "r0-r6"                 ; Stack registers
        MOV     r4, #0                  ; Context for first call
loop$l  BL      next$l                  ; Find the next object details
        BVS     done$l                  ; No more objects if error produced
        TEQ     r6, #0                  ; Was an object found
        BEQ     done$l                  ; No more objects if not found
        BL      do$l                    ; Handle this entry
        B       loop$l                  ; Loop for the next object
done$l  BL      device_dir_entry        ; Obtain a pointer to the entry
        TEQ     r3, #0                  ; Is the pointer valid
        RTSS EQ                         ; Exit if all finished
        BL      device_object_zero      ; Clear the directory entry
        MOV     r4, #directory_unused   ; Value to indicate entry unused
        STRB    r4, [r3, #directory_name]; Mark entry as unused
        ADD     r2, r2, #1              ; Advance to the next entry
        B       done$l                  ; Loop for the next entry
next$l  JSR     "r0-r3"                 ; Stack registers
        TEQ     r1, #0                  ; Is there an object record
        ADDNE   r1, r1, #device_object_name; Pointer to subdirectory search path
        ADDEQ   r1, r0, #device_path    ; Pointer to root directory search path
        MOV     r0, #OSGBPB_DirEntriesInfoStamped; Reason code to read entries
        ADRL    r2, ws_buffer           ; Pointer to a buffer
        MOV     r3, #1                  ; Only read a single object name
        MOV     r5, #String             ; Size of buffer
        MOV     r6, #0                  ; Include all files
        SWI     XOS_GBPB                ; Read the next directory entry
        RTS VS                          ; Exit if error produced
        MOV     r5, r1                  ; Copy object path
        TEQ     r3, #0                  ; Were any objects read
        MOVEQ   r6, #0                  ; Clear details pointer if not
        MOVNE   r6, r2                  ; Copy details pointer if valid
        RTS                             ; Return from subroutine
do$l    JSR     "r3-r4"                 ; Stack registers
        MOV     r4, r6                  ; Copy the buffer pointer
        BL      device_dir_add          ; Attempt to add this entry
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - Pointer to the parent directory object
        ;                         record, or 0 for the root directory.
        ;                 r2    - Zero based index for the first directory
        ;                         entry to use.
        ;                 r4    - Pointer to buffer filled by OS_GBPB 12.
        ;                 r5    - Pointer to path of the directory containing
        ;                         this object.
        ;   Returns     : r2    - Zero based index for the next directory
        ;                         entry to use.
        ;   Description : Add a new directory entry. This may use multiple
        ;                 directory entries to store a long filename. This also
        ;                 creates the required FAT and inverse FAT entries, and
        ;                 a corresponding object record.
device_dir_add
        LocalLabels
        JSR     "r0-r1, r3-r8"          ; Stack registers
        BL      new$l                   ; Allocate a new object record
        TEQ     r7, #0                  ; Was the memory allocated
        RTSS VS                         ; Exit if unable to allocate memory
        MOV     r6, r2                  ; Copy initial entry index
        BL      device_dir_entry        ; Obtain a pointer to the current entry
        TEQ     r3, #0                  ; Is the pointer valid
        BEQ     fail$l                  ; Fail if not a valid directory entry
        BL      pre$l                   ; Preprocess the object details
        BVS     fail$l                  ; Fail if an error produced
        BL      trans$l                 ; Generate the Windows 95 style name
        BL      dos$l                   ; Generate the DOS style name
        BL      long$l                  ; Store long filename if required
        BL      device_dir_entry        ; Obtain a pointer to the actual entry
        TEQ     r3, #0                  ; Is the pointer valid
        BNE     ok$l                    ; Skip the next bit if it is
        MOV     r2, r6                  ; As a last resort do not use long name
        BL      device_dir_entry        ; Obtain a pointer to the actual entry
        TEQ     r3, #0                  ; Is the pointer valid
        BEQ     fail$l                  ; Fail if not a valid directory entry
ok$l    BL      device_object_zero      ; Clear the directory entry
        BL      fill$l                  ; Complete the directory entry details
        BL      dos$l                   ; Regenerate the DOS style name
        BL      fat$l                   ; Allocate cluster chain
        BVS     fail$l                  ; Exit if unable to allocate chain
        STR     r1, [r7, #device_object_parent]; Store pointer to parent object
        STR     r3, [r7, #device_object_directory]; Store pointer to entry
        MOV     r8, #0                  ; Value to clear file handle with
        STR     r8, [r7, #device_object_handle]; Clear the file handle
        LDR     r8, [r0, #device_objects]; Read previous head of list
        STR     r8, [r7, #device_object_next]; Place at head of the list
        STR     r7, [r0, #device_objects]; Update the master pointer
        ADD     r2, r2, #1              ; Increment index for the next object
        RTSS                            ; Return from subroutine
fail$l  MOV     r2, r7                  ; Copy pointer to new record
        MOV     r0, #OSModule_Free      ; Reason code to release memory
        BL      dynamic_osmodule        ; Free this block
        MOV     r2, r6                  ; Restore the original entry index
        RTSS                            ; Return from subroutine
new$l   JSR     "r0-r3"                 ; Stack registers
        MOV     r3, #device_object      ; Initial size required
        MOV     r0, r5                  ; Copy pointer to path
newsp$l ADD     r3, r3, #1              ; Increment size required
        LDRB    r1, [r0], #1            ; Read character of path
        CMP     r1, #' '                ; Is it a terminator
        BGT     newsp$l                 ; Loop if not finished
        ADD     r0, r4, #OSGBPB_InfoStamped_name; Calculate pointer to leafname
newsl$l ADD     r3, r3, #1              ; Increment size required
        LDRB    r1, [r0], #1            ; Read character of leaf
        CMP     r1, #' '                ; Is it a terminator
        BGT     newsl$l                 ; Loop if not finished
        MOV     r0, #OSModule_Alloc     ; Reason code to allocate memory
        BL      dynamic_osmodule        ; Attempt to allocate the memory
        MOVVS   r2, #0                  ; Clear pointer if error produced
        MOV     r7, r2                  ; Copy pointer to the new record
        TEQ     r7, #0                  ; Is the pointer valid
        RTSS EQ                         ; Exit if pointer not valid
        ADD     r2, r7, #device_object_name; Pointer to destination buffer
        MOV     r0, r5                  ; Copy pointer to path
newcp$l LDRB    r1, [r0], #1            ; Read character of path
        CMP     r1, #' '                ; Is it a terminator
        MOVLE   r1, #'.'                ; Change terminator to separator
        STRB    r1, [r2], #1            ; Write to destination buffer
        BGT     newcp$l                 ; Loop if not finished
        ADD     r0, r4, #OSGBPB_InfoStamped_name; Calculate pointer to leafname
newcl$l LDRB    r1, [r0], #1            ; Read character of leaf
        CMP     r1, #' '                ; Is it a terminator
        MOVLE   r1, #0                  ; Change terminator to null byte
        STRB    r1, [r2], #1            ; Write to destination buffer
        BGT     newcl$l                 ; Loop if not finished
        RTSS                            ; Return from subroutine
trans$l JSR     "r0-r3"                 ; Stack registers
        ADD     r0, r4, #OSGBPB_InfoStamped_name; Calculate pointer to leafname
        LDR     r1, [r4, #OSGBPB_InfoStamped_file_type]; Read object filetype
        CMP     r1, #&1000              ; Is the filetype valid
        MOVGE   r1, #-1                 ; Do not add automatic extension if not
        ADRL    r2, ws_buffer + 128     ; Pointer to destination buffer
        MOV     r3, #String - 128       ; Size of destination buffer
        BL      dosmap_name_riscos_to_win95; Build Windows 95 style filename
        MOV     r8, r2                  ; Copy destination buffer pointer
        RTSS                            ; Return from subroutine
long$l  JSR     "r0-r1, r3-r4"          ; Stack registers
        BL      size$l                  ; Calculate number of directory entries
        TEQ     r3, #0                  ; Is this a long filename
        RTSS EQ                         ; Return from subroutine if not
        MOV     r4, r8                  ; Copy the pointer to the long name
        ADD     r3, r2, r3              ; Index of the real directory entry
        BL      device_dir_long_store   ; Store the long filename
        MOVVC   r2, r3                  ; Copy the last used entry index
        RTSS                            ; Return from subroutine
fat$l   JSR     "r1-r2, r4"             ; Stack registers
        LDR     r1, [r3, #directory_size]; Get object size in bytes
        LDRB    r2, [r3, #directory_attribute]; Get the object attributes
        TST     r2, #attribute_subdirectory; Is it a subdirectory
        TEQEQ   r1, #0                  ; Is it a zero length file
        BEQ     fatd$l                  ; Skip allocation if zero length file
        SUBS    r1, r1, #1              ; Decrease required size by one byte
        MOVMI   r1, #0                  ; Prevent size going negative
        LDR     r4, ws_device_cluster_shift; Shift from sectors to clusters
        ADD     r4, r4, #sector_shift   ; Shift from bytes to clusters
        MOV     r1, r1, LSR r4          ; Convert to clusters
        ADD     r1, r1, #1              ; Add an extra cluster to correct
        TST     r2, #attribute_subdirectory; Is it a subdirectory
        MOVNE   r2, r1, LSL r4          ; Convert to bytes
        STRNE   r2, [r3, #directory_size]; Store rounded size if a directory
        MOV     r2, r7                  ; Copy pointer to the object record
        BL      device_fat_allocate     ; Attempt to allocate the cluster chain
        TEQ     r1, #0                  ; Was the allocation successful
        BNE     fatd$l                  ; Finish the job if successful
        SetV                            ; Set overflow flag to indicate error
        RTS                             ; Return from subroutine
fatd$l  STR     r1, [r7, #device_object_start]; Store initial cluster number
        STRB    r1, [r3, #directory_cluster]; Store least significant byte
        MOV     r1, r1, LSR#8           ; Shift to obtain MSB
        STRB    r1, [r3, #directory_cluster + 1]; Store most significant byte
        RTS                             ; Return from subroutine
pre$l   JSR     "r1-r2"                 ; Stack registers
        LDR     r1, [r0, #device_flags] ; Get the flags for this device
        LDR     r2, [r4, #OSGBPB_InfoStamped_obj_type]; Read the object type
        TEQ     r2, #OSFile_IsImage     ; Is it an image file
        BNE     pren$l                  ; Skip next bit if not an image file
        TST     r1, #device_flag_imagefs; Are image files directories
        MOVEQ   r2, #OSFile_IsFile      ; Treat image file as a normal file
        MOVNE   r2, #OSFile_IsDir       ; Treat image file as a directory
        STR     r2, [r4, #OSGBPB_InfoStamped_obj_type]; Write the object type
pren$l  TEQ     r2, #OSFile_IsDir       ; Is object a directory
        BLEQ    dir$l                   ; Calculate size of a directory
        LDR     r1, ws_device_limit     ; Get the maximum size to include
        TEQ     r1, #0                  ; Is there a maximum specified
        RTS EQ                          ; Accept the file if not
        LDR     r2, [r4, #OSGBPB_InfoStamped_size]; Read the object size
        CMP     r2, r1                  ; Is the file acceptable
        RTS LS                          ; Return if file is small enough
        SetV                            ; Set overflow flag to indicate an error
        RTS                             ; Return from subroutine
dos$l   JSR     ""                      ; Stack registers
        STMFD   r13!, {r0-r1}           ; Stack registers
        LDR     r0, [r0, #device_flags] ; Get the flags for this device
        TST     r0, #device_flag_long   ; Are long filenames enabled
        MOV     r0, r8                  ; Copy pointer to Windows 95 name
        ADD     r1, r3, #directory_name ; Pointer to destination buffer
        BLNE    dosmap_name_win95_to_dos; Perform Windows 95 name conversion
        BLEQ    dosmap_name_win95_to_dos_strict; Perform strict name conversion
        LDMFD   r13!, {r0-r1}           ; Restore registers
        BL      device_object_unique    ; Ensure that the name is unique
        RTSS                            ; Return from subroutine
fill$l  JSR     "r0-r1"                 ; Stack registers
        MOV     r0, #0                  ; Default object attributes
        LDR     r1, [r4, #OSGBPB_InfoStamped_attr]; Read the object attributes
        TST     r1, #FileSwitch_AttrOwnerLocked; Is the file locked
        ORRNE   r0, r0, #attribute_read_only; Convert locked to read only
        LDR     r1, [r4, #OSGBPB_InfoStamped_obj_type]; Read the object type
        TEQ     r1, #OSFile_IsDir       ; Is it a directory
        ORREQ   r0, r0, #attribute_subdirectory; Set subdirectory attribute
        STRB    r0, [r3, #directory_attribute]; Store the attributes byte
        LDR     r0, [r4, #OSGBPB_InfoStamped_file_type]; Read file type
        CMP     r0, #OSFile_TypeUntyped ; Is object date stamped
        MOVEQ   r0, #0                  ; Default date if not stamped
        MOVEQ   r1, #0                  ; Default time if not stamped
        LDRNE   r0, [r4, #OSGBPB_InfoStamped_load_addr]; High byte of datestamp
        ANDNE   r0, r0, #&ff            ; Only require high byte of datestamp
        LDRNE   r1, [r4, #OSGBPB_InfoStamped_exec_addr]; Low word of datestamp
        BLNE    date_to_dos             ; Convert the datestamp
        STRB    r0, [r3, #directory_time]; Store least significant byte of time
        MOV     r0, r0, LSR#8           ; Extract most significant byte
        STRB    r0, [r3, #directory_time + 1]; Store most significant time byte
        STRB    r1, [r3, #directory_date]; Store least significant byte of date
        MOV     r1, r1, LSR#8           ; Extract most significant byte
        STRB    r1, [r3, #directory_date + 1]; Store most significant date byte
        LDR     r0, [r4, #OSGBPB_InfoStamped_size]; Read the object size
        STR     r0, [r3, #directory_size]; Store the object size
        RTSS                            ; Return from subroutine
dir$l   JSR     "r0-r9"                 ; Stack registers
        LDR     r9, [r0, #device_flags] ; Get the flags for this device
        MOV     r8, r4                  ; Copy pointer to object details
        MOV     r0, #OSGBPB_DirEntries  ; Reason code to read entries
        ADD     r1, r7, #device_object_name; Pointer to search path
        ADRL    r2, ws_buffer + 128     ; Pointer to a buffer
        MOV     r4, #0                  ; Start with the first entry
        MOV     r5, #String - 128       ; Size of buffer
        MOV     r6, #0                  ; Match with all files
        MOV     r7, #0                  ; Number of entries so far
dirl$l  MOV     r3, #1                  ; Only read a single object name
        SWI     XOS_GBPB                ; Read next directory entry
        BVS     dird$l                  ; Exit loop if error produced
        TEQ     r3, #0                  ; Was an object found
        BEQ     dird$l                  ; Exit loop if no object found
        ADD     r7, r7, #1              ; Increment counter
        TST     r9, #device_flag_long   ; Are long filenames enabled
        BLNE    dire$l                  ; Add extra entries for long files
        B       dirl$l                  ; Loop for the next entry
dird$l  ADD     r7, r7, #2              ; Add the two extra DOS entries
        MOV     r0, #directory          ; Size of each entry
        MUL     r0, r7, r0              ; Calculate the directory size
        STR     r0, [r8, #OSGBPB_InfoStamped_size]; Store the directory size
        RTSS                            ; Return from subroutine
dire$l  JSR     "r0-r3"                 ; Stack registers
        MOV     r0, #4                  ; Assume that an extension will be added
direl$l ADD     r0, r0, #1              ; Increment number of characters read
        LDRB    r1, [r2], #1            ; Read character from filename
        CMP     r1, #' '                ; Is this a terminator
        BGT     direl$l                 ; Loop for the next character
        MOV     r1, #13                 ; Number of characters in each entry
        DivRem  r2, r0, r1, r3          ; Calculate number of entries
        ADD     r7, r7, r2              ; Add on the number of entries
        TEQ     r0, #0                  ; Is there a remainder
        ADDNE   r7, r7, #1              ; Add an extra one for the remainder
        RTSS                            ; Return from subroutine
size$l  JSR     "r0-r2"                 ; Stack registers
        MOV     r3, #0                  ; Assume not long filename initially
        LDR     r0, [r0, #device_flags] ; Get the flags for this device
        TST     r0, #device_flag_long   ; Are long filenames enabled
        RTSS EQ                         ; No special action if they are not
        MOV     r0, r8                  ; Copy pointer to the Windows 95 name
        BL      dosmap_name_win95_is_long; Is this a long filename
        RTSS EQ                         ; No special behaviour if standard name
sizel$l ADD     r3, r3, #1              ; Increment character count
        LDRB    r1, [r0], #1            ; Read low byte of character
        LDRB    r2, [r0], #1            ; Read high byte of character
        ORRS    r1, r1, r2              ; Combine to form a single character
        BNE     sizel$l                 ; Loop for next character
        MOV     r1, #13                 ; Number of characters in each entry
        DivRem  r0, r3, r1, r2          ; Calculate number of entries
        TEQ     r3, #0                  ; Is there a remainder
        MOV     r3, r0                  ; Copy number of entries
        ADDNE   r3, r3, #1              ; Add an extra one for the remainder
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - Pointer to the directory object record, or
        ;                         0 for the root directory.
        ;                 r2    - Zero based index for the required directory
        ;                         entry.
        ;   Returns     : r3    - Pointer to the specified directory entry, or
        ;                         0 if it does not exist.
        ;   Description : Obtain a pointer to the specified directory entry.
device_dir_entry
        LocalLabels
        JSR     "r0-r2, r4-r5"          ; Stack registers
        MOV     r3, r2                  ; Copy required entry index
        BL      device_dir_entries      ; Check size of directory
        CMP     r3, r2                  ; Is the index legal
        BGE     fail$l                  ; Fail if index is out of range
        MOV     r2, #directory          ; Size of each directory entry
        MUL     r2, r3, r2              ; Offset from start of directory
        TEQ     r1, #0                  ; Is it the root directory
        BEQ     root$l                  ; Special case if it is
        MOV     r3, r2, LSR#sector_shift; Offset in sectors from the start
        SUB     r2, r2, r3, LSL#sector_shift; Remove sector offset
        LDR     r5, ws_device_cluster_shift; Shift from sectors to clusters
        MOV     r4, r3, LSR r5          ; Offset in clusters from the start
        SUB     r3, r3, r4, LSL r5      ; Remove cluster offset
        BL      fat$l                   ; Obtain the absolute cluster number
        SUB     r4, r4, #2              ; Subtract nonexistant cluster offset
        ADD     r3, r3, r4, LSL r5      ; Convert to sectors
        STMFD   r13!, {r2}              ; Stack registers
        LDR     r2, ws_device_skip_sectors; Number of sectors to skip
        ADD     r2, r3, r2              ; Include sectors before files area
        BL      device_sector_find      ; Obtain pointer to the record
        LDMFD   r13!, {r2}              ; Restore registers
        TEQ     r3, #0                  ; Was a sector found
        ADDNE   r3, r3, #device_sector_data; Advance to data area
        ADDNE   r3, r3, r2              ; Add the byte offset to the entry
        RTSS                            ; Return from subroutine
root$l  LDR     r1, [r0, #device_root]  ; Pointer to start of root directory
        ADD     r3, r1, r2              ; Pointer to the requested entry
        RTSS                            ; Return from subroutine
fail$l  MOV     r3, #0                  ; Directory entry does not exist
        RTSS                            ; Return from subroutine
fat$l   JSR     "r0-r3, r5"             ; Stack registers
        MOV     r5, r4                  ; Copy required cluster offset
        LDR     r4, [r1, #device_object_start]; First cluster number
fatl$l  SUBS    r5, r5, #1              ; Decrement cluster offset
        RTSS MI                         ; Return from subroutine when finished
        MOV     r1, r4                  ; Copy cluster number
        BL      device_fat_read         ; Find the next cluster in the chain
        MOV     r4, r2                  ; Copy next cluster number
        B       fatl$l                  ; Loop until required cluster found

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - Pointer to the directory object record, or
        ;                         0 for the root directory.
        ;   Returns     : r2    - Maximum number of directory entries.
        ;   Description : Calculate the number of entries that can fit in the
        ;                 specified directory.
device_dir_entries
        LocalLabels
        JSR     "r0-r1, r3"             ; Stack registers
        TEQ     r1, #0                  ; Is it the root directory
        BEQ     root$l                  ; Special case if it is
        LDR     r1, [r1, #device_object_directory]; Get pointer to entry
        LDR     r1, [r1, #directory_size]; Read the directory size
        MOV     r0, #directory          ; Size of each directory entry
        DivRem  r2, r1, r0, r3          ; Calculate number of entries
        RTSS                            ; Return from subroutine
root$l  LDR     r2, ws_device_root_directory_sectors; Root directory sectors
        MOV     r2, r2, LSL#sector_shift; Root directory bytes
        MOV     r2, r2, LSR#directory_shift; Root directory entries
        RTSS                            ; Return from subroutine

        ;   Parameters  : r3    - Pointer to the directory entry.
        ;   Returns     : None
        ;   Description : Zero a directory entry.
device_object_zero
        LocalLabels
        JSR     "r0-r1"                 ; Stack registers
        MOV     r0, #0                  ; Value to clear with
        MOV     r1, #directory          ; Number of bytes to clear
loop$l  SUBS    r1, r1, #1              ; Decrement number of bytes
        RTSS MI                         ; Return from subroutine
        STRB    r0, [r3, r1]            ; Clear this byte
        B       loop$l                  ; Loop for the next byte

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - Pointer to the parent directory object
        ;                         record, or 0 for the root directory.
        ;                 r2    - Zero based index of the first directory
        ;                         entry to use.
        ;                 r3    - Zero based index of the standard DOS style
        ;                         directory entry for this object. This is the
        ;                         entry immediately after the last one to use
        ;                         for the long filename.
        ;                 r4    - Pointer to the Windows 95 style leafname for
        ;                         the object to be added.
        ;   Returns     : None
        ;   Description : Store a long filename to a series of directory
        ;                 entries. The first directory entry contains the
        ;                 DOS version of the filename.
device_dir_long_store
        LocalLabels
        JSR     "r0-r7"                 ; Stack registers
        MOV     r5, r2                  ; Copy the first directory index
        STMFD   r13!, {r0, r3}          ; Stack registers
        BL      device_dir_entry        ; Obtain a pointer to the first entry
        TEQ     r3, #0                  ; Is the pointer valid
        ADDNE   r0, r3, #directory_name ; Obtain pointer to the name buffer
        BLNE    dosmap_name_dos_checksum; Calculate the name checksum
        MOVNE   r6, r0                  ; Copy the checksum
        LDMFD   r13!, {r0, r3}          ; Restore registers
        BEQ     fail$l                  ; Exit if failed
        MOV     r2, r3                  ; Copy the DOS directory entry index
        MOV     r7, #0                  ; No long filename entries yet
entry$l SUB     r2, r2, #1              ; Move to the previous entry
        CMP     r2, r5                  ; Compare to the first entry
        RTS LT                          ; Return from subroutine if finished
        ADD     r7, r7, #1              ; Increment sequence byte
        ORREQ   r7, r7, #1<<6           ; Mark the final entry
        BL      device_dir_entry        ; Obtain a pointer to this entry
        TEQ     r3, #0                  ; Is the pointer valid
        BEQ     fail$l                  ; Exit if failed
        BL      fill$l                  ; Fill in the details
        B       entry$l                 ; Loop for the next directory entry
fail$l  SetV                            ; Set overflow flag to indicate error
        RTS                             ; Return from subroutine
fill$l  JSR     "r0-r3"                 ; Stack registers
        BL      device_object_zero      ; Zero the directory entry
        MOV     r1, r3                  ; Copy pointer to directory entry
        STRB    r7, [r1, #longdir_sequence]; Store the sequence byte
        MOV     r0, #attribute_longname ; Attribute byte for long filename
        STRB    r0, [r1, #longdir_attribute]; Store the attribute byte
        STRB    r6, [r1, #longdir_checksum]; Store the filename checksum byte
        ADD     r2, r1, #longdir_fivechars; First byte to contain a character
        ADD     r3, r2, #10             ; Byte after end of characters
        BL      char$l                  ; Copy five characters of the filename
        ADD     r2, r1, #longdir_sixchars; First byte to contain a character
        ADD     r3, r2, #12             ; Byte after end of characters
        BL      char$l                  ; Copy nine characters of the filename
        ADD     r2, r1, #longdir_twochars; First byte to contain a character
        ADD     r3, r2, #4              ; Byte after end of characters
        BL      char$l                  ; Copy nine characters of the filename
        RTSS                            ; Return from subroutine
char$l  JSR     "r0-r3"                 ; Stack registers
charl$l TEQ     r2, r3                  ; Have all characters been written
        RTSS EQ                         ; Return from subroutine when done
        TEQ     r4, #0                  ; Has the filename been finished
        MOVEQ   r0, #&ff                ; Padding low byte
        MOVEQ   r1, #&ff                ; Padding high byte
        LDRNEB  r0, [r4], #1            ; Read low byte from filename
        LDRNEB  r1, [r4], #1            ; Read high byte from filename
        STRB    r0, [r2], #1            ; Store low byte of filename
        STRB    r1, [r2], #1            ; Store high byte of filename
        TEQ     r0, #0                  ; Is the low byte zero
        TEQEQ   r1, #0                  ; Is the high byte zero
        MOVEQ   r4, #0                  ; End of filename if both zero
        B       charl$l                 ; Loop for the next character

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - Pointer to the directory object record, or
        ;                         0 for the root directory.
        ;                 r2    - Zero based index for the directory entry to
        ;                         process.
        ;   Returns     : None
        ;   Description : Check a single entry in the specified directory for
        ;                 uniqueness. If the name is not unique then it is
        ;                 adjusted as required.
device_object_unique
        LocalLabels
        JSR     "r0-r5"                 ; Stack registers
        MOV     r5, r2                  ; Copy the slow scan index
main$l  MOV     r2, r5                  ; Restore the original index
        BL      device_dir_entry        ; Convert the index to a pointer
        MOVS    r4, r3                  ; Copy the slow scan pointer
        RTSS EQ                         ; Exit if pointer is not valid
        BL      spec$l                  ; Is the name reserved
        BEQ     main$l                  ; Try again if name was reserved
        MOV     r2, #0                  ; Start from the first entry
loop$l  TEQ     r2, r5                  ; Have all entries been compared
        RTSS EQ                         ; Return from subroutine if finished
        BL      device_dir_entry        ; Convert the index to a pointer
        BL      cmp$l                   ; Compare these two entries
        BEQ     main$l                  ; Back to the start if matched
        ADD     r2, r2, #1              ; Advance the index to check against
        B       loop$l                  ; Check the next pair of entries
cmp$l   JSR     "r0-r1"                 ; Stack registers
        MOV     r0, r4                  ; Copy pointer to first entry
        MOV     r1, r3                  ; Copy pointer to second entry
        BL      device_object_cmp       ; Compare the filenames
        BLEQ    dosmap_name_dos_increment; Increment the first entry if same
        RTS                             ; Return from subroutine
spec$l  JSR     "r0"                    ; Stack registers
        MOV     r0, r4                  ; Copy pointer to directory entry
        BL      dosmap_name_dos_is_special; Check if name is reserved
        BLEQ    dosmap_name_dos_increment; Increment the first entry if same
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the first directory entry.
        ;                 r1    - Pointer to the second directory entry.
        ;   Returns     : flags - Z flag set (EQ) if names match.
        ;   Description : Compare two directory entries and check whether the
        ;                 name has been duplicated.
device_object_cmp
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        MOV     r2, r0                  ; Copy the first entry pointer
        BL      check$l                 ; Check if it is a normal entry
        RTS NE                          ; Exit if not accepted
        MOV     r2, r1                  ; Copy the second entry pointer
        BL      check$l                 ; Check if it is a normal entry
        RTS NE                          ; Exit if not accepted
        BL      dosmap_name_dos_compare ; Compare the filenames
        RTS                             ; Return from subroutine
check$l JSR     "r0"                    ; Stack registers
        LDRB    r0, [r2, #directory_name]; Get first character of name
        TEQ     r0, #directory_unused   ; Is the entry used
        TEQNE   r0, #directory_erased   ; Has the entry been erased
        BEQ     fail$l                  ; Not interested in these entries
        LDRB    r0, [r2, #directory_attribute]; Get attribute byte
        TST     r0, #attribute_volume   ; Is it a volume label
        RTS                             ; Return from subroutine
fail$l  ClrZ                            ; Filename is not valid
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - Pointer to the object record.
        ;   Returns     : None
        ;   Description : Destroy an object record. This performs any other
        ;                 house keeping required, such as closing associated
        ;                 files.
device_object_clear
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        LDR     r2, [r1, #device_object_handle]; Get file handle
        TEQ     r2, #0                  ; Is handle valid
        BLNE    close$l                 ; Close the file
        MOV     r2, r1                  ; Copy pointer to the object record
        MOV     r0, #OSModule_Free      ; Reason code to release memory
        BL      dynamic_osmodule        ; Free this block
        RTSS                            ; Return from subroutine
close$l JSR     "r0-r1"                 ; Stack registers
        MOV     r0, #OSFind_Close       ; Reason code to close file
        MOV     r1, r2                  ; Copy file handle
        SWI     XOS_Find                ; Close the file
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the device record.
        ;   Returns     : None
        ;   Description : Destroy all object records for the specified device.
device_object_reset
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        LDR     r1, [r0, #device_objects]; Pointer to the first object record
loop$l  TEQ     r1, #0                  ; Is the pointer valid
        BEQ     done$l                  ; Exit loop when finished
        LDR     r2, [r1, #device_object_next]; Pointer to the next record
        BL      device_object_clear     ; Destroy this object record
        MOV     r1, r2                  ; Restore next object pointer
        B       loop$l                  ; Loop for the next record
done$l  MOV     r1, #0                  ; Value to clear pointer with
        STR     r1, [r0, #device_objects]; Clear the head of list pointer
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the device record
        ;   Returns     : None
        ;   Description : Close all files related to objects.
device_object_close
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        LDR     r1, [r0, #device_objects]; Pointer to first object record
loop$l  TEQ     r1, #0                  ; Is the pointer valid
        RTSS EQ                         ; Return from subroutine
        LDR     r2, [r1, #device_object_handle]; Get file handle
        TEQ     r2, #0                  ; Is the file handle
        BLNE    close$l                 ; Close the file if open
        MOV     r2, #0                  ; Null file handle
        STR     r2, [r1, #device_object_handle]; Store null handle
        LDR     r1, [r1, #device_object_next]; Pointer to next record
        B       loop$l                  ; Loop for next record
close$l JSR     "r0-r1"                 ; Stack registers
        MOV     r0, #OSFind_Close       ; Reason code to close file
        MOV     r1, r2                  ; Copy file handle
        SWI     XOS_Find                ; Close the file
        RTSS                            ; Return from subroutine

; A literal pool

        LTORG

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - Sector number to add.
        ;   Returns     : r2    - Pointer to the new record, or 0 if failed.
        ;   Description : Create a new sector record.
device_sector_new
        LocalLabels
        JSR     "r0-r1"                 ; Stack registers
        BL      alloc$l                 ; Attempt to allocate a block of memory
        TEQ     r2, #0                  ; Check if successful
        RTSS EQ                         ; Exit if failed
        STR     r1, [r2, #device_sector_no]; Store sector number
        LDR     r1, [r0, #device_sectors]; Pointer to head of list
        STR     r1, [r2, #device_sector_next]; Place at the head of the list
        TEQ     r1, #0                  ; Is there a next sector
        STRNE   r2, [r1, #device_sector_prev]; Update the next previous entry
        STR     r2, [r0, #device_sectors]; Store pointer to this record
        MOV     r1, #0                  ; Value to clear pointer with
        STR     r1, [r2, #device_sector_prev]; No previous sector
        RTSS                            ; Return from subroutine
alloc$l JSR     "r0-r1, r3"             ; Stack registers
        MOV     r0, #OSModule_Alloc     ; Reason code to allocate memory
        MOV     r3, #device_sector      ; Size of block required
        BL      dynamic_osmodule        ; Attempt to allocate the memory
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - The first cluster number.
        ;   Returns     : r2    - Pointer to the first record, or 0 if failed.
        ;   Description : Allocate a chain of clusters starting at the
        ;                 specified cluster.
device_sector_chain
        LocalLabels
        JSR     "r0-r1, r3-r6"          ; Stack registers
        LDR     r4, [r0, #device_sectors]; Current first sector
        MOV     r5, #0                  ; No initial sector pointer
        LDR     r6, ws_device_total_clusters; Total number of clusters
        ADD     r6, r6, #2              ; End of cluster chain
loop$l  BL      clust$l                 ; Attempt to allocate the cluster
        TEQ     r2, #0                  ; Was it successful
        BEQ     fail$l                  ; Handle failure
        TEQ     r5, #0                  ; Is it the first cluster
        MOVEQ   r5, r2                  ; Copy sector pointer if it is
        BL      device_fat_read         ; Read the FAT entry
        MOV     r1, r2                  ; Copy FAT entry
        TEQ     r1, #fat_available      ; Is the entry unused
        BEQ     fail$l                  ; Fail if not
        CMP     r1, r6                  ; Is it the end of a file
        BLT     loop$l                  ; Loop if not finished
        MOV     r2, r5                  ; Restore the initial sector pointer
        RTSS                            ; Return from the subroutine
fail$l  LDR     r1, [r0, #device_sectors]; Get first sector
        TEQ     r1, r4                  ; Is it the original one
        BEQ     faild$l                 ; Skip next bit if unallocated
        BL      device_sector_remove    ; Deallocate the sector
        B       fail$l                  ; Loop for the next sector
faild$l MOV     r2, #0                  ; Clear record pointer
        RTSS                            ; Return from subroutine
clust$l JSR     "r0-r1, r3-r4"          ; Stack registers
        SUB     r1, r1, #2              ; Subtract nonexistant cluster offset
        LDR     r4, ws_device_cluster_shift; Shift from sectors to clusters
        MOV     r1, r1, LSL r4          ; Convert to sectors
        LDR     r3, ws_device_skip_sectors; Number of sectors to skip
        ADD     r1, r1, r3              ; Include sectors before files area
        MOV     r3, #1                  ; Convert a single sector
        MOV     r4, r3, LSL r4          ; Number of sectors to allocate
        MOV     r3, #0                  ; No sector pointer defined yet
clusl$l SUBS    r4, r4, #1              ; Decrement sector count
        BMI     clusd$l                 ; Exit loop if finished
        BL      device_sector_new       ; Allocate a sector
        TEQ     r2, #0                  ; Was it successful
        RTSS EQ                         ; Exit if not
        TEQ     r3, #0                  ; Is it the first sector
        MOVEQ   r3, r2                  ; Copy the sector pointer
        ADD     r1, r1, #1              ; Increment the sector number
        B       clusl$l                 ; Loop for the next sector
clusd$l MOV     r2, r3                  ; Restore first sector pointer
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the device record.
        ;                 r1    - Pointer to sector record.
        ;   Returns     : None
        ;   Description : Destroy a sector record.
device_sector_remove
        LocalLabels
        JSR     "r0-r3"                 ; Stack registers
        LDR     r2, [r1, #device_sector_next]; Get next sector record pointer
        LDR     r3, [r1, #device_sector_prev]; Get previous sector record
        TEQ     r2, #0                  ; Is there a next record
        STRNE   r3, [r2, #device_sector_prev]; Set the previous pointer
        TEQ     r3, #0                  ; Is there a previous record
        STRNE   r2, [r3, #device_sector_next]; Set the next sector record
        STREQ   r2, [r0, #device_sectors]; Set new pointer to first record
        MOV     r2, r1                  ; Copy sector record pointer
        MOV     r0, #OSModule_Free      ; Reason code to release memory
        BL      dynamic_osmodule        ; Free this block
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to unit record.
        ;                 r2    - Sector number to find.
        ;   Returns     : r3    - Pointer to sector record.
        ;   Description : Find the record for the specified sector.
device_sector_find
        LocalLabels
        JSR     "r0"                    ; Stack registers
        LDR     r3, [r0, #device_sectors]; Pointer to first record
loop$l  TEQ     r3, #0                  ; Is it a valid pointer
        RTSS EQ                         ; Exit if not
        LDR     r0, [r3, #device_sector_no]; Get sector number
        TEQ     r0, r2                  ; Does sector number match
        RTSS EQ                         ; Exit if it does
        LDR     r3, [r3, #device_sector_next]; Pointer to next record
        B       loop$l                  ; Loop for the next record

        ;   Parameters  : r0    - Pointer to the device record.
        ;   Returns     : None
        ;   Description : Destroy all sector records for the specified device.
device_sector_reset
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        LDR     r2, [r0, #device_sectors]; Pointer to sector list
loop$l  TEQ     r2, #0                  ; Is the pointer valid
        RTSS EQ                         ; Exit loop if not
        MOV     r1, r2                  ; Copy this pointer
        LDR     r2, [r2, #device_sector_next]; Pointer to next record
        BL      device_sector_remove    ; Deallocate this sector record
        B       loop$l                  ; Loop for next sector record

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Reset the device handling when the PC is quit or
        ;                 reset.
device_reset
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        pipe_string "device_reset"
        LDR     r1, ws_device_head      ; Pointer to first device driver
loop$l  TEQ     r1, #0                  ; Is it a valid pointer
        RTSS EQ                         ; Return from subroutine if finished
        MOV     r0, r1                  ; Copy device driver pointer
        BL      device_object_reset     ; Reset the object list
        BL      device_sector_reset     ; Reset the sector list
        MOV     r0, #OSModule_Free      ; Reason code to release memory
        LDR     r2, [r1, #device_fat]   ; Get pointer to FAT
        BL      dynamic_osmodule        ; Free this block
        LDR     r2, [r1, #device_inverse_fat]; Get pointer to inverse FAT
        BL      dynamic_osmodule        ; Free this block
        LDR     r2, [r1, #device_root]  ; Get pointer to root directory
        BL      dynamic_osmodule        ; Free this block
        MOV     r2, r1                  ; Copy device driver pointer
        LDR     r1, [r1, #device_next]  ; Get pointer to the next record
        STR     r1, ws_device_head      ; Unlink this record from the list
        BL      dynamic_osmodule        ; Free this block
        B       loop$l                  ; Loop for the next device

        ;   Parameters  : r0    - Unit code (drive number), or -1 for
        ;                         initialisation.
        ;                 r1    - Pointer to buffer to contain BIOS
        ;                         parameter block.
        ;   Returns     : None
        ;   Description : Build a parameter block for the specified drive.
        ;                 This also indicates that the disc contents may be
        ;                 changed.
device_build_bpb
        LocalLabels
        JSR     "r0-r3"                 ; Stack registers
        pipe_string "device_build_bpb: unit %0 @ %1", r0, r1
        TEQ     r0, #&ff                ; Is it initialisation
        BLNE    reset$l                 ; Reset the device otherwise
        ADRL    r0, ws_device_boot + boot_bpb; Pointer to BIOS parameter block
        ADD     r2, r0, #bpb            ; Pointer to end of BPB
loop$l  LDRB    r3, [r0], #1            ; Read a byte of the BPB
        STRB    r3, [r1], #1            ; Write it to the destination buffer
        CMP     r0, r2                  ; Has the end of the BPB been reached
        BLO     loop$l                  ; Loop if not finished
        RTSS                            ; Return from subroutine
reset$l JSR     "r0-r1"                 ; Stack registers
        BL      device_find             ; Find the device record
        TEQ     r0, #0                  ; Was record found
        RTSS EQ                         ; Exit if not
        MOV     r1, #0                  ; No PC files open when changed
        STR     r1, [r0, #device_open_files]; Clear the number of files
        LDR     r1, [r0, #device_flags] ; Get flags
        ORR     r1, r1, #device_flag_relog_avail; Set flag to indicate state
        BIC     r1, r1, #device_flag_relog_forced; Clear the forced flag
        BIC     r1, r1, #device_flag_changed; Clear the structure changed flag
        TST     r1, #device_flag_relog_pending; Is a reset required
        BICNE   r1, r1, #device_flag_relog_pending; Clear flag if set
        STR     r1, [r0, #device_flags] ; Store modified flags
        RTSS EQ                         ; Exit if update not required
        BL      device_object_reset     ; Reset the object list
        BL      device_sector_reset     ; Reset the sector list
        BL      device_prepare          ; Ensure everything is initialised
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Unit code (drive number).
        ;   Returns     : r0    - Pointer to the device record.
        ;   Description : Find the device record for a specified unit.
device_find
        LocalLabels
        JSR     "r1"                    ; Stack regusters
        MOV     r1, r0                  ; Copy unit code
        LDR     r0, ws_device_head      ; Get pointer to first record
loop$l  TEQ     r0, #0                  ; Check if pointer is valid
        RTSS EQ                         ; Exit if unable to find record
        SUBS    r1, r1, #1              ; Decrement unit code
        RTSS MI                         ; Exit if found correct record
        LDR     r0, [r0, #device_next]  ; Pointer to next device record
        B       loop$l                  ; Loop if not found yet

        ;   Parameters  : r0    - Unit code (drive number).
        ;                 r1    - Pointer to buffer to contain read data.
        ;                 r2    - Sector number to read.
        ;   Returns     : None
        ;   Description : Read a sector from the specified drive.
device_read_sector
        LocalLabels
        JSR     "r0-r5"                 ; Stack registers
        pipe_string "device_read_sector: unit %0 sector %1 @ %2", r0, r2, r1
        BL      device_find             ; Find device description
        TEQ     r0, #0                  ; Was record found
        BEQ     fail$l                  ; Handle failure
        LDR     r3, [r0, #device_flags] ; Get flags
        BIC     r3, r3, #device_flag_relog_avail; Clear resettable flag
        STR     r3, [r0, #device_flags] ; Store modified flags
        BL      device_sector_find      ; Find the required sector
        TEQ     r3, #0                  ; Was record found
        BNE     found$l                 ; Use the record if it exists
        TEQ     r2, #0                  ; Is it the boot block
        BEQ     boot$l                  ; Handle boot block
        SUBS    r2, r2, #reserved_sectors; Skip any other reserved sectors
        BLO     none$l                  ; Do not bother with reserved sectors
        LDR     r4, ws_device_sectors_per_fat; Size of file allocation table
        MOV     r5, #number_fats        ; Number of FATs
fatl$l  SUBS    r3, r2, r4              ; Check if part of this FAT
        BLO     fat$l                   ; Handle file allocation table
        MOV     r2, r3                  ; Copy updated sector offset
        SUBS    r5, r5, #1              ; Decrement number of FATs remaining
        BNE     fatl$l                  ; Try the next copy of the FAT
        LDR     r4, ws_device_root_directory_sectors; Size of root directory
        SUBS    r2, r3, r4              ; Skip root directory
        BLO     root$l                  ; Handle root directory
        LDR     r4, ws_device_cluster_shift; Shift from sectors to clusters
        MOV     r5, #1                  ; Convert a single cluster
        MOV     r5, r5, LSL r4          ; Number of sectors in a cluster
        SUB     r5, r5, #1              ; Sector offset mask
        MOV     r3, r2, LSR r4          ; Convert to a cluster number
        ADD     r3, r3, #2              ; Convert to a FAT cluster number
        AND     r2, r2, r5              ; Offset from cluster start
        BL      obj$l                   ; Handle the files area
        RTS VS                          ; Return any error produced
        TEQ     r0, #0                  ; Was valid data produced
        BEQ     none$l                  ; Return zero data if not
        RTSS                            ; Return from subroutine
none$l  ADD     r2, r1, #bytes_per_sector; End of sector pointer
        MOV     r0, #0                  ; Value to clear buffer with
clear$l STR     r0, [r1], #4            ; Clear the next word of the buffer
        CMP     r1, r2                  ; Has the buffer been cleared
        BLO     clear$l                 ; Loop if not finished
        RTSS                            ; Return from subroutine
found$l ADD     r0, r3, #device_sector_data; Pointer to sector data
        BL      device_copy             ; Copy the sector
        RTSS                            ; Return from subroutine
boot$l  BL      device_boot_update      ; Update the boot sector for device
        ADR     r0, ws_device_boot      ; Constant shared boot sector
        BL      device_copy             ; Copy the sector
        RTSS                            ; Return from subroutine
fat$l   LDR     r0, [r0, #device_fat]   ; Pointer to start of FAT
        MOV     r3, #bytes_per_sector   ; Number of bytes in a sector
        MLA     r0, r2, r3, r0          ; Calculate pointer to data
        BL      device_copy             ; Copy the sector
        RTSS                            ; Return from subroutine
root$l  LDR     r2, [r0, #device_root]  ; Pointer to root directory
        LDRB    r2, [r2]                ; Get first character of first entry
        TEQ     r2, #directory_unused   ; Has the directory been initialised
        BLEQ    device_root_create      ; Create the root directory
        LDR     r0, [r0, #device_root]  ; Pointer to start of root directory
        MOV     r2, #bytes_per_sector   ; Number of bytes in a sector
        MLA     r0, r3, r2, r0          ; Calculate pointer to data
        BL      device_copy             ; Copy the sector
        RTSS                            ; Return from subroutine
fail$l  SetV                            ; Set error flag
        RTS                             ; Return from subroutine
obj$l   JSR     "r1-r6"                 ; Stack registers
        MOV     r5, r1                  ; Copy output buffer pointer
        MOV     r4, r2                  ; Copy sector offset from cluster start
        MOV     r1, r3                  ; Copy cluster number
        BL      device_fat_decode       ; Find object details
        TEQ     r1, #0                  ; Was an object found
        MOVEQ   r0, #0                  ; Clear pointer if not
        RTS EQ                          ; Return with null pointer if not found
        LDR     r6, [r1, #device_object_directory]; Get pointer to directory
        LDRB    r6, [r6, #directory_attribute]; Get attribute byte
        TST     r6, #attribute_subdirectory; Is it a directory
        BNE     dir$l                   ; Skip next bit if it is
        LDR     r6, ws_device_cluster_shift; Shift from sectors to clusters
        ADD     r4, r4, r2, LSL r6      ; Calculate sector offset
        MOV     r4, r4, LSL#sector_shift; Convert it to bytes
        LDR     r2, [r1, #device_object_handle]; Get file handle
        TEQ     r2, #0                  ; Is the file open
        BLEQ    open$l                  ; Open it if not
        TEQ     r2, #0                  ; Is the file open
        BEQ     objf$l                  ; Fail with error if not
        BL      read$l                  ; Attempt to read from the file
        RTSS VC                         ; Return if successful
        MOV     r2, #0                  ; Assume file is not open
        STR     r2, [r1, #device_object_handle]; Clear file handle
        BL      open$l                  ; Attempt to open file again
        TEQ     r2, #0                  ; Is the file open
        BEQ     objf$l                  ; Fail with error if not
        BL      read$l                  ; Another go at reading from file
        RTS                             ; Return from subroutine
objf$l  SetV                            ; Set error flag
        RTS                             ; Return from subroutine
dir$l   BL      device_subdir_create    ; Create the subdirectory
        RTS VS                          ; Exit if error produced
        SUB     r3, r3, #2              ; Subtrack nonexistant clusters
        LDR     r6, ws_device_cluster_shift; Shift from sectors to clusters
        ADD     r2, r4, r3, LSL r6      ; Sector offset from files area
        LDR     r3, ws_device_skip_sectors; Number of sectors to skip
        ADD     r2, r2, r3              ; Skip sectors before file area
        BL      device_sector_find      ; Find the relevant sector
        TEQ     r3, #0                  ; Was the sector found
        BNE     dirc$l                  ; Skip next bit if it was
        MOV     r0, #0                  ; Indicate a blank sector
        RTSS                            ; Return from subroutine
dirc$l  ADD     r0, r3, #device_sector_data; Pointer to the actual data
        MOV     r1, r5                  ; Copy output buffer pointer
        BL      device_copy             ; Copy the subdirectory data
        RTSS                            ; Return from subroutine
open$l  JSR     "r0-r1, r3"             ; Stack registers
        MOV     r3, r1                  ; Copy pointer to object record
        MOV     r0, #OSFind_Openin :OR: OSFind_NoPath; Reason code to open file
        ADD     r1, r1, #device_object_name; Pointer to filename
        SWI     XOS_Find                ; Attempt to open the file
        MOVVS   r0, #0                  ; Clear handle if error
        STR     r0, [r3, #device_object_handle]; Store new file handle
        MOV     r2, r0                  ; Copy file handle
        RTSS                            ; Return from subroutine
read$l  JSR     "r0-r4"                 ; Stack registers
        MOV     r0, #OSGBPB_ReadAt      ; Reason code to read file at
        MOV     r1, r2                  ; Copy the file handle
        MOV     r2, r5                  ; Copy buffer pointer
        MOV     r3, #bytes_per_sector   ; Number of bytes to read
        SWI     XOS_GBPB                ; Read bytes from the file
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - Unit code (drive number).
        ;                 r1    - Pointer to buffer containing data to write.
        ;                 r2    - Sector number to write.
        ;   Returns     : None
        ;   Description : Write a sector to the specified drive.
device_write_sector
        LocalLabels
        JSR     "r0-r4"                 ; Stack registers
        pipe_string "device_write_sector: unit %0 sector %1 @ %2", r0, r2, r1
        BL      device_find             ; Find device description
        TEQ     r0, #0                  ; Was record found
        BEQ     fail$l                  ; Handle failure
        LDR     r3, [r0, #device_flags] ; Get flags
        TST     r3, #device_flag_write  ; Are write operations supported
        BEQ     fail$l                  ; Fail if they are not
        BIC     r3, r3, #device_flag_relog_avail; Clear resettable flag
        STR     r3, [r0, #device_flags] ; Store modified flags
        BL      device_sector_find      ; Find the required sector
        TEQ     r3, #0                  ; Was record found
        BNE     done$l                  ; Skip next bit if it was
        BL      new$l                   ; Create a new sector
        TEQ     r3, #0                  ; Was the sector created
        BEQ     fail$l                  ; Handle failure
done$l  MOV     r0, r1                  ; Copy pointer to buffer
        ADD     r1, r3, #device_sector_data; Pointer to destination buffer
        BL      device_copy             ; Copy the sector
        RTSS                            ; Return from subroutine
fail$l  SetV                            ; Set error flag
        RTS                             ; Return from subroutine
new$l   JSR     "r0-r2"                 ; Stack registers
        MOV     r1, r2                  ; Copy the sector number
        BL      device_sector_new       ; Create a new sector
        MOV     r3, r2                  ; Copy the sector pointer
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to source sector.
        ;                 r1    - Pointer to destination sector buffer.
        ;   Returns     : None
        ;   Description : Copy a device sector in memory.
device_copy
        LocalLabels
        JSR     "r2-r3"                 ; Stack registers
        MOV     r2, #bytes_per_sector   ; Number of bytes to copy
loop$l  SUBS    r2, r2, #4              ; Decrement bytes to copy
        RTSS MI                         ; Return from subroutine
        LDR     r3, [r0, r2]            ; Read source word
        STR     r3, [r1, r2]            ; Write destination word
        B       loop$l                  ; Loop for next word

; A literal pool

        LTORG

; Dynamic area handling

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Initialise the dynamic area manager.
dynamic_initialise
        LocalLabels
        JSR     "r0-r8"                 ; Stack registers
        MOV     r0, #OSDynamicArea_Create; Reason code to create area
        MOV     r1, #OSDynamicArea_AllocateArea; Allocate number automatically
        MOV     r2, #0                  ; Initially zero sized
        MOV     r3, #OSDynamicArea_AllocateBase; Alloacte base address
        MOV     r4, #1 << 7             ; Flags for not draggable bar
        MOV     r5, #-1                 ; Maximum size is all memory
        MOV     r6, #0                  ; No handler routine
        MOV     r7, #0                  ; No workspace for handler routine
        ADR     r8, name$l              ; Pointer to dynamic area name
        SWI     XOS_DynamicArea         ; Attempt to create dynamic area
        MOVVS   r1, #0                  ; Set area number to zero if error
        STR     r1, ws_dynamic_area     ; Store dynamic area number
        STR     r2, ws_dynamic_size     ; Store initial size of dynamic area
        STR     r3, ws_dynamic_ptr      ; Store pointer to dynamic area
        MOV     r0, #1                  ; Value to set flag with
        STR     r0, ws_dynamic_enable   ; Enable use of dynamic area
        RTSS                            ; Return from subroutine

name$l  =       "ARMEdit", 0            ; Dynamic area name
        ALIGN

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Tidy up after using the dynamic area manager.
dynamic_finalise
        LocalLabels
        JSR     "r0, r1"                ; Stack registers
        MOV     r0, #OSDynamicArea_Delete; Reason code to remove dynamic area
        LDR     r1, ws_dynamic_area     ; Read number of dynamic area
        TEQ     r1, #0                  ; Is there a dynamic area
        RTSS EQ                         ; Exit if no dynamic area created
        SWI     XOS_DynamicArea         ; Remove the dynamic area
        RTSS                            ; Return from subroutine

        ;   Parameters  : As for OS_Module.
        ;   Returns     : As for OS_Module.
        ;   Description : Perform an OS_Module operation, but use the
        ;                 dynamic area, if possible, instead of the RMA.
dynamic_osmodule
        LocalLabels
        JSR     "r11"                   ; Stack registers
        LDR     r11, ws_dynamic_area    ; Get dynamic area number
        TEQ     r11, #0                 ; Is there a dynamic area
        BEQ     orig$l                  ; Skip next bit if not
        TEQ     r0, #OSModule_Alloc     ; Is it a memory allocation request
        BEQ     alloc$l                 ; Jump to allocation handler if it is
        TEQ     r0, #OSModule_Free      ; Is it a memory deallocation request
        BEQ     free$l                  ; Jump to deallocation handler if it is
orig$l  SWI     XOS_Module              ; Otherwise pass on to OS_Module
        RTS                             ; Return from subroutine
alloc$l LDR     r11, ws_dynamic_enable  ; Should dynamic area be used
        TEQ     r11, #0                 ; Is the flag set
        BEQ     orig$l                  ; Use RMA if not
        BL      dynamic_alloc           ; Perform allocation
        RTS                             ; Return from subroutine
free$l  LDR     r11, ws_dynamic_ptr     ; Pointer to start of dynamic area
        CMP     r2, r11                 ; Is the pointer in the area
        BLO     orig$l                  ; Deallocate from RMA if not
        BL      dynamic_free            ; Perform deallocation
        RTS                             ; Return from subroutine

        ;   Parameters  : r3    - Size of block required.
        ;   Returns     : r2    - Pointer to block, or 0 if not allocated.
        ;   Description : Attempt to allocate a block of memory.
dynamic_alloc
        LocalLabels
        JSR     "r0-r1, r3-r4"          ; Stack registers
        LDR     r0, ws_dynamic_size     ; Get size of dynamic area heap
        TEQ     r0, #0                  ; Check if heap exists
        BNE     skip$l                  ; Skip next bit if heap created
        MOV     r0, #24                 ; Minimum size for a heap
        BL      dynamic_grow            ; Create minimum sized heap
        MOVVS   r2, #0                  ; Clear pointer if error produced
        RTE VS                          ; Return if error produced
skip$l  MOV     r0, #OSHeap_Alloc       ; Reason code to allocate memory
        LDR     r1, ws_dynamic_ptr      ; Pointer to heap
        SWI     XOS_Heap                ; Attempt to allocate the memory
        RTSS VC                         ; Return if successful
        MOV     r0, r3                  ; Copy required size
        BL      dynamic_grow            ; Attempt to claim the memory
        BVS     err$l                   ; Skip next bit if error produced
        MOV     r0, #OSHeap_Alloc       ; Reason code to allocate memory
        LDR     r1, ws_dynamic_ptr      ; Pointer to heap
        SWI     XOS_Heap                ; Attempt to allocate the memory again
        RTSS VC                         ; Return if successful
err$l   BL      dynamic_minimise        ; Minimise the size of the heap
        MOV     r2, #0                  ; Ensure block pointer is cleared
        RTE                             ; Return with error

        ;   Parameters  : r2    - Pointer to block to deallocate.
        ;   Returns     : None
        ;   Description : Attempt to release a block of memory
dynamic_free
        LocalLabels
        JSR     "r0-r3"                 ; Stack registers
        MOV     r0, #OSHeap_Free        ; Reason code to free block
        LDR     r1, ws_dynamic_ptr      ; Pointer to heap
        SWI     XOS_Heap                ; Free the heap block
        RTE VS                          ; Exit if error produced
        BL      dynamic_minimise        ; Minimise the size of the heap
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Increase in size required.
        ;   Returns     : None
        ;   Description : Attempt to enlarge the dynamic area. If the heap
        ;                 did not previously exist then it is initialised,
        ;                 otherwise the heap is resized to fill the dynamic
        ;                 area.
dynamic_grow
        LocalLabels
        JSR     "r0-r3"                 ; Stack registers
        MOV     r1, r0                  ; Copy the required change in size
        LDR     r0, ws_dynamic_area     ; Get dynamic area number
        SWI     XOS_ChangeDynamicArea   ; Change the size of the dynamic area
        RTE VS                          ; Exit if error produced
        MOV     r3, r1                  ; Copy actual change in size
        LDR     r0, ws_dynamic_size     ; Get previous size of dynamic area
        ADD     r1, r0, r1              ; Calculate new size of dynamic area
        STR     r1, ws_dynamic_size     ; Store new size of area
        LDR     r1, ws_dynamic_ptr      ; Get pointer to heap
        TEQ     r0, #0                  ; Did heap previously exist
        MOVEQ   r0, #OSHeap_Initialise  ; Reason code to initialise the heap
        MOVNE   r0, #OSHeap_Resize      ; Reason code to resize heap
        SWI     XOS_Heap                ; Grow the heap
        RTE VS                          ; Exit if error produced
        RTS                             ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Shrink the heap as far as possible. The dynamic area
        ;                 is then shrunk to match, and the heap size rounded
        ;                 to the actual dynamic area size. If the heap is
        ;                 empty then it is deleted.
dynamic_minimise
        LocalLabels
        JSR     "r0-r3"                 ; Stack registers
        LDR     r0, ws_dynamic_size     ; Get size of dynamic area
        TEQ     r0, #0                  ; Does any heap exist
        RTSS EQ                         ; Exit if not
        LDR     r1, ws_dynamic_ptr      ; Pointer to start of heap
        LDR     r0, [r1, #8]            ; Get pointer to heap base
        TEQ     r0, #&10                ; Is the heap empty
        BEQ     end$l                   ; Remove the heap if it is
        MOV     r0, #OSHeap_Resize      ; Reason code to resize the heap
        LDR     r1, ws_dynamic_ptr      ; Get pointe to heap
        LDR     r3, ws_dynamic_size     ; Get size of heap
        RSB     r3, r3, #0              ; Negate size to shrink
        SWI     XOS_Heap                ; Minimise the size of the heap
        LDR     r0, ws_dynamic_area     ; Get dynamic area number
        RSB     r1, r3, #0              ; Change in size of heap
        SWI     XOS_ChangeDynamicArea   ; Attempt to shrink dynamic area
        LDR     r0, ws_dynamic_size     ; Get original size of dynamic area
        SUB     r0, r0, r1              ; New size of dynamic area
        STR     r0, ws_dynamic_size     ; Store new size of dynamic area
        SUB     r3, r3, r1              ; Subtract change in size of area
        MOV     r0, #OSHeap_Resize      ; Reason code to resize the heap
        LDR     r1, ws_dynamic_ptr      ; Get pointer to heap
        SWI     XOS_Heap                ; Grow heap to match area size
        RTSS                            ; Return from subroutine
end$l   LDR     r0, ws_dynamic_area     ; Get dynamic area number
        LDR     r1, ws_dynamic_size     ; Get size of area
        RSB     r1, r1, #0              ; Negate size to shrink area
        SWI     XOS_ChangeDynamicArea   ; Minimise the dynamic area size
        MOV     r0, #0                  ; New size of dynamic area
        STR     r0, ws_dynamic_size     ; Store new size of area
        RTSS                            ; Return from subroutine

; Conversions between DOS and RISC OS time and date formats

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Initialise date and time conversions. This reads the
        ;                 current time zone information.
date_initialise
        LocalLabels
        JSR     "r0-r1"                 ; Stack registers
        SWI     XTerritory_ReadCurrentTimeZone; Read time zone information
        MOVVS   r1, #0                  ; Assume GMT if error
        STR     r1, ws_time_zone        ; Store time zone
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - The high byte of the date and time.
        ;                 r1    - The low word of the date and time.
        ;   Returns     : r0    - The equivalent 2 byte time.
        ;                 r1    - The equivalent 2 byte date.
        ;   Description : Convert a 5 byte RISC OS time into the equivalent
        ;                 DOS format time.
date_to_dos
        LocalLabels
        JSR     "r2-r6"                 ; Stack registers
        LDR     r2, ws_time_zone        ; Time zone offset
        ADDS    r1, r1, r2              ; Add offset to low word
        ADC     r0, r0, #0              ; Add offset to high byte
        LDR     r2, = &1ff              ; Mask for remainder
        AND     r2, r1, r2              ; Remainder after division by 512
        MOV     r1, r1, LSR#9           ; Divide low word by 512
        ORR     r1, r1, r0, LSL#32 - 9  ; Include high byte divided by 512
        LDR     r3, = 16875             ; Remaining divisor
        DivRem  r0, r1, r3, r4          ; Finish division to get days
        ORR     r1, r2, r1, LSL#9       ; Combine result with previous cs value
        MOV     r3, #200                ; Divisor for double seconds
        DivRem  r2, r1, r3, r4          ; Calculate double seconds
        MOV     r3, #30                 ; Divisor for minutes
        DivRem  r1, r2, r3, r4          ; Calculate minutes
        MOV     r4, #60                 ; Divisor for hours
        DivRem  r3, r1, r4, r5          ; Calculate hours
        ORR     r2, r2, r1, LSL#5       ; Combine seconds and minutes
        ORR     r2, r2, r3, LSL#11      ; Combine hours also
        MOV     r1, r0                  ; Copy days value
        MOV     r0, r2                  ; Copy time result
        LDR     r2, = 29219             ; Offset for 1980
        SUBS    r1, r1, r2              ; Subtract offset
        BMI     fail$l                  ; Fail if out of range
        LDR     r2, = 43889             ; Days until end of February 2100
        CMP     r1, r2                  ; Should an extra day be inserted
        ADDHS   r1, r1, #1              ; Fake 29th February 2100
        LDR     r3, = 365 * 3 + 366     ; Divisor for 4 years
        DivRem  r2, r1, r3, r4          ; Calculate 4 year multiple
        MOV     r2, r2, LSL#2           ; Convert to years
        LDR     r3, = 366               ; Days in a leap year
        CMP     r1, r3                  ; Is it in first year
        BLT     year$l                  ; Skip next bit if it is
        SUB     r1, r1, r3              ; Decrease number of days
        ADD     r2, r2, #1              ; Increment number of years
        SUB     r3, r3, #1              ; Number of days in other years
        CMP     r1, r3                  ; Is it in second year
        BLT     year$l                  ; Skip next bit if it is
        SUB     r1, r1, r3              ; Decrease number of days
        ADD     r2, r2, #1              ; Increment number of years
        CMP     r1, r3                  ; Is it in third year
        BLT     year$l                  ; Skip next bit if it is
        SUB     r1, r1, r3              ; Decrease number of days
        ADD     r2, r2, #1              ; Increment number of years
year$l  MOV     r3, #0                  ; Start from first month
        ADR     r4, date_month          ; Pointer to table of days in months
        MOV     r5, #0                  ; Start with no days covered
loop$l  ADD     r3, r3, #1              ; Increment month number
        MOV     r6, r5                  ; Copy previous total
        LDR     r5, [r4, r3, LSL#2]     ; Get cumulative days
        CMP     r3, #2                  ; Is it after February
        BLT     nly$l                   ; Skip next bit if not
        TST     r2, #3                  ; Is it a leap year
        ADDEQ   r5, r5, #1              ; Add extra day for leap year
nly$l   CMP     r1, r5                  ; Has correct month been found
        BGE     loop$l                  ; Loop if not
        SUB     r1, r1, r6              ; Calculate days into month
        ADD     r1, r1, #1              ; Correct day number
        ADD     r1, r1, r2, LSL#9       ; Combine day and year
        ADD     r1, r1, r3, LSL#5       ; Combine month also
        RTSS                            ; Return from subroutine
fail$l  MOV     r0, #0                  ; Blank time value
        MOV     r1, #0                  ; Invalid date value
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - The 2 byte time.
        ;                 r1    - The 2 byte date.
        ;   Returns     : r0    - The high byte of the date and time.
        ;                 r1    - The low word of the date and time.
        ;   Description : Convert a DOS format time and date into the
        ;                 equivalent 5 byte RISC OS format time.
date_to_riscos
        LocalLabels
        JSR     "r2-r5"                 ; Stack registers
        MOV     r2, #80                 ; Offset for the year
        ADD     r2, r2, r1, LSR#9       ; Extract the year and offset
        MOV     r3, r1, LSR#5           ; Shift the month
        AND     r3, r3, #&f             ; Only keep bottom 4 bits
        SUBS    r3, r3, #1              ; Remove month offset
        BMI     fail$l                  ; Fail if invalid value
        CMP     r3, #12                 ; Is the month in range
        BGE     fail$l                  ; Fail if not
        AND     r1, r1, #&1f            ; Extract days
        SUBS    r1, r1, #1              ; Remove day offset
        BMI     fail$l                  ; Fail if invalid value
        CMP     r1, #31                 ; Is the day in range
        BGE     fail$l                  ; Fail if not
        ADD     r4, r2, r2, LSL#2       ; Calculate year * 5
        ADD     r1, r1, r4              ; Add year * 5
        ADD     r1, r1, r4, LSL#3       ; Add year * 40
        ADD     r1, r1, r4, LSL#6       ; Add year * 320
        ADR     r4, date_month          ; Pointer to table of months
        LDR     r4, [r4, r3, LSL#2]     ; Get days up to month
        ADD     r1, r1, r4              ; Days in year
        CMP     r3, #2                  ; Is it after February
        BLT     nly$l                   ; Skip next bit if before
        TEQ     r2, #200                ; Is it a special case year
        BEQ     nly$l                   ; Skip next bit if it is
        TST     r2, #3                  ; Is it a leap year
        ADDEQ   r1, r1, #1              ; Include extra day if it is
nly$l   ADD     r3, r2, #3              ; Add an extra offset to the year
        ADD     r1, r1, r3, LSR#2       ; Add leap year cumulative correction
        CMP     r2, #200                ; Is it after non leap year
        SUBGT   r1, r1, #1              ; Remove century correction
        SUB     r1, r1, #1              ; Remove 1900 correction
        MOV     r1, r1, LSL#3           ; Calculate days * 8
        ADD     r1, r1, r1, LSL#1       ; Convert days to hours
        MOV     r2, r0, LSR#11          ; Extract hours
        CMP     r2, #24                 ; Are the hours in range
        BGE     fail$l                  ; Fail if not
        ADD     r1, r1, r2              ; Include hours in result
        ADD     r1, r1, r1, LSL#1       ; Calculate hours * 3
        ADD     r1, r1, r1, LSL#2       ; Calculate hours * 15
        MOV     r1, r1, LSL#2           ; Convert hours to minutes
        MOV     r2, r0, LSR#5           ; Shift minutes
        AND     r2, r2, #&3f            ; Extract minutes
        CMP     r2, #60                 ; Are the minutes in range
        BGE     fail$l                  ; Fail if not
        ADD     r1, r1, r2              ; Include minutes in result
        AND     r0, r0, #&1f            ; Extract double seconds
        CMP     r0, #30                 ; Are the double seconds in range
        BGE     fail$l                  ; Fail if not
        ADD     r1, r1, r1, LSL#1       ; Calculate minutes * 3
        ADD     r1, r1, r1, LSL#2       ; Calculate minutes * 15
        ADD     r1, r0, r1, LSL#1       ; Calculate double seconds
        MOV     r2, r1, LSR#32 - 3      ; MSB of seconds * 4
        MOV     r3, r1, LSL#3           ; LSW of seconds * 4
        MOV     r4, r1, LSR#32 - 6      ; MSB of seconds * 32
        MOV     r5, r1, LSL#6           ; LSW of seconds * 32
        MOV     r0, r1, LSR#32 - 7      ; MSB of seconds * 64
        MOV     r1, r1, LSL#7           ; LSW of seconds * 64
        ADDS    r1, r1, r3              ; LSW of seconds * 36
        ADC     r0, r0, r2              ; MSB of seconds * 36
        ADDS    r1, r1, r5              ; LSW of centi-seconds
        ADC     r0, r0, r4              ; MSB of centi-seconds
        LDR     r2, ws_time_zone        ; Time zone offset
        SUBS    r1, r1, r2              ; Subtract offset from low word
        SBC     r0, r0, #0              ; Subtract offset from high byte
        RTSS                            ; Return from subroutine
fail$l  MOV     r0, #0                  ; Clear high byte
        MOV     r1, #0                  ; Clear low word
        RTSS                            ; Return from subroutine

        ; Table of cumulative days in non-leap year months
date_month
        &       0                       ; Before January
        &       31                      ; Before February
        &       59                      ; Before March
        &       90                      ; Before April
        &       120                     ; Before May
        &       151                     ; Before June
        &       181                     ; Before July
        &       212                     ; Before August
        &       243                     ; Before September
        &       273                     ; Before October
        &       304                     ; Before November
        &       334                     ; Before December
        &       365                     ; Days in whole year

; Execution of *commands with redirected input and output

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Initialise the command handler.
oscli_initialise
        LocalLabels
        JSR     "r0"                    ; Stack registers
        MOV     r0, #0                  ; Value to clear pointers with
        STR     r0, ws_oscli_head       ; Clear linked list head pointer
        RTSS                            ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Finalise the command handler.
oscli_finalise
        LocalLabels
        JSR     "r0-r1"                 ; Stack registers
loop$l  LDR     r0, ws_oscli_head       ; Pointer to first record
        TEQ     r0, #0                  ; Is record valid
        BEQ     done$l                  ; Skip next bit if not
        MOV     r1, #0                  ; Do not keep any error message
        BL      oscli_end               ; End this command
        B       loop$l                  ; Loop for next record
done$l  RTSS                            ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Reset the command handler.
oscli_reset
        LocalLabels
        JSR     ""                      ; Stack registers
        BL      oscli_finalise          ; Release all memory used
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Pointer to command to execute.
        ;   Returns     : r0    - Handle for the command.
        ;   Description : Start the execution of a *command.
oscli_start
        LocalLabels
        JSR     "r1"                    ; Stack registers
        MOV     r1, r0                  ; Copy pointer to command
        BL      alloc$l                 ; Allocate memory
        RTS VS                          ; Exit if failed
        BL      buf$l                   ; Prepare buffers
        RTS VS                          ; Exit if failed
        BL      oscli_init_stack        ; Initialise the new stack
        RTS VS                          ; Exit if failed
        BL      oscli_init_environment  ; Initialise the new environment
        RTS VS                          ; Exit if failed
        BL      cmd$l                   ; Start executing the command
        RTSS                            ; Return from subroutine

alloc$l JSR     "r1-r3"                 ; Stack registers
        MOV     r0, #OSModule_Alloc     ; Reason code to allocate memory
        LDR     r3, = oscli             ; Size of block required
        BL      dynamic_osmodule        ; Attempt to allocate memory
        RTE VS                          ; Exit if error produced
        LDR     r0, ws_oscli_head       ; Get previous head of list
        STR     r2, ws_oscli_head       ; Make this the head of the list
        TEQ     r0, #0                  ; Was there a previous head
        STRNE   r2, [r0, #oscli_prev]   ; Store previous pointer in next
        STR     r0, [r2, #oscli_next]   ; Store next entry for this record
        MOV     r0, #0                  ; There is no previous entry
        STR     r0, [r2, #oscli_prev]   ; Clear previous pointer
        STR     r0, [r2, #oscli_redir_in]; Default to no input redirection
        STR     r0, [r2, #oscli_redir_out]; Default to no output redirection
        STR     r12, [r2, #oscli_ws]    ; Store pointer to module workspace
        MOV     r0, r2                  ; Copy record pointer
        RTSS                            ; Return from subroutine

buf$l   JSR     "r0-r4"                 ; Stack registers
        MOV     r4, r0                  ; Copy record pointer
        MOV     r0, #0                  ; Default buffer flags
        LDR     r1, = oscli_in          ; Offset to start of buffer
        ADD     r1, r4, r1              ; Pointer to start of buffer
        LDR     r2, = oscli_buffer_size ; Size of buffer
        ADD     r2, r1, r2              ; Pointer to end of buffer
        MOV     r3, #-1                 ; Allocate a handle automatically
        SWI     XBuffer_Register        ; Register input buffer
        RTE VS                          ; Exit if error produced
        STR     r0, [r4, #oscli_in_handle]; Store buffer handle
        MOV     r0, #0                  ; Default buffer flags
        LDR     r1, = oscli_out         ; Offset to start of buffer
        ADD     r1, r4, r1              ; Pointer to start of buffer
        LDR     r2, = oscli_buffer_size ; Size of buffer
        ADD     r2, r1, r2              ; Pointer to end of buffer
        MOV     r3, #-1                 ; Allocate a handle automatically
        SWI     XBuffer_Register        ; Register output buffer
        RTE VS                          ; Exit if error produced
        STR     r0, [r4, #oscli_out_handle]; Store buffer handle
        MOV     r0, #0                  ; Value to clear end of input flag
        STRLT   r0, [r4, #oscli_eof]    ; Set end of input flag
        RTSS                            ; Return from subroutine

cmd$l   JSR     "r0, r12"               ; Stack registers
        MOV     r12, r0                 ; Copy command handle
        STR     r1, [r12, #oscli_restart]; Store command pointer
        ADR     r0, cmd_ret$l           ; Return address
        STMFD   r13!, {r0}              ; Stack return address
        BL      oscli_claim             ; Set environment
        BL      oscli_swap              ; Swap stacks
        LDR     r0, [r12, #oscli_restart]; Restore command pointer
        SWI     XOS_CLI                 ; Execute the command
        MOVVC   r0, #0                  ; No restart address
        STR     r0, [r12, #oscli_restart]; Store restart address
        BL      oscli_swap              ; Swap stacks
        STMFD   r13!, {r1-r3}           ; Stack registers
        LDR     r0, [r12, #oscli_restart]; Pointer to any error
        TEQ     r0, #0                  ; Is there a valid error pointer
        BEQ     cmd_ok$l                ; Skip the next bit if not
        ADD     r1, r12, #oscli_stack   ; Pointer to stack space
        MOV     r2, #OS_Error           ; Size of block to copy
cmd_loop$l
        SUBS    r2, r2, #4              ; Decrement number of bytes to copy
        BMI     cmd_done$l              ; Exit loop if finished
        LDR     r3, [r0, r2]            ; Read word from source buffer
        STR     r3, [r1, r2]            ; Write word to destination buffer
        B       cmd_loop$l              ; Loop for the next word
cmd_done$l
        MOV     r0, #-1                 ; Error indicator
        STR     r0, [r12, #oscli_restart]; Clear the restart address
cmd_ok$l
        LDMFD   r13!, {r1-r3}           ; Restore registers
        MOV     r0, #oscli_status_done  ; Set return status
        LDMFD   r13!, {pc}              ; Return to caller
cmd_ret$l
        BL      oscli_release           ; Restore environment
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Command handle.
        ;                 r1    - Number of bytes input.
        ;                 r2    - Pointer to input buffer.
        ;                 r3    - Pointer to output buffer.
        ;   Returns     : r0    - The status.
        ;                 r1    - Number of bytes output.
        ;   Description : Continue the exectution of a *command.
oscli_poll
        LocalLabels
        JSR     "r12"                   ; Stack registers
        MOV     r12, r0                 ; Copy command handle
        BL      ins$l                   ; Insert data into input buffer
        BL      cmd$l                   ; Continue command
        BL      rem$l                   ; Remove data from output buffer
        TEQ     r1, #0                  ; Was any output waiting
        MOVNE   r0, #oscli_status_active; Ensure output buffer is flushed
        RTSS                            ; Return from subroutine

ins$l   JSR     "r0-r3, r9"             ; Stack registers
        CMP     r1, #0                  ; Check size of input
        STRLT   r1, [r12, #oscli_eof]   ; Set end of input flag
        RTSS LE                         ; Exit if no input
        MOV     r3, r1                  ; Copy number of bytes
        LDR     r1, [r12, #oscli_in_handle]; Get input buffer number
        ORR     r1, r1, #&80000000      ; Set flag for block insertion
        MOV     r9, #InsV_Block         ; Vector number
        SWI     XOS_IntOff              ; Disable interrupts
        SWI     XOS_CallAVector         ; Insert the data into input buffer
        SWI     XOS_IntOn               ; Enable interrupts
        RTSS                            ; Return from subroutine

rem$l   JSR     "r0, r2-r3, r9"         ; Stack registers
        LDR     r1, [r12, #oscli_out_handle]; Get output buffer number
        MOV     r9, #CnpV               ; Vector number
        ClearFlags                      ; Flags to return number of entries
        SWI     XOS_IntOff              ; Disable interrupts
        SWI     XOS_CallAVector         ; Find number of entries in buffer
        SWI     XOS_IntOn               ; Enable interrupts
        ORR     r1, r1, r2, LSL#8       ; Calculate number of entries
        CMP     r1, #256                ; Compare to message buffer limit
        MOVGT   r1, #256                ; Limit bytes to remove
        MOV     r2, r3                  ; Copy buffer pointer
        MOV     r0, r3                  ; Make another copy of buffer pointer
        MOV     r3, r1                  ; Copy number of bytes to remove
        LDR     r1, [r12, #oscli_out_handle]; Get output buffer number
        ORR     r1, r1, #&80000000      ; Set flag for block insertion
        MOV     r9, #RemV_Block         ; Vector number
        SWI     XOS_IntOff              ; Disable interrupts
        SWI     XOS_CallAVector         ; Remove the data from output buffer
        SWI     XOS_IntOn               ; Enable interrupts
        SUB     r1, r2, r0              ; Calculate number of bytes removed
        MOVCS   r1, #0                  ; No bytes removed if buffer empty
        RTSS                            ; Return from subroutine

cmd$l   JSR     "r1"                    ; Stack registers
        LDR     r0, [r12, #oscli_restart]; Get restart address
        CMP     r0, #0                  ; Is pointer valid
        MOVLE   r0, #oscli_status_done  ; Set return status
        RTSS LE                         ; Exit if pointer not valid
        ADR     r0, cmd_ret$l           ; Return address
        STMFD   r13!, {r0}              ; Stack return address
        BL      oscli_claim             ; Set environment
        BL      oscli_swap              ; Swap supervisor stacks
        LDR     pc, [r12, #oscli_restart]; Restart where execution stopped
cmd_ret$l
        BL      oscli_release           ; Restore environment
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - Command handle.
        ;                 r1    - Pointer to buffer to receive any error
        ;                         message, or 0 to discard.
        ;   Returns     : None
        ;   Description : End the execution of a *command.
oscli_end
        LocalLabels
        JSR     ""                      ; Stack registers
        BL      cmd$l                   ; Kill the command if required
        BL      err$l                   ; Copy any error message
        BL      buf$l                   ; Deregister buffers
        BL      free$l                  ; Free the block of memory
        RTS                             ; Return from subroutine

cmd$l   JSR     "r0-r1, r12"            ; Stack registers
        MOV     r12, r0                 ; Copy command handle
        LDR     r0, [r12, #oscli_restart]; Get restart address
        CMP     r0, #0                  ; Is it a valid pointer
        RTSS LE                         ; Exit if not
        ADR     r0, cmd_ret$l           ; Return address
        STMFD   r13!, {r0}              ; Stack return address
        BL      oscli_claim             ; Set environment
        BL      oscli_swap              ; Swap supervisor stacks
        SWI     XOS_Exit                ; Kill the command
        BL      oscli_swap              ; Restore the original stack
cmd_ret$l
        BL      oscli_release           ; Restore the environment
        RTSS                            ; Return from subroutine

err$l   JSR     "r0, r2-r3"             ; Stack registers
        LDR     r2, [r0, #oscli_restart]; Get restart address
        CMP     r2, #0                  ; Was an error produced
        RTSS GE                         ; Exit if not
        ADD     r0, r0, #oscli_stack    ; Pointer to source buffer
        MOV     r2, #OS_Error           ; Size of block to copy
err_loop$l
        SUBS    r2, r2, #4              ; Decrement number of bytes to copy
        BMI     err_done$l              ; Exit loop if finished
        LDR     r3, [r0, r2]            ; Read word from source buffer
        STR     r3, [r1, r2]            ; Write word to destination buffer
        B       err_loop$l              ; Loop for the next word
err_done$l
        SetV                            ; Set error flag
        RTS                             ; Return from subroutine

buf$l   JSR     "r0-r1"                 ; Stack registers
        MOV     r1, r0                  ; Copy record pointer
        LDR     r0, [r1, #oscli_in_handle]; Get input buffer pointer
        SWI     XBuffer_Deregister      ; Deregister input buffer
        LDR     r0, [r1, #oscli_out_handle]; Get output buffer pointer
        SWI     XBuffer_Deregister      ; Deregister output buffer
        RTSS                            ; Return from subroutine

free$l  JSR     "r0-r2"                 ; Stack registers
        LDR     r1, [r0, #oscli_next]   ; Pointer to next record
        LDR     r2, [r0, #oscli_prev]   ; Pointer to previous record
        TEQ     r1, #0                  ; Is next record valid
        STRNE   r2, [r1, #oscli_prev]   ; Set previous field of next record
        TEQ     r2, #0                  ; Is previous record valid
        STRNE   r1, [r2, #oscli_next]   ; Set next field of previous record
        STREQ   r1, ws_oscli_head       ; Correct head pointer if required
        MOV     r2, r0                  ; Copy pointer to record
        MOV     r0, #OSModule_Free      ; Reason code to release memory
        BL      dynamic_osmodule        ; Free the block
        RTSS                            ; Return from subroutine

        ;   Parameters  : r12   - Command handle.
        ;   Returns     : None
        ;   Description : Swap the current supervisor stack with the one in
        ;                 the command record. The SVC and USR registers (except
        ;                 r12, r14 and r15) are also swapped.
oscli_swap
        LocalLabels
        STMFD   r13!, {r0-r11}          ; Stack some registers
        STMFD   r13, {r13-r14}^         ; Stack user bank registers
        SUB     r13, r13, #8            ; Correct stack pointer
        STMFD   r13!, {r14}             ; Stack link register
        BL      oscli_find_stack        ; Find start and end of stack
        SWI     XOS_IntOff              ; Disable interrupts
        LDMFD   r13!, {r14}             ; Restore link register
        ADD     r2, r12, #oscli_r13     ; Pointer to alternate stack pointer
        SWP     r13, r13, [r2]          ; Swap stack pointers
        ADD     r2, r12, #oscli_stack   ; Pointer to stack buffer
loop$l  LDR     r3, [r0]                ; Get word from actual stack
        LDR     r4, [r2]                ; Get word from buffer
        STR     r3, [r2], #4            ; Write word to buffer
        STR     r4, [r0], #4            ; Write word to actual stack
        TEQ     r0, r1                  ; Is the copy finished
        BNE     loop$l                  ; Loop if not finished
        LDMFD   r13, {r13-r14}^         ; Restore user bank registers
        NOP                             ; Allow banked registers to be correct
        ADD     r13, r13, #8            ; Correct stack pointer
        STMFD   r13!, {r14}             ; Stack link register
        SWI     XOS_IntOn               ; Reenable interrupts
        LDMFD   r13!, {r14}             ; Restore link register
        LDMFD   r13!, {r0-r11}          ; Restore other registers
        MOVS    pc, r14                 ; Return from subroutine

        ;   Parameters  : r0    - Command handle.
        ;   Returns     : None
        ;   Description : Prepare the supervisor stack for use by the command
        ;                 to be executed.
oscli_init_stack
        LocalLabels
        JSR     "r0-r3"                 ; Stack registers
        ADD     r1, r0, #oscli_stack    ; Offset to stack buffer
        ADD     r2, r1, #svc_stack_size ; End of stack buffer
        MOV     r3, #0                  ; Value to clear entries with
loop$l  STR     r3, [r1], #4            ; Clear this word
        TEQ     r1, r2                  ; Have all entries been cleared
        BNE     loop$l                  ; Loop if not
        MOV     r2, r0                  ; Copy command handle
        BL      oscli_find_stack        ; Find the supervisor stack
        SUB     r0, r1, #4 * 14         ; Fake stack pointer
        STR     r0, [r2, #oscli_r13]    ; Store fake stack pointer
        RTSS                            ; Return from subroutine

        ;   Parameters  : None
        ;   Returns     : r0    - Pointer to start of supervisor stack.
        ;                 r1    - First byte after end of stack.
        ;   Description : Calculate the position of the supervisor stack.
oscli_find_stack
        LocalLabels
        JSR     ""                      ; Stack registers
        LDR     r0, = &fffff            ; Mask to clear to MB boundary
        BIC     r0, r13, r0             ; Start of stack
        ADD     r1, r0, #svc_stack_size ; End of stack
        RTSS                            ; Return from subroutine

        ;   Parameters  : r12   - Command handle.
        ;   Returns     : None
        ;   Description : Set the environment for executing the command. This
        ;                 includes claiming any required vectors.
oscli_claim
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        MOV     r2, r12                 ; Copy command handle
        MOV     r0, #WrchV              ; Vector number to claim
        ADR     r1, oscli_wrchv         ; Pointer to vector handler
        SWI     XOS_Claim               ; Claim the wrchv vector
        MOV     r0, #RdchV              ; Vector number to claim
        ADR     r1, oscli_rdchv         ; Pointer to vector handler
        SWI     XOS_Claim               ; Claim the rdchv vector
        MOV     r0, #UpCallV            ; Vector number to claim
        ADR     r1, oscli_upcallv       ; Pointer to vector handler
        SWI     XOS_Claim               ; Claim the upcallv vector
        MOV     r0, #10                 ; Delay in cs before preempting
        ADR     r1, oscli_call_after    ; Routine to call
        SWI     XOS_CallAfter           ; Add a routine to call
        BL      oscli_environment       ; Swap environments
        RTSS                            ; Return from subroutine

        ;   Parameters  : r12   - Command handle.
        ;   Returns     : None
        ;   Description : Restore the environment used before the command was
        ;                 executed.
oscli_release
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        BL      oscli_environment       ; Swap environments
        MOV     r2, r12                 ; Copy command handle
        ADR     r0, oscli_call_after    ; Routine to call
        MOV     r1, r2                  ; Copy workspace pointer
        SWI     XOS_RemoveTickerEvent   ; Ensure no call after pending
        ADR     r0, oscli_call_back     ; Routine to call
        MOV     r1, r2                  ; Copy workspace pointer
        SWI     XOS_RemoveCallBack      ; Ensure no call after pending
        MOV     r0, #UpCallV            ; Vector number to release
        ADR     r1, oscli_upcallv       ; Pointer to vector handler
        SWI     XOS_Release             ; Release the upcallv vector
        MOV     r0, #RdchV              ; Vector number to release
        ADR     r1, oscli_rdchv         ; Pointer to vector handler
        SWI     XOS_Release             ; Release the rdchv vector
        MOV     r0, #WrchV              ; Vector number to release
        ADR     r1, oscli_wrchv         ; Pointer to vector handler
        SWI     XOS_Release             ; Release the wrchv vector
        RTSS                            ; Return from subroutine

        ;   Parameters  : r12   - Command handle.
        ;   Returns     : None
        ;   Description : Swap the environment with the stored copy.
        ;                 Input and output redirection is also swapped.
oscli_environment
        LocalLabels
        JSR     "r0-r4"                 ; Stack registers
        LDR     r0, [r12, #oscli_redir_in]; Get alternative input redirection
        LDR     r1, [r12, #oscli_redir_out]; Get alternative output redirection
        SWI     XOS_ChangeRedirection   ; Swap redirection handles
        STR     r0, [r12, #oscli_redir_in]; Store old input handle
        STR     r1, [r12, #oscli_redir_out]; Store old output handle
        ADD     r4, r12, #oscli_env     ; Pointer to stored environment
loop$l  LDMIA   r4, {r0-r3}             ; Read next handler details
        CMP     r0, #0                  ; Is it a valid entry
        RTSS LT                         ; Exit if not
        SWI     XOS_ChangeEnvironment   ; Swap the handler
        STMIA   r4!, {r0-r3}            ; Store the previous details
        B       loop$l                  ; Loop for the next handler

        ;   Parameters  : r0    - Command handle.
        ;   Returns     : None
        ;   Description : Initialise the stored environment for use by the
        ;                 command being executed.
oscli_init_environment
        LocalLabels
        JSR     "r0-r5"                 ; Stack registers
        MOV     r5, r0                  ; Copy command handle
        ADD     r4, r0, #oscli_env      ; Pointer to the stored environment
        MOV     r0, #OS_HandlerMemoryLimit; Set new application memory limit
        MOV     r1, #&8000              ; No application memory for the command
        MOV     r2, #0                  ; No workspace for memory limit
        MOV     r3, #0                  ; No buffer for memory limit
        STMIA   r4!, {r0-r3}            ; Add this handler record
        MOV     r0, #OS_HandlerExit     ; New exit handler
        ADR     r1, oscli_exit          ; Pointer to exit handler
        MOV     r2, r5                  ; Copy workspace pointer
        MOV     r3, #0                  ; No buffer for exit handler
        STMIA   r4!, {r0-r3}            ; Add this handler record
        MOV     r0, #OS_HandlerUpCall   ; Handler identifier
        ADR     r1, oscli_upcall        ; Pointer to upcall handler
        MOV     r2, r5                  ; Copy workspace pointer
        MOV     r3, #0                  ; No buffer for upcall
        STMIA   r4!, {r0-r3}            ; Add this handler record
        MOV     r0, #OS_HandlerError    ; Handler identifier
        ADR     r1, oscli_error         ; Pointer to error handler
        MOV     r2, r5                  ; Copy workspace pointer
        ADRL    r3, ws_buffer           ; Pointer to an error buffer
        STMIA   r4!, {r0-r3}            ; Add this handler record
        MOV     r0, #OS_HandlerUndefinedInstruction; Handler identifier
        BL      cur$l                   ; Use the current handler details
        MOV     r0, #OS_HandlerPrefetchAbort; Handler identifier
        BL      cur$l                   ; Use the current handler details
        MOV     r0, #OS_HandlerDataAbort; Handler identifier
        BL      cur$l                   ; Use the current handler details
        MOV     r0, #OS_HandlerAddressException; Handler identifier
        BL      cur$l                   ; Use the current handler details
        MOV     r0, #OS_HandlerCallBack ; Handler identifier
        BL      cur$l                   ; Use the current handler details
        MOV     r0, #OS_HandlerBreakPt  ; Handler identifier
        BL      cur$l                   ; Use the current handler details
        MOV     r0, #OS_HandlerEscape   ; Handler identifier
        BL      cur$l                   ; Use the current handler details
        MOV     r0, #OS_HandlerEvent    ; Handler identifier
        BL      cur$l                   ; Use the current handler details
        MOV     r0, #OS_HandlerUnusedSWI; Handler identifier
        BL      cur$l                   ; Use the current handler details
        MOV     r0, #OS_HandlerExceptionRegisters; Handler identifier
        BL      cur$l                   ; Use the current handler details
        MOV     r0, #OS_HandlerApplicationSpace; Handler identifier
        BL      cur$l                   ; Use the current handler details
        MOV     r0, #OS_HandlerCAO      ; Handler identifier
        BL      cur$l                   ; Use the current handler details
        MOV     r0, #-1                 ; Value to terminate list with
        STR     r0, [r4]                ; Terminate the list of handlers
        RTSS                            ; Return from subroutine

cur$l   JSR     "r0-r3"                 ; Stack registers
        MOV     r1, #0                  ; Read current handler
        MOV     r2, #0                  ; Read current workspace
        MOV     r3, #0                  ; Read current buffer
        SWI     XOS_ChangeEnvironment   ; Read current handler
        STMIA   r4!, {r0-r3}            ; Add this handler record
        RTSS                            ; Return from subroutine

        ;   Parameters  : r0    - The character to output.
        ;                 r12   - Command handle.
        ;   Returns     : None
        ;   Description : WrchV vector handler.
oscli_wrchv
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        MOV     r2, r0                  ; Copy character to insert
        MOV     r0, #OSByte_BufferInsert; Reason code to insert in buffer
        LDR     r1, [r12, #oscli_out_handle]; Get buffer handle
        SWI     XOS_Byte                ; Insert character in buffer
        BCC     done$l                  ; Return to caller if successful
        ADR     r0, oscli_wrchv         ; Restart address
        STR     r0, [r12, #oscli_restart]; Store restart address
        PULL                            ; Restore registers
        BL      oscli_swap              ; Swap stacks
        MOV     r0, #oscli_status_active; Set return status
        LDMFD   r13!, {pc}              ; Return to caller
done$l  PULL                            ; Restore registers
        LDMFD   r13!, {pc}^             ; Return to caller

        ;   Parameters  : r12   - Command handle.
        ;   Returns     : r0    - ASCII code if C clear, or error (&1B is
        ;                         escape) if C set.
        ;   Description : RdchV vector handler.
oscli_rdchv
        LocalLabels
        JSR     "r1, r2"                ; Stack registers
        MOV     r0, #OSByte_BufferRemove; Reason code to remove from buffer
        LDR     r1, [r12, #oscli_in_handle]; Get buffer handle
        SWI     XOS_Byte                ; Remove character from buffer
        BCC     done$l                  ; Return to caller if successful
        LDR     r0, [r12, #oscli_eof]   ; Has end of input been reached
        TEQ     r0, #0                  ; Is the flag set
        BNE     esc$l                   ; Generate an escape if it is
        ADR     r0, oscli_rdchv         ; Restart address
        STR     r0, [r12, #oscli_restart]; Store restart address
        PULL                            ; Restore registers
        BL      oscli_swap              ; Swap stacks
        MOV     r0, #oscli_status_input ; Set return status
        LDMFD   r13!, {pc}              ; Return to caller
done$l  MOV     r0, r2                  ; Copy character extracted
        PULL                            ; Restore registers
        LDMFD   r13!, {pc}              ; Return to caller
esc$l   PULL                            ; Restore registers
        MOV     r0, #OS_VDUEscape       ; Code for escape
        SetC                            ; Set carry flag
        LDMFD   r13!, {pc}              ; Return to caller

        ;   Parameters  : r0    - Reason code.
        ;                 r12   - Command handle.
        ;   Returns     : Depends upon reason code.
        ;   Description : UpCallV vector handler.
oscli_upcallv
        LocalLabels
        TEQ     r0, #UpCall_NewApplication; Is a new application being started
        BEQ     new$l                   ; Jump to handler if it is
        MOVS    pc, r14                 ; Pass on to previous

new$l   MOV     r0, #0                  ; Stop the application from starting
        LDMFD   r13!, {pc}              ; Claim the call

        ;   Parameters  : r12   - Command handle.
        ;   Returns     : None
        ;   Description : The exit handler has been called so restore the
        ;                 original environment and exit.
oscli_exit
        LocalLabels
        SWI     XOS_EnterOS             ; Enter supervisor mode
        MOV     r0, #0                  ; No restart address
        STR     r0, [r12, #oscli_restart]; Clear restart address
        BL      oscli_swap              ; Swap stacks
        MOV     r0, #oscli_status_done  ; Set done status
        LDMFD   r13!, {pc}              ; Return to caller

        ;   Parameters  : r0    - Command handle.
        ;   Returns     : None
        ;   Description : The error handler has been called so restore the
        ;                 original environment and exit.
oscli_error
        LocalLabels
        MOV     r12, r0                 ; Copy workspace pointer
        SWI     XOS_EnterOS             ; Enter supervisor mode
        BL      oscli_swap              ; Swap stacks
        STMFD   r13!, {r1-r3}           ; Stack registers
        MOV     r0, #OS_HandlerError    ; Handler identifier
        MOV     r1, #0                  ; Do not change the handler
        MOV     r2, #0                  ; Do not change the workspace
        MOV     r3, #0                  ; Do not change the buffer pointer
        SWI     XOS_ChangeEnvironment   ; Read the current handler details
        ADD     r0, r3, #4              ; Pointer to error block
        ADD     r1, r12, #oscli_stack   ; Pointer to stack space
        MOV     r2, #OS_Error           ; Size of block to copy
loop$l  SUBS    r2, r2, #4              ; Decrement number of bytes to copy
        BMI     done$l                  ; Exit loop if finished
        LDR     r3, [r0, r2]            ; Read word from source buffer
        STR     r3, [r1, r2]            ; Write word to destination buffer
        B       loop$l                  ; Loop for the next word
done$l  LDMFD   r13!, {r1-r3}           ; Restore registers
        MOV     r0, #-1                 ; Value to indicate an error
        STR     r0, [r12, #oscli_restart]; Store error indicator
        MOV     r0, #oscli_status_done  ; Set done status
        LDMFD   r13!, {pc}              ; Return to caller

        ;   Parameters  : r12   - Command handle.
        ;   Returns     : None
        ;   Description : The upcall handler has been called so restore the
        ;                 original environment and exit.
oscli_upcall
        LocalLabels
        TEQ     r0, #UpCall_NewApplication; Is a new application being started
        BEQ     new$l                   ; Jump to handler if it is
        MOVS    pc, r14                 ; Return to caller

new$l   SWI     XOS_EnterOS             ; Enter supervisor mode
        MOV     r0, #0                  ; No restart address
        STR     r0, [r12, #oscli_restart]; Clear restart address
        BL      oscli_swap              ; Swap stacks
        MOV     r0, #oscli_status_done  ; Set done status
        LDMFD   r13!, {pc}              ; Return to caller

        ;   Parameters  : r12   - Command handle.
        ;   Returns     : None
        ;   Description : Called a short time after command started to ensure
        ;                 output is displayed quickly.
oscli_call_after
        LocalLabels
        JSR     "r0-r1"                 ; Stack registers
        ADR     r0, oscli_call_back     ; Address of call back handler
        MOV     r1, r12                 ; Copy command handle
        SWI     XOS_AddCallBack         ; Install the real handler
        RTS                             ; Return from subroutine

        ;   Parameters  : r12   - Command handle.
        ;   Returns     : None
        ;   Description : Suspend the current command to allow input and
        ;                 output.
oscli_call_back
        LocalLabels
        JSR     "r0"                    ; Stack registers
        ADR     r0, cont$l              ; Continuation address
        STR     r0, [r12, #oscli_restart]; Store restart address
        BL      oscli_swap              ; Swap stacks
        MOV     r0, #oscli_restart      ; Set return status
        LDMFD   r13!, {pc}              ; Return to caller
cont$l  RTS                             ; Return from call back

        [       pipe

; Tracing code

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Open the pipe output file.
pipe_open
        LocalLabels
        JSR     "r0-r2"                 ; Stack registers
        MOV     r0, #OSFind_Openout     ; Reason code to create a new pipe
        ADR     r1, filename$l          ; Name of pipe
        SWI     XOS_Find                ; Open the pipe file
        STR     r0, ws_pipe_handle      ; Store pipe handle
        RTS                             ; Return from subroutine

        ; The filename of the pipe
filename$l
        =       "pipe:$.ARMEdit.Trace", 0
        ALIGN

        ;   Parameters  : None
        ;   Returns     : None
        ;   Description : Close the pipe output file.
pipe_close
        LocalLabels
        JSR     "r0, r1"                ; Stack registers
        MOV     r0, #OSFind_Close       ; Reason code to close the pipe
        LDR     r1, ws_pipe_handle      ; Get the pipe handle
        SWI     XOS_Find                ; Close the pipe file
        RTS                             ; Return from subroutine

        ;   Parameters  : r0    - Pointer to the template string to write.
        ;                 r1    - Length of template string.
        ;   Returns     : None
        ;   Description : Write the specified template string to the pipe.
        ;                 Any occurrencies of %0 to %9 are replaced by the
        ;                 appropriate entry from the argument list.
pipe_write
        LocalLabels
        JSR     "r0-r4"                 ; Stack registers
        MOV     r3, r0                  ; Copy template string pointer
        MOV     r4, r1                  ; Copy template string length
        LDR     r0, ws_pipe_output      ; Pointer to end of argument list
        MOV     r1, #0                  ; Null terminator
        STRB    r1, [r0]                ; Terminate argument list
        ADRL    r0, ws_pipe_args        ; Pointer to argument list
        ORR     r0, r0, #1 << 31        ; Set the top bit to prevent appending
        ADRL    r1, ws_pipe_result      ; Pointer to output buffer
        MOV     r2, #String             ; Length of output buffer
        SWI     XOS_SubstituteArgs      ; Perform any required substitution
        RTE VS                          ; Return if error produced
        MOV     r2, r1                  ; Copy pointer to string
        LDR     r1, ws_pipe_handle      ; Get the pipe handle
write$l LDRB    r0, [r2], #1            ; Read a character from the string
        TEQ     r0, #0                  ; Is it the end of the string
        BEQ     done$l                  ; Exit loop if it is
        SWI     XOS_BPut                ; Write the character
        RTE VS                          ; Return if error produced
        B       write$l                 ; Loop for the next character
done$l  MOV     r0, #13                 ; Carriage return character
        SWI     XOS_BPut                ; Write the carriage return character
        RTE VS                          ; Return if error produced
        MOV     r0, #10                 ; Line feed character
        SWI     XOS_BPut                ; Write line feed character
        RTE VS                          ; Return if error produced
        MOV     r0, #OSArgs_Ensure      ; Reason code to flush buffer
        SWI     XOS_Args                ; Flush the buffer
        RTE VS                          ; Return if error produced
        RTS                             ; Return from subroutine

        ]

; Usage of workspace

                    ^ 0, r12
ws_start            * @                 ; Start of workspace
ws_message_ptr      # Ptr               ; Pointer to memory for messages
ws_message          # MessageTrans_ControlBlock; Message file control block
ws_pc_active        # Int               ; Is the PC front-end active
ws_hdr_current_ver  # Int               ; Last hardware structure version
ws_hdr_min_compat_ver # Int             ; Last hardware structure version
ws_fe_current_ver   # Int               ; Last frontend structure version
ws_fe_min_compat_ver # Int              ; Last frontend structure version
ws_hdr_ver          # Int               ; Indexed hardware structure version
ws_fe_ver           # Int               ; Indexed front-end structure version
ws_hdr_state        # Ptr               ; Pointer to hardware state structure
ws_fe_state         # Ptr               ; Pointer to front-end state structure
ws_event_reset      # call_evt_list     ; Hard reset event handler record
ws_event_shutdown   # call_evt_list     ; Shutdown event handler record
ws_event_poll       # call_evt_list     ; Poll event handler record
ws_event_config     # call_evt_list     ; Set configuration handler record
ws_config           # call_cfg_list     ; Unknown configuration item handler
ws_hpc_handler      # hpc_handler       ; HPC handler record
ws_packet_ptr       # Ptr               ; Pointer to current HPC packet
ws_swi              # Bits * 2          ; Code in workspace
ws_pc_memory        # Ptr               ; Pointer to start of allocations
ws_pc_files         # Ptr               ; Pointer to files opened by PC
ws_control_cb_block # callback          ; Callback descriptor block
ws_clients          # Ptr               ; Pointer to first client
ws_client_handle    # Int               ; The last client handle
ws_client_next      # Int               ; The next message number
ws_client_armedit   # client_small      ; Messages from this module
ws_time_zone        # Int               ; Time zone offset
ws_dosmap_start     # Ptr               ; Start of list of mappings
ws_dosmap_end       # Ptr               ; End of list of mappings
ws_device_prefix    # String            ; Prefix to configuration command line
ws_device_suffix    # String            ; Suffix to configuration command line
ws_device_head      # Ptr               ; Pointer to first device driver
ws_device_first     # Int               ; The most recent first drive number
ws_device_limit     # Int               ; The maximum size object to include
ws_device_cluster_shift # Int           ; Shift from sectors to clusters
ws_device_root_directory_sectors # Int  ; Number of sectors for root directory
ws_device_total_clusters # Int          ; Number of clusters
ws_device_sectors_per_fat # Int         ; Number of sectors per FAT
ws_device_skip_sectors # Int            ; The offset to the first data sector
ws_device_total_sectors # Int           ; The total number of sectors
ws_device_boot      # boot              ; The device driver boot sector
ws_buffer           # String            ; A general purpose buffer
ws_emulate_status   # Int               ; Current status of HPC emulation
ws_emulate_ptr      # Int               ; Current position pointer of emulation
ws_dynamic_area     # Int               ; Dynamic area number
ws_dynamic_size     # Int               ; Current size of dynamic area
ws_dynamic_ptr      # Ptr               ; Pointer to dynamic area base
ws_dynamic_enable   # Int               ; Should dynamic area be used
ws_oscli_head       # Ptr               ; Pointer to list of command handlers
ws_activity_fore    # Int               ; Foreground activity limit
ws_activity_back    # Int               ; Background activity limit
ws_activity_last    # Int               ; The previous activity timeout
ws_activity_delay   # Int               ; Time to resume multitasking
        [       pipe
ws_pipe_handle      # Int               ; Pipe file handle
ws_pipe_args        # String            ; A string to contain the arguments
ws_pipe_result      # String            ; A string to contain the result
ws_pipe_output      # Ptr               ; Current position in argument string
        ]
ws_temporary_name   # String            ; Temporary filename
ws_message_buffer   # hpc_packet        ; Buffer to hold emulated HPC packets
ws_end              * @                 ; End of workspace

        END
