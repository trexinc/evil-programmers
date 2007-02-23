/*
 MailView plugin for FAR Manager
 Copyright (C) 2005 Alex Yaroslavsky
 Copyright (C) 2002-2003 Dennis Trachuk

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
*/
#ifndef ___FarMessages_H___
#define ___FarMessages_H___

#if _MSC_VER >= 1000
#pragma once
#endif

#include <wincon.h>

struct TMessage
  {
  //int  Msg;
  FarMessagesProc Msg;
  int  Param1;
  long Param2;
  long Result;
  };

struct TMessage2
  {
  int  Msg;
  union
    {
    int Unused;
    int ItemId;
    int Count;
    } Param1;
  union
    {
    long                Unused;
    long                State;
    DWORD               Attr;
    LPDWORD             AttrList;
    FarDialogItem     * Item;
    LPCSTR              HelpTopic;
    DWORD               Key;
    LPVOID              Data;
    FarList           * List;
    MOUSE_EVENT_RECORD *Rec;
    } Param2;
  long Result;
  };

struct TDNNoParams
  {
  int  Msg;
  BYTE Unused[ sizeof( int ) + sizeof ( long ) ];
  long Result;
  };

struct TDnItem
  {
  int  Msg;
  int  Id;
  long Unused;
  long Result;
  };

struct TDnBtnClick
  {
  int  Msg;
  int  Id;
  long State;
  long Result;
  };

struct TDnColor
  {
  int   Msg;
  int   Id;
  DWORD Color;
  long  Result;
  };

struct TDnKey
  {
  int   Msg;
  int   Id;
  DWORD Key;
  long  Result;
  };

struct TDnDrawItem
  {
  int             Msg;
  int             Id;
  FarDialogItem * Item;
  long            Result;
  };
/*
DN_CLOSE                Id, 0                    ; BOOL   ; TDnItem
DN_BTNCLICK             Id, State                ; BOOL   ; TDnBtnClick
DN_CTLCOLORDIALOG        0, Attr                 ; Attr   ; TDnColor
DN_CTLCOLORDLGITEM      Id, Attr                 ; Attr   ; TDnColor
DN_CTLCOLORDLGLIST     Num, Attr*         (DWORD); BOOL   ; TDnColorList
DN_DRAWDIALOG            0, 0                    ; BOOL   ; TDnNoParams
DN_DRAWDLGITEM          Id, FarDialogItem*       ; BOOL   ; TDnDialogItem
DN_EDITCHANGE           Id, FarDialogItem*       ; BOOL   ; TDnDialogItem
DN_ENTERIDLE             0, 0                    ; void   ; TDnNoParams
DN_GOTFOCUS             Id, 0                    ; void   ; TDnItem
DN_HELP                 Id, LPCSTR               ; LPCSTR ; TDnHelp
DN_HOTKEY               Id, Key                  ; BOOL   ; TDnKey
DN_INITDIALOG           Id, Data(0)              ; BOOL
DN_KEY                  Id, Key                  ; BOOL   ; TDnKey
DN_KILLFOCUS            Id, 0                    ; long   ; TDnItem
DN_LISTCHANGE           Id, FarList*             ; BOOL   ; TDnListChange
DN_MOUSECLICK           Id, MOUSE_EVENT_RECORD * ; BOOL   ; TDnMouse
*/

#endif //!defined(___FarMessages_H___)
