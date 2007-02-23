������ ��� FAR Manager Dialog Manager.

0. ��業���.

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

1. ��������.

����� ������ �㦨� ��⮬ ��� ᢮�� �㡯�������, �������� �������
�⠭����� ���������� �������� FAR Manager. ��᫥ ��� ��⠭���� � ��������
�������� ������ ����������� �� ������ CtrlF11 ������� ���� � ᯨ᪮�
�����������. ��� 㤮��⢠ �맮�� �⮣� ���� �ਫ������� 䠩� F11.reg.

2. ���ᨨ.

2.1. ����� Windows.

NT4.0 � ���.

2.2. ����� FAR Manager.

1.70 build 1638 � ���.

3. �⠭����� ����������.

3.1 Case.

�������� �������� ॣ���� ⥪�� � ��ப�� ।���஢����. ��������� ����
ॣ���� ᫮�� ��� ����஬ (��� �ᥣ� ⥪��), ���� �뤥�����.

3.2. Pwd.

�������� ����拉�� ��஫� �� ������窠��. ��筮 ࠡ�⠥� � FARMail, FarFTP,
FARNav � MultiArc.

3.3. OpenFile.

�����뢠�� ������ �롮� 䠩�� ������� �⠭���⭮�� ������� OS. ������� �
�ਬ��� �� ����⨨ 䠩�� � ।���� �� ShiftF4, �⮡� �� ����� ⥪�騥
��४�ਨ �� �������.

3.4. Logger.

���� � ��� ��, �� ��室�� � DefDlgProc ��� �������� ��� �஢.
�ਬ��:
Copy <0x00000868:0x000007b8> - 00126F48 0x1005 - [DN_DRAWDIALOG] 0x0 0x0
Copy - ��������� ⥪�饣� ���� ��.
<0x00000868:0x000007b8> - <ProcessID:ThreadID>.
00126F48 - hDlg.
0x1005 - Msg.
0x0 - Parm1.
0x0 - Parm2.

3.5. GrabDialog.

���࠭�� ������ � 䠩�. �������� ��� ०��� - ���⮩ � �����. �ਬ�� ࠡ���
� ���⮬ ०���:
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

�����뢠�� ���ଠ樮���� ���� ������� Background Copy. ��� ��ଠ�쭮��
�㭪樮��஢���� ����室�� ���� ������� �� ���� 51 � ᮮ⢥�����騩 ��ࠬ��� �
॥���:
REGEDIT4

[HKEY_CURRENT_USER\Software\Far\Plugins\BCopy]
"TechPreload"=dword:1

����� �������� ���ॡ���� ���⪠ ��� ��������.

3.7. CharMap.

�������� ��⠢��� � ��ப� ।���஢���� �� ᨬ���, �ᯮ���� ������
Character Map. ��� ��ଠ�쭮�� �㭪樮��஢���� ����室��� ����� ������� ��
���� 3.1 � ᮮ⢥�����騩 ��ࠬ��� � ॥���:
REGEDIT4

[HKEY_CURRENT_USER\Software\Far\Plugins\CharacterMap]
"Preload"=dword:1

����� �������� ���ॡ���� ���⪠ ��� ��������.

3.8. Macro.

�������� ����� ��� ������� ������� ᢮� ����� ����ᮢ. ������� ⠪��
����������� ������ �������� ������ ��� ��� ��������. �� ������ ����� �
��⪥ ॥���:
HKEY_CURRENT_USER\Software\Far\Plugins\DialogM\Plugins\Macro
����� �⮣� ���� ����� ��室����� �� ������⢮ ������祩, ����� ��
������ ᮮ⢥����� ��।������� ��㯯� ��������.
���祭��
[HKEY_CURRENT_USER\Software\Far\Plugins\DialogM\Plugins\Macro]
"Global"="@Global"
������ �������� ������쭮� ��㯯�.
����� ������ ��㯯� ᮤ�ঠ���� ���ᠭ�� ����ᮢ. ���ਬ��:
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

��ࠬ��� Title ��।���� ��������� �������, � ���஬ ���� �ࠡ��뢠��
������. ����� �ᯮ�짮���� ��᪨, ��������� �஢᪨� ��� 䠩���.
��ࠬ��� Count ��।���� ������⢮ ����⮢ � �������, � ���஬ ���� �ࠡ��뢠��
������.
��ࠬ��� Type ��।���� ⨯ ����� �������, ��� ���ண� �㤥� �ࠡ��뢠��
����� �����.
������ ��⨢�樨 ����� ����� ���������� ���� ������ ����, ����, � ��砥,
�᫨ ����� � ⮩ �� ������ ��᢮��� ��᪮�쪮 ����ᮢ � ࠧ�묨 �᫮��ﬨ,
�� ��ࠬ��� Key.

��� ��� ��������� ������� �� �ᥣ�� ����� �㦨�� 㭨����� �����䨪��஬
�������, �� ����� � ᢮�� ������� ������ ��㣮� �����䨪��� �
��ࠡ��稪� ᮮ�饭�� DN_MACRO_GETDIALOGINFO ᫥���騬 ��ࠧ��:
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

��� �� ����� ������� ��ࠡ��� ����ᮢ � ����� ���� ������� ᫥���騬
��ࠧ��:
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

�������� � ��ப�� ����� �������� ����樨 undo � redo. ������⢮ �⪠⮢
����࠭�祭��, �� �� ���� ��ப�� ����� 䮪�� ��� �� �������.

3.10. Paste Selection.

�������� � �⠢��� � ��ப� ����� ⥪�饥 �뤥����� � ⥪�饬 ।����.

3.11. Default Button.

�⬥砥� ������ �� 㬮�砭�� � ������� ��� �� � 䨣��� ᪮���.

4. ��࠭�祭��.

� �������� �������� ��������� CtrlF11 �� �ࠡ��뢠��. ��稭�� ����
�㭪��-��ࠡ��稪 ������� �������, ᮤ�ঠ�� �ਬ�୮ ᫥���騩 ���:
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
�᫨ ��� �������� ��:
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
� � ࠡ�� ������� ��祣� �� ���������, � CtrlF11 ��筥� �ࠡ��뢠��.

5. ���䨣����.

������ ����� � ����⢮����� �������� ���祩 ॥���:
REGEDIT4

[HKEY_CURRENT_USER\Software\Far\Plugins\DialogM]
; ������� ������� ��������� �㭪� � ���� ���䨣��樨 ��������.
"ShowInConfig"=dword:00000000
; ������� ������� ��������� �㭪� � ���� ��������.
"ShowInPanels"=dword:00000000
; ������ ��� �맮�� �������.
"HotKey"="CtrlAltF1"
; ������ ��� �믮������ int 3.
"DebugKey"="CtrlF12"

6. �����.

v0.6 build 9 23-02-2007
- �������� �㡯�����: Replace
- �������� �㡯�����: Search
- �������� �㡯�����: Search and Replace
- charmap: ��������� ࠡ�� � DI_FIXEDIT.

v0.6 build 8 23-02-2007
- grabber: ���������� 䫠��� DI_VTEXT.

v0.6 build 7 11-04-2005
- openfiledialog: ��ࠢ����� � ��⨬���樨 �� Andrey Budko. ����砥�
  ��ࠢ����� ������� ������� �� ����⪥ �室� � �����஢����� �����.
- openfiledialog: ��᫥ ���室� � ᯨ᮪ ��᪮� ��⠭���� ������� ���� ��
  ��⨢�� �� ���室� ���.
- openfiledialog: ��� �� �।������ 㯠��� �� ���⠭�� �� ������.
- macro: ⥯��� �������� ����� ��������� ��������.

v0.6 build 6 23-03-2005
- ���襭� �⠡��쭮��� � ���᪥.
- ������᪨� ��ॢ��.
- case: ���� ��� ��ࠡ�⪨ �ᥩ ��ப�, � �� ⮫쪮 ᫮�� ��� ����஬.
- charmap: ��������� ࠡ�� � DI_PSWEDIT.
- openfiledialog: ��ࠢ���� ࠡ�� � 䠩���� ��� ������ ������� 128 ᨬ�����.

v0.6 build 5 15-10-2004
- ������ �� ࠡ�⠫ � ������ �� ��� 1832.
- charmap: �� ��������� ⥪�� ���뫠���� DN_EDITCHANGE.
- case: �� ��������� ⥪�� ���뫠���� DN_EDITCHANGE.
- case: title case, cyclic change � ࠡ�� ��� ᫮��� ��� ����஬.
- pwd: ��஫� �����뢠���� � �������.
- pasteselection.
- undo.
- macro.
- defbutton.

v0.5 build 4 30-11-2003
- version info.
- charmap.
- logger: new messages.
- grabdialog: ������ �� � ���䨣��樨.
- bcopy.
- grabdialog: ���ࠢ��쭮 ���뢠��� 䠩�.
- openfiledialog: Left/Right ࠡ���� ��� Home/End.
- openfiledialog: 䫠� DIF_LISTWRAPMODE �࠭.
- openfiledialog: ������ ������ 䠩�� �����ࠨ���� ᢮� ࠧ��� ���
  ࠧ��� ���᮫�.
- openfiledialog: Shift-Enter �� �������� � �஬.
- openfiledialog: �� ����⨨ �� ".." ����� ����樮������� �� ��४�ਨ
  �� ���ன ��諨.

v0.0 build 3 23-04-2003
- ����ன��.
- grabdialog.
- logger.
- �맮� �� �������.
- ���䨣����.
- case: ���襭� ࠡ�� � �������.
- ��������� �ॡ㥬� ���� �� - 1638.

v0.0 build 2 05-04-2003
- openfiledialog: ����� C:\Program Files\Far\Plugins\_Add\.. ��⠢�����
  C:\Program Files\Far\Plugins\_Add\
- 㤠����� �⪥�.
- �᫨ � �몮��� 䠩�� ��� ⥪�饣� �몠, ��६ ������᪨�.

v0.0 build 1 06-03-2003
- ��ࢠ� �����.

7. ���न����.

Vadim Yegorov - at bmg.lv / zg
Alex Yaroslavsky - at yandex.ru / trexinc
