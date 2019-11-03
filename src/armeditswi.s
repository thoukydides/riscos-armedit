;   File        : armeditswi.s
;   Date        : 15-May-01
;   Author      : © A.Thoukydides, 1996-2001, 2019
;   Description : Veneers for calling ARMEdit SWIs from C.
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

        GET OS:Hdr.Macros
        GET OS:Hdr.OS

; Include my header files

        GET AT:Hdr.macros

; Exported symbols

        EXPORT xarmedit_control_pc, armedit_control_pc
        EXPORT xarmedit_talk_start, armedit_talk_start
        EXPORT xarmedit_talk_end, armedit_talk_end
        EXPORT xarmedit_talk_tx, armedit_talk_tx
        EXPORT xarmedit_talk_rx, armedit_talk_rx
        EXPORT xarmedit_talk_ack, armedit_talk_ack
        EXPORT xarmedit_hpc, armedit_hpc
        EXPORT xarmedit_polling, armedit_polling
        EXPORT xarmedit_talk_reply, armedit_talk_reply

; Constants

XARMEdit_ControlPC              *       &6BC40
ARMEdit_ControlPC               *       &4BC40
XARMEdit_TalkStart              *       &6BC41
ARMEdit_TalkStart               *       &4BC41
XARMEdit_TalkEnd                *       &6BC42
ARMEdit_TalkEnd                 *       &4BC42
XARMEdit_TalkTX                 *       &6BC43
ARMEdit_TalkTX                  *       &4BC43
XARMEdit_TalkRX                 *       &6BC44
ARMEdit_TalkRX                  *       &4BC44
XARMEdit_TalkAck                *       &6BC45
ARMEdit_TalkAck                 *       &4BC45
XARMEdit_HPC                    *       &6BC46
ARMEdit_HPC                     *       &4BC46
XARMEdit_Polling                *       &6BC47
ARMEdit_Polling                 *       &4BC47
XARMEdit_TalkReply              *       &6BC48
ARMEdit_TalkReply               *       &4BC48

; Define an area to dump everything into

        AREA    |C$$code|, CODE, READONLY

; The SWI veneers

; os_error *xarmedit_control_pc(int operation)
xarmedit_control_pc
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {lr}               ; Stack return address
        SWI     XARMEdit_ControlPC      ; Call the SWI
        MOVVC   r0, #0                  ; Clear error pointer if successful
        LDMFD   sp!, {pc}^              ; Return from subroutine

; void armedit_control_pc(int operation)
armedit_control_pc
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {lr}               ; Stack return address
        SWI     ARMEdit_ControlPC       ; Call the SWI
        LDMFD   sp!, {pc}^              ; Return from subroutine

; os_error *xarmedit_talk_start(int id, int flags, void *func, int r12,
;                               int *rhandle, int **rpoll)
xarmedit_talk_start
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {lr}               ; Stack return address
        SWI     XARMEdit_TalkStart      ; Call the SWI
        LDR     r3, [ip]                ; Get rhandle pointer
        TEQ     r3, #0                  ; Is rhandle a valid pointer
        STRNE   r0, [r3]                ; Store return r0 if it is
        LDR     r3, [ip, #4]            ; Get rpoll pointer
        TEQ     r3, #0                  ; Is rpoll a valid pointer
        STRNE   r1, [r3]                ; Store return r1 if it is
        MOVVC   r0, #0                  ; Clear error pointer if successful
        LDMFD   sp!, {pc}^              ; Return from subroutine

; int armedit_talk_start(int id, int flags, void *func, int r12, int *rhandle,
;                        int **rpoll)
armedit_talk_start
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {lr}               ; Stack return address
        SWI     ARMEdit_TalkStart       ; Call the SWI
        LDR     r3, [ip]                ; Get rhandle pointer
        TEQ     r3, #0                  ; Is rhandle a valid pointer
        STRNE   r0, [r3]                ; Store return r0 if it is
        LDR     r3, [ip, #4]            ; Get rpoll pointer
        TEQ     r3, #0                  ; Is rpoll a valid pointer
        STRNE   r1, [r3]                ; Store return r1 if it is
        LDMFD   sp!, {pc}^              ; Return from subroutine

; os_error *xarmedit_talk_end(int handle)
xarmedit_talk_end
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {lr}               ; Stack return address
        SWI     XARMEdit_TalkEnd        ; Call the SWI
        MOVVC   r0, #0                  ; Clear error pointer if successful
        LDMFD   sp!, {pc}^              ; Return from subroutine

; void armedit_talk_end(int handle)
armedit_talk_end
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {lr}               ; Stack return address
        SWI     ARMEdit_TalkEnd         ; Call the SWI
        LDMFD   sp!, {pc}^              ; Return from subroutine

; os_error *xarmedit_talk_tx(int handle, int dest, void *msg, void **rmsg)
xarmedit_talk_tx
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {lr}               ; Stack return address
        SWI     XARMEdit_TalkTX         ; Call the SWI
        TEQ     r3, #0                  ; Is rmsg a valid pointer
        STRNE   r2, [r3]                ; Store return r2 if it is
        MOVVC   r0, #0                  ; Clear error pointer if successful
        LDMFD   sp!, {pc}^              ; Return from subroutine

; void *armedit_talk_tx(int handle, int dest, void *msg, void **rmsg)
armedit_talk_tx
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {lr}               ; Stack return address
        SWI     ARMEdit_TalkTX          ; Call the SWI
        TEQ     r3, #0                  ; Is rmsg a valid pointer
        STRNE   r2, [r3]                ; Store return r2 if it is
        MOV     r0, r2                  ; Copy result value
        LDMFD   sp!, {pc}^              ; Return from subroutine

; os_error *xarmedit_talk_rx(int handle, void **rmsg, int *rid, int *rhandle)
xarmedit_talk_rx
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {v1-v2, lr}        ; Stack return address
        MOV     r4, r1                  ; Copy rmsg value
        MOV     r5, r2                  ; Copy rid value
        SWI     XARMEdit_TalkRX         ; Call the SWI
        TEQ     r4, #0                  ; Is rmsg a valid pointer
        STRNE   r0, [r4]                ; Store return r0 if it is
        TEQ     r5, #0                  ; Is rid a valid pointer
        STRNE   r1, [r5]                ; Store return r1 if it is
        TEQ     r3, #0                  ; Is rhandle a valid pointer
        STRNE   r2, [r3]                ; Store return r2 if it is
        MOVVC   r0, #0                  ; Clear error pointer if successful
        LDMFD   sp!, {v1-v2, pc}^       ; Return from subroutine

; void *armedit_talk_rx(int handle, void **rmsg, int *rid, int *rhandle)
armedit_talk_rx
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {v1-v2, lr}        ; Stack return address
        MOV     r4, r1                  ; Copy rmsg value
        MOV     r5, r2                  ; Copy rid value
        SWI     ARMEdit_TalkRX          ; Call the SWI
        TEQ     r4, #0                  ; Is rmsg a valid pointer
        STRNE   r0, [r4]                ; Store return r0 if it is
        TEQ     r5, #0                  ; Is rid a valid pointer
        STRNE   r1, [r5]                ; Store return r1 if it is
        TEQ     r3, #0                  ; Is rhandle a valid pointer
        STRNE   r2, [r3]                ; Store return r2 if it is
        LDMFD   sp!, {v1-v2, pc}^       ; Return from subroutine

; os_error *xarmedit_talk_ack(int handle)
xarmedit_talk_ack
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {lr}               ; Stack return address
        SWI     XARMEdit_TalkAck        ; Call the SWI
        MOVVC   r0, #0                  ; Clear error pointer if successful
        LDMFD   sp!, {pc}^              ; Return from subroutine

; void armedit_talk_ack(int handle)
armedit_talk_ack
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {lr}               ; Stack return address
        SWI     ARMEdit_TalkAck         ; Call the SWI
        LDMFD   sp!, {pc}^              ; Return from subroutine

; os_error *xarmedit_hpc(int tx1_size, void *tx1_buf, int tx2_size,
;                        void *tx2_buf, int rx1_size, void *rx1_buf,
;                        int rx2_size, void *rx2_buf)
xarmedit_hpc
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {v1-v4, lr}        ; Stack return address
        LDR     r4, [ip]                ; Get rx1_size
        LDR     r5, [ip, #4]            ; Get rx1_buf pointer
        LDR     r6, [ip, #8]            ; Get rx2_size
        LDR     r7, [ip, #12]           ; Get rx2_buf pointer
        SWI     XARMEdit_HPC            ; Call the SWI
        MOVVC   r0, #0                  ; Clear error pointer if successful
        LDMFD   sp!, {v1-v4, pc}^       ; Return from subroutine

; void armedit_hpc(int tx1_size, void *tx1_buf, int tx2_size, void *tx2_buf,
;                  int rx1_size, void *rx1_buf, int rx2_size, void *rx2_buf)
armedit_hpc
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {v1-v4, lr}        ; Stack return address
        LDR     r4, [ip]                ; Get rx1_size
        LDR     r5, [ip, #4]            ; Get rx1_buf pointer
        LDR     r6, [ip, #8]            ; Get rx2_size
        LDR     r7, [ip, #12]           ; Get rx2_buf pointer
        SWI     ARMEdit_HPC             ; Call the SWI
        LDMFD   sp!, {v1-v4, pc}^       ; Return from subroutine

; os_error *xarmedit_polling(int fore, int back, int *rfore, int *rback)
xarmedit_polling
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {lr}               ; Stack return address
        SWI     XARMEdit_Polling        ; Call the SWI
        TEQ     r2, #0                  ; Is rfore a valid pointer
        STRNE   r0, [r2]                ; Store return r0 if it is
        TEQ     r3, #0                  ; Is rback a valid pointer
        STRNE   r1, [r3]                ; Store return r1 if it is
        MOVVC   r0, #0                  ; Clear error pointer if successful
        LDMFD   sp!, {pc}^              ; Return from subroutine

; void armedit_polling(int fore, int back, int *rfore, int *rback)
armedit_polling
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {lr}               ; Stack return address
        SWI     ARMEdit_Polling         ; Call the SWI
        TEQ     r2, #0                  ; Is rfore a valid pointer
        STRNE   r0, [r2]                ; Store return r0 if it is
        TEQ     r3, #0                  ; Is rback a valid pointer
        STRNE   r1, [r3]                ; Store return r1 if it is
        LDMFD   sp!, {pc}^              ; Return from subroutine

; os_error *xarmedit_talk_reply(int handle, int dest, void *msg)
xarmedit_talk_reply
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {lr}               ; Stack return address
        SWI     XARMEdit_TalkReply      ; Call the SWI
        MOVVC   r0, #0                  ; Clear error pointer if successful
        LDMFD   sp!, {pc}^              ; Return from subroutine

; void *armedit_talk_reply(int handle, int dest, void *msg)
armedit_talk_reply
        MOV     ip, sp                  ; Copy stack pointer
        STMFD   sp!, {lr}               ; Stack return address
        SWI     ARMEdit_TalkReply       ; Call the SWI
        LDMFD   sp!, {pc}^              ; Return from subroutine

        END
