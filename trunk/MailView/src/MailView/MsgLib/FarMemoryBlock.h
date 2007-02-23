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
#ifndef ___FarMemoryBlock_H___
#define ___FarMemoryBlock_H___

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct TMemoryBlock * PMemoryBlock;

struct TMemoryBlock
{
  DWORD size;
  PBYTE data;

  TMemoryBlock();
  TMemoryBlock(PBYTE lpData, DWORD dwSize);

  bool SaveToFile(LPCSTR fileName, bool bOverwrite = true);
};

inline TMemoryBlock::TMemoryBlock() : size(0), data(NULL)
{
}

inline TMemoryBlock::TMemoryBlock(PBYTE lpData, DWORD dwSize)
: size(dwSize), data(lpData)
{
}


inline bool TMemoryBlock::SaveToFile( LPCSTR FileName, bool bOverwrite )
{
  HANDLE hFile = CreateFile( FileName, GENERIC_WRITE, FILE_SHARE_READ,
    NULL, bOverwrite ? CREATE_ALWAYS : CREATE_NEW, 0, 0 );

  if ( hFile == INVALID_HANDLE_VALUE )
    return false;

  DWORD dwWritten;

  WriteFile( hFile, data, size, &dwWritten, NULL );

  return CloseHandle( hFile ) != 0 && dwWritten == size;
}

#endif //!defined(___FarMemoryBlock_H___)
