<*
    File        : vers.hsc
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

<PAGE TITLE="Version History" PARENT=":index.html" KEYWORDS="Version,History,Bugs,Problems" NORULE>

<REL>
    <RELI VER="1.05" DATE="17-May-01" RELEASE>
        New version under development...<BR>
        More efficient service call handling under RISC OS 4.<BR>
        Smaller message size used to prevent data corruption.
    </RELI>
    
    <RELI VER="1.04" DATE="07-Dec-97">
        Corrected device driver operation for large logical media.<BR>
        Added device driver support for removable media.<BR>
        Extra information is displayed for *ARMEdit_Devices.<BR>
        *ARMEdit_Files also lists files used by the device driver.<BR>
        Spaces no longer added to end of OSCLI commands.<BR>
        Shell mode added to OSCLI tool with command line editing and history.<BR>
        VDU codes in OSCLI output processed before being displayed.<BR>
        OSCLI commands may be executed in a RISC OS TaskWindow.
    </RELI>

    <RELI VER="1.03" DATE="21-Feb-95">
        Added SWI to set multitasking speed of PC front-end.<BR>
        Added SWI to reply to a message.<BR>
        Broadcast messages from the ARMEdit module are now sent.<BR>
        Speed control now works correctly with PC front-end 2v02.<BR>
        Extended *ARMEdit_DevicesRelog to force immediate relog.<BR>
        Added ARMEDIT command line tools.<BR>
        Speeded up file transfer when multitasking.<BR>
        Included support for wildcards and recursion.<BR>
        Unique temporary filenames used.<BR>
        Operation of command line tools under Windows now more reliable.<BR>
        Added option to use a RISC OS &quot;Save as&quot; window for PUTFILE.
    </RELI>

    <RELI VER="1.02" DATE="06-Aug-96">
        Some memory management moved to a dynamic area if available.<BR>
        Full device driver support added.<BR>
        Routines to convert date and time stamps included.<BR>
        Added SWI to call internal HPC services.<BR>
        Corrected command help texts.<BR>
        Added command to set multitasking speed of PC front-end.<BR>
        Provided mechanism for checking delivery of messages.<BR>
        Date and time stamps are preserved on copied files.<BR>
        Support for Acorn's software PC emulator included.<BR>
        Included redirection of OSCLI input stream through DOS.
    </RELI>

    <RELI VER="1.01" DATE="11-Feb-96">
        Does not crash when no DOSMap mappings defined.<BR>
        Module can be loaded after new versions of the PC front-end.
    </RELI>

    <RELI VER="1.00" DATE="12-Jan-96">
        First official release version.
    </RELI>

    <RELI VER="0.07" DATE="28-Dec-95">
        Added message passing protocol.
    </RELI>

    <RELI VER="0.06" DATE="22-Dec-95">
        Added SWI to control the PC front-end.
    </RELI>

    <RELI VER="0.05" DATE="17-Dec-95">
        Improved filename translation.
    </RELI>

    <RELI VER="0.04" DATE="21-Nov-95">
        Implemented the *commands and real HPC support.<BR>
        Experimental device driver support added.
    </RELI>

    <RELI VER="0.03" DATE="17-Nov-95">
        Uses correct HPC service identifier.<BR>
        Includes development versions of device driver support.
        Changed command line tools to .COM files to make them smaller.<BR>
        Made error checking more comprehensive and added help text to command line tools.<BR>
        Output redirection of OSCLI command made optional.<BR>
        Filetype of destination files for PUTFILE based on file extension.
    </RELI>

    <RELI VER="0.02" DATE="15-Nov-95">
        Included extra command to display PC front-end version.<BR>
        Fixed workspace handling for *commands.<BR>
    </RELI>
    
    <RELI VER="0.01" DATE="14-Nov-95">
        Fixed bugs in file handling.<BR>
        Removed stray debugging code.<BR>
        Sets variable for scrap directory and ensure it exists.<BR>
        Added version number and copyright message to command line tools.<BR>
        Output of commands executed by the OSCLI utility passed through standard output.
    </RELI>
    
    <RELI VER="0.00" DATE="12-Nov-95">
        Original development version.
    </RELI>
</REL>

</PAGE>
