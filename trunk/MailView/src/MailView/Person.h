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
#ifndef ___Person_H___
#define ___Person_H___

#include <FarString.h>
#include <FarArray.h>

#include "Plugins/FidoSuite.h"


typedef class CPerson * PPerson;

class CPerson
{
  friend PPerson rfc822_parse_adrlist( PPerson Top, LPCSTR s );
  friend void add_addrspec( PPerson * Top, PPerson * Last, LPCSTR phrase,
    LPSTR comment, size_t * commentlen, size_t commentmax );
private:
  PPerson   m_Next;
public:
  CPerson();
  ~CPerson();

  FarString Name;
  FarString Addr;
  TFTNAddr  FTNAddr;

  FarString GetMailboxName() const;

  PPerson GetNext() const
  {
    return m_Next;
  }

  FarString getAddresses() const;

  static PPerson Create( LPCSTR AddrList );
};


#endif //!defined(___Person_H___)
