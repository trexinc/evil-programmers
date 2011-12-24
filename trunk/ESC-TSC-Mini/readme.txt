ESC-TSC-Mini v1.3
or
ESC's Temporary Settings Changer - Minimalistic
or
Editor Settings Changer - Temporary Settings Changer - Minimalistic

(That's a pretty long name for a very small plugin :-)

--------------------------------------------------------------------------------

Author: Alex Yaroslavsky
Home page: http://code.google.com/p/evil-programmers/

--------------------------------------------------------------------------------

About
-----

What this plugin does, it lets you change the settings of ESC for  the  current
editor session. To use this plugin you'll need Far version 3.0 or newer and ESC
plugin version 2.7 or newer. ESC is an excellent creation  by
Ivan Sintyurin <spinoza@mail.ru> which is currently maintained by me.
To get hold of your own copy of ESC visit:
http://plugring.farmanager.com/ - The PlugRinG


License
-------

ESC-TSC-Mini plugin for FAR Manager
Copyright (C) 2001 Alex Yaroslavsky

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


How to use the plugin:
----------------------

Parameter: [            ]

Here you write the name of one of ESC's parameters, like wrap, autowrap etc.
Read ESC's docs for the full list of settable parameters.

Value:     [            ]   or   Value:     [            ]

Here you write the value you want to set for that parameter.
Read ESC's docs for the range of values for each parameter.
*Note: This value can only be a number (an int), empty value or a text starting
with letters are taken for zero. Only the number part of what you have  written
is used.

[ Update ]

Pressing this button will cause the plugin to try and update the value  of  the
parameter you entered to the value you entered. This button will not close  the
dialog so you can continue and update more parameters, to exit the dialog  just
press Esc or F10. A message will be shown in the status bar upon succession  or
failure of the update action.
*Note1: Pressing Enter anywhere not on either of the buttons will be considered
as pressing this button.
*Note2: If the action is successful the parameter and the value will  be  added
to the history of parameters and values accordingly, for quick future access.

[ Check ]

Pressing this button will cause the plugin to try and get the current value  of
the parameter you entered. This button will not close the  dialog  so  you  can
continue and check more parameters, to exit the dialog just press Esc or F10. A
message will be shown in the status bar upon succession or failure of the check
action. Upon succession the value read will be shown.
*Note: If the action is successful the parameter will be added to  the  history
of parameters, for quick future access.

+----------Status----------+
|Waiting...                |
+--------------------------+

Here you will be shown messages upon succession or failure of  the  action  you
have made. "Waiting..." means that the plugin is waiting for the next action.

Advanced usage:
---------------

This plugin can be used with any future ESC version without any change  in  the
code as long as the esc.dll stays with that name and the names of the functions
that are used for Check (GetEditorSettings) and  Update  (SetEditorOption)  are
not changed and the value for any settable parameter continues to  be  a  32bit
integer.
Because the history of  the  parameters  (and  values)  is  updated  only  with
legitimate items (that where accepted by ESC) you can lock  some  of  the  most
frequently used parameters (or values) in the history by pressing insert on the
needed parameter (or value), for more extensive history management  option  see
the FAR help for keyboard reference.
