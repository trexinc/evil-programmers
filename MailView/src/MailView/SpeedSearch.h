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
#ifndef ___SpeedSearch_H___
#define ___SpeedSearch_H___

#include <FarPlus.h>
#include "FarPlugin.h"

class FarSpeedSearch
{
public:
  typedef int (*TSelect)(LPCSTR mask,PluginPanelItem * items, int itemsCount, int start );
private:
  PFarCustomPanelPlugin m_panel;
  FarString             m_title;

  TSelect select;

  static long WINAPI DlgProc( Far::HDIALOG hDlg, int msg, int param1, long param2 );

public:
  FarSpeedSearch( PFarCustomPanelPlugin panel, TSelect selectFunc );
  FarSpeedSearch( PFarCustomPanelPlugin panel, TSelect selectFunc, int title );
  FarSpeedSearch( PFarCustomPanelPlugin panel, TSelect selectFunc, const FarString& title );
  ~FarSpeedSearch();
  void Execute( int FirstKey = 0 );
};


#endif //!definec(___SpeedSearch_H___)
