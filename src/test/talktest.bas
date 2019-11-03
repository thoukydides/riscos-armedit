REM File        : TalkTest
REM Date        : 24-Jul-97
REM Author      : © A.Thoukydides, 1996-1997, 2019
REM Description : Test proxying of a *command by the ARMEdit module.
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

REM Allocate some memory to use
talk_block_size% = 1024
DIM talk_block% talk_block_size%

REM Register a new client task
SYS "ARMEdit_TalkStart", 42*0, 0, 0, 0 TO my_talk_handle%

!talk_block% = 0
SYS "ARMEdit_TalkTX", my_talk_handle%, 256, talk_block%
PROCrx
wimp_talk_handle% = reply_handle%

PRINT "!ARMEdit has handle &"; ~reply_handle%

REM Start a *command
!talk_block% = 4
$(talk_block% + 4) = "help ." + CHR$0
$(talk_block% + 260) = "Test Task" + CHR$0
PROCtxrx
cmd_handle% = reply_ptr%!4

PRINT "Command handle &"; ~cmd_handle%

REM Poll the command
finished% = FALSE
WHILE NOT(finished%)
    !talk_block% = 5
    talk_block%!4 = cmd_handle%
    talk_block%!8 = 0
    PROCtxrx
    IF reply_ptr%!4 = 0 THEN finished% = TRUE
    ptr% = 0
    WHILE ptr% < reply_ptr%!8
        VDU ?(reply_ptr% + 12 + ptr%)
        ptr% += 1
    ENDWHILE
ENDWHILE

REM End a *command
!talk_block% = 6
talk_block%!4 = cmd_handle%
PROCtxrx

REM Deregister this client task
SYS "ARMEdit_TalkEnd", my_talk_handle%
END

DEFPROCrx
    reply_ptr% = 0
    WHILE reply_ptr% = 0
        SYS "ARMEdit_TalkRX", my_talk_handle% TO reply_ptr%, reply_id%, reply_handle%
    ENDWHILE
    SYS "ARMEdit_TalkAck", my_talk_handle%
ENDPROC

DEFPROCtxrx
    SYS "ARMEdit_TalkTX", my_talk_handle%, reply_handle%, talk_block%
    PROCrx
ENDPROC
