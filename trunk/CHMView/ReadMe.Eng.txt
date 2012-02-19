CHMView - list and extract files from CHM/CHI HtmlHelp archives
Copyright (C) 2002-2012 Alex Yaroslavsky
Using:
CHMLib library by Jed Wing (http://66.93.236.84/~jedwin/projects/chmlib/)
LZX library by Stuart Caie (http://www.kyz.uklinux.net/)
UTF8 en/decoder by Oleg Bondar at mail.ru / hobo-mts

-------------------------------------------------------------------------------
License:

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

-------------------------------------------------------------------------------
Usage:

Just  add  this  to  the  custom.ini  file  of  MultiArc  plugin  and  put  the
chmview.exe somewhere in your path.

------------- system clipboard -------->8----
[chm]
TypeName=CHM
ID=49 54 53 46
IDPos=0
IDOnly=1
Extension=chm
List="chmview l"
Start="^--------"
End="^--------"
Format0="zzzzzzzzzzzz nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"
Extract="chmview x %%A @%%LA"
ExtractWithoutPath="chmview e %%A @%%LA"
Test=
Delete=
Add=
AddRecurse=
AllFilesMask=
----8<---- end of system clipboard ----------

-------------------------------------------------------------------------------
History:

Legend:  + added
         - removed
         * changed
         ! fixed

v2.0 beta 4 (19/02/12)
----------------------
 + Unix support, from Alex Kozlov.

v2.0 beta 3 (07/03/05)
----------------------
 ! Zero size files were not extracted.
 ! Files with illegal characters in path were not extracted  (now  the  illegal
   character are always replaced).
 ! When extracting with a given internal folder path, not the whole folder tree
   was extracted.

v2.0 beta 2 (13/11/04)
----------------------
 * UTF8 processing is done by an internal library (thanks to  Oleg  Bondar)  so
   Unicode support is available under Win95 too.
 - Due to the new UTF8 processing removed the chmviewu.exe file  as  it  is  no
   longer needed.

v2.0 beta 1 (23/10/04)
----------------------
 * Now using chmlib library by Jed Wing, should work faster.
 + Added limited support for non-english filenames.  Use  the  chmviewu.exe  to
   open chm files containing files with non-english filenames, should work well
   if filename codepage matches user's windows codepage. Should work  in  Win98
   or newer.
 * Some changes in error reporting.

v1.1 (14/06/03)
---------------
 + No history maintained.

v1.0 (01/12/02)
---------------
 + Initial release.
