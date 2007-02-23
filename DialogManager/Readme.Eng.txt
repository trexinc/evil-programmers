Dialog Manager - a plugin for FAR Manager.

0. License.

DialogManager plugin for FAR Manager
Copyright (C) 2003 Vadim Yegorov
Copyright (C) 2004-2007 Vadim Yegorov and Alex Yaroslavsky

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

1. Intro.

This plugin serves as a host for its sub-plugins, allowing to extend the
standard functionality of FAR Manager dialogs. It allows to run its sub-plugins
from inside dialogs (most of them) by pressing CtrlF11 which will present you
with a menu listing the sub-plugins. For a more convenient way of calling that
menu use the supplied F11.reg.

2. Versions.

2.1. Windows version.

NT4.0 or newer.

2.2. FAR Manager version.

1.70 build 1638 or newer.

3. Standard sub-plugins.

3.1 Case.

Allows to change case of text in input lines. Changes the case of the current
word under cursor (or the whole line), or the selection.

3.2. Pwd.

Allows to view the password hidden by asterisks. Known to work in FARMail,
FarFTP, FARNav and MultiArc.

3.3. OpenFile.

Shows an "Open file" dialog similar to the one provided by the OS. For example
can be useful for opening a file in the editor by ShiftF4, when you do not want
to lose the current directory in the panels.

3.4. Logger.

Writes to a log file all input received by the DefDlgProc of all dialogs of all
FAR instances.
For example:
Copy <0x00000868:0x000007b8> - 00126F48 0x1005 - [DN_DRAWDIALOG] 0x0 0x0
Copy - Windows title of the FAR instance.
<0x00000868:0x000007b8> - <ProcessID:ThreadID>.
00126F48 - hDlg.
0x1005 - Msg.
0x0 - Parm1.
0x0 - Parm2.

3.5. GrabDialog.

Saves dialog structure into a file. There are two modes for saving - simple and
full. Below is an example of simple mode output:
InitDialogItem idi[] =
{
        /*Type,X1,Y1,X2,Y2,Param,Flags,Data*/
  /*00*/{DI_DOUBLEBOX,3,1,30,4,0,0,"Quit"},
  /*01*/{DI_TEXT,-1,2,0,2,0,DIF_SHOWAMPERSAND,"Do you want to quit FAR?"},
  /*02*/{DI_BUTTON,12,3,12,3,0,DIF_CENTERGROUP|DIF_NOBRACKETS," Yes "},
  /*03*/{DI_BUTTON,17,3,17,3,0,DIF_CENTERGROUP|DIF_NOBRACKETS," No "},
  /*DefaultButton=2, Focus=2*/
}

3.6. BCopy.

Shows the information menu of Background Copy plugin. For this to function
properly you will need BCopy of build 51 or newer and the following registry
value:
REGEDIT4

[HKEY_CURRENT_USER\Software\Far\Plugins\BCopy]
"TechPreload"=dword:1

It may also be needed to clear plugins chache.

3.7. CharMap.

Allows to insert any symbol into an input line using the Character Map plugin.
For this to function properly you will need Character Map of version 3.1 or
newer and the following registry value:
REGEDIT4

[HKEY_CURRENT_USER\Software\Far\Plugins\CharacterMap]
"Preload"=dword:1

It may also be needed to clear plugins chache.

3.8. Macro.

Lets you have a different set of macros for each dialog, and also to create
global macros for all dialogs. All macros are kept in the following registry
key:
HKEY_CURRENT_USER\Software\Far\Plugins\DialogM\Plugins\Macro
This key may contain any number of sub-keys, each corresponding to a specific
group of dialogs.
The value
[HKEY_CURRENT_USER\Software\Far\Plugins\DialogM\Plugins\Macro]
"Global"="@Global"
defines the name of the global group.
Macro descriptions are contained inside each group. For example:
[HKEY_CURRENT_USER\Software\Far\Plugins\DialogM\Plugins\Macro\Findfile]
"Title"="Find file"
"Count"=dword:00000020
[HKEY_CURRENT_USER\Software\Far\Plugins\DialogM\Plugins\Macro\Findfile\F1@checkbox]
"Key"="F1"
"Sequence"="Space"
"Type"=dword:00000008
"DisableOutput"=dword:00000001
[HKEY_CURRENT_USER\Software\Far\Plugins\DialogM\Plugins\Macro\Findfile\F1@edit]
"Sequence"="A"
"Type"=dword:00000004
"Key"="F1"
"DisableOutput"=dword:00000000

The Title value defines the dialog title in which the macros will work.
Wildcards may be used, analogous to FAR file masks.
The Count value defines the total number of elements in the dialog in which the
macros will work.
The Type value defines the dialog element type for which the current macro will
work.
The hotkey for the macro can be defined either by the name of the key, or in
the case one hotkey is assigned to several macros with different rules, by
setting the Key value.

Since not all dialogs can be uniquely identified by their title, it is possible
to set another unique identifier from the code of your plugin when processing
the DN_MACRO_GETDIALOGINFO dialog message in the following way:
#include "dm_macro.hpp"
...
  switch(Msg)
  {
...
    case DN_MACRO_GETDIALOGINFO:
      MacroDialogInfo *mde=(MacroDialogInfo *)Param2;
      if(mde&&mde->StructSize>=(long)sizeof(MacroDialogInfo))
      {
        strcpy(mde->DialogId,"your_unique_value");
        return TRUE;
      }
      break;
...
  }

Additionally you can disable macro processing in any dialog in the following
way:
#include "dm_macro.hpp"
...
  switch(Msg)
  {
...
    case DN_MACRO_DISABLE:
      return TRUE;
...
  }


3.9. Undo.

Makes possible to perform undo and redo operations in dialog input lines. The
number or undo/redo operation on a single input line is unlimited but as soon
as it looses focus they are lost.

3.10. Paste Selection.

Allow to insert into an input line the current selection of the current editor.

3.11. Default Button.

Marks the default button in a dialog by enclosing it in curly brackets.

4. Limitations.

In some dialog the CtrlF11 hotkey does not work. It happens because the
dialog-callback function of this dialog probably contains the following code:
long WINAPI DlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  ...
  switch(Msg)
  {
    case DN_KEY:
      if(Param2==KEY_???)
      {
        ...
        return TRUE;
      }
      else
        return FALSE;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}
If you change it to the following code:
long WINAPI DlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  ...
  switch(Msg)
  {
    case DN_KEY:
      if(Param2==KEY_???)
      {
        ...
        return TRUE;
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}
then the dialog will work as if nothing has changed, but the CtrlF11 hotkey
will work.

5. Configuration.

The plugin is aware of the following registry keys:
REGEDIT4

[HKEY_CURRENT_USER\Software\Far\Plugins\DialogM]
; don't show the plugin in plugins configuration menu.
"ShowInConfig"=dword:00000000
; don't show the plugin in plugins menu.
"ShowInPanels"=dword:00000000
; combination to call the plugin (main menu).
"HotKey"="CtrlAltF1"
; combination to execute int 3.
"DebugKey"="CtrlF12"

6. History.

v0.6 build 9 23-02-2007
- Add subplugin: Replace
- Add subplugin: Search
- Add subplugin: Search and Replace
- charmap: now also works in DI_FIXEDIT.

v0.6 build 8 23-02-2007
- grabber: update DI_VTEXT flags.

v0.6 build 7 11-04-2005
- openfiledialog: incorporated bug fixes and optimizations by Andrey Budko.
  which include a crash when trying to change into a locked folder.
- openfiledialog: when changing to the drive list cursor is positioned on the
  drive from which you changed.
- openfiledialog: could crash randomly when browsing folders.
- macro: can now understand empty dialog titles.

v0.6 build 6 23-03-2005
- improved stability in file search.
- english translation.
- case: added an option to process the whole line.
- charmap: now also works in DI_PSWEDIT.
- openfiledialog: fixed listing of file names that are longer than 128 chars.

v0.6 build 5 15-10-2004
- plugin did not work in FAR builds higher than 1832.
- charmap: DN_EDITCHANGE is send upon text change.
- case: DN_EDITCHANGE is send upon text change.
- case: title case, cyclic change and processing of word under cursor.
- pwd: the password is shown in a dialog.
- pasteselection.
- undo.
- macro.
- defbutton.

v0.5 build 4 30-11-2003
- version info.
- charmap.
- logger: new messages.
- grabdialog: small bugs in configuration.
- bcopy.
- grabdialog: file was not opened correctly.
- openfiledialog: Left/Right work as Home/End.
- openfiledialog: the DIF_LISTWRAPMODE was removed.
- openfiledialog: the open file dialog adjusts its size for the current console
  size.
- openfiledialog: Shift-Enter as in FAR.
- openfiledialog: when browsing up-dir ("..") cursor is positioned on the
  directory that was just left.

v0.0 build 3 23-04-2003
- settings.
- grabdialog.
- logger.
- can be called from the panels.
- configuration.
- case: operation with blocks is improved.
- minimal FAR build for the plugin is 1638.

v0.0 build 2 05-04-2003
- openfiledialog: instead of C:\Program Files\Far\Plugins\_Add\.. inserts
  C:\Program Files\Far\Plugins\_Add\
- hotkeys can be removed.
- if current language is not found in the language file, english is used.

v0.0 build 1 06-03-2003
- first version.

7. Contacts.

Vadim Yegorov - at bmg.lv / zg
Alex Yaroslavsky - at yandex.ru / trexinc
