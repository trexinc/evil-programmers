m4_include(`../FARMail/fm_version.m4')m4_dnl
.Language=English,English
.PluginContents=Filter - FARMail sub plug-in

@Contents
$^`#'Filter - FARMail sub plug-in `v'MAJOR.MINOR#

 - ~Pattern filter~@PatternFilter@
 - ~Configuration~@Config@
 - ~Authors~@Authors@
 - ~License~@License@


@PatternFilter
$^#Pattern filter#


  You can quickly select messages containing a pattern in header.
Press Alt-F6 while in messages panel, then You'll see a dialog like that:

 ษออออออออออออออออ Messages selection ออออออออออออออออป
 บ                                                    บ
 บ Search for expression :                            บ
 บ from:*president@@mycountry.gov*                    บ
 บ                                                    บ
 วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ
 บ                                                    บ
 บ [ ] Search in selected messages                    บ
 บ [ ] Invert selection                               บ
 บ [ ] Case-sensitive                [ Use file... ]  บ
 บ                                                    บ
 ศออออออออออออออออออออออออออออออออออออออออออออออออออออผ
 [  Ok  ][  Cancel  ]

   Enter a pattern to search in message headers using standard conventions
(that is, '?' means any 1 symbol, '*' means zero or more of any symbols).
Note that pattern is compared to each line of message header. You may want
to search for pattern only in selected messages, to search for messages not
containing the pattern or to use case-sensitive search - just turn on
the appropriate checkbox. This feature may be useful if You have some hundreds
of messages on server and want to find quickly some important messages or
delete a lot of spam without reading. For example, to quickly find a message
from President@@MyCountry.gov, enter the pattern:

 From:*president@@mycountry.gov*

  The asteriks are very useful here, because the real line in header may look like
this:

 From:
  "John Doe" <president@@mycountry.gov> (some comments here)

  If You want to kill a lot of spam, and You know that most of the spam goes
through a relay named "mail.suckingspammer.org", You may use such a
pattern:

 Received:*from*mail.suckingspammer.org*by*

 and press F8 to kill all selected messages. Note that I'm not responsible
for any loss of mail caused by illegal pattern or program fault :) So
You may want to check the option "Invert selection", copy all the other
messages on Your HDD and leave the spam for later processing :)

  If You don't want to enter a lot of information in the dialog every time
You check Your mail, You can make some filter files in the subdirectory
"Filters" and then just select "Use file...". You will see a list of all Your
filters. A filter file is a simple text file with extension "fmf" containing
the following definitions:

   name     = <filter name which will be shown in menu>
   select   = <a pattern>
   unselect = <a pattern>

All lines beginning with ';' are ignored.

You can define a hotkey in the name, just enter a "&" symbol before the
letter You want to use as a hotkey. Messages matching the pattern in "select"
statement will be selected, messages matching the pattern in "unselect"
statement will be unselected :) The last lines have the highest priority.
For example, if You want to quickly select spam messages by pressing Ctrl-S,
just do the following:


1. Make a filter file, for example spam.fmf in the "filters" subdirectory.
This file will contain something like:

  ; The name with a hotkey
  name = &Spam

  ; spam servers
  select=received:*from *.suckingspammer.org *by*
  select=received:*from *.damnspamsender.com *by*

  ; mailers You know spammers are using
  select=X-Mailer:*CoolSpamMailer v10.1*

  ;  But You don't want to select messages from the site admins
  unselect=from:*admin@@*

2. Record a macro for the Ctrl-S sequence, or start a reg-file containing
something like that:

  REGEDIT4

  [HKEY_CURRENT_USER\Software\Far\KeyMacros\Shell\CtrlS]
  "Sequence"="AltF6 AltU s"
  "DisableOutput"=dword:00000001

That's all, now just press Ctrl-S in the messages panel to select the spam.



 ~Contents~@Contents@


@Config
$^#Configuration#

#Filters' dir# - enter here the full path to the directory to search in for
filters. Leave empty to search in FILTERS sub folder in plug-in's folder.



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

 Filter sub-plugin for FARMail
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
