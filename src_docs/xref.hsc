<*
    File        : xref.hsc
    Date        : 04-Mar-01
    Author      : © A.Thoukydides, 1998-2001, 2019
    Description : Part of the PsiFS documentation.
    
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

<PAGE TITLE="Documentation" SUBTITLE="Index" PARENT=":index.html" KEYWORDS="Documentation,Contents,Index,Keywords" INDEX>

<$macro XREF/CLOSE/NAW="XREF"><DL><$content></DL></$macro>
<$macro XREFK/CLOSE/MBI="XREF"/NAW="XREFK" KEYWORD:string/REQUIRED><DT><(KEYWORD)><DD><UL><$stripws><$content></UL><$stripws></$macro>
<$macro XREFI/MBI="XREFK" HREF:uri/REQUIRED TITLE:string/REQUIRED><LI><A HREF=(HREF) TITLE=(TITLE)><(TITLE)></A><$stripws></$macro>

<HEADING>Documentation Index</HEADING>
<PARA>
    <XREF_INDEX>
</PARA>

</PAGE>
