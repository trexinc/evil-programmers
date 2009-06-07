m4_include(`../FARMail/fm_version.m4')m4_dnl
.Language=English,English
.PluginContents=Address book - FARMail sub plug-in

@Contents
$^`#'Address book - FARMail sub plug-in `v'MAJOR.MINOR#

 - ~How to use~@AdrBook@
 - ~Configuration~@AddrBookSet@
 - ~Authors~@Authors@
 - ~License~@License@


@AdrBook
$^#How to use#

  Address book is just a list of recipients with names, email addresses
and comments. Press the "(?)" button in the send dialog to see the list of
all recipients. You can enter new recipient by pressing F7, edit a
record - F4, (un)delete a recipient - F8. Press ENTER to select a recipient.
You can select multiple recipients, just press INSERT to mark selected
records and then press ENTER. You can also shorten the list by entering any
text on the keyboard, the list will be filtered and only the items that
contain the entered text (anywhere in the name, e-mail or comment fields) will
be displayed for quick access.

  If Your address book is empty on start, You will be prompted to enter
new recipient.

  You can edit the address book at any time by pressing Shift-F1 in plug-in's
panel or opening the plug-in from the editor, when opened from editor upon
pressing of the ENTER key the selected addresses will be inserted in current
cursor position.

  Note that Name, e-mail and comment must be less than 80 letters.

  Address book fields, delimiter and sorting field can be changed
in program settings.

  Address book is just a text file. Each line is a recipient record. You can
add Your own records in a text editor, just enter recipient's name, e-mail and
comment, divided by "|". You don't need to sort records, this is done
automatically. Note that You can change the fields order and delimiter in
~program settings~@AddrBookSet@. Lines beginning with ';' are ignored.



 ~Contents~@Contents@


@AddrBookSet
$^#Configuration#

 ÉÍÍÍÍÍÍÍ FARMail options: address book ÍÍÍÍÍÍÍÍ»
 º                                              º
 º Fields order:                            NEC º
 º Fields delimiter:                          | º
 º Sort by:                                     º
 º  () Name                                    º
 º  ( ) E-Mail                                  º
 º  ( ) Comment                                 º
 º Address book file directory:                 º
 º C:\Far\FARMail\FMP\AddressBook\              º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º              [ OK ]  [ Cancel ]              º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼

#Fields order# - the order of fields in address book (farmail.adr):
                    N - name
                    E - e-mail
                    C - comment

#Fields delimiter# - delimiter in addressbook.adr, "|" by default.

#Sort by# - Addresses on screen will be sorted by this field.

Note that if You change fields order or delimiter, address book will NOT
be converted!

#Address book file directory# - Full path to the directory where the
addressbook.adr file (the address book itself) is stored or will be stored.
Leave empty to search in plug-in's folder.



 ~Contents~@Contents@


@Authors
$^#Authors#

  Currently the development of this plug-in is maintained by FARMail Group:

  #Alex Yaroslavsky#
  Homepage : ~http://code.google.com/p/evil-programmers/~@http://code.google.com/p/evil-programmers/@

  #Vadim Yegorov#
  E-mail   : at bmg.lv / zg
  Homepage : ~http://zg.times.lv/~@http://zg.times.lv/@

  In the years of 1999 and 2000 the plug-in was developed by its original
author:

  #Serge Alexandrov#
  E-mail  : at zmail.ru / poseidon
  Homepage: ~http://alsea.euro.ru/~@http://alsea.euro.ru/@
            ~http://dpidb.genebee.msu.ru/users/poseidon/~@http://dpidb.genebee.msu.ru/users/poseidon/@



 ~Contents~@Contents@

@License
$ #License#

 AddressBook sub-plugin for FARMail
 Copyright (C) COPYRIGHT FARMail Group
 Copyright (C) 1999,2000 Serge Alexandrov

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
