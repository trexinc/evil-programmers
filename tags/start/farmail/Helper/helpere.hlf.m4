m4_include(`../FARMail/fm_version.m4')m4_dnl
.Language=English,English
.PluginContents=Helper - FARMail sub plug-in

@Contents
$^`#'Helper - FARMail sub plug-in `v'MAJOR.MINOR#

 - ~Usage~@Usage@
 - ~Open file dialog~@FileDialog@
 - ~Configuration~@Config@
 - ~Authors~@Authors@
 - ~License~@License@


@Usage
$^#Usage#

 #1. Attach# - Lets You select a file to attach. Attach directive is inserted
at the end of the %start%-%end% block or in the current line if no such block
exists.

 #2. Insert# - Lets You select a file which contents you want to insert in the
current cursor position. The contents of the file is enclosed with the strings
defined in the ~Configuration~@Config@.

 #3. Paste# - If the clipboard is not empty, inserts the strings defined in
~Configuration~@Config@ and positions the cursor in the right position for you
to paste the clipboard contents.

 #4. FAR version# - Inserts FAR version in the current cursor position.

 #5. FARMail version# - Inserts FARMail version in the current cursor
position.

 #6. Windows version# - Inserts Windows version in the current cursor
position.



 ~Contents~@Contents@


@FileDialog
$^#Open file dialog#

 Here You see a list of files and directories. You can browse all your drives
to find the file you need. Directories are marked by "+".

 #Keys#

 #Enter# - Change directory / select file.

 #Shift-Enter# - run the file in a separate window

 #Ctrl-\# - Shows You the list of all available drives.

 #Alt-Letter# - Changes to current directory in drive "Letter".

 #RCtrl0-RCtrl9# - As in FAR, go to a defined folder shortcut.

 #Shift-Enter# - Execute in a separate window.



 ~Contents~@Contents@


@Config
$^#Configuration#

 #Clipboard start# and #Clipboard end# - Strings used by the ~Paste~@Usage@
command.

 #File start# and #File end# - Strings used by ~Insert~@Usage@. First
occurrence of "%s" will be replaced with the full path of the inserted file.



 ~Contents~@Contents@


@Authors
$^#Authors#

  Currently the development of this plug-in is maintained by FARMail Group:

  #Alex Yaroslavsky#
  E-mail   : at yandex.ru / trexinc
  Homepage : ~http://trexinc.sf.net/~@http://trexinc.sf.net/@

  #Vadim Yegorov#
  E-mail   : at bmg.lv / zg
  Homepage : ~http://zg.times.lv/~@http://zg.times.lv/@



 ~Contents~@Contents@

@License
$ #License#

 Helper sub-plugin for FARMail
 Copyright (C) COPYRIGHT FARMail Group

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

 ~Contents~@Contents@
