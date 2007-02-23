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
#include "stdafx.h"
#include <FarPlus.h>
#include "Mailbox.h"
#include "LangId.h"

CMailbox::CMailbox( HMODULE hPluginDLL )
  : m_hMailbox( NULL )
  , m_pFile( NULL )
  , m_shortName( NULL )
  , m_pluginDll( hPluginDLL )
{
  m_GetPlugName  = (TGetStringProc)GetProcAddress( hPluginDLL, "Mailbox_GetName" );
  m_GetShortName = (TGetStringProc)GetProcAddress( hPluginDLL, "Mailbox_GetShortName" );

  m_OpenMem  = (TOpenMemProc) GetProcAddress( hPluginDLL, "Mailbox_OpenMem");
  m_Open     = (TOpenFileProc)GetProcAddress( hPluginDLL, "Mailbox_OpenFile" );
  m_Close    = (TCloseProc)   GetProcAddress( hPluginDLL, "Mailbox_Close" );
  m_Create   = (TCreateProc)  GetProcAddress( hPluginDLL, "Mailbox_Create");

  m_GetMsgType = (TGetMsgTypeProc)GetProcAddress( hPluginDLL, "Mailbox_GetMsgType" );
  m_GetNextMsg = (TGetNextMsgProc)GetProcAddress( hPluginDLL, "Mailbox_GetNextMsg");
  m_GetMsgInfo = (TGetMsgInfoProc)GetProcAddress( hPluginDLL, "Mailbox_GetMsgInfo");
  m_SetMsgInfo = (TSetMsgInfoProc)GetProcAddress( hPluginDLL, "Mailbox_SetMsgInfo");

  m_GetMsgHead = (TGetMsgHeadProc)GetProcAddress( hPluginDLL, "Mailbox_GetMsgHead");

  m_GetMsg = (TGetMsgProc)GetProcAddress( hPluginDLL, "Mailbox_GetMsg");
  m_AddMsg = (TAddMsgProc)GetProcAddress( hPluginDLL, "Mailbox_AddMsg");
  m_DelMsg = (TDelMsgProc)GetProcAddress( hPluginDLL, "Mailbox_DelMsg");

  m_Purge  = (TPurgeProc)GetProcAddress( hPluginDLL, "Mailbox_Purge");
}

CMailbox::~CMailbox()
{
  close();
}

bool CMailbox::open( const FarFileName& fileName, bool createNew )
{
  close();

  if ( createNew )
  {
    if ( m_Create == NULL )
      return false;

    m_hMailbox = m_Create( fileName.c_str() );
  }
  else
  {
    if ( m_Open )
      m_hMailbox = m_Open( fileName.c_str() );

    if ( m_hMailbox == NULL && m_OpenMem )
    {
      if ( m_pFile == NULL )
        m_pFile = create FarMemoryMappedFile;

      DWORD flags = FILE_SHARE_READ;
      //if ( m_Config->GetAllowWriteAccess() != 0 )
      //  Flags |= FILE_SHARE_WRITE;

      if ( m_pFile->Open( fileName, GENERIC_READ, flags, OPEN_EXISTING ) == false )
        return false;

      m_hMailbox = m_OpenMem( m_pFile->GetMemory(), m_pFile->GetSize() );

      if ( m_hMailbox == NULL )
      {
        delete m_pFile;
        m_pFile = NULL;
        return false;
      }
    }
  }

  if ( m_hMailbox )
  {
    m_fileName = fileName;
    return true;
  }

  return false;
}

void CMailbox::close()
{
  if ( m_hMailbox && m_Close )
    m_Close( m_hMailbox );

  m_hMailbox = NULL;

  if ( m_pFile )
  {
    delete m_pFile;
    m_pFile = NULL;
  }

  if ( m_shortName )
  {
    delete [] m_shortName;
    m_shortName = NULL;
  }

  m_fileName.Empty();
}

LPCSTR CMailbox::getShortName()
{
  DWORD size = 0;
  if ( !m_shortName && m_GetShortName && m_GetShortName( NULL, &size ) && size > 0 )
  {
    m_shortName = new char[ size + 1 ];
    if ( m_GetShortName( m_shortName, &size ) )
      m_shortName[ size ] = '\0';
    else
    {
      delete [] m_shortName;
      m_shortName = NULL;
    }
  }
  return m_shortName;
}
