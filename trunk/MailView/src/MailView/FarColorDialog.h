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
#ifndef ___FarColorDialog_H___
#define ___FarColorDialog_H___

#if _MSC_VER >= 1000
#pragma once
#endif

#include "FarDialogEx.h"

class CFarColorDialog : public CFarDialog
{
private:
  int m_Color;
  CFarBox * m_SampleTextBox;

  far_msg bool OnCtlColorDlgItem( TDnColor & Item );
  far_msg bool OnBtnClick( TDnBtnClick & Item );
public:
  CFarColorDialog( const int Color );
  virtual ~CFarColorDialog()
  {
  }

  int GetColorValue() const;
  void SetColorValue( const int Value );

FAR_BEGIN_MESSAGE_MAP
  FAR_MESSAGE_HANDLER( DN_BTNCLICK, TDnBtnClick, OnBtnClick )
//  FAR_MESSAGE_HANDLER( DN_CTLCOLORDLGITEM, TDnColor, OnCtlColorDlgItem )
FAR_END_MESSAGE_MAP( CFarDialog )
};

#endif //!defined(___FarColorDialog_H___)
