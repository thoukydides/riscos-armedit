; File          : armdrv.asm
; Date          : 15-May-01
; Author        : © A.Thoukydides, 1995-2001, 2019
; Description   : Device driver to access RISC OS files.
;
; License       : ARMEdit is free software: you can redistribute it and/or
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

        name    ARMEdit
        title   'ARMEdit - Device Driver to access RISC OS'

.186
code    segment public 'CODE'

armedit proc    far

        assume  cs:code,ds:code,es:code,ss:code

        org     0

; Constants

max_cmd                 equ     16      ; DOS command code maximum
                                        ; this is 16 for DOS 3.x
                                        ; and 12 for DOS 2.x

cr                      equ     0dh     ; ASCII carriage return
lf                      equ     0ah     ; ASCII line feed
eom                     equ     '$'     ; End of message signal

p_stat                  equ     02e0h   ; Emulated HPC status port
p_cmd                   equ     02e0h   ; Emulated HPC command port
p_data                  equ     02e2h   ; Emulated HPC data port
hpc_id                  equ     0105h   ; HPC service identifier
hpc_size                equ     4000h   ; Size of HPC buffer block

        ; Offsets of useful fields in request header
head_field_unit         equ     1       ; Offset of unit code
head_field_cmd          equ     2       ; Offset of command field
head_field_status       equ     3       ; Offset of status field
head_field_number       equ     13      ; Offset of number of supported devices
head_field_media        equ     13      ; Offset of media descriptor byte
head_field_change       equ     14      ; Offset of media change code
head_field_addr         equ     14      ; Offset of transfer address
head_field_vol          equ     15      ; Offset of previous volume ID pointer
head_field_bpb          equ     18      ; Offset of BPB pointer or array
head_field_text         equ     18      ; Offset of pointer to configuration
head_field_count        equ     18      ; Offset of sector count
head_field_start        equ     20      ; Offset of starting sector number
head_field_first        equ     22      ; Offset of first drive number
head_field_volerr       equ     22      ; Offset of volume ID if error returned
head_field_start32      equ     26      ; Offset of 32bit sector number

        ; Codes returned by HPC function call
hpc_return_success      equ     00000h  ; Service successful
hpc_return_busy         equ     00001h  ; ARM interface is already in use
hpc_return_not_working  equ     00002h  ; ARM interface is not working
hpc_return_failed       equ     00003h  ; Service failed
hpc_return_unknown      equ     0ffffh  ; Service unknown

        ; Start of the module
the_start               equ     this byte; First byte of the driver

; Device driver header

header  dd      -1                      ; Link to next device, -1 = end of list
        dw      802h                    ; Device attribute word
        dw      strat                   ; Device "strategy" entry point
        dw      intr                    ; Device "interrupt" entry point
        db      0                       ; Number of units
        db      7 dup (0)               ; Pad to 8 bytes

; Local variables

        ; Various pointers
rh_ptr  dw      (?),(?)                 ; Address of the data block passed
bpb_ptr dw      4 dup (offset bpb)      ; Array of pointers to the BPB
bios_ptr dw     0ffeh, 0ff00h           ; Address of location in BIOS ROM

        ; Some data blocks
bpb     db      13 dup (?)              ; BIOS Parameter Block (BPB)
hpc_block dw    13 dup (?)              ; HPC parameter block
hpc_head db     16 dup (?)              ; A common HPC request header
hpc_ptr dd      (?)                     ; Address of HPC block

        ; A SWI number and register block
        if ($-header) mod 4
                org ($-header) + 4 - (($-header) mod 4)
        endif
swi_num dd      (?)                     ; The SWI number
swi_r0  dd      (?)                     ; ARM r0
swi_r1  dd      (?)                     ; ARM r1
swi_r2  dd      (?)                     ; ARM r2
swi_r3  dd      (?)                     ; ARM r3
swi_r4  dd      (?)                     ; ARM r4
swi_r5  dd      (?)                     ; ARM r5
swi_r6  dd      (?)                     ; ARM r6
swi_r7  dd      (?)                     ; ARM r7
swi_r8  dd      (?)                     ; ARM r8
swi_r9  dd      (?)                     ; ARM r9
swi_r10 dd      (?)                     ; ARM r10
swi_r11 dd      (?)                     ; ARM r11
swi_r12 dd      (?)                     ; ARM r12
swi_r13 dd      (?)                     ; ARM r13
swi_r14 dd      (?)                     ; ARM r14
swi_r15 dd      (?)                     ; ARM r15

; DOS command codes dispatch table

fkt_tab dw      init                    ; 0 = Initialise driver
        dw      media_check             ; 1 = Media check on block device
        dw      build_bpb               ; 2 = Build BIOS parameter block
        dw      dummy                   ; 3 = I/O control read
        dw      read                    ; 4 = Read from device
        dw      dummy                   ; 5 = Non-destructive read
        dw      dummy                   ; 6 = Return current input status
        dw      dummy                   ; 7 = Flush device input buffers
        dw      write                   ; 8 = Write to device
        dw      write                   ; 9 = Write with verify
        dw      dummy                   ; 10 = Return current output status
        dw      dummy                   ; 11 = Flush output buffers
        dw      dummy                   ; 12 = I/O control write
        dw      open                    ; 13 = Device open (DOS 3.X)
        dw      close                   ; 14 = Device close (DOS 3.X)
        dw      removable               ; 15 = Removable media (DOS 3.X)
        dw      dummy                   ; 16 = Output until busy (DOS 3.X)

        ; Parameters    : es:bx - Pointer to request header.
        ; Returns       : All registers preserved.
        ; Description   : The device driver "strategy routine". The strategy
        ;                 routine is passed the address of the request header
        ;                 which it saves in a local variable and then returns
        ;                 to DOS.
strat   proc    far

        mov     cs:rh_ptr,bx            ; Save address of request header
        mov     cs:rh_ptr+2,es
        ret                             ; Back to DOS

strat   endp

        ; Parameters    : None
        ; Returns       : All registers preserved.
        ; Description   : The device driver "interrupt routine". This entry
        ;                 point is called by DOS immediately after the call
        ;                 to the "strategy routine", which saved the long
        ;                 address of the request header in the local variable
        ;                 rh_ptr. This uses the command code passed in the
        ;                 request header to transfer to the appropriate device
        ;                 handling routine. Each command code routine must
        ;                 place any necessary return information into the
        ;                 request header, then perform a near return with
        ;                 ax = status.
intr    proc    far

        push    ax                      ; Stack all registers
        push    bx
        push    cx
        push    dx
        push    ds
        push    es
        push    di
        push    si
        push    bp
        pushf                           ; Also store flag register

        push    cs                      ; Make local data addressable
        pop     ds

        les     di,dword ptr rh_ptr     ; Let es:di = request header

        mov     bl,es:[di+head_field_cmd]; Get bx = command code
        xor     bh,bh
        cmp     bx,max_cmd              ; Make sure it's legal
        jle     intr_ok                 ; Jump, function code is OK
        mov     ax,8003h                ; Set error bit and "unknown command"
        jmp     intr_done               ; Skip decoding

intr_ok:
        shl     bx,1                    ; Form index to dispatch table
        call    [fkt_tab+bx]            ; and branch to driver routine
                                        ; Should return ax = status
        les     di,dword ptr rh_ptr     ; Restore es:di = request header

intr_done:
        or      ax,0100h                ; Merge done bit into status
        mov     es:[di+head_field_status],ax; and store into request header

        popf                            ; Restore flag register
        pop     bp                      ; Restore all registers
        pop     si
        pop     di
        pop     es
        pop     ds
        pop     dx
        pop     cx
        pop     bx
        pop     ax

        ret                             ; Back to DOS

intr    endp

; Command code subroutines called by interrupt routine

        ; Parameters    : es:di - Pointer to request header.
        ; Returns       : ax    - 0 if successful, or 8000H + error code.
        ; Description   : Dummy function called for unused command codes.
dummy   proc    near

        mov     ax,0                    ; Set return code to success
        ret                             ; Return from subroutine

dummy   endp

        ; Parameters    : es:di - Pointer to request header.
        ; Returns       : ax    - 0 if successful, or 8000H + error code.
        ; Description   : Handle media check command code (1). It is called
        ;                 when there is a pending drive access other than a
        ;                 simple file read or write, and is passed the media
        ;                 descriptor byte for the disk that DOS assumes is
        ;                 in the drive.
        ;
        ;                 On entry the request header contains:
        ;                       +1      byte    Unit code
        ;                       +2      byte    Command code = 1
        ;                       +13     byte    Media descriptor byte
        ;
        ;                 On exit the request header contains:
        ;                       +3      word    Return status
        ;                       +14     byte    Media change code:
        ;                                               -1 if disk changed
        ;                                               0 if don't know
        ;                                               1 if not changed
        ;                       +15     dword   Pointer to previous volume ID,
        ;                                       if device attribute bit 11 = 1
        ;                                       and disk has been changed
media_check proc near

        push    es                      ; Stack registers
        push    di

        mov     hpc_block+2,5           ; Size of message block
        mov     hpc_block+4,offset hpc_head; Pointer to message block
        mov     hpc_block+6,ds
        mov     hpc_block+8,0           ; No second section of message

        mov     hpc_block+14,8          ; Size of block for return code
        mov     hpc_block+16,offset hpc_head; Pointer to reply block
        mov     hpc_block+18,ds
        mov     hpc_block+20,0          ; No second section of reply

        mov     word ptr hpc_head,hpc_id; Store service identifier
        mov     word ptr hpc_head+2,11h ; Store reason code
        mov     al,byte ptr es:[di+head_field_unit]; Get unit code
        mov     byte ptr hpc_head+4,al  ; Store unit code

        call    hpc                     ; Perform the HPC call
        cmp     ax,hpc_return_success   ; Was it successful
        jne     media_check_fail        ; Jump to error exit if a problem

        les     di,dword ptr rh_ptr     ; Restore request header address
        mov     al,byte ptr hpc_head+4  ; Get media change code
        mov     byte ptr es:[di+head_field_change],al; Store media change code

        mov     word ptr es:[di+head_field_vol],offset media_check_vol
        mov     word ptr es:[di+head_field_vol+2],ds; Store volume pointer

        pop     di                      ; Restore registers
        pop     es
        mov     ax,0                    ; Set return code to success
        ret                             ; Return from subroutine

media_check_vol:
        db      'NO NAME',0             ; Dummy previous volume ID

media_check_fail:
        pop     di                      ; Restore registers
        pop     es
        mov     byte ptr es:[di+head_field_change],0; Store don't know as code
        mov     ax,8007h                ; Set return code to unknown medium
        ret                             ; Return from subroutine

media_check endp

        ; Parameters    : es:di - Pointer to request header.
        ; Returns       : ax    - 0 if successful, or 8000H + error code.
        ; Description   : Handle build BIOS Parameter Block (BPB) command code
        ;                 (2). DOS uses this function to get a pointer to the
        ;                 valid BIOS parameter block for the current disk, and
        ;                 calls it when the "disk changed" code is returned by
        ;                 the media check routine, or if the "don't know" code
        ;                 is returned and there are no dirty buffers (buffers
        ;                 with changed data that has not yet been written to
        ;                 disk). Thus, a call to this function indicates that
        ;                 the disk has been legally changed.
        ;
        ;                 On entry the request header contains:
        ;                       +1      byte    Unit code
        ;                       +2      byte    Command code = 2
        ;                       +13     byte    Media descriptor byte
        ;                       +14     dword   Buffer address
        ;
        ;                 On exit the request header contains:
        ;                       +3      word    Return status
        ;                       +18     dword   Pointer to new BPB
build_bpb proc  near

        push    es                      ; Stack registers
        push    di

        mov     hpc_block+2,5           ; Size of message block
        mov     hpc_block+4,offset hpc_head; Pointer to message block
        mov     hpc_block+6,ds
        mov     hpc_block+8,0           ; No second section of message

        mov     hpc_block+14,4          ; Size of block for return code
        mov     hpc_block+16,offset hpc_head; Pointer to reply block
        mov     hpc_block+18,ds
        mov     hpc_block+20,13         ; Second part of reply is BPB
        mov     hpc_block+22,offset bpb ; Pointer to BPB
        mov     hpc_block+24,ds

        mov     word ptr hpc_head,hpc_id; Store service identifier
        mov     word ptr hpc_head+2,10h ; Store reason code
        mov     al,byte ptr es:[di+head_field_unit]; Get unit code
        mov     byte ptr hpc_head+4,al  ; Store unit code

        call    hpc                     ; Perform the HPC call
        cmp     ax,hpc_return_success   ; Was it successful
        jne     build_bpb_fail          ; Jump to error exit if a problem

        les     di,dword ptr rh_ptr     ; Restore request header address
        mov     word ptr es:[di+head_field_bpb],offset bpb; Store pointer
        mov     word ptr es:[di+head_field_bpb+2],ds; to BPB

        pop     di                      ; Restore registers
        pop     es
        mov     ax,0                    ; Set return code to success
        ret                             ; Return from subroutine

build_bpb_fail:
        pop     di                      ; Restore registers
        pop     es
        mov     ax,8007h                ; Set return code to unknown medium
        ret                             ; Return from subroutine

build_bpb endp

        ; Parameters    : es:di - Pointer to request header.
        ; Returns       : ax    - 0 if successful, or 8000H + error code.
        ; Description   : Handle read command code (4). The read function
        ;                 transfers data from the device into the specified
        ;                 memory buffer. If an error is encountered during the
        ;                 read, the function must set the error status and,
        ;                 in addition, report the number of sectors
        ;                 successfully transferred. It is not sufficient to
        ;                 simply report an error.
        ;
        ;                 On entry the request header contains:
        ;                       +1      byte    Unit code
        ;                       +2      byte    Command code = 4
        ;                       +13     byte    Media descriptor byte
        ;                       +14     dword   Transfer address
        ;                       +18     word    Sector count
        ;                       +20     word    Starting sector number
        ;                       +22     dword   Volume ID if error 0FH returned
        ;                       +26     dword   32 bit start sector
        ;
        ;                 On exit the request header contains:
        ;                       +3      word    Return status
        ;                       +18     word    Actual sectors transferred
read    proc    near

        push    es                      ; Stack registers
        push    di
        push    bx
        push    cx
        push    dx

        mov     bx,word ptr es:[di+head_field_count]; Number of sectors to read
        mov     cl,byte ptr es:[di+head_field_unit]; Unit code
        mov     dx,word ptr es:[di+head_field_start]; Number of first sector
        mov     si,0                    ; Assume small sector number
        cmp     dx,0ffffh               ; Is start sector more than 16 bits
        jne     read_addr               ; Skip the next bit if not
        mov     dx,word ptr es:[di+head_field_start32]; Number of first sector
        mov     si,word ptr es:[di+head_field_start32+2]; and the high word
read_addr:
        les     di,dword ptr es:[di+head_field_addr]; Address of transfer

read_loop:
        cmp     bx,0                    ; Any sectors left to transfer
        je      read_done               ; Exit loop if not
        call    read_sector             ; Read the next sector
        cmp     ax,0                    ; Was it successful
        jne     read_fail               ; Exit loop if not
        jmp     read_loop               ; Loop for next sector

read_done:
        pop     dx                      ; Restore registers
        pop     cx
        pop     bx
        pop     di
        pop     es
        mov     ax,0                    ; Set return code to success
        ret                             ; Return from subroutine

read_fail:
        les     di,dword ptr rh_ptr     ; Restore request header address
        mov     ax,word ptr es:[di+head_field_count]; Number of sectors required
        sub     ax,bx                   ; Number of sectors transferred
        mov     word ptr es:[di+head_field_count],ax; Store transferred count
        pop     dx                      ; Restore registers
        pop     cx
        pop     bx
        pop     di
        pop     es
        mov     ax,800bh                ; Set return code to read fault
        ret                             ; Return from subroutine

read    endp

        ; Parameters    : bx    - Number of sectors left to read.
        ;                 cl    - Unit code to read from.
        ;                 dx    - Number of next sector to read.
        ;                 es:di - Address of buffer for next sector.
        ; Returns       : ax    - Return code (0 = success).
        ;                 bx    - Number of sectors left to read.
        ;                 dx    - Number of next sector to read.
        ;                 es:di - Address of buffer for next sector.
        ; Description   : Read a single sector.
read_sector proc near

        mov     hpc_block+2,12          ; Size of message block
        mov     hpc_block+4,offset hpc_head; Pointer to message block
        mov     hpc_block+6,ds
        mov     hpc_block+8,0           ; No second section of message

        mov     hpc_block+14,4          ; Size of block for return code
        mov     hpc_block+16,offset hpc_head; Pointer to reply block
        mov     hpc_block+18,ds
        mov     ax,word ptr bpb         ; Get bytes per sector
        mov     hpc_block+20,ax         ; Second part of reply is sector
        mov     hpc_block+22,di         ; Pointer to transfer address
        mov     hpc_block+24,es

        mov     word ptr hpc_head,hpc_id; Store service identifier
        mov     word ptr hpc_head+2,1ch ; Store reason code
        mov     byte ptr hpc_head+4,cl  ; Store unit code
        mov     word ptr hpc_head+8,dx  ; Store sector number
        mov     word ptr hpc_head+10,si ; and the high word

        call    hpc                     ; Perform the HPC call
        cmp     ax,hpc_return_success   ; Was it successful
        je      read_sector_done        ; Skip the retry if successful

        mov     hpc_block+2,7           ; Size of message block
        mov     hpc_block+4,offset hpc_head; Pointer to message block
        mov     hpc_block+6,ds
        mov     hpc_block+8,0           ; No second section of message

        mov     hpc_block+14,4          ; Size of block for return code
        mov     hpc_block+16,offset hpc_head; Pointer to reply block
        mov     hpc_block+18,ds
        mov     ax,word ptr bpb         ; Get bytes per sector
        mov     hpc_block+20,ax         ; Second part of reply is sector
        mov     hpc_block+22,di         ; Pointer to transfer address
        mov     hpc_block+24,es

        mov     word ptr hpc_head,hpc_id; Store service identifier
        mov     word ptr hpc_head+2,12h ; Store reason code
        mov     byte ptr hpc_head+4,cl  ; Store unit code
        mov     word ptr hpc_head+5,dx  ; Store sector number

        call    hpc                     ; Perform the HPC call
        cmp     ax,hpc_return_success   ; Was it successful
        jne     read_sector_fail        ; Jump to error exit if a problem

read_sector_done:
        dec     bx                      ; Decrement number of sectors left
        add     dx,1                    ; Increment number of next sector
        adc     si,0                    ; Add any cary to the high word
        push    dx                      ; Stack register so it can be used
        push    cx                      ; And another one
        mov     ax,word ptr bpb         ; Get bytes per sector
        add     di,ax                   ; Add sector size to address
        mov     dx,di                   ; Copy offset
        and     di,0fh                  ; Normalise offset
        mov     cl,4                    ; Number of bits to shift for /16
        shr     dx,cl                   ; Convert offset to segment change
        mov     ax,es                   ; Copy segment address
        add     ax,dx                   ; Update segment part of address
        mov     es,ax                   ; Transfer segment address to es
        pop     cx                      ; Restore registers
        pop     dx
        mov     ax,0                    ; Set return code for success
        ret                             ; Return from subroutine

read_sector_fail:
        mov     ax,1                    ; Set return code to indicate failed
        ret                             ; Return from subroutine

read_sector endp

        ; Parameters    : es:di - Pointer to request header.
        ; Returns       : ax    - 0 if successful, or 8000H + error code.
        ; Description   : Handle write and write with verify command codes
        ;                 (8 and 9). The write functions transfers data from
        ;                 the specified memory buffer to the device. If an
        ;                 error is encountered during the write, the write
        ;                 function must set the error status and, in addition,
        ;                 report the number of sectors successfully
        ;                 transferred. It is not sufficient to simply report
        ;                 an error.
        ;
        ;                 On entry the request header contains:
        ;                       +1      byte    Unit code
        ;                       +2      byte    Command code = 8 or 9
        ;                       +13     byte    Media descriptor byte
        ;                       +14     dword   Transfer address
        ;                       +18     word    Sector count
        ;                       +20     word    Starting sector number
        ;                       +22     dword   Volume ID if error 0FH returned
        ;                       +26     dword   32 bit start sector
        ;
        ;                 On exit the request header contains:
        ;                       +3      word    Return status
        ;                       +18     word    Actual sectors transferred
        ;                       +22     dword   Pointer to volume
        ;                                       identification if error 0FH
        ;                                       returned
write   proc    near

        push    es                      ; Stack registers
        push    di
        push    bx
        push    cx
        push    dx

        mov     bx,word ptr es:[di+head_field_count]; Number of sectors to write
        mov     cl,byte ptr es:[di+head_field_unit]; Unit code
        mov     dx,word ptr es:[di+head_field_start]; Number of first sector
        mov     si,0                    ; Assume small sector number
        cmp     dx,0ffffh               ; Is start sector more than 16 bits
        jne     write_addr              ; Skip the next bit if not
        mov     dx,word ptr es:[di+head_field_start32]; Number of first sector
        mov     si,word ptr es:[di+head_field_start32+2]; and the high word
write_addr:
        les     di,dword ptr es:[di+head_field_addr]; Address of transfer

write_loop:
        cmp     bx,0                    ; Any sectors left to transfer
        je      write_done              ; Exit loop if not
        call    write_sector            ; Write the next sector
        cmp     ax,0                    ; Was it successful
        jne     write_fail              ; Exit loop if not
        jmp     write_loop              ; Loop for next sector

write_done:
        pop     dx                      ; Restore registers
        pop     cx
        pop     bx
        pop     di
        pop     es
        mov     ax,0                    ; Set return code to success
        ret                             ; Return from subroutine

write_fail:
        les     di,dword ptr rh_ptr     ; Restore request header address
        mov     ax,word ptr es:[di+head_field_count]; Number of sectors required
        sub     ax,bx                   ; Number of sectors transferred
        mov     word ptr es:[di+head_field_count],ax; Store transferred count
        pop     dx                      ; Restore registers
        pop     cx
        pop     bx
        pop     di
        pop     es
        mov     ax,8000h                ; Set return code to write protected
        ret                             ; Return from subroutine

write   endp

        ; Parameters    : bx    - Number of sectors left to write.
        ;                 cl    - Unit code to write to.
        ;                 si:dx - Number of next sector to write.
        ;                 es:di - Address of buffer for next sector.
        ; Returns       : ax    - Return code (0 = success).
        ;                 bx    - Number of sectors left to write.
        ;                 si:dx - Number of next sector to write.
        ;                 es:di - Address of buffer for next sector.
        ; Description   : Write a single sector.
write_sector proc near

        mov     hpc_block+2,12          ; Size of message block
        mov     hpc_block+4,offset hpc_head; Pointer to message block
        mov     hpc_block+6,ds
        mov     ax,word ptr bpb         ; Get bytes per sector
        mov     hpc_block+8,ax          ; Second part of message is sector
        mov     hpc_block+10,di         ; Pointer to transfer address
        mov     hpc_block+12,es

        mov     hpc_block+14,4          ; Size of block for return code
        mov     hpc_block+16,offset hpc_head; Pointer to reply block
        mov     hpc_block+18,ds
        mov     hpc_block+20,0          ; No second part of reply

        mov     word ptr hpc_head,hpc_id; Store service identifier
        mov     word ptr hpc_head+2,1dh ; Store reason code
        mov     byte ptr hpc_head+4,cl  ; Store unit code
        mov     word ptr hpc_head+8,dx  ; Store sector number
        mov     word ptr hpc_head+10,si ; and the high word

        call    hpc                     ; Perform the HPC call
        cmp     ax,hpc_return_success   ; Was it successful
        je      write_sector_done       ; Skip the retry if successful

        mov     hpc_block+2,8           ; Size of message block
        mov     hpc_block+4,offset hpc_head; Pointer to message block
        mov     hpc_block+6,ds
        mov     ax,word ptr bpb         ; Get bytes per sector
        mov     hpc_block+8,ax          ; Second part of message is sector
        mov     hpc_block+10,di         ; Pointer to transfer address
        mov     hpc_block+12,es

        mov     hpc_block+14,4          ; Size of block for return code
        mov     hpc_block+16,offset hpc_head; Pointer to reply block
        mov     hpc_block+18,ds
        mov     hpc_block+20,0          ; No second part of reply

        mov     word ptr hpc_head,hpc_id; Store service identifier
        mov     word ptr hpc_head+2,13h ; Store reason code
        mov     byte ptr hpc_head+4,cl  ; Store unit code
        mov     word ptr hpc_head+5,dx  ; Store sector number

        call    hpc                     ; Perform the HPC call
        cmp     ax,hpc_return_success   ; Was it successful
        jne     write_sector_fail       ; Jump to error exit if a problem

write_sector_done:
        dec     bx                      ; Decrement number of sectors left
        add     dx,1                    ; Increment number of next sector
        adc     si,0                    ; Add any cary to the high word
        push    dx                      ; Stack register so it can be used
        push    cx                      ; And another one
        mov     ax,word ptr bpb         ; Get bytes per sector
        add     di,ax                   ; Add sector size to address
        mov     dx,di                   ; Copy offset
        and     di,0fh                  ; Normalise offset
        mov     cl,4                    ; Number of bits to shift for /16
        shr     dx,cl                   ; Convert offset to segment change
        mov     ax,es                   ; Copy segment address
        add     ax,dx                   ; Update segment part of address
        mov     es,ax                   ; Transfer segment address to es
        pop     cx                      ; Restore registers
        pop     dx
        mov     ax,0                    ; Set return code for success
        ret                             ; Return from subroutine

write_sector_fail:
        mov     ax,1                    ; Set return code to indicate failed
        ret                             ; Return from subroutine

write_sector endp

        ; Parameters    : es:di - Pointer to request header.
        ; Returns       : ax    - 0 if successful, or 8000H + error code.
        ; Description   : Handle Open command code (13). This can be used to
        ;                 increment a reference count of open files.
        ;
        ;                 On entry the request header contains:
        ;                       +1      byte    Unit code
        ;                       +2      byte    Command code = 13
        ;
        ;                 On exit the request header contains:
        ;                       +3      word    Return status
open    proc  near

        push    es                      ; Stack registers
        push    di

        mov     hpc_block+2,5           ; Size of message block
        mov     hpc_block+4,offset hpc_head; Pointer to message block
        mov     hpc_block+6,ds
        mov     hpc_block+8,0           ; No second section of message

        mov     hpc_block+14,4          ; Size of return block
        mov     hpc_block+16,offset hpc_head; Pointer to reply block
        mov     hpc_block+18,ds
        mov     hpc_block+20,0          ; No second section of reply

        mov     word ptr hpc_head,hpc_id; Store service identifier
        mov     word ptr hpc_head+2,1eh ; Store reason code
        mov     al,byte ptr es:[di+head_field_unit]; Get unit code
        mov     byte ptr hpc_head+4,al  ; Store unit code

        call    hpc                     ; Perform the HPC call

        pop     di                      ; Restore registers
        pop     es
        mov     ax,0                    ; Set return code to success
        ret                             ; Return from subroutine

open    endp

        ; Parameters    : es:di - Pointer to request header.
        ; Returns       : ax    - 0 if successful, or 8000H + error code.
        ; Description   : Handle Close command code (14). This can be used to
        ;                 decrement a reference count of open files.
        ;
        ;                 On entry the request header contains:
        ;                       +1      byte    Unit code
        ;                       +2      byte    Command code = 14
        ;
        ;                 On exit the request header contains:
        ;                       +3      word    Return status
close   proc  near

        push    es                      ; Stack registers
        push    di

        mov     hpc_block+2,5           ; Size of message block
        mov     hpc_block+4,offset hpc_head; Pointer to message block
        mov     hpc_block+6,ds
        mov     hpc_block+8,0           ; No second section of message

        mov     hpc_block+14,4          ; Size of return block
        mov     hpc_block+16,offset hpc_head; Pointer to reply block
        mov     hpc_block+18,ds
        mov     hpc_block+20,0          ; No second section of reply

        mov     word ptr hpc_head,hpc_id; Store service identifier
        mov     word ptr hpc_head+2,1fh ; Store reason code
        mov     al,byte ptr es:[di+head_field_unit]; Get unit code
        mov     byte ptr hpc_head+4,al  ; Store unit code

        call    hpc                     ; Perform the HPC call

        pop     di                      ; Restore registers
        pop     es
        mov     ax,0                    ; Set return code to success
        ret                             ; Return from subroutine

close   endp

        ; Parameters    : es:di - Pointer to request header.
        ; Returns       : ax    - 0 if removable or 200H if nonremovable.
        ; Description   : Handle the removable media code.
        ;
        ;                 On entry the request header contains:
        ;                       +1      byte    Unit code
        ;                       +2      byte    Command code = 15
        ;
        ;                 On exit the request header contains:
        ;                       +3      word    Return status
removable proc  near

        push    es                      ; Stack registers
        push    di

        mov     hpc_block+2,5           ; Size of message block
        mov     hpc_block+4,offset hpc_head; Pointer to message block
        mov     hpc_block+6,ds
        mov     hpc_block+8,0           ; No second section of message

        mov     hpc_block+14,8          ; Size of return block
        mov     hpc_block+16,offset hpc_head; Pointer to reply block
        mov     hpc_block+18,ds
        mov     hpc_block+20,0          ; No second section of reply

        mov     word ptr hpc_head,hpc_id; Store service identifier
        mov     word ptr hpc_head+2,20h ; Store reason code
        mov     al,byte ptr es:[di+head_field_unit]; Get unit code
        mov     byte ptr hpc_head+4,al  ; Store unit code

        call    hpc                     ; Perform the HPC call
        cmp     ax,hpc_return_success   ; Was it successful
        jne     removable_fail          ; Jump to error exit if a problem

        mov     ax,word ptr hpc_head+4  ; Get the removable device status
        pop     di                      ; Restore registers
        pop     es
        ret                             ; Return from subroutine

removable_fail:
        pop     di                      ; Restore registers
        pop     es
        mov     ax,0                    ; Set return code for removable
        ret                             ; Return from subroutine

removable endp

        ; Parameters    : None
        ; Returns       : ax    - Return code:
        ;                               0000h   Service successful
        ;                               0001h   ARM interface is already in use
        ;                               0002h   ARM interface is not working
        ;                               0003h   Service failed
        ;                               ffffh   Service unknown
        ; Description   : Execute an HPC call. The HPC parameter block hpb_blk
        ;                 should contain the required addresses and lengths.
        ;                 If the call is successful then the first word of the
        ;                 reply block is used to generate the return code.
hpc     proc    near

        push    es                      ; Stack registers
        push    di
        push    bx

        mov     hpc_block,0             ; Use HPC buffer A

        les     di,dword ptr bios_ptr   ; Get pointer into BIOS
        mov     ax,word ptr es:[di]     ; Read word from BIOS ROM
        cmp     ax,0ffffh               ; Is it the software emulator
        je      hpc_soft                ; Jump to software method if it is

        mov     ax,0                    ; Reason code to identify HPC services
        int     4dh                     ; Check if HPC services present
        cmp     ax,4850h                ; Are they present
        je      hpc_true                ; Jump to proper method if they are

        call    hpc_emu                 ; Perform an emulated HPC call
        jmp     hpc_done                ; Skip over proper method

hpc_soft:
        call    hpc_svc                 ; Perform HPC via SVC emulation
        jmp     hpc_done                ; Skip over proper method

hpc_true:
        push    cs                      ; Copy segment of parameter block
        pop     es
        mov     bx,offset hpc_block     ; Get offset to parameter block
        mov     ax,1                    ; Reason code to execute HPC call
        int     4dh                     ; Perform HPC call

hpc_done:
        cmp     ax,0                    ; Was the operation successful
        jne     hpc_exit                ; Skip next bit if call failed
        les     di,dword ptr hpc_block+16; Get pointer to reply buffer
        mov     ax,[es:di]              ; Get return code
        cmp     ax,1                    ; Is code success or unknown service
        jle     hpc_exit                ; Skip next bit if success or unknown
        add     ax,2                    ; Place code in correct range

hpc_exit:
        pop     bx                      ; Restore registers
        pop     di
        pop     es
        ret                             ; Return from subroutine

hpc     endp

        ; Parameters    : None
        ; Returns       : ax    - Return code as for hpc.
        ; Description   : Execute an HPC call using the SVC opcodes.
hpc_svc proc    near

        push    es                      ; Stack registers
        push    bx
        push    cx
        push    dx
        push    di
        push    si

        ; Convert and copy details of first input buffer
        mov     ax,hpc_block+2          ; Get length of buffer
        mov     word ptr swi_r0,ax      ; Store least significant word
        mov     word ptr swi_r0+2,0     ; Clear most significant word
        les     bx,dword ptr hpc_block+4; Get pointer to buffer
        dw      -1,257                  ; SVC257 translates to ARM address
        jc      hpc_svc_fail            ; Check for failure
        mov     word ptr swi_r1,ax      ; Save 32 bit ARM address
        mov     word ptr swi_r1+2,dx

        ; Convert and copy details of second input buffer
        mov     ax,hpc_block+8          ; Get length of buffer
        mov     word ptr swi_r2,ax      ; Store least significant word
        mov     word ptr swi_r2+2,0     ; Clear most significant word
        les     bx,dword ptr hpc_block+10; Get pointer to buffer
        dw      -1,257                  ; SVC257 translates to ARM address
        jc      hpc_svc_fail            ; Check for failure
        mov     word ptr swi_r3,ax      ; Save 32 bit ARM address
        mov     word ptr swi_r3+2,dx

        ; Convert and copy details of first output buffer
        mov     ax,hpc_block+14         ; Get length of buffer
        mov     word ptr swi_r4,ax      ; Store least significant word
        mov     word ptr swi_r4+2,0     ; Clear most significant word
        les     bx,dword ptr hpc_block+16; Get pointer to buffer
        dw      -1,257                  ; SVC257 translates to ARM address
        jc      hpc_svc_fail            ; Check for failure
        mov     word ptr swi_r5,ax      ; Save 32 bit ARM address
        mov     word ptr swi_r5+2,dx

        ; Convert and copy details of second output buffer
        mov     ax,hpc_block+20         ; Get length of buffer
        mov     word ptr swi_r6,ax      ; Store least significant word
        mov     word ptr swi_r6+2,0     ; Clear most significant word
        les     bx,dword ptr hpc_block+22; Get pointer to buffer
        dw      -1,257                  ; SVC257 translates to ARM address
        jc      hpc_svc_fail            ; Check for failure
        mov     word ptr swi_r7,ax      ; Save 32 bit ARM address
        mov     word ptr swi_r7+2,dx

        jmp     hpc_svc_ok              ; Skip over the error handler

hpc_svc_fail:
        mov     ax,2                    ; Reason code for failure
        jmp     hpc_svc_exit            ; Return from subroutine

hpc_svc_ok:

        ; Set SWI number to call
        mov     ax,0bc46h               ; Low word of SWI number to call
        mov     word ptr swi_num,ax     ; Store low word of SWI number
        mov     ax,0004h                ; High word of SWI number to call
        mov     word ptr swi_num+2,ax   ; Store high word of SWI number

        ; Perform SWI call
        mov     dx,'sa'                 ; First two characters of magic number
        mov     ax,'fe'                 ; Last two characters of magic number
        push    cs                      ; Copy segment of HPC buffer
        pop     es
        mov     bx,offset swi_num       ; Offset to SWI number and registers
        dw      -1,258                  ; SVC258 performs general purpose SWI
        jc      hpc_svc_fail            ; Bad parameter block if carry set

        ; Check if SWI produced an error
        mov     ax,word ptr swi_r15+2   ; Get most significant word of flags
        test    ax,1000h                ; Is the overflow flag set
        jnz     hpc_svc_fail            ; Bad SWI if V set

        mov     ax,0                    ; Return code to indicate success

hpc_svc_exit:
        pop     si                      ; Restore registers
        pop     di
        pop     dx
        pop     cx
        pop     bx
        pop     es

        ret                             ; Return from subroutine

hpc_svc endp

        ; Parameters    : None
        ; Returns       : ax    - Return code as for hpc.
        ; Description   : Execute an HPC using the I/O port emulation.
hpc_emu proc    near

        push    es                      ; Stack registers
        push    cx
        push    dx
        push    di
        push    si

        mov     dx,p_stat               ; Port to read status from
        in      ax,dx                   ; Read the status
        cmp     ax,4d45h                ; Is the emulation busy
        je      hpc_emu_busy            ; Skip to code to handle busy
        cmp     ax,454dh                ; Is the emulation available
        je      hpc_emu_avail           ; Skip to code to perform HPC call

        ; Handle emulated HPC not available
        mov     ax,2                    ; Return code to indicate not available
        jmp     hpc_emu_exit            ; Return to caller

        ; Handle emulated HPC is busy
hpc_emu_busy:
        mov     ax,1                    ; Return code to indicate busy
        jmp     hpc_emu_exit            ; Return to caller

        ; Perform the emulated HPC call
hpc_emu_avail:

        ; Send the data
        mov     dx,p_cmd                ; Port to write commands to
        mov     ax,0                    ; Reason code to start transmit
        out     dx,ax                   ; Write command to start sending

        mov     cx,hpc_block+2          ; Get length of first portion
        lds     si,dword ptr hpc_block+4; Get pointer to first buffer ds:si
        mov     dx,p_data               ; Port to write data to
        rep     outsb                   ; Output the first portion
        push    cs                      ; Make local data addressable again
        pop     ds

        mov     cx,hpc_block+8          ; Get length of second portion
        lds     si,dword ptr hpc_block+10; Get pointer to second buffer ds:si
        mov     dx,p_data               ; Port to write data to
        rep     outsb                   ; Output the second portion
        push    cs                      ; Make local data addressable again
        pop     ds

        ; Perform the call
        mov     dx,p_cmd                ; Port to write commands to
        mov     ax,1                    ; Reason code to start processing
        out     dx,ax                   ; Write command to start processing

        ; Receive the reply
        mov     ax,2                    ; Reason code to start receive
        out     dx,ax                   ; Write command to start reception

        mov     cx,hpc_block+14         ; Get length of first portion
        les     di,dword ptr hpc_block+16; Get pointer to first buffer es:di
        mov     dx,p_data               ; Port to read data from
        rep     insb                    ; Input the first portion

        mov     cx,hpc_block+20         ; Get length of second portion
        les     di,dword ptr hpc_block+22; Get pointer to second buffer es:di
        mov     dx,p_data               ; Port to read data from
        rep     insb                    ; Input the second portion

        ; End the transfer to allow the buffer to be released
        mov     dx,p_cmd                ; Port to write commands to
        mov     ax,3                    ; Reason code to release
        out     dx,ax                   ; Write command to release

        mov     ax,0                    ; Return code to indicate success

hpc_emu_exit:
        pop     si                      ; Restore registers
        pop     di
        pop     dx
        pop     cx
        pop     es

        ret                             ; Return from subroutine

hpc_emu endp

; This is the end of the resident portion of the driver
break_point:

        ; Parameters    : es:di - Pointer to request header.
        ; Returns       : ax    - 0 if successful, or 8000H + erro code.
        ; Description   : Handle driver initialisation command code (0).
        ;                 The initialisation function for the driver is called
        ;                 only once, when the driver is loaded. It is
        ;                 responsible for performing any necessary device
        ;                 hardware initialisation, setup of interrupt vectors,
        ;                 and so forth. It must return the address of the
        ;                 position where free memory begins after the driver
        ;                 code (the break address), so that DOS knows where
        ;                 it can build certain control structures and then
        ;                 load the next installable driver. It must also return
        ;                 the number of units and the address of a BPB pointer
        ;                 array. If all units are the same, all the pointers
        ;                 can point to the same BPB. Only DOS services
        ;                 01-0CH and 30H can be called by the initialisation
        ;                 function.
        ;
        ;                 On entry the request header contains:
        ;                       +2      byte    Command code = 0
        ;                       +18     dword   Pointer to character after
        ;                                       equal sign on CONFIG.SYS line
        ;                                       that loaded driver
        ;                       +22     byte    Drive number for first unit of
        ;                                       this block driver
        ;
        ;                 On exit the request header contains:
        ;                       +3      word    Return status
        ;                       +13     byte    Number of units
        ;                       +14     dword   Address of first free memory
        ;                                       above driver (break address)
        ;                       +18     dword   BPB pointer array
init    proc    near

        push    es                      ; Stack registers
        push    di
        push    dx

        ; Display the internal banner message
        push    cs                      ; Ensure local data is addressable
        pop     ds
        mov     ah,9                    ; Print sign-on message and
        mov     dx,offset init_banner   ; the load address of driver
        int     21h

        ; Initialise the communications and display the external banner message
        les     di,dword ptr rh_ptr     ; Restore request header address
        mov     hpc_block+2,5           ; Size of message block
        mov     hpc_block+4,offset hpc_head; Pointer to message block
        mov     hpc_block+6,ds
        mov     hpc_block+8,256         ; Size of text after equals sign
        mov     ax,word ptr es:[di+head_field_text]; Get pointer to text
        mov     hpc_block+10,ax         ; Store offset
        mov     ax,word ptr es:[di+head_field_text+2]
        mov     hpc_block+12,ax         ; Store segment

        mov     hpc_block+14,8          ; Size of block for return code
        mov     hpc_block+16,offset hpc_head; Pointer to reply block
        mov     hpc_block+18,ds
        mov     hpc_block+20,256        ; Second part of reply is a string
        mov     hpc_block+22,offset init_banner; to overwrite the initial banner
        mov     hpc_block+24,ds

        mov     word ptr hpc_head,hpc_id; Store service identifier
        mov     word ptr hpc_head+2,0fh ; Store reason code
        mov     al,byte ptr es:[di+head_field_first]; Get first drive number
        mov     byte ptr hpc_head+4,al  ; Store first drive number

        call    hpc                     ; Perform the HPC call
        cmp     ax,hpc_return_success   ; Was it successful
        je      init_ok                 ; Skip next bit if successful
        jmp     init_fail               ; Jump to error exit if a problem

init_ok:
        mov     ah,9                    ; Print ARM sign-on message
        mov     dx,offset init_banner
        int     21h

        les     di,dword ptr rh_ptr     ; Restore request header address

        mov     al,byte ptr hpc_head+4  ; Get number of devices
        mov     byte ptr es:[di+head_field_number],al; Store number of devices

        ; Get BIOS parameter block
        mov     hpc_block+2,5           ; Size of message block
        mov     hpc_block+4,offset hpc_head; Pointer to message block
        mov     hpc_block+6,ds
        mov     hpc_block+8,0           ; No second section of message

        mov     hpc_block+14,4          ; Size of block for return code
        mov     hpc_block+16,offset hpc_head; Pointer to reply block
        mov     hpc_block+18,ds
        mov     hpc_block+20,13         ; Second part of reply is BPB
        mov     hpc_block+22,offset bpb ; Pointer to BPB
        mov     hpc_block+24,ds

        mov     word ptr hpc_head,hpc_id; Store service identifier
        mov     word ptr hpc_head+2,10h ; Store reason code
        mov     byte ptr hpc_head+4,-1  ; Store fake unit code

        call    hpc                     ; Perform the HPC call
        cmp     ax,hpc_return_success   ; Was it successful
        jne     init_fail               ; Jump to error exit if a problem

        ; Fill in the remaining details
        les     di,dword ptr rh_ptr     ; Restore request header address

        mov     word ptr es:[di+head_field_bpb],offset bpb_ptr; Store pointer
        mov     word ptr es:[di+head_field_bpb+2],ds; to BPB pointer array

        mov     ax,offset break_point   ; Pointer to the module end
        mov     word ptr es:[di+head_field_addr],ax; Set first free memory
        mov     word ptr es:[di+head_field_addr+2],cs

        pop     dx                      ; Restore registers
        pop     di
        pop     es
        mov     ax,0                    ; Return status is no error
        ret                             ; Return from subroutine

        ; Handle errors during initialisation
init_fail:
        mov     ah,9                    ; Display error message
        mov     dx,offset init_error
        int     21h

        les     di,dword ptr rh_ptr     ; Restore request header address
        mov     byte ptr es:[di+head_field_number],0; No drives supported
        mov     word ptr es:[di+head_field_addr],0; Free memory address is
        mov     word ptr es:[di+head_field_addr+2],cs; start of the driver

        pop     dx                      ; Restore registers
        pop     di
        pop     es
        mov     ax,800ch                ; Return a general error
        ret                             ; Return from subroutine

init    endp

        ; The text to display if there is a problem
init_error:
        db      'ARMEdit initialisation failed',cr,lf,lf,eom

        ; The banner text that gets overwritten by the ARM message
init_banner:
        db      cr,lf,lf
        db      'ARMEdit Device Driver   1.05 (15-May-01) [TEST]'
        db      ' (c) A.Thoukydides, 1995-2001',cr,lf
        db      eom

armedit endp

code    ends

        end
