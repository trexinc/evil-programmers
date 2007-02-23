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
#ifndef ___FarCopyDlg_H___
#define ___FarCopyDlg_H___

#include <FarString.h>

class FarCopyDlg
{
private:
  HANDLE m_hScreen;
  DWORD  m_StartTime;
  int    m_TotalSize;
  int    m_CurSize;
  LPSTR  m_Items[ 10 ];
  int    m_ItemsCount;

  char m_CurBar[ 42 ];
  char m_AllBar[ 42 ];
  char m_Total[ 42 ];
  char m_Times[ 42 ];

  void Init();

  int m_AcceptForAll;

  int m_Confirmations;

public:
  FarCopyDlg( int nTotalSize );
  ~FarCopyDlg();

  enum TOperation { opNone, opCancel, opSkip, opOverwrite, opAppend };

  TOperation Copy( LPCSTR From, LPCSTR To, int nSize, const FILETIME& WriteTime );

};

#endif //!defined(___FarCopyDlg_H___)
