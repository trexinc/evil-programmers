HXSView - list and extract files from HXS/HXI MS-Help 2 archives
Copyright (C) 2004-2005 Alex Yaroslavsky
Using OpenCLit library by Dan Jackson (http://www.convertlit.com/)
and LZX library by Stuart Caie (http://www.kyz.uklinux.net/)


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
Info:

I have accidentally noticed that LIT files have  the  "ITOLITLS"  file  ID  and
knowing that MS-Help 2 files have it too I tried to open an HXS file using  the
CLit library and to my surprise it worked (after some hacking). So I built this
beta release as a technology preview and mainly for usage with FAR Manager.

-------------------------------------------------------------------------------
Usage:

Just  add  this  to  the  custom.ini  file  of  MultiArc  plugin  and  put  the
hxsview.exe somewhere in your path.

------------- system clipboard -------->8----
[HXS]
TypeName=HXS
ID=49 54 4F 4C 49 54 4C 53
IDOnly=1
Extension=hxs
List="hxsview l"
Start="^--------"
End="^--------"
Format0="zzzzzzzzzzzz nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"
Extract="hxsview x %%A @%%LA"
ExtractWithoutPath="hxsview e %%A @%%LA"
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

v1.0 beta 3 (07/03/05)
----------------------
 ! Files with illegal characters in path were not extracted  (now  the  illegal
   character are always replaced).
 ! When extracting with a given internal folder path, not the whole folder tree
   was extracted.

v1.0 beta 2 (23/10/04)
----------------------
 * Some optimization, exe size reduced.
 * No longer needs msvcrt.dll.
 * Some changes in error reporting.
 ! Wrong copyright notice for the author of OpenCLit library.

v1.0 beta 1 (06/10/04)
----------------------
 + Initial release.
