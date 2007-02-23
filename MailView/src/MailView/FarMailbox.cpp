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
#include "MailView.h"
#include "FarDialogEx.h"
#include "SpeedSearch.h"
#include "MailViewTpl.h"
#include "MailViewDlg.h"
#include "FarCopyDlg.h"
#include "FarDialog.h"
#include <FarMenu.h>
#include <FarLog.h>

#include "FarMailbox.h"
#include "Mime.h"
#include "Message.h"

#include "FarInetMessage.h"
#include "FarFidoMessage.h"
#include <ctype.h>

#define EMF__ATTACH (1<<31)

#define HISTORY_OUTPUTFMT "MailView\\OutputFormat"
#define HISTORY_COPYFILES "MailView\\CopyFiles"

//#define _SHOW_SPEED 1

#ifndef _FAR_USE_FARFINDDATA
#error _FAR_USE_FARFINDDATA not defined!!!
#endif

bool CreateDirectory( LPSTR path )
{
  //BOOL fileApisAnsi = AreFileApisANSI();

  //if ( fileApisAnsi )
  //  SetFileApisToOEM();

  for ( char * c = path; *c; c ++ )
  {
    if ( *c != '\x20' )
    {
      for ( ; *c; c ++ )
      {
        if ( *c == '\\' || *c == '/' )
        {
          char sep = *c;

          *c = 0;

          if ( GetFileAttributes( path ) == -1 && !CreateDirectory( path, NULL ) )
            return false;

          *c = sep;
        }
      }

      if ( GetFileAttributes( path ) == -1 && !CreateDirectory( path, NULL ) )
        return false;

      break;
    }
  }

  //if ( fileApisAnsi )
  //  SetFileApisToANSI();

  return true;
}


void ProcessSubject( LPSTR pStr );
void ProcessSubjectStub( LPSTR )
{
}

void CorrectFileName( LPSTR fileName, int nameSize, bool allowPathChar = false )
{
  char invalidSymbols[] = { '<', '>', ':', '|', '?', '*', '\"', '/', '\\' };
  char correctSymbols[] = { '(', ')', '_', '!', '_', '_', '\'', '!', '!' };
  far_assert( sizeof(invalidSymbols) == sizeof(correctSymbols) );

  int symlen = sizeof( invalidSymbols );
  if ( allowPathChar )
    correctSymbols[ symlen - 1 ] = invalidSymbols[ --symlen ];

  for ( int i = nameSize - 1; i >= 0; i -- )
  {
    for ( int j = 0; j < symlen; j ++ )
    {
      if ( fileName[ i ] == invalidSymbols[ j ] )
      {
        fileName[ i ] = correctSymbols[ j ];
        break;
      }
    }
  }
}

void CorrectFileName( LPSTR fileName, bool allowPathChar = false )
{
  CorrectFileName( fileName, strlen( fileName ), allowPathChar );
}

/////////////////////////////////////////////////////////////////////////////
// Constructor
CFarMailbox::CFarMailbox( HMODULE hPluginDLL, const FarFileName& fileName, CMailboxCfg * GlobalCfg )
: FarCustomPanelPlugin( fileName ), m_mailbox( hPluginDLL ), m_RefMap( NULL )
{
  m_Config = GlobalCfg;

  m_CurRef = NULL;

  KillRe = m_Config->GetProcessSubject() == 0 ? ProcessSubjectStub : ProcessSubject;

  m_msgExt = m_mailbox.getMsgType() == EMT_INET ? ".eml" : ".msg";

  m_noneSubj = m_Config->getEmptySubj();

  LPCSTR shortName = m_mailbox.getShortName();
  if ( !shortName )
    shortName = Far::GetMsg( MMailbox_Unknown );

  m_formatName.Format( MMailbox_FormatName, shortName );

  m_panelTitle.Format( MPanelTitle_Format, m_formatName.c_str(),
    fileName.GetName().c_str() );
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
CFarMailbox::~CFarMailbox()
{
  Close();

  if ( m_RefMap )
    delete m_RefMap;
}

//////////////////////////////////////////////////////////////////////////
//
bool CFarMailbox::Open()
{
  if ( !m_mailbox.open( m_HostFileName ) )
    return false;

  far_assert( m_Config != NULL );

  FarFileName SettingsName = m_HostFileName;
  if ( *m_Config->GetConfigExt() != '.' )
    SettingsName.SetExt( '.' + FarFileName( m_Config->GetConfigExt() ) );
  else
    SettingsName += m_Config->GetConfigExt();

/*  if ( m_Config->GetUseNTFSStreams() && !File( SettingsName ).exists() )
  {
    char FileSystemName[ 5 ];
    if ( GetVolumeInformation( SettingsName.GetDrive() + '\\',
      NULL, 0, NULL, NULL, NULL, FileSystemName, sizeof( FileSystemName ) ) &&
      FarSF::LStricmp( FileSystemName, "NTFS" ) == 0 )
    {
      SettingsName = m_HostFileName + ":mailview.mailbox.settings";
    }
  }
*/
  m_Config = create CMailboxCfg( SettingsName, m_Config );

  LoadCache();

  KillRe = m_Config->GetProcessSubject() == 0 ? ProcessSubjectStub : ProcessSubject;

  return true;
}

//////////////////////////////////////////////////////////////////////////
//
void CFarMailbox::Close( bool saveCache )
{
  if ( m_mailbox.isClosed() )
    return;

  if ( saveCache )
    SaveCache();

  m_mailbox.close();

  CMailboxCfg * Parent = m_Config->GetParent();
  if ( Parent )
  {
    delete m_Config;
    m_Config = Parent;
  }

  KillRe = m_Config->GetProcessSubject() == 0 ? ProcessSubjectStub : ProcessSubject;
}

/////////////////////////////////////////////////////////////////////////////
//
void CFarMailbox::GetOpenInfo( OpenPluginInfo * pInfo )
{
  pInfo->Flags |= OPIF_USEHIGHLIGHTING;
  if (m_Config->GetUseAttributeHighlighting())
    pInfo->Flags |= OPIF_USEATTRHIGHLIGHTING;
  pInfo->Format     = m_formatName.c_str();
  pInfo->PanelTitle = m_panelTitle.c_str();
  pInfo->HostFile   = m_HostFileName.c_str();
//  m_RefMap.LookupKey( )
//  pInfo->CurDir     = (LPSTR)m_RefMap[ m_CurRef->GetKey() ];


  pInfo->PanelModesArray  = m_Config->GetMailboxPanelMode();
  pInfo->PanelModesNumber = 1;
  pInfo->StartPanelMode   = m_Config->GetStartPanelMode() < 0 ? 0 : '0' + m_Config->GetStartPanelMode();
  pInfo->StartSortMode = SM_UNSORTED;
  //pInfo->StartSortMode    = m_Config->GetStartSortMode();
  //pInfo->StartSortOrder   = m_Config->GetStartSortOrder();

  static KeyBarTitles kbTitles;
  ZeroMemory( &kbTitles, sizeof( KeyBarTitles ) );

  kbTitles.Titles     [ 5 ] = (LPSTR)STR_EmptyStr;
  kbTitles.Titles     [ 6 ] = (LPSTR)STR_EmptyStr;
  kbTitles.AltTitles  [ 5 ] = (LPSTR)STR_EmptyStr;
  kbTitles.ShiftTitles[ 5 ] = (LPSTR)STR_EmptyStr;

  for ( int i = 0; i < 12; i ++ )
    kbTitles.CtrlTitles[ i ] = getKbTitleMsg( MkbCtrlF1 + i );

  for ( int i = 0; i < 12; i ++ )
    kbTitles.ShiftTitles[ i ] = getKbTitleMsg( MkbShiftF1 + i );

  pInfo->KeyBar = &kbTitles;
}

/////////////////////////////////////////////////////////////////////////////
//
class CSearchMessagesDlg
{
private:
  FarString m_HostFile;
  DWORD     m_MsgFlags;
  DWORD     m_Ticks;
  FarString m_FilesMsg;
  HANDLE    m_hScreen;

  LPCSTR    title;
  LPCSTR    message;
  int       m_cPrevLength;
  int       m_cTitleLength;

public:
  CSearchMessagesDlg( const FarString& hostFile )
    : m_HostFile( FarSF::TruncPathStr( hostFile, 64 ) )
    , m_MsgFlags( 0 )
    , m_Ticks( GetTickCount() )
    , m_hScreen( Far::SaveScreen() )
    , title( Far::GetMsg( MReadingMailbox ) )
    , message( Far::GetMsg( MFoundNMessages ) )
    , m_cPrevLength( 0 )
  {
    m_cTitleLength = strlen(title);
  }

  ~CSearchMessagesDlg()
  {
    Far::RestoreScreen( NULL );
    Far::RestoreScreen( m_hScreen );
  }

  void setTitle( LPCSTR title )
  {
    this->title = title;
    m_cTitleLength = strlen(title);
  }

  void setTitle( int titleId )
  {
    this->title = Far::GetMsg( titleId );
    m_cTitleLength = strlen(title);
  }

  void setMessage( LPCSTR message )
  {
    this->message = message;
  }

  void setMessage( int messageId )
  {
    this->message = Far::GetMsg( messageId );
  }

  void force()
  {
    m_Ticks += 100;
  }

  void update( int Number )
  {
    if ( GetTickCount() - m_Ticks > 100 )
    {
      m_FilesMsg.Format( message, Number );

      int NewLength = m_FilesMsg.Length();
      if (NewLength>m_cPrevLength && NewLength>m_cTitleLength)
        m_MsgFlags=0;
      m_cPrevLength = NewLength;

      FarMessage().SimpleMsg( m_MsgFlags, title, m_HostFile.c_str(), m_FilesMsg.c_str(), -1 );

      m_MsgFlags = FMSG_KEEPBACKGROUND;

      m_Ticks = GetTickCount();
    }
  }
};

/////////////////////////////////////////////////////////////////////////////
//
PMessage CFarMailbox::CreateFarMessage()
{
  DWORD MsgType = m_mailbox.getMsgType();

  if ( MsgType == EMT_INET )
    return create CFarInetMessage;

  if ( MsgType == EMT_FIDO )
    return create CFarFidoMessage;

  DbgMsg( Far::GetMsg( MWarning ), "mailbox contain unknown format of messages" );
  return NULL;
}

bool CFarMailbox::addCacheItem(CMailbox * mailbox, DWORD msgId, PMessage Msg, long defaultEncoding)
{
  if ( !Msg->read( (long)mailbox, msgId, FCT_DEFAULT, true ) )
    return false;

  enum TGrouping
  {
    grpByNone, grpByFrom, grpByTo, grpBySubject
  };

  TGrouping grp = grpByFrom;

  if (grp > grpByNone)
  {
    FarString curr;

    switch (grp)
    {
    case grpByFrom:
      FarString s = Msg->GetFrom()->GetMailboxName();
      break;

    }
  }

  TCacheEntry * ce = create TCacheEntry;
  m_Cache.Items.Add( ce );

  ce->Handle    = msgId;
  ce->Size      = Msg->GetSize();
  ce->Encoding  = Msg->GetEncoding();

  if ( Msg->GetEncoding() == FCT__INVALID )
    Msg->SetEncoding( defaultEncoding );

  ce->Subject = Msg->GetSubject();

  ce->From = Msg->GetFrom()->GetMailboxName();

  ce->To = Msg->GetTo()->GetMailboxName();

  Msg->GetSent( &ce->Info.Sent );
  Msg->GetReceived( &ce->Info.Received );
  Msg->GetAccessed( &ce->Info.Accessed );
  ce->Info.Priority = Msg->GetPriority();
  ce->Info.Flags = Msg->GetFlags();

  KillRe( ce->Subject.GetBuffer() );
  ce->Subject.ReleaseBuffer();
  if ( ce->Subject.IsEmpty() )
    ce->Subject = m_noneSubj;

  ce->MessageID = Msg->GetId();
  if ( ce->MessageID[ 0 ] == '<' )
    ce->MessageID.Delete( 0 );
  if ( ce->MessageID[ ce->MessageID.Length() - 1 ] == '>' )
    ce->MessageID.Delete( ce->MessageID.Length() - 1 );
  if ( ce->MessageID.Length() == 0 )
  {
    DWORD size = MAX_COMPUTERNAME_LENGTH + 64;
    LPSTR data = new char[ size ];

    if ( GetComputerName( data, &size ) )
      data[ size ] = '\0';
    else
      strcpy( data, "localhost" );

    ce->MessageID.Format( "MailView$%08X.%08X@%s", GetTickCount(), ce, data );

    delete [] data;

  }

  Msg->GetReferences( ce->ParentIDs );

  // remove references to itself
  int DupIdx;
  while ( ( DupIdx = ce->ParentIDs.IndexOf( ce->MessageID ) ) != -1 )
    ce->ParentIDs.Delete( DupIdx );

  // используется нестандартный флаг, если его значение поменяется,
  // то надо увеличить версию файла кэша.
  if ( Msg->GetAttchmentsCount() > 0 )
    ce->Info.Flags |= EMF__ATTACH;

  return true;
}

int CFarMailbox::sortBySubject( const TCacheEntry ** ppce1, const TCacheEntry ** ppce2, void * user )
{
  far_assert( *ppce1 != NULL );
  far_assert( *ppce2 != NULL );

  int result = (*ppce2)->Subject.Compare( (*ppce1)->Subject );

  if ( result == 0 )
    return sortBySent( ppce1, ppce2, user );

  if ( user != 0 )
    result = ~(result-1);

  return result;
}

int CFarMailbox::sortByFrom( const TCacheEntry ** ppce1, const TCacheEntry ** ppce2, void * user )
{
  far_assert( *ppce1 != NULL );
  far_assert( *ppce2 != NULL );

  int result = (*ppce2)->From.Compare( (*ppce1)->From );

  if ( result == 0 )
    return sortBySent( ppce1, ppce2, user );

  if ( user != 0 )
    result = ~(result-1);

  return result;
}

int CFarMailbox::sortByTo( const TCacheEntry ** ppce1, const TCacheEntry ** ppce2, void * user )
{
  far_assert( *ppce1 != NULL );
  far_assert( *ppce2 != NULL );

  int result = (*ppce2)->To.Compare( (*ppce1)->To );

  if ( result == 0 )
    return sortBySent( ppce1, ppce2, user );

  if ( user != 0 )
    result = ~(result-1);

  return result;
}

int CFarMailbox::sortBySize( const TCacheEntry ** ppce1, const TCacheEntry ** ppce2, void * user )
{
  far_assert( *ppce1 != NULL );
  far_assert( *ppce2 != NULL );

  int result = (*ppce1)->Size < (*ppce2)->Size ? -1 : (*ppce1)->Size > (*ppce2)->Size ? 1 : 0;

  if ( result == 0 )
    return sortBySent( ppce1, ppce2, user );

  if ( user != 0 )
    result = ~(result-1);

  return result;
}

int CFarMailbox::sortBySent( const TCacheEntry ** ppce1, const TCacheEntry ** ppce2, void * user )
{
  far_assert( *ppce1 != NULL );
  far_assert( *ppce2 != NULL );

  int result = 0;

  if (&(*ppce1)->Info.Flags==0 && &(*ppce2)->Info.Flags!=0)
    result=1;
  else if (&(*ppce1)->Info.Flags!=0 && &(*ppce2)->Info.Flags==0)
    result=-1;

  if (!result)
    result = CompareFileTime( &(*ppce1)->Info.Sent, &(*ppce2)->Info.Sent );

  if ( result == 0 )
    return sortByHandle( ppce1, ppce2, user );

  if ( user != 0 )
    result = ~(result-1);

  return result;
}

int CFarMailbox::sortByReceived( const TCacheEntry ** ppce1, const TCacheEntry ** ppce2, void * user )
{
  far_assert( *ppce1 != NULL );
  far_assert( *ppce2 != NULL );

  int result = 0;

  if (&(*ppce1)->Info.Flags==0 && &(*ppce2)->Info.Flags!=0)
    result=1;
  else if (&(*ppce1)->Info.Flags!=0 && &(*ppce2)->Info.Flags==0)
    result=-1;

  if (!result)
    result = CompareFileTime( &(*ppce1)->Info.Received, &(*ppce2)->Info.Received );

  if ( result == 0 )
    return sortBySent( ppce1, ppce2, user );

  if ( user != 0 )
    result = ~(result-1);

  return result;
}

int CFarMailbox::sortByAccessed( const TCacheEntry ** ppce1, const TCacheEntry ** ppce2, void * user )
{
  far_assert( *ppce1 != NULL );
  far_assert( *ppce2 != NULL );

  int result = 0;

  if (&(*ppce1)->Info.Flags==0 && &(*ppce2)->Info.Flags!=0)
    result=1;
  else if (&(*ppce1)->Info.Flags!=0 && &(*ppce2)->Info.Flags==0)
    result=-1;

  if (!result)
    result = CompareFileTime( &(*ppce1)->Info.Accessed, &(*ppce2)->Info.Accessed );

  if ( result == 0 )
    return sortBySent( ppce1, ppce2, user );

  if ( user != 0 )
    result = ~(result-1);

  return result;
}

int CFarMailbox::sortByHandle( const TCacheEntry ** ppce1, const TCacheEntry ** ppce2, void * user )
{
  if ( user == 0 )
    return (*ppce1)->Handle < (*ppce2)->Handle ? -1 : (*ppce1)->Handle > (*ppce2)->Handle ? 1 : 0;
  else
    return (*ppce1)->Handle > (*ppce2)->Handle ? -1 : (*ppce1)->Handle < (*ppce2)->Handle ? 1 : 0;

  return 0;
}

int CFarMailbox::sortByCacheIndex( const TCacheEntry ** ppce1, const TCacheEntry ** ppce2 )
{
  return (*ppce1)->index < (*ppce2)->index ? -1 : (*ppce1)->index > (*ppce2)->index ? 1 : 0;
}

/////////////////////////////////////////////////////////////////////////////
//
bool CFarMailbox::GetFindData( FarPluginPanelItems *Items, const int OpMode )
{
#if defined(_DEBUG) && defined(_SHOW_SPEED)
  DWORD chktc = GetTickCount();
#endif

  bool updateOnly = true;
  FarSaveScreen *SaveScreen=NULL;

  if ( m_Cache.IsUpToDate() )
  {
    PMessage msg = CreateFarMessage();
    if ( msg == NULL )
      return false;

    m_Cache.Items.Clear();

    CSearchMessagesDlg sm( m_HostFileName );
    sm.setTitle( MReadingMailbox );
    sm.setMessage( MFoundNMessages );

    DWORD msgId = BAD_MSG_ID, esc = 0, tc = GetTickCount();

    long defaultEncoding = getCharacterTable(m_Config->GetDefaultCharset());

    while ( (msgId = m_mailbox.getNextMsg( msgId )) != BAD_MSG_ID )
    {
      addCacheItem(&m_mailbox, msgId, msg, defaultEncoding);

      sm.update( m_Cache.Items.Count() );

      if ( GetTickCount() - tc > 500 )
      {
        if ( (esc = FarSF::CheckForEsc()) != 0 && confirm( MReadingMailbox ) )
          break;
        tc = GetTickCount();
      }
    }
    m_Cache.bInterrupted = esc;

    delete msg;

    m_Cache.sortMode = m_Cache.sortOrder = -1;

    updateOnly = false;
  }
  else
  {
    if ( m_Config->GetStartSortMode() != m_Cache.sortMode ||
      m_Config->GetStartSortOrder() != m_Cache.sortOrder ||
      m_Config->GetTheradsViewMode() != m_Cache.theradsViewMode )
    {
      SaveScreen = new FarSaveScreen(true);
      FarMessage().SimpleMsg( 0, MReadingMailboxCache, m_HostFileName.c_str(), -1 );

      updateOnly = false;
    }
  }

  if ( !updateOnly )
  {
    MakeRefs();

    m_Cache.sortMode  = m_Config->GetStartSortMode();
    m_Cache.sortOrder = m_Config->GetStartSortOrder();
    m_Cache.theradsViewMode = m_Config->GetTheradsViewMode();

    memset( &m_Cache.levelUp, 0, sizeof( m_Cache.levelUp ) );

    static LPSTR STR_LevelUp    = "..";
    m_Cache.levelUp.UserData    = 0;
    m_Cache.levelUp.Description = STR_LevelUp;

    strcpy( m_Cache.levelUp.FindData.cFileName, STR_LevelUp );
    strcpy( m_Cache.levelUp.FindData.cAlternateFileName, STR_LevelUp );

    m_Cache.levelUp.FindData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
    HANDLE hFile = CreateFile( m_HostFileName, 0, FILE_SHARE_READ|FILE_SHARE_WRITE,
      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( hFile != INVALID_HANDLE_VALUE )
    {
      GetFileTime( hFile,
        &m_Cache.levelUp.FindData.ftCreationTime,
        &m_Cache.levelUp.FindData.ftLastAccessTime,
        &m_Cache.levelUp.FindData.ftLastWriteTime );
      CloseHandle( hFile );
    }

    Items->SetCount( m_Cache.Items.Count() + 1 ); // one room for level up

    PluginPanelItem * curItem = Items->GetItems();

    memcpy( curItem, &m_Cache.levelUp, sizeof( m_Cache.levelUp ) );

    curItem = InsertTreeItems( m_Cache.Ref, ++curItem, 0 );

    far_assert( (&((*Items)[ 0 ]) + Items->Count()) == curItem );

    for ( int i = 1; i < Items->Count(); i ++ )
    {
      (*Items)[ i ].CRC32 = i;
      ((TCacheEntry*)(*Items)[ i ].UserData)->index = i;

    }

    m_Cache.Items.Sort( sortByCacheIndex );
  }
  else
  {
    Items->SetCount( m_Cache.Items.Count() + 1 ); // one room for level up

    PluginPanelItem * curItem = Items->GetItems();

    memcpy( curItem, &m_Cache.levelUp, sizeof( m_Cache.levelUp ) );

    for ( int i = 0; i < m_Cache.Items.Count(); i ++ )
    {
      SetItem( ++curItem, m_Cache.Items.At( i ) );

      curItem->CRC32 = i + 1;
    }
  }

/*  PluginPanelItem &item = Items.Add();
  item.FindData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
  strcpy(item.FindData.cFileName, "this is  a folder");
*/
#if defined(_DEBUG) && defined(_SHOW_SPEED)
  DbgMsg( "open time", "%d ms", GetTickCount() - chktc );
#endif

  if (SaveScreen)
    delete SaveScreen;

  return true;
}

class CCacheEntryOnMessageTemplate : public CMessageTemplate
{
private:

  TCacheEntry * ce;

  class CWriteSink : public CTemplate::IWriteSink
  {
  private:
    char * o, * s;

  public:
    CWriteSink(char * ss) : o(ss), s(ss)
    {
    }

    void reset()
    {
      s = o;
    }

    virtual ~CWriteSink()
    {
    }

    virtual void write(LPCSTR str, int len)
    {
      s = (char*)memcpy(s, str, len) + len;

      *s = '\0';
    }
  } ws;

public:
  CCacheEntryOnMessageTemplate(char * s)
    : CMessageTemplate()
    , ce(NULL)
    , ws(s)
  {
  }

  virtual ~CCacheEntryOnMessageTemplate()
  {
  }

  void assign(TCacheEntry * ce)
  {
    this->ce = ce;

  }

  void processLine(LPCSTR str)
  {
    ws.reset();
    CMessageTemplate::processLine(str, &ws);
  }

private:

  virtual void writeFormatted( const FarString& str, int len, int fill, IWriteSink & f )
  {
    FarString s = str;
    CorrectFileName( s.GetBuffer(), s.Length() );
    CMessageTemplate::writeFormatted( s, len, 0, f );
  }

  virtual void writeFormatted( const FarString& str, int len, IWriteSink & f )
  {
    writeFormatted( str, len, 0, f );
  }

  virtual void processSubject( int len, int fill, IWriteSink & f )
  {
    writeFormatted( ce->Subject, len, f );
  }

  virtual void processFrom( int len, int fill, IWriteSink & f )
  {
    writeFormatted( ce->From, len, f );
  }

  virtual void processTo( int len, int fill, IWriteSink & f )
  {
    writeFormatted( ce->To, len, f );
  }

  virtual void processOSubject( int len, int fill, IWriteSink & f )
  {
    writeFormatted( ce->Subject, len, f );
  }

  virtual void processMessageId( int len, int fill, IWriteSink & f )
  {
    writeFormatted( ce->MessageID, len, f );
  }

  virtual void processFromName( int len, int fill, IWriteSink & f )
  {
    PPerson person = CPerson::Create( ce->From );
    if ( person )
    {
      writeFormatted( person->Name, len, f );
      delete person;
    }
  }

  virtual void processFromAddr( int len, int fill, IWriteSink & f )
  {
    PPerson person = CPerson::Create( ce->From );
    if ( person )
    {
      writeFormatted( person->Addr, len, f );
      delete person;
    }
  }

  virtual void processToName( int len, int fill, IWriteSink & f )
  {
    PPerson person = CPerson::Create( ce->To );
    if ( person )
    {
      writeFormatted( person->Name, len, f );
      delete person;
    }
  }

  virtual void processToAddr( int len, int fill, IWriteSink & f )
  {
    PPerson person = CPerson::Create( ce->To );
    if ( person )
    {
      writeFormatted( person->Addr, len, f );
      delete person;
    }
  }

  virtual void processSentDate( int len, int fill, IWriteSink & f )
  {
    writeFormatted(((FileTime)ce->Info.Sent).getDate(ViewCfg()->getDateFormat(), LCID_DateTime), len, f);
  }

  virtual void processSentTime( int len, int fill, IWriteSink & f )
  {
    writeFormatted(((FileTime)ce->Info.Sent).getTime(ViewCfg()->getTimeFormat(), LCID_DateTime), len, f);
  }

  virtual void processReceivedDate( int len, int fill, IWriteSink & f )
  {
    writeFormatted(((FileTime)ce->Info.Received).getDate(ViewCfg()->getDateFormat(), LCID_DateTime), len, f);
  }

  virtual void processReceivedTime( int len, int fill, IWriteSink & f )
  {
    writeFormatted(((FileTime)ce->Info.Received).getTime(ViewCfg()->getTimeFormat(), LCID_DateTime), len, f);
  }
};

/////////////////////////////////////////////////////////////////////////////
//
int CFarMailbox::GetFiles( FarPluginPanelItems *Items, bool bMove, LPSTR DestPath, const int OpMode )
{
  if ( Items->Count() == 0 )
    return 1;

  FarSF::AddEndSlash( DestPath );

  LPSTR fn = create char[ 2048 ];

  FarCopyDlg * CopyDlg = NULL;

  FarString tpl = m_Config->getCopyOutputFormat();

  if ( ( OpMode == 0 || OpMode == OPM_TOPLEVEL ) )
  {
    FarDialog dlg( MdlgCopy_Copy, "CopyFiles" );
    if ( Items->Count() != 1 )
      FarSF::snprintf( fn, 2048, Far::GetMsg( MdlgCopy_CopyItems ), Items->Count() );
    else
    {
      TCacheEntry * ce = (TCacheEntry*)(*Items)[ 0 ].UserData;
      if (!ce)
      {
        delete [] fn;
        return 1;
      }
      int txLen = strlen( Far::GetMsg( MdlgCopy_CopyItem ) );
      FarString Subj = ce->Subject;
      if ( Subj.Length() + txLen > 60 )
      {
        Subj.SetLength( 60 - txLen );
        Subj += "...";
      }
      FarSF::snprintf( fn, 2048, Far::GetMsg( MdlgCopy_CopyItem ), Subj.c_str() );
    }
    dlg.AddText( fn );
    FarEditCtrl edtTo( &dlg, DestPath, 0, 66, HISTORY_COPYFILES );
    edtTo.SetNextY();

    dlg.AddSeparator();

    // TODO: локализовать

    dlg.AddText( "Output &format:" );
    FarEditCtrl edtTpl( &dlg, &tpl, 0, 64 - m_msgExt.Length(), HISTORY_OUTPUTFMT );
    edtTpl.SetNextY();

    FarTextCtrl extText( &dlg, m_msgExt, 66 - m_msgExt.Length() );

    dlg.AddSeparator();

    FarCheckCtrl chkProcessMultipleDestinations( &dlg, "Process multiple destinations", false );
    chkProcessMultipleDestinations.SetNextY();
    chkProcessMultipleDestinations.SetFlags( DIF_DISABLE );

    FarCheckCtrl chkOnlyNewer( &dlg, "Only newer file(s)", false );
    chkOnlyNewer.SetNextY();
    chkOnlyNewer.SetFlags( DIF_DISABLE );


    dlg.AddSeparator();

    FarButtonCtrl btnOk( &dlg, MdlgCopy_btnCopy );
    FarButtonCtrl btnTree( &dlg, MdlgCopy_btnTree, DIF_CENTERGROUP );
    FarButtonCtrl btnCancel( &dlg, MdlgCopy_btnCancel, DIF_CENTERGROUP );
    btnTree.SetFlags( DIF_DISABLE );

    dlg.SetDefaultControl( &btnOk );
    dlg.SetFocusControl( &edtTo );

    if ( dlg.Show() != &btnOk )
    {
      delete fn;
      return -1;
    }

    edtTo.GetText( DestPath, NM );
    FarSF::AddEndSlash( DestPath );

    tpl = edtTpl.GetText();

    int nTotalSize = 0;
    for ( int i = 0; i < Items->Count(); i ++ )
      nTotalSize += (*Items)[ i ].FindData.nFileSizeLow;

    CopyDlg = create FarCopyDlg( nTotalSize );
  }

  tpl = tpl.Trim();

  CreateDirectory( DestPath );

  strcpy( fn, DestPath );

  LPSTR pn = fn + strlen( fn );

  CCacheEntryOnMessageTemplate cetpl(pn);

  for ( int i = 0; i < Items->Count(); i ++ )
  {
    TCacheEntry * ce = (TCacheEntry*)(*Items)[ i ].UserData;

#if (FARMANAGERVERSION <= MAKEFARVERSION(1,70,1376))
#pragma message ("CFarMailbox::GetFiles - using UserData stub...")
    if ( ce == NULL )
    {
      FarFileName FileName = (*Items)[ i ].FindData.cFileName;
      FileName.SetLength( FileName.Length() - 6 );
      CThread tmpThread( FileName, NULL ),*pr;
      pr = m_RefMap->Find( &tmpThread );
      if ( pr )
      {
        ce = pr->GetData();
        if ( ce == NULL )
        {
          if ( ( OpMode & OPM_SILENT ) == 0 )
            DbgMsg( Far::GetMsg( MMailView ), "this feature allowed only in newer version of Far Manager" );
          continue;
        }
      }
    }
#endif
    far_assert( ce != NULL );

    if (!ce)
      continue;

    if (OpMode & (OPM_VIEW|OPM_EDIT|OPM_SILENT|OPM_FIND))
    {
      //strcpy( pn, DestPath );
      strcat( pn, (*Items)[ i ].FindData.cFileName );
      if ( OpMode & OPM_QUICKVIEW )
      {
        (*Items)[ i ].Flags &= ~PPIF_SELECTED;

        PMessage Msg = CreateFarMessage();
        if (Msg != NULL)
        {
          if (Msg->read((long)&m_mailbox, ce->Handle, ce->Encoding, false))
            CMailViewTpl(ViewCfg()->GetQTpl(), Msg, getCharacterTable(m_Config->GetDefaultCharset())).SaveToFile(fn);
          else
            CMessage::SaveToFile(&m_mailbox, ce->Handle, fn);

          delete Msg;
        }
        else
          CMessage::SaveToFile(&m_mailbox, ce->Handle, fn);
      }
      else
      {
        CMessage::SaveToFile(&m_mailbox, ce->Handle, fn);
      }
    }
    else
    {
      if ( tpl.Length() > 0 )
      {
        cetpl.assign(ce);

        cetpl.processLine( tpl );

        File::mkdirs( File::getParent( fn ) );

        strcat( pn, m_msgExt );
      }
      else
      {
        strcpy( pn, (*Items)[ i ].FindData.cFileName );
      }

      FarCopyDlg::TOperation op;
      if ( CopyDlg )
        op = CopyDlg->Copy( ce->Subject, fn, ce->Size, ce->Info.Received );
      else
        op = FarCopyDlg::opOverwrite;

      if ( op == FarCopyDlg::opOverwrite )
      {
        (*Items)[ i ].Flags &= ~PPIF_SELECTED;

        CMessage::SaveToFile( &m_mailbox, ce->Handle, fn );
      }
      else if ( op == FarCopyDlg::opCancel )
      {
        if ( CopyDlg )
          delete CopyDlg;
        delete [] fn;
        return -1;
      }
    }
  }

  if ( CopyDlg )
    delete CopyDlg;

  delete [] fn;

  m_Config->setCopyOutputFormat( tpl );

  return 1;
}

/////////////////////////////////////////////////////////////////////////////
//
void CFarMailbox::changeSortMode( CMailboxCfg::TSortMode newMode )
{
  if ( Far::AdvControl( ACTL_GETPANELSETTINGS, NULL ) & FPS_ALLOWREVERSESORTMODES )
  {
    if ( m_Config->GetStartSortMode() != newMode )
      m_Config->SetStartSortOrder( 0 );
    else
      m_Config->SetStartSortOrder( m_Config->GetStartSortOrder() == 0 ? 1 : 0 );

    m_Config->SetStartSortMode( newMode );

    update();
    redraw();
  }
  else if ( m_Config->GetStartSortMode() != newMode )
  {
    m_Config->SetStartSortMode( newMode );

    update();
    redraw();
  }

}

/////////////////////////////////////////////////////////////////////////////
//
bool CFarMailbox::ProcessKey( const int Key, const UINT ControlState )
{
  if ( ControlState == PKF_CONTROL )
  {
    if ( Key == VK_F3 )
    {
      changeSortMode( CMailboxCfg::smSubject );

      return true;
    }

    if ( Key == VK_F4 )
    {
      changeSortMode( CMailboxCfg::smFrom );

      return true;
    }

    if ( Key == VK_F5 )
    {
      changeSortMode( CMailboxCfg::smTo );

      return true;
    }

    if ( Key == VK_F6 )
    {
      changeSortMode( CMailboxCfg::smSize );

      return true;
    }

    if ( Key == VK_F7 )
    {
      changeSortMode( CMailboxCfg::smNone );

      return true;
    }

    if ( Key == VK_F8 )
    {
      changeSortMode( CMailboxCfg::smSent );

      return true;
    }

    if ( Key == VK_F9 )
    {
      changeSortMode( CMailboxCfg::smAccessed );

      return true;
    }

    if ( Key == VK_F10 )
    {
      changeSortMode( CMailboxCfg::smReceived );

      return true;
    }


    if ( Key == VK_F12 )
    {
      PanelInfo panelInfo = getShortInfo();

      FarMenu mnu( MsmMnuSortBy );
      mnu.SetX( panelInfo.PanelRect.left + 4 );

      FarMenu::PItem item;

      char mark = m_Config->GetStartSortOrder() == 0 ? '+' : '-';

      item = mnu.AddItem( MsmMnuSubject );
      item->SetAccelKey( KEY_CTRLF3 );
      item->SetData( CMailboxCfg::smSubject );
      if ( m_Config->GetStartSortMode() == CMailboxCfg::smSubject )
        item->SetChecked( true, mark );

      item = mnu.AddItem( MsmMnuFrom );
      item->SetAccelKey( KEY_CTRLF4 );
      item->SetData( CMailboxCfg::smFrom );
      if ( m_Config->GetStartSortMode() == CMailboxCfg::smFrom )
        item->SetChecked( true, mark );

      item = mnu.AddItem( MsmMnuTo );
      item->SetAccelKey( KEY_CTRLF5 );
      item->SetData( CMailboxCfg::smTo );
      if ( m_Config->GetStartSortMode() == CMailboxCfg::smTo )
        item->SetChecked( true, mark );

      item = mnu.AddItem( MsmMnuSize );
      item->SetAccelKey( KEY_CTRLF6 );
      item->SetData( CMailboxCfg::smSize );
      if ( m_Config->GetStartSortMode() == CMailboxCfg::smSize )
        item->SetChecked( true, mark );


      item = mnu.AddItem( MsmMnuUnsorted );
      item->SetAccelKey( KEY_CTRLF7 );
      item->SetData( CMailboxCfg::smNone );
      if ( m_Config->GetStartSortMode() == CMailboxCfg::smNone )
        item->SetChecked( true, mark );


      item = mnu.AddItem( MsmMnuCreationTime );
      item->SetAccelKey( KEY_CTRLF8 );
      item->SetData( CMailboxCfg::smSent );
      if ( m_Config->GetStartSortMode() == CMailboxCfg::smSent )
        item->SetChecked( true, mark );


      item = mnu.AddItem( MsmMnuReceivedTime );
      item->SetAccelKey( KEY_CTRLF9 );
      item->SetData( CMailboxCfg::smReceived );
      if ( m_Config->GetStartSortMode() == CMailboxCfg::smReceived )
        item->SetChecked( true, mark );

      item = mnu.AddItem( MsmMnuAccessedTime );
      item->SetAccelKey( KEY_CTRLF10 );
      item->SetData( CMailboxCfg::smAccessed );
      if ( m_Config->GetStartSortMode() == CMailboxCfg::smAccessed )
        item->SetChecked( true, mark );

      mnu.AddSeparator();

      FarMenu::PItem tv = mnu.AddItem( MsmMnuViewThreadBy );
      tv->SetSubMenu();

      far_assert( tv->GetSubMenu() != NULL );

      FarMenu& sub = *tv->GetSubMenu();
      sub.SetTitle( MsmMnuViewThreadBy );
      sub.SetX( mnu.GetX() );

      item = sub.AddItem( MvtMnuNone );
      item->SetData( tvmNone );
      if ( m_Config->GetTheradsViewMode() == tvmNone )
        item->SetChecked();

      sub.AddSeparator();

      item = sub.AddItem( MvtMnuReferences );
      item->SetData( tvmReferences_Std );
      if ( m_Config->GetTheradsViewMode() == tvmReferences_Std )
        item->SetChecked();
      item = sub.AddItem( MvtMnuSubject );
      item->SetData( tvmSubject );
      if ( m_Config->GetTheradsViewMode() == tvmSubject )
        item->SetChecked();
      item = sub.AddItem( MvtMnuFrom );
      item->SetData( tvmFrom );
      if ( m_Config->GetTheradsViewMode() == tvmFrom )
        item->SetChecked();
      item = sub.AddItem( MvtMnuTo );
      item->SetData( tvmTo );
      if ( m_Config->GetTheradsViewMode() == tvmTo )
        item->SetChecked();
      item = sub.AddItem( MvtMnuReferencesExt );
      item->SetData( tvmReferences_Ext );
      if ( m_Config->GetTheradsViewMode() == tvmReferences_Ext )
        item->SetChecked();

      item = mnu.Show();
      if ( item != NULL )
      {
        if ( mnu.GetItemIndex( item ) != -1 )
        {
          changeSortMode( (CMailboxCfg::TSortMode)item->GetData() );
        }
        else
        {
          TTheradsViewMode vm = (TTheradsViewMode)item->GetData();
          m_Config->SetTheradsViewMode( vm );
          update( true );
          m_Cache.theradsViewMode = vm;
        }
      }

      return true;
    }

    if ( Key == 'R' )
    {
      m_Cache.bInterrupted = 1;

      //update( true ); - это должен сделать сам фар

      return false; // дадим фару сделать свое грязное дело :)
    }

    if ( Key == 'A' )
    {
      return setAttributes();
    }

    return false;
  }

  if ( ControlState == PKF_SHIFT )
  {
    if ( Key == VK_DELETE )
    {
      PanelInfo info = getInfo();

      // TODO: локализовать
      FarMessage msg;
      msg.AddLine( "Delete" );
      msg.AddLine( "Do you wish to delete" );
      msg.AddFmt( "%d items", info.SelectedItemsNumber );
      msg.AddButton( "&Delete" );
      msg.AddButton( MCancel );
      if ( msg.Show() )
        return false;

      FarPluginPanelItems items( info.SelectedItems, info.SelectedItemsNumber );
      if ( deleteFiles( &items, 0, false ) )
      {
        update( true );
        redraw();
      }

      return true;
    }
    else if ( Key == VK_F7 )
    {
      PanelInfo info = getInfo();

      FarPluginPanelItems items( info.SelectedItems, info.SelectedItemsNumber );
      if ( undeleteFiles( &items ) )
      {
        update();
      }

      return true;
    }

  }

  if ( Key == VK_F9 && ControlState & (PKF_ALT|PKF_SHIFT) )
  {
    FarDialog dlg( FarString().Format( MMailbox_ConfigureS, m_HostFileName.GetName().c_str() ) );

    long defaultTable = getCharacterTable( m_Config->GetDefaultCharset() );

    FarTextCtrl textDefaultCharset( &dlg, MMailbox_DefailtEncoding );

    FarStringArray name;
    FarIntArray data;
    getFarCharacterTables( name, data, true );
    name.Add( STR_EmptyStr );
    data.Add( FCT__INVALID );
    getRfcCharacterTables( name, data, false );

    int maxlen = 30;

    for ( int i = 0; i < name.Count(); i ++ )
      maxlen = max( maxlen, (int)strlen( name.At( i ) ) );

    FarComboBox cbCharset( &dlg, 0, maxlen + 6 );
    cbCharset.SetFlags( DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND );
    cbCharset.SetNextY();

    for ( int i = 0; i < name.Count(); i ++ )
    {
      long enc = data.At( i );
      DWORD flags = 0;
      if ( enc == FCT__INVALID )
      {
        flags = LIF_SEPARATOR;
      }
      else if ( enc == defaultTable )
      {
        flags = LIF_SELECTED;
      }

      cbCharset.AddItem( name.At( i ), flags );
    }

    dlg.AddSeparator();

    FarButtonCtrl btnOk( &dlg, MOk, DIF_CENTERGROUP );
    FarButtonCtrl btnCancel( &dlg, MCancel, DIF_CENTERGROUP );
    dlg.SetDefaultControl( &btnOk );

    if ( dlg.Show() == &btnOk )
    {
      defaultTable = data.At( cbCharset.GetSelected() );
      m_Config->SetDefaultCharset( getCharsetName( defaultTable ) );
    }

    return true;
  }

  /*if ( Key == VK_F8 && ControlState & (PKF_ALT|PKF_SHIFT) )
  {
    PMessage Msg = CreateFarMessage();
    if ( Msg == NULL )
      return false;

    PanelInfo panelInfo = getPanelInfo();

    long newEncoding = SelectCharacterTableMenu( panelInfo.SelectedItemsNumber == 1 ?
      ((TCacheEntry*)panelInfo.SelectedItems[ 0 ].UserData)->Encoding : FCT_DEFAULT );

    bool needUpdate = false;

    for ( int i = 0; i < panelInfo.SelectedItemsNumber; i ++ )
    {
      TCacheEntry * ce = (TCacheEntry*)panelInfo.SelectedItems[ i ].UserData;
      if ( ce == NULL ) // не реально ??
        continue;

      if ( (long)ce->Encoding != newEncoding )
      {
        Msg->Read( (long)m_mailbox, ce->Handle, newEncoding );

        ce->Encoding = Msg->GetEncoding();
        ce->Subject  = Msg->GetSubject();

        needUpdate = true;
      }

      panelInfo.SelectedItems[ i ].Flags &= ~PPIF_SELECTED;
    }

    if ( needUpdate )
    {
      PanelRedrawInfo redrawInfo;
      redrawInfo.CurrentItem = panelInfo.CurrentItem;
      redrawInfo.TopPanelItem = panelInfo.TopPanelItem;

      Ctrl.UpdatePanel( true );
      Ctrl.RedrawPanel( &redrawInfo );
    }

    return true;
  }*/

  INPUT_RECORD rec;
  memset( &rec, 0, sizeof( INPUT_RECORD ) );
  rec.EventType = KEY_EVENT;
  rec.Event.KeyEvent.wRepeatCount      = 1;
  rec.Event.KeyEvent.wVirtualKeyCode   = Key;
  rec.Event.KeyEvent.dwControlKeyState = ControlState;
  rec.Event.KeyEvent.bKeyDown          = TRUE;
  //rec.Event.KeyEvent.wVirtualScanCode  = MapVirtualKey( Key, 0 );
  //rec.Event.KeyEvent.uChar.AsciiChar   = MapVirtualKey( Key, 2 );
  int FarKey = FarSF::InputRecordToKey( &rec );

  //far_trace( "Key=0x%08X (%d)\n", Key, Key );

  /*UINT sc = MapVirtualKey( Key, 0 );
  WORD ch = 0;

  ToAsciiEx( Key, sc, NULL, &ch, 0, GetKeyboardLayout( GetCurrentThreadId() ) );

  //far_trace( "Key=%c: 0x%08X (%d), sc=%0x%08X (%d)\n", ch, ch, ch, sc, sc );
  far_trace( "Key=0x%08X (%d), sc=%0x%08X (%d)\n", (int)ch, (int)ch, sc, sc );
*/
  if ( FarSF::LIsAlphanum((unsigned)Key)/*(FarKey&0xFF) > KEY_SPACE && (FarKey&0xFF) <= '_'*/ && ( FarKey & ~(KEY_ALT|KEY_SHIFT|0xFF) ) == 0 )
  {
    FarSpeedSearch( this, SpeedSearchSelect, MDlgSearch ).Execute( Key );
    return true;
  }

  if ( ControlState != 0 )
    return false;

  if ( Key != VK_F3 && Key != VK_F4 /*&& Key != VK_F7*/ )
    return false;

  PanelInfo info = getInfo();

  long defaultEncoding = getCharacterTable(m_Config->GetDefaultCharset());

  if ( info.ItemsNumber > 0 && info.PanelItems[ info.CurrentItem ].UserData )
  {
    PluginPanelItem& CurItem = info.PanelItems[ info.CurrentItem ];

    TCacheEntry * ce = (TCacheEntry*)CurItem.UserData;

    PMessage Msg = CreateFarMessage();
    if ( Msg == NULL )
      return false;

    if (!Msg->read((long)&m_mailbox, ce->Handle, ce->Encoding, false))
    {
      delete Msg;
      return false;
    }

    if ( Key == VK_F3 )
    {
      DWORD tc = GetTickCount();

      long defaultEncoding = getCharacterTable(m_Config->GetDefaultCharset());

      bool bFullWnd = ViewCfg()->GetFullScreen();
      bool bWordWrap = ViewCfg()->GetWordWrap();
      while (CMailViewDialog(Msg, bFullWnd, defaultEncoding).Execute( &bWordWrap ) == NULL)
        bFullWnd = !bFullWnd;
      ViewCfg()->SetFullScreen( bFullWnd );
      ViewCfg()->SetWordWrap( bWordWrap );

      bool needUpdate = false;

      tc = GetTickCount() - tc;
      if ( tc > (DWORD)m_Config->GetMarkMsgAsReadTime() && (ce->Info.Flags & EMF_READED) == 0 )
      {
        ce->Info.Flags |= EMF_READED;
        m_mailbox.setMsgInfo( ce->Handle, &ce->Info );

        needUpdate = true;
      }

      if ( (long)ce->Encoding != Msg->GetEncoding() )
      {
        ce->Encoding = Msg->GetEncoding();
        ce->Subject  = Msg->GetSubject();
        KillRe( ce->Subject.GetBuffer() );
        ce->Subject.ReleaseBuffer();
        if ( ce->Subject.IsEmpty() )
          ce->Subject = m_noneSubj;
        needUpdate = true;
      }

      if ( needUpdate )
      {
        update();
        redraw();
      }

      delete Msg;

      return true;
    }
    else
    {
      if ( CMailViewTpl( ViewCfg()->GetETpl(), Msg, defaultEncoding ).View( true, ViewCfg()->GetLockView() ) )
      {
        delete Msg;
        return true;
      }
    }

    delete Msg;
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////
//
int CFarMailbox::Compare( const PluginPanelItem * Item1, const PluginPanelItem * Item2, UINT Mode )
{
  /*PanelInfo info = getShortInfo();
  if ( info.Flags & PFLAGS_REVERSESORTORDER )
  {
    if ( Item1->CRC32 > Item2->CRC32 )
      return -1;
    if ( Item1->CRC32 < Item2->CRC32 )
      return 1;
  }
  else*/
  {
    if ( Item1->CRC32 > Item2->CRC32 )
      return 1;
    if ( Item1->CRC32 < Item2->CRC32 )
      return -1;
  }
  return 0; //-2

  //return Item1->CRC32 < Item2->CRC32 ? 1 : Item1->CRC32 > Item2->CRC32 ? -1 : 0;
}

/////////////////////////////////////////////////////////////////////////////
//
bool CFarMailbox::SetDirectory( LPCSTR Dir, const int OpMode )
{
  far_assert( Dir /*&& *Dir*/ );

  if ( *Dir == '\0' )
    return true;

  CThread tmpThread( Dir, NULL );

  CThread * cr;

  if ( Dir[ 0 ] == '.' && Dir[ 1 ] == '.' && m_CurRef )
  {
    cr = m_CurRef->GetParent();
  }
  else if ( *Dir == '\\' )
  {
    cr = m_Cache.Ref;
  }
  else if ( m_CurRef )
  {
    cr = m_RefMap->Find( &tmpThread );
  }
  else
  {
    cr = NULL;
  }

  if ( cr )
  {
    m_CurRef = cr;
    return true;
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////
//
void CFarMailbox::FreeFindData( FarPluginPanelItems *Items )
{
  for ( int i = 0; i < Items->Count(); i ++ )
  {
    if ( (*Items)[ i ].CustomColumnData )
    {
      if ( (*Items)[ i ].CustomColumnData[ 2 ] )
        delete [] (*Items)[ i ].CustomColumnData[ 2 ]; // flags
      delete [] (*Items)[ i ].CustomColumnData;
    }
  }

  m_dirList.Clear();
}

/////////////////////////////////////////////////////////////////////////////
//
void ProcessSubject( LPSTR s )
{
  LPSTR p = s;
  while ( *p != '\0' && *p == '\x20' ) p ++;

  memmove( s, p, strlen( p ) + 1 );

  if ( *s == '[' )
  {
    p = s + 1;
    while ( *p != '\0' && *p != ']' ) p ++;
    if ( *p == ']' )
    {
      p ++;
      while ( *p != '\0' && *p == '\x20' ) p ++;
      s = p;
    }
  }
  else
    p = s;

  int n = 0;
  if ( FarSF::LStrnicmp( p, "Fwd", 3 ) == 0 )
    n = 3;
  else if ( FarSF::LStrnicmp( p, "Re", 2 ) == 0 || FarSF::LStrnicmp( p, "Fw", 2 ) == 0 ||
    FarSF::LStrnicmp( p, "na", 2 ) == 0 || FarSF::LStrnicmp( p, "ha", 2 ) == 0 ||
    FarSF::LStrnicmp( p, "\x8d\xa0", 2 ) == 0 )
    n = 2;
  else if ( FarSF::LStrnicmp( p, "\x8e\xe2\xa2\xa5\xe2", 5 ) == 0 )
    n = 5;

  if ( n )
  {
    p = p + n;

    if ( *p == '[' )
    {
      p ++;
      while ( *p != '\0' && ( isdigit( *p ) || *p == '\x20' ) ) p ++;
      if ( *p == ']' )
        p ++;
    }
    else if ( *p == '^' )
    {
      p ++;
      while ( *p != '\0' && ( isdigit( *p ) || *p == '\x20' ) ) p ++;
    }

    if ( *p == ':' )
    {
      p++;
      memmove( s, p, strlen( p ) + 1 );

      ProcessSubject( s );
    }
  }
}
//////////////////////////////////////////////////////////////////////////
#define CMailbox_CacheSignature "mbc!"
#define CMailbox_CacheSignatureSize (sizeof(CMailbox_CacheSignature)-1)
#define CMailbox_CacheVersion       MAKELONG( MAKEWORD( 1, 6 ), 0 )
void CFarMailbox::LoadCache()
{
  if ( m_Cache.Ref )
    delete m_Cache.Ref;

  m_Cache.Ref = NULL; //create CThread; // base holder
  m_CurRef    = m_Cache.Ref;

  m_Cache.sortMode  = -1;
  m_Cache.sortOrder = -1;
  m_Cache.theradsViewMode = -1;

  m_Cache.bInterrupted = 1;

  HANDLE hFile = CreateFile( m_HostFileName, 0, FILE_SHARE_READ,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if ( hFile != INVALID_HANDLE_VALUE )
  {
    GetFileTime( hFile, NULL, NULL, &m_Cache.FileAge );
    m_Cache.FileSize.LowPart = GetFileSize( hFile, &m_Cache.FileSize.HighPart );
    CloseHandle( hFile );
  }
  else
  {
    *(PINT64)&m_Cache.FileAge = 0;
    m_Cache.FileSize.QuadPart = 0;
  }

  // compare
  FarFile CacheFile;
  if ( !CacheFile.OpenForRead( getCacheFileName() ) )
    return;

  char Signature[ CMailbox_CacheSignatureSize ];
  if ( CacheFile.Read( Signature, CMailbox_CacheSignatureSize ) != CMailbox_CacheSignatureSize ||
    memcmp( Signature, CMailbox_CacheSignature, CMailbox_CacheSignatureSize ) != 0 )
    return;

  if ( CacheFile.ReadDword( 0 ) != CMailbox_CacheVersion )
    return;

  if ( CacheFile.ReadInt64( 0ll ) != *(PINT64)&m_Cache.FileAge )
    return;

  if ( CacheFile.ReadInt64( 0ll ) != (INT64)m_Cache.FileSize.QuadPart )
    return;

  CacheFile.Seek( 4 * sizeof( DWORD ), FILE_CURRENT );

  int Count = CacheFile.ReadDword( 0 );

  for ( int i = 0; i < Count; i ++ )
  {
    TCacheEntry * ce = create TCacheEntry;
    ce->Read( CacheFile );
    if ( ce->Handle != BAD_MSG_ID )
      m_Cache.Items.Add( ce );
  }

  m_Cache.bInterrupted = 0;
}

void CFarMailbox::SaveCache()
{
  if ( m_Cache.Ref == NULL ) // уже записано
    return;

  delete m_Cache.Ref;
  m_Cache.Ref = NULL;
  m_CurRef    = NULL;

  bool bUpdateRealAge = false;

  FarFileName CacheFileName = getCacheFileName();

  DeleteFile( CacheFileName );

  if ( m_Config->GetMinCacheSize() == 0 )
    return;

  if ( m_Cache.Items.Count() < m_Config->GetMinCacheSize() )
    return;

  if ( m_Cache.bInterrupted )
    return;

  FarFile File;
  File.CreateForWrite( CacheFileName );

  File.Write( CMailbox_CacheSignature, CMailbox_CacheSignatureSize );
  File.WriteDword( CMailbox_CacheVersion );
  File.WriteInt64( *(PINT64)&m_Cache.FileAge );
  File.WriteInt64( m_Cache.FileSize.QuadPart );
  File.WriteDword( 0 ); // reserved1
  File.WriteDword( 0 ); // reserved2
  File.WriteDword( 0 ); // reserved3
  File.WriteDword( 0 ); // reserved4

  // size of array
  File.WriteDword( m_Cache.Items.Count() );
  for ( int i = 0; i < m_Cache.Items.Count(); i ++ )
    m_Cache.Items[ i ]->Write( File );

  if ( bUpdateRealAge )
    File.SetTime( NULL, NULL, &m_Cache.FileAge );
}

void CFarMailbox::MakeRefs()
{
  int (__cdecl*sortFunc)(const TCacheEntry **, const TCacheEntry **, void *);

  switch ( m_Config->GetStartSortMode() )
  {
  case CMailboxCfg::smSubject:
    sortFunc = CFarMailbox::sortBySubject;
    break;
  case CMailboxCfg::smFrom:
    sortFunc = CFarMailbox::sortByFrom;
    break;
  case CMailboxCfg::smTo:
    sortFunc = CFarMailbox::sortByTo;
    break;
  case CMailboxCfg::smSize:
    sortFunc = CFarMailbox::sortBySize;
    break;
  case CMailboxCfg::smNone:
    sortFunc = CFarMailbox::sortByHandle;
    break;
  case CMailboxCfg::smSent:
    sortFunc = CFarMailbox::sortBySent;
    break;
  case CMailboxCfg::smReceived:
    sortFunc = CFarMailbox::sortByReceived;
    break;
  case CMailboxCfg::smAccessed:
    sortFunc = CFarMailbox::sortByAccessed;
    break;
  default:
    sortFunc = CFarMailbox::sortBySent;
    break;
  }

  if ( sortFunc )
    m_Cache.Items.Sort( sortFunc, (void*)m_Config->GetStartSortOrder() );

  if ( m_RefMap )
    delete m_RefMap;
  if ( m_Cache.Ref )
    delete m_Cache.Ref;

  m_RefMap = create CThreads( m_Cache.Items.Count() * 2 );

  m_Cache.Ref = create CThread( STR_EmptyStr, NULL );

  m_CurRef = m_Cache.Ref;

  TTheradsViewMode tvMode = m_Config->GetTheradsViewMode();

  if ( tvMode == tvmReferences_Std || tvMode == tvmReferences_Ext )
  {
    for ( int i = 0; i < m_Cache.Items.Count(); i ++ )
    {
      TCacheEntry * ce = m_Cache.Items[ i ];
      CThread * ref = create CThread( ce->MessageID, ce, m_Cache.Ref );
      if ( !m_RefMap->Insert( ref ) )
      {
        ce->MessageID += ".dup$";
        ce->MessageID += IntToString( (int)ref, 16 );
        ref->SetKey( ce->MessageID );
        if ( !m_RefMap->Insert( ref ) ) // impossible
          DbgMsg( "fatal error", "duplicate message id: %s", ref->GetKey() );
      }
      // память чиститься в SaveCache при удалении m_Cache.Ref
    }

    CThread tmpThread( STR_EmptyStr, NULL );

    for ( int i = 0; i < m_Cache.Items.Count(); i ++ )
    {
      TCacheEntry * ce = m_Cache.Items[ i ];

      for ( int j = 0; j < ce->ParentIDs.Count(); j ++ )
      {
        tmpThread.SetKey( ce->ParentIDs[ j ] );
        CThread * cr = m_RefMap->Find( &tmpThread );
        if ( cr )
        {
          tmpThread.SetKey( ce->MessageID );
          cr->SetChild( m_RefMap->Find( &tmpThread ) );
          break;
        }
      }
    }

    // extended threading view
    if ( tvMode == tvmReferences_Ext )
    {
      FarStringHash subjHash( m_Cache.Items.Count() * 2 );
      CThread * cr = m_CurRef->GetChild();
      while ( cr )
      {
        CThread * nr = cr->GetNext();

        CThread * fr = (CThread*)subjHash.Find( cr->GetData()->Subject );
        if ( fr == NULL )
          subjHash.Insert( cr->GetData()->Subject, (DWORD)cr );
        else
        {
          if ( CompareFileTime( &fr->GetData()->Info.Sent, &cr->GetData()->Info.Sent ) < 0 )
            cr->SetChild( fr );
          else
            fr->SetChild( cr );
        }

        cr = nr;
      }
    }

    return;
  }


  for ( int i = 0; i < m_Cache.Items.Count(); i ++ )
  {
    TCacheEntry * ce = m_Cache.Items[ i ];

    CThread * cr;

    switch ( m_Config->GetTheradsViewMode() )
    {
    case tvmSubject:
      cr = create CThread( ce->Subject, ce, m_Cache.Ref );
      break;

    case tvmFrom:
      cr = create CThread( ce->From, ce, m_Cache.Ref );
      break;

    case tvmTo:
      cr = create CThread( ce->To, ce, m_Cache.Ref );
      break;

    default:
      cr = create CThread( ce->MessageID, ce, m_Cache.Ref );;
    }

    far_assert( cr );

    CThread * fr = m_RefMap->Find( cr );
    if ( fr == NULL )
      m_RefMap->Insert( cr );
    else
      fr->SetChild( cr );
  }
}

void CFarMailbox::SetItem( PluginPanelItem * item, TCacheEntry * ce, const FarString& graph )
{
  ce->DiplayName = graph + ce->Subject;

  SetItem( item, ce );
}

void CFarMailbox::SetItem( PluginPanelItem * item, TCacheEntry * ce )
{
  strcpy( item->FindData.cFileName, ce->MessageID.c_str() );
  strcat( item->FindData.cFileName, m_msgExt.c_str() );

  CorrectFileName( item->FindData.cFileName );

  item->UserData                  = (DWORD)ce;
  item->FindData.nFileSizeLow     = ce->Size;
  item->PackSize                  = ce->Size;
  item->FindData.ftCreationTime   = ce->Info.Sent;
  item->FindData.ftLastWriteTime  = ce->Info.Received;
  item->FindData.ftLastAccessTime = ce->Info.Accessed;

  if ( ce->Info.Flags & EMF_READED )
    item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_ARCHIVE;

  //if ( ce->Info.Flags & EMF_FLAGGED )
  //  item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_SYSTEM;

  if ( ce->Info.Flags & EMF_DELETED )
    item->FindData.dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;

  item->Description = (LPSTR)ce->DiplayName.data();

  // From
  item->Owner = (LPSTR)ce->From.data();

  int ColCount = 3;
  item->CustomColumnData   = create LPSTR[ ColCount ];
  item->CustomColumnNumber = 3;

  // To
  item->CustomColumnData[ 0 ] = (LPSTR)ce->To.c_str();

  // Priority
  switch ( ce->Info.Priority )
  {
    case EMP_HIGH:
      item->CustomColumnData[ 1 ] = (LPSTR)Far::GetMsg( MPriorityHigh );
      break;
    case EMP_LOW:
      item->CustomColumnData[ 1 ] = (LPSTR)Far::GetMsg( MPriorityLow );
      break;
    default:
      item->CustomColumnData[ 1 ] = (LPSTR)Far::GetMsg( MPriorityNormal );
      break;
  }

  // Flags
  item->CustomColumnData[ 2 ] = create char[ 5 ];
  memset( item->CustomColumnData[ 2 ], '\x20', 4 );
  item->CustomColumnData[ 2 ][ 4 ] = '\0';
  if ( ce->Info.Flags & EMF__ATTACH )
    item->CustomColumnData[ 2 ][ 0 ] = '@';
  if ( ce->Info.Flags & EMF_FLAGGED )
    item->CustomColumnData[ 2 ][ 1 ] = '!';
  if ( ce->Info.Flags & EMF_REPLIED )
    item->CustomColumnData[ 2 ][ 2 ] = 'R';
  if ( ce->Info.Flags & EMF_FORWDED )
    item->CustomColumnData[ 2 ][ 3 ] = 'F';
}

/*PluginPanelItem * CFarMailbox::InsertItems( CThread * Ref, PluginPanelItem * CurItem )
{
  for ( CThread * cr = Ref->GetChild(); cr; cr = cr->GetNext() )
  {
    ZeroMemory( CurItem, sizeof ( PluginPanelItem ) );

    TCacheEntry * ce = cr->GetData();

    if ( ce->Info.Flags & EMF_READED )
      FarSF::snprintf( CurItem->FindData.cFileName, MAX_PATH, "%s.o.eml", cr->GetKey() );
    else
      FarSF::snprintf( CurItem->FindData.cFileName, MAX_PATH, "%s.n.eml", cr->GetKey() );

    SetItem( CurItem, ce, STR_EmptyStr );

    CurItem ++;
  }

  return CurItem;
}
*/

PluginPanelItem * CFarMailbox::InsertTreeItems( CThread * Ref, PluginPanelItem * CurItem, int level )
{
  far_assert( Ref != NULL );

  const char TreeViewMode1[4][4] = { "", "\x20", "\xC2", "\xC2\x20" };
  const char TreeViewMode2[4][4] = { "", "\x20", "\xC4", "\xC4\x20" };

  LPCSTR TreeViewMode3[2] = { "\xB3", "\x20" };
  LPCSTR TreeViewMode4[2] = { "\xC3", "\xC0" };

  LPCSTR TreeViewMode5[2] = { "\x20", "\x20" };
  LPCSTR TreeViewMode6[2] = { "", "" };

  int bDrawRoot;

  LPCSTR * TreeViewModeL;
  LPCSTR * TreeViewModeE;
  if ( m_Config->GetTreeViewMode() & TVM_DRAW_TREE )
  {
    TreeViewModeL = TreeViewMode3;
    TreeViewModeE = TreeViewMode4;
    bDrawRoot = m_Config->GetTreeViewMode() & TVM_DRAW_ROOT;
  }
  else
  {
    if ( m_Config->GetTreeViewMode() & TVM_INSERT_SP )
      TreeViewModeL = TreeViewModeE = TreeViewMode5;
    else
      TreeViewModeL = TreeViewModeE = TreeViewMode6;
    bDrawRoot = 0;
  }

  int bAddSp = m_Config->GetTreeViewMode() & TVM_INSERT_SP &&
    m_Config->GetTreeViewMode() & TVM_DRAW_TREE &&
    ( m_Config->GetTreeViewMode() & TVM_WIDE_TREE ) == 0;

  FarString graph;
  for ( CThread * cr = Ref->GetChild(); cr; cr = cr->GetNext() )
  {
    memset( CurItem, 0, sizeof ( PluginPanelItem ) );

    TCacheEntry * ce = cr->GetData();

    graph.Empty();

    if ( bDrawRoot || level != 0 )
    {
      CThread * pr = cr->GetParent();
      while ( pr && pr->GetData() != NULL )
      {
        if ( bDrawRoot == 0 && pr->GetParent()->GetData() == NULL )
          break;

        if ( bAddSp )
          graph.Insert( 0, *TreeViewMode5, 1 );

        graph.Insert( 0, TreeViewModeL[ pr->GetNext() ? 0 : 1 ], 1 );

        pr = pr->GetParent();
      }

      graph += TreeViewModeE[ cr->GetNext() ? 0 : 1 ];
    }

    if ( cr->GetChild() != NULL )
    {
      if ( level != 0 || bDrawRoot )
        graph += TreeViewMode1[ m_Config->GetTreeViewMode2() ];
      SetItem( CurItem, ce, graph );
      CurItem = InsertTreeItems( cr, CurItem + 1, TRUE );
    }
    else
    {
      if ( level != 0 || bDrawRoot )
        graph += TreeViewMode2[ m_Config->GetTreeViewMode2() ];
      SetItem( CurItem, ce, graph );
      CurItem ++;
    }
  }

  return CurItem;
}

void CFarMailbox::needMore()
{
  DbgMsg( Far::GetMsg( MMailView )  + FarString( " - ArcCmd" ),
    "you need more?\nplease, let me know about." );
}

bool CFarMailbox::ProcessHostFile( const FarPluginPanelItems *Items, const int OpMode )
{
  // TODO: локализовать

  FarMenu menu( "Mailbox commands" );
  menu.SetHelpTopic( "MailboxCommands" );

  menu.AddItem( "Purge" )->SetData( (DWORD)call_purge );
  menu.AddItem( "need more..." )->SetData( (DWORD)call_needMore );

  FarMenu::PItem item = menu.Show();

  if ( item )
  {
    DWORD data = item->GetData();
    if ( data )
      ((void(*)(CFarMailbox*))data)( this );
  }

  return false;
}

LPCSTR strstri( LPCSTR str1, LPCSTR str2 )
{
  if ( *str2 == '\0' )
    return str1;

  int len1 = strlen( str1 );
  int len2 = strlen( str2 );

  if ( len1 < len2 )
    return NULL;

  LPCSTR eptr = str1 + len1 - len2;

  for ( ; str1 <= eptr; str1 ++ )
  {
    if ( FarSF::LStrnicmp( str1, str2, len2 ) == 0 )
      return str1;
  }

  return NULL;
}

int CFarMailbox::SpeedSearchSelect( LPCSTR mask, PluginPanelItem * items, int itemsCount, int start )
{
  far_assert( items != NULL );

  for ( int i = start; i < itemsCount; i ++ )
  {
    TCacheEntry * ce = (TCacheEntry*)items[ i ].UserData;
    if ( ce )
    {
      if ( strstri( ce->Subject, mask ) )
        return i;
    }
  }

  for ( int i = 1; i < start; i ++ )
  {
    TCacheEntry * ce = (TCacheEntry*)items[ i ].UserData;
    if ( ce )
    {
      if ( strstri( ce->Subject, mask ) )
        return i;
    }
  }

  return -1;
}

bool CFarMailbox::DeleteFiles( FarPluginPanelItems  *items, int opMode )
{
  if ( !deleteFiles( items, opMode, true ) )
    return false;

  update( true );

  return true;
}

bool CFarMailbox::PutFiles( const FarPluginPanelItems *items, bool bMove, int opMode )
{
/*char buf[ MAX_PATH ];
GetCurrentDirectory( sizeof( buf ), buf );
  DbgMsg( buf );*/

  if ( !m_mailbox.checkFeature( CMailbox::featureAddMsg ) )
  {
    DbgMsg( Far::GetMsg( MMailView ), "This mailbox not implement this feature" );
    return false;
  }

  return false;
}

bool CFarMailbox::undeleteFiles( FarPluginPanelItems *items, int opMode )
{
  int updated = 0;

  bool bErrMsg = false;

  for ( int i = 0; i < items->Count(); i ++ )
  {
    TCacheEntry * ce = (TCacheEntry*)(*items)[ i ].UserData;
    if ( ce->Info.Flags & EMF_DELETED )
    {
      ce->Info.Flags &= ~EMF_DELETED;
      if ( !m_mailbox.setMsgInfo( ce->Handle, &ce->Info ) && !bErrMsg )
      {
        DbgMsg( "Undelete", "Information is not really updated." );
        bErrMsg = true;
      }
      updated ++;

      (*items)[ i ].Flags &= ~PPIF_SELECTED;
    }
  }

  return updated > 0;
}

bool CFarMailbox::deleteFiles( FarPluginPanelItems *items, int opMode, bool markOnly )
{
  if ( !markOnly && !m_mailbox.checkFeature( CMailbox::featureDelMsg ) )
  {
    DbgMsg( Far::GetMsg( MMailView ), "%s mailbox not implement this feature", m_mailbox.getShortName() );

    return true;
  }

  int updated = 0;

  bool bErrMsg = false;

  for ( int i = 0; i < items->Count(); i ++ )
  {
    TCacheEntry * ce = (TCacheEntry*)(*items)[ i ].UserData;
    if ( markOnly )
    {
      if ( !(ce->Info.Flags & EMF_DELETED) )
      {
        ce->Info.Flags |= EMF_DELETED;
        if ( !m_mailbox.setMsgInfo( ce->Handle, &ce->Info ) && !bErrMsg )
        {
          DbgMsg( "Delete", "Information is not really updated." );
          bErrMsg = true;
        }
        updated ++;

        (*items)[ i ].Flags &= ~PPIF_SELECTED;
      }
    }
    else
    {
      if ( !m_mailbox.delMsg( ce->Handle ) )
      {
        // TODO: локализовать
        FarMessage msg( FMSG_WARNING|FMSG_LEFTALIGN );
        msg.AddLine( "Delete" );
        msg.AddFmt( "Can'n delete message \"%s\"", ce->MessageID.c_str() );
        msg.AddSeparator();
        msg.AddFmt( "%s: \"%s\"", Far::GetMsg( MFrom ), ce->From.c_str() );
        msg.AddFmt( "%s: \"%s\"", Far::GetMsg( MTo ), ce->To.c_str() );
        msg.AddFmt( "%s: \"%s\"", Far::GetMsg( MSubject ), ce->Subject.c_str() );
        msg.AddSeparator();
        msg.AddButton( MOk );
        msg.Show();
      }
      else
      {
        (*items)[ i ].Flags &= ~PPIF_SELECTED;

        m_Cache.remove( ce );

        updated ++;
      }
    }
  }

  return updated > 0;
}

bool CFarMailbox::setAttributes()
{
  PMessage Msg = CreateFarMessage();
  if ( Msg == NULL )
    return true;

  PanelInfo info = getInfo();

  if ( info.SelectedItemsNumber < 1 )
    return true;


  // TODO: сделать update контролов

  FarDialog dlg( MAttributes );

  FarCheckCtrl chkReaded ( &dlg, MAttributes_Readed, 0 );
  FarCheckCtrl chkFlagged( &dlg, MAttributes_Flagged, 0 );
  FarCheckCtrl chkReplied( &dlg, MAttributes_Replied, 0 );
  FarCheckCtrl chkForwded( &dlg, MAttributes_Forwarded, 0 );

  chkReaded.SetState( FarCheckCtrl::Undefined );
  chkFlagged.SetState( FarCheckCtrl::Undefined );
  chkReplied.SetState( FarCheckCtrl::Undefined );
  chkForwded.SetState( FarCheckCtrl::Undefined );

  if ( info.SelectedItemsNumber > 1 )
  {
    chkReaded.SetFlags( DIF_3STATE );
    chkFlagged.SetFlags( DIF_3STATE );
    chkReplied.SetFlags( DIF_3STATE );
    chkForwded.SetFlags( DIF_3STATE );
  }

  dlg.AddSeparator( MAttributes_Priority );

  FarRadioGroup grpPriority( &dlg, -1 );
  int radX = -1;
  FarRadioCtrl * radNoraml = grpPriority.AddItem( MAttributes_Priority_Normal, radX );
  //radX += radNoraml->GetTitle().Length() + 4;
  FarRadioCtrl * radHigh   = grpPriority.AddItem( MAttributes_Priority_High, radX );
  //radX += radHigh->GetTitle().Length() + 4;
  FarRadioCtrl * radLow    = grpPriority.AddItem( MAttributes_Priority_Low, radX );

  long encoding = FCT__INVALID;

  DWORD t_f=0, t_p=0, t_e=0;
  for ( int i = 0; i < info.SelectedItemsNumber; i ++ )
  {
    TCacheEntry * ce = (TCacheEntry*)info.SelectedItems[ i ].UserData;
    if ( i == 0 )
    {
      chkReaded.SetState( ce->Info.Flags & EMF_READED ?
        FarCheckCtrl::Checked : FarCheckCtrl::Unchecked );
      chkFlagged.SetState( ce->Info.Flags & EMF_FLAGGED ?
        FarCheckCtrl::Checked : FarCheckCtrl::Unchecked );
      chkReplied.SetState( ce->Info.Flags & EMF_REPLIED ?
        FarCheckCtrl::Checked : FarCheckCtrl::Unchecked );
      chkForwded.SetState( ce->Info.Flags & EMF_FORWDED ?
        FarCheckCtrl::Checked : FarCheckCtrl::Unchecked );

      switch ( ce->Info.Priority )
      {
      case EMP_LOW:
        radNoraml->SetSelected( false );
        radHigh->SetSelected( false );
        radLow->SetSelected( true );
        break;
      case EMP_HIGH:
        radNoraml->SetSelected( false );
        radHigh->SetSelected( true );
        radLow->SetSelected( false );
        break;
      default:
        radNoraml->SetSelected( true );
        radHigh->SetSelected( false );
        radLow->SetSelected( false );
        break;
      }
      encoding = ce->Encoding;

      t_f = ce->Info.Flags;
      t_p = ce->Info.Priority;
      t_e = ce->Encoding;
    }
    else
    {
      if ( (t_f & EMF_READED) != (ce->Info.Flags & EMF_READED) )
      {
        chkReaded.SetState( FarCheckCtrl::Undefined );
      }

      if ( (t_f & EMF_FLAGGED) != (ce->Info.Flags & EMF_FLAGGED) )
      {
        chkFlagged.SetState( FarCheckCtrl::Undefined );
      }

      if ( (t_f & EMF_REPLIED) != (ce->Info.Flags & EMF_REPLIED) )
      {
        chkReplied.SetState( FarCheckCtrl::Undefined );
      }

      if ( (t_f & EMF_FORWDED) != (ce->Info.Flags & EMF_FORWDED) )
      {
        chkForwded.SetState( FarCheckCtrl::Undefined );
      }

      if ( t_p != ce->Info.Priority )
      {
        radNoraml->SetSelected( false );
        radHigh->SetSelected( false );
        radLow->SetSelected( false );
      }

      if ( t_e != ce->Encoding )
      {
        encoding = FCT__INVALID;
      }

    }
  }

  dlg.AddSeparator()/*->SetStyle( FarSeparatorCtrl::Double )*/;


  FarTextCtrl textEncoding( &dlg, MAttributes_Encoding );

  FarStringArray name;
  FarIntArray data;
  getFarCharacterTables( name, data, true );
  name.Add( STR_EmptyStr );
  data.Add( FCT__INVALID );
  getRfcCharacterTables( name, data, false );

  int maxlen = 30;

  for ( int i = 0; i < name.Count(); i ++ )
    maxlen = max( maxlen, (int)strlen( name.At( i ) ) );

  FarComboBox cbCharset( &dlg, 0, maxlen + 6 );
  cbCharset.SetFlags( DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND );
  cbCharset.SetNextY();

  for ( int i = 0; i < name.Count(); i ++ )
  {
    long enc = data.At( i );
    DWORD flags = 0;
    if ( enc == FCT__INVALID )
    {
      flags = LIF_SEPARATOR;
    }
    else if ( enc == encoding )
    {
      flags = LIF_SELECTED;
    }

    cbCharset.AddItem( name.At( i ), flags );
  }


  dlg.AddSeparator();

  FarButtonCtrl btnSet( &dlg, MColorDlg_Set, DIF_CENTERGROUP );
  FarButtonCtrl btnCancel( &dlg, MCancel, DIF_CENTERGROUP );

  dlg.SetDefaultControl( &btnSet );

  if ( dlg.Show() == &btnSet )
  {
    encoding = cbCharset.GetSelected();
    if ( encoding != -1 )
    {
      encoding = data.At( encoding );
    }

    for ( int i = 0; i < info.SelectedItemsNumber; i ++ )
    {
      TCacheEntry * ce = (TCacheEntry*)info.SelectedItems[ i ].UserData;
      if ( ce == NULL ) // не реально ??
        continue;

      if ( encoding != FCT__INVALID && (long)ce->Encoding != encoding )
      {
        Msg->read( (long)&m_mailbox, ce->Handle, encoding, false );

        ce->Encoding = Msg->GetEncoding();
        ce->Subject  = Msg->GetSubject();
        KillRe( ce->Subject.GetBuffer() );
        ce->Subject.ReleaseBuffer();
        if ( ce->Subject.IsEmpty() )
          ce->Subject = m_noneSubj;
      }

      FarCheckCtrl::State state;

      state = chkReaded.GetState();
      if ( state != FarCheckCtrl::Undefined )
      {
        if ( state == FarCheckCtrl::Checked )
          ce->Info.Flags |= EMF_READED;
        else
          ce->Info.Flags &= ~EMF_READED;
      }

      state = chkFlagged.GetState();
      if ( state != FarCheckCtrl::Undefined )
      {
        if ( state == FarCheckCtrl::Checked )
          ce->Info.Flags |= EMF_FLAGGED;
        else
          ce->Info.Flags &= ~EMF_FLAGGED;
      }

      state = chkReplied.GetState();
      if ( state != FarCheckCtrl::Undefined )
      {
        if ( state == FarCheckCtrl::Checked )
          ce->Info.Flags |= EMF_REPLIED;
        else
          ce->Info.Flags &= ~EMF_REPLIED;
      }


      state = chkForwded.GetState();
      if ( state != FarCheckCtrl::Undefined )
        {
        if ( state == FarCheckCtrl::Checked )
          ce->Info.Flags |= EMF_FORWDED;
        else
          ce->Info.Flags &= ~EMF_FORWDED;

      }

      if ( grpPriority.GetSelectedIndex() != -1 )
      {
        if ( radNoraml->GetSelected() )
          ce->Info.Priority = EMP_NORMAL;
        else if ( radHigh->GetSelected() )
          ce->Info.Priority = EMP_HIGH;
        else if ( radLow->GetSelected() )
          ce->Info.Priority = EMP_LOW;
      }

      m_mailbox.setMsgInfo( ce->Handle, &ce->Info );

      info.SelectedItems[ i ].Flags &= ~PPIF_SELECTED;
    }

    update( true );
    redraw();
  }

  return true;
}

bool CFarMailbox::confirm( const char * title, const char * text )
{
  if ( (Far::AdvControl( ACTL_GETCONFIRMATIONS, 0 ) & FCS_INTERRUPTOPERATION) == 0 )
    return true;

  FarMessage msg( FMSG_WARNING|FMSG_LEFTALIGN );

  msg.AddLine( title );

  // TODO: локализовать

  if ( text )
    msg.AddLine( text );
  else
    msg.AddLine( "Interrupt operation?" );

  msg.AddButton( MYes );
  msg.AddButton( MNo );

  return msg.Show() == 0;
}

void CFarMailbox::purge()
{
  // TODO: локализовать

  DeleteFile( getCacheFileName() );

  MV_RESULT hr = m_mailbox.purge();
  if ( SUCCEEDED( hr ) )
  {
    m_Cache.bInterrupted = 1;

    update( true );

    return;
  }

  if ( hr != E_NOTIMPL )
  {
    DbgMsg( Far::GetMsg( MMailView ) + FarString( "Purge" ), "Failed." );
    return;
  }

  CSearchMessagesDlg sm( m_mailbox.getFileName() );
  static LPCSTR sm_title = "purge mailbox";
  sm.setTitle( sm_title );

  if ( m_mailbox.checkFeature( CMailbox::featureDelMsg ) )
  {
    DWORD msgId = BAD_MSG_ID, tc = GetTickCount();

    while ( (msgId = m_mailbox.getNextMsg( msgId )) != BAD_MSG_ID )
    {
      if ( GetTickCount() - tc > 500 && FarSF::CheckForEsc() )
      {
        if ( confirm( "Purge" ) )
          return;

        m_mailbox.delMsg( msgId );
      }
    }

    update( true );

    return;
  }

  FarFileName tempName = FarFileName::MakeTemp( "mv" );

  CMailbox newmbox( m_mailbox.getModule() );
  if ( !newmbox.open( tempName, true ) ||
    !newmbox.checkFeature( CMailbox::featureAddMsg ) )
    return;

  PMessage msg = CreateFarMessage();
  if ( msg == NULL )
    return;

  m_Cache.Items.Clear();

  long defaultEncoding = getCharacterTable( m_Config->GetDefaultCharset() );

  DWORD size, capacity = 4096, msgId = BAD_MSG_ID;
  PBYTE data = create BYTE[ capacity ];

  DWORD tc = GetTickCount();

  while ( (msgId = m_mailbox.getNextMsg( msgId )) != BAD_MSG_ID )
  {
    if ( GetTickCount() - tc > 500 && FarSF::CheckForEsc() )
    {
      if ( confirm( "Purge", "Abort operation?" ) )
      {
        delete [] data;
        return;
      }

      m_mailbox.delMsg( msgId );
    }

    if ( m_mailbox.getMsg( msgId, NULL, &size ) == MV_OK )
    {
      if ( size > capacity )
      {
        delete [] data;
        capacity = size;
        data = create BYTE[ capacity ];
      }

      if ( m_mailbox.getMsg( msgId, data, &size ) == MV_OK )
      {
        addCacheItem( &newmbox, newmbox.addMsg( data, size ), msg, defaultEncoding );
      }
    }
  }

  delete msg;

  delete [] data;

  m_Cache.bInterrupted = 1;

  m_mailbox.close();

  MoveFile( m_HostFileName, m_HostFileName + ".bak" );
  MoveFile( tempName, m_HostFileName );

  update( true );
  //Close( false );

  // это не совсем правильно, но зато быстро :)
  memcpy( &m_mailbox, &newmbox, sizeof( CMailbox ) );
  memset( &newmbox, 0, sizeof( CMailbox ) );
}
