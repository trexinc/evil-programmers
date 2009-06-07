m4_include(`../FARMail/fm_version.m4')m4_dnl
.Language=Russian,Russian (���᪨�)
.PluginContents=������ - ������ ��ண� �஢�� ��� FARMail

@Contents
$^`#'������ - ������ ��ண� �஢�� ��� FARMail `v'MAJOR.MINOR#

 - ~�롮� �� 蠡����~@PatternFilter@
 - ~���䨣����~@Config@
 - ~�����~@Authors@
 - ~��業���~@License@


@PatternFilter
$^#�롮� �� 蠡����#


  ���� ����������� ����� ����� ᮮ�饭��, ᮤ�ঠ騥 � ���������
��ப�, ���������� ��� ������� 蠡���. ����� Alt-F6 � ������ ᮮ�饭��,
�� 㢨��� ���������� ������:

 ������������������ �롮� ᮮ�饭�� �����������������ͻ
 �                                                    �
 � �᪠�� ��ࠦ���� :                                 �
 � from:*president@@mycountry.gov*                    �
 �                                                    �
 ����������������������������������������������������Ķ
 �                                                    �
 � [ ] �᪠�� � �⬥祭��� ᮮ�饭���                 �
 � [ ] ������� �롮ન                               �
 � [ ] ���뢠�� ॣ����              [ �� 䠩��... ] �
 �                                                    �
 ����������������������������������������������������ͼ
                 [  ��  ][  �⬥��  ]


   ������ 蠡��� ��� ���᪠ � ����������, �ᯮ���� ��饯ਭ��� �ࠢ���
(�.�., '?' ����砥� ��ண� ���� �� ᨬ���, '*' - 0 ��� ����� ����
ᨬ�����). ������ �ࠢ�������� ��᫥����⥫쭮 � ������ ��ப�� ���������.
����� �᪠�� ⮫쪮 � �뤥������ ᮮ�饭���, �᪠�� ᮮ�饭��, �� ��������騥
��� 蠡���, � ⠪�� �᪠�� � ��⮬ ॣ���� �㪢 - �㦭� ���� �⬥���
ᮮ⢥�����騩 �㭪�. ������ ����������� ����� ���� �������, �᫨ �㦭�
���� ������ ᮮ�饭�� �।� �⥭ ������ �� �ࢥ�, ���� �����
ᯠ� �� ���. ���ਬ��, �⮡� ����� ���᪠�� ���쬮 � ����
President@@MyCountry.gov, 㪠��� 蠡���:

 From:*president@@mycountry.gov*

  ������窨 � 蠡���� �祭� �������, ⠪ ��� ॠ�쭠� ��ப� ��������� �����
����� ���:

 From:
  "John Doe" <president@@mycountry.gov> (� ��� �� � �������ਨ)

  �᫨ �㦭� ����� ᯠ�, � �����⭮, �� ���� ᯠ�� ���� �१ �ࢥ�
� ���� ������ "mail.suckingspammer.org", ����� 㪠���� 蠡���:

 Received:*from*mail.suckingspammer.org*by*

 � ������ F8, ��ॢ �� ��࠭�� ᮮ�饭��. ����, �� � �� ����
�⢥��⢥����� �� ����� �����, �맢����� ������ 蠡����� ��� �訡���
�ணࠬ�� :) ��� �� ����� ���� ���� �㤥� ���⠢��� ������ � �㭪�
"������� �롮ન", ᪮��஢��� �� �⬥祭�� ᮮ�饭�� �� ���, � ᯠ�
��⠢��� ��� ��᫥���饩 ��ࠡ�⪨ :)

  �᫨ ��� ������� ����� ࠧ �� ࠡ�� � ���⮩ ������� ������ ���� �
� ��, ����� ᮧ���� 䨫���� � �����⠫��� "Filters" � ���� �����
"�� 䠩��...". ����� ����誠 � ᯨ᪮� ��� �������� 䨫��஢. ������ -
�� ����� ⥪�⮢� 䠩� � ���७��� "fmf", ᮤ�ঠ騩 ᫥���饥:

   name     = <�������� 䨫���, �����뢠���� � ����>
   select   = <蠡��� ���⠭���� �뤥�����>
   unselect = <蠡��� ���� �뤥�����>

��ப�, ��稭��騥�� � ';' �����������.

����� ������ "������� �������", ���⠢�� ᨬ��� "&" ��। �㦭�� �㪢��
� �������� 䨫���. ����饭��, �������騥 ��� 蠡��� � "select", �뤥������,
� � ��, ����� ������ ��� 蠡��� "unselect", �뤥����� ᭨������. ��� �����
����� 蠡���, ⥬ ��� �ਮ���. ���ਬ��, �᫨ �㦭� ����� �����������
�뤥���� ᯠ� �� Ctrl-S, �㦭� ᤥ���� ᫥���饥:

1. ������� 䨫���, ���ਬ�� spam.fmf � ��४�ਨ "filters", ᮤ�ঠ騩 ����
�த�:

  ; �������� � "����祩 �����襩"
  name = &Spam

  ; ��ࢥ�, � ������ ������� ᯠ�
  select=received:*from *.suckingspammer.org *by*
  select=received:*from *.damnspamsender.com *by*

  ; ���⮢� �������, �ᯮ��㥬� ��� ���뫪� ᯠ��
  select=X-Mailer:*CoolSpamMailer v10.1*

  ; ����饭�� �� ������� �ய�᪠�� �ᥣ��
  unselect=from:*admin@@*


2. ������� ����� �� ������� Ctrl-S, ��� �������� reg-䠩�, ᮤ�ঠ騩
��-� �த�:

  REGEDIT4

  [HKEY_CURRENT_USER\Software\Far\KeyMacros\Shell\CtrlS]
  "Sequence"="AltF6 AltU s"
  "DisableOutput"=dword:00000001

(AltU �ᯮ������ � �।���������, �� �� ����䥩� - ������᪨�).

��� ᮡ�⢥��� � ��, ⥯��� �� ����⨨ Ctrl-S ᯠ�, �����訩 ���
䨫���, �㤥� �뤥���.



 ~����ঠ���~@Contents@


@Config
$^#���䨣����#

#����� 䨫��஢# - ����� 㪠�뢠���� ����� ���� � �����, � ���ன ����
�᪠���� 䨫����. �᫨ �� ���� ���⮥, � 䨫���� ������ � �������� FILTERS
�����, � ���ன ��室���� ������.



 ~����ঠ���~@Contents@


@Authors
$^#�����#

  �� ᥩ ������ ࠧࠡ�⪮� ������� ���������� FARMail Group:

  #����ᥩ ��᫠�᪨�#
  Homepage : ~http://code.google.com/p/evil-programmers/~@http://code.google.com/p/evil-programmers/@

  #����� ���஢#
  E-mail   : at bmg.lv / zg
  Homepage : ~http://zg.times.lv/~@http://zg.times.lv/@

  � 1999-�� � 2000-�� ����� ࠧࠡ�⪮� ������� ��������� ��� ��ࢮ��砫��
����:

  #��ࣥ� ����ᠭ�஢#
  E-mail  : at zmail.ru / poseidon
  Homepage: ~http://alsea.euro.ru/~@http://alsea.euro.ru/@
            ~http://dpidb.genebee.msu.ru/users/poseidon/~@http://dpidb.genebee.msu.ru/users/poseidon/@



 ~����ঠ���~@Contents@

@License
$ #��業���#

 ������ - ������ ��ண� �஢�� ��� FARMail
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

 ~����ঠ���~@Contents@
