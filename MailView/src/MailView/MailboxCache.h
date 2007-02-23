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
#ifndef ___MailboxCache_H___
#define ___MailboxCache_H___

#include <FarPlus.h>
#include <FarHash.h>
#include "References.h"

struct TCacheEntry
{
  DWORD          Handle;       // handle to message in mailbox
  DWORD          Size;         // raw size
  DWORD          Encoding;     // index of encoging in far table of FCT_*
  FarString      Subject;      // processed subject (without Re: etc...)
  FarString      From;
  FarString      To;
  FarString      MessageID;    // message's mail id
  FarStringArray ParentIDs;    // ids of parents (References, In-Reply-To)
  TMsgInfo       Info;
  bool Read( FarFile& File );
  bool Write( FarFile& File );
  FarString      DiplayName;   // subject + tree part || other cur display field
  DWORD          index; // for sort
};

struct TMailboxCache
{
  ULARGE_INTEGER FileSize;
  FILETIME  FileAge;
  CThread * Ref;
  FarArray<TCacheEntry> Items;
  DWORD bInterrupted;

  PluginPanelItem levelUp;

  int sortMode, sortOrder, theradsViewMode;

  void remove( TCacheEntry * ce );

  bool IsUpToDate()
  {
    if ( Items.Count() > 0 && !bInterrupted )
      return false;
    return true;
  }
  TMailboxCache()
    : Ref( NULL )
    , bInterrupted( FALSE )
    , sortMode( -1 )
    , sortOrder( -1 )
  {
  }
};
/*
class CCacheEntryHash : public FarHashT
{
private:
  virtual DWORD HashKey( void * );
  virtual void  Destroy( void * );
  virtual int   Compare( void *, void * );

public:
  CCacheEntryHash( int nTableSize );
  virtual ~CCacheEntryHash();

  bool Insert( TCacheEntry * ce );
  bool Remove( TCacheEntry * ce );

  TCacheEntry * Find( TCacheEntry * ce );
  //TCacheEntry * operator[]( TCacheEntry * ce );
};
*/

#endif //!defined(___MailboxCache_H___)
