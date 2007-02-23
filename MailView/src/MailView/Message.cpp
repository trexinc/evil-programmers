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
#include <stdlib.h>
#include <FarRegistry.h>
#include <FarDialog.h>

#include "Message.h"
#include "MailView.h"
#include "MultiLng.h"
#include "Decoder.h"
#include "StrPtr.h"
#include "MsgLib/MimeContent.h"
#include "FarCopyDlg.h"

extern bool CreateDirectory( LPSTR fullPath );
extern void CorrectFileName( LPSTR fileName, bool allowPathChar = false );

#define HISTORY_COPYFILES "MailView\\CopyFiles"

//////////////////////////////////////////////////////////////////////////
//
CMessage::CMessage( PMessage Msg, const FarFileName& HostFile )
  : FarCustomPanelPlugin( HostFile )
  , m_Msg( Msg )
  , m_Mailbox( NULL )
{
  far_assert( m_Msg != NULL );

  m_panelTitle.Format( MPanelTitle_Format, m_Msg->GetFmtName(),
    HostFile.GetName().c_str() );
}

CMessage::~CMessage()
{
  delete m_Msg;
}

//////////////////////////////////////////////////////////////////////////
//
bool CMessage::SaveToFile( CMailbox * pMailbox, DWORD dwMsgID, LPCSTR FileName )
{
  if ( FileName == NULL )
    return false;

  DWORD Size = 0;
  if ( pMailbox->getMsg( dwMsgID, NULL, &Size ) != MV_OK )
    return false;

  PBYTE Data = create BYTE[ Size + 1 ];

  if ( pMailbox->getMsg( dwMsgID, Data, &Size ) != MV_OK )
  {
    delete [] Data;
    return false;
  }

  FarFile File( FileName, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS );
  if ( File.GetLastError() != 0 )
  {
    delete [] Data;
    return false;
  }

  File.Write( Data, Size );

  delete [] Data;

  return true;
}

//////////////////////////////////////////////////////////////////////////
//
void CMessage::GetOpenInfo( OpenPluginInfo * pInfo )
{
  pInfo->Flags     |= OPIF_ADDDOTS|OPIF_SHOWPRESERVECASE|OPIF_USEHIGHLIGHTING;
  pInfo->Format     = m_Msg->GetFmtName();
  pInfo->PanelTitle = m_panelTitle.c_str();
  pInfo->HostFile   = m_HostFileName.c_str();
//  pInfo->StartPanelMode = m_Config->GetStartPanelMode() < 0 ? 0 : '0' + m_Config->GetStartPanelMode();
}

//////////////////////////////////////////////////////////////////////////
//
bool CMessage::GetFindData( FarPluginPanelItems *Items, int OpMode )
{
  PMsgPart text = m_Msg->GetTextPart();
  PMsgPart part = NULL;

  int nHTM = 0, nTXT = 0, nEML = 0, nUNK = 0;
  char cbuf[ 16 ];

  FarString fn;

  LPCSTR name;

  CMimeContent mc;

  fn = m_Msg->GetFrom()->GetMailboxName();
  // !! память выделяем только для одного элемента,
  // !! потому что нефиг, и удалять надо будет, например,
  // !! только первый элемент.
  LPSTR Owner = strcpy( create char[ fn.Length() + 1 ], fn );

  fn.Empty();

  while ( part = m_Msg->GetNextPart( part ) )
  {
    mc.assign(part->GetKludge( K_RFC_ContentType));

    if (*mc.getType() && (name = mc.getDataValue("name")) != NULL)
    {
      fn = name;
      m_Msg->DecodeKludge( fn.GetBuffer() ); fn.ReleaseBuffer();
    }

    if ( fn.IsEmpty() )
    {
      if (*mc.getType() && (name = mc.getDataValue("file")) != NULL)
      {
        fn = name;
        m_Msg->DecodeKludge( fn.GetBuffer() ); fn.ReleaseBuffer();
      }
    }

    if ( fn.IsEmpty() )
    {
      fn = part->GetKludge( K_RFC_ContentLocation );

      m_Msg->DecodeKludge( fn.GetBuffer() ); fn.ReleaseBuffer();

      fn = FarFileName( fn ).GetName();

      /*if ( !fn.IsEmpty() )
      {
        int pos = fn.IndexOf( '?' );
        if ( pos != -1 )
          fn.SetLength( pos );
      }*/
    }

    DWORD DecodedSize = part->GetContentSize();

    if ( fn.IsEmpty() )
    {
      if (part == text || FarSF::LStricmp(mc.getType(), "text") == 0 )
      {
        int * pn;
        fn = "message.";
        if (FarSF::LStricmp(mc.getSubType(), "html") == 0)
          fn += "htm", pn = &nHTM;
        else
          fn += "txt", pn = &nTXT;

        if ( *pn != 0 )
          FarSF::snprintf( cbuf, sizeof(cbuf), "%02X", *pn );
        else
          *cbuf = '\0';

        (*pn)++;

        fn.Insert( 7, cbuf );
      }
      else if ( DecodedSize > 0 )
      {
        fn = "unknown";
        int * pn;
        if (FarSF::LStricmp(mc.getType(), "message") == 0 && FarSF::LStricmp(mc.getSubType(), "rfc822") == 0)
          fn += ".eml", pn = &nEML;
        else
          fn += ".dat", pn = &nUNK;

        FarSF::snprintf( cbuf, sizeof(cbuf), "%02X", *pn );
        fn.Insert( 7, cbuf );

        (*pn)++;
      }
      else
        continue;
    }

    if ( DecodedSize == 0 )
    {
      FarFileName XContentFile = part->GetKludge( K_RFC_XContentFile );
      if ( !XContentFile.IsEmpty() )
      {
        WIN32_FIND_DATA fd;
        if ( FarFileInfo::GetInfo( XContentFile, fd ) )
          DecodedSize = fd.nFileSizeLow;
        /*else if ( m_Mailbox != NULL )
        {
          XContentFile = m_Mailbox->GetDefAttachDir();
          XContentFile.SetName( part->GetKludge( K_RFC_XContentFile ) );
          if ( FarFileInfo::GetInfo( XContentFile, fd ) )
            DecodedSize = fd.nFileSizeLow;
        }*/
      }
    }

    //if ( DecodedSize > 0 )
    {
      PluginPanelItem& CurItem = Items->Add();

      strcpy( CurItem.FindData.cFileName, fn );
      CorrectFileName( CurItem.FindData.cFileName );

      CurItem.UserData = (DWORD)part;
      CurItem.CRC32 = (DWORD)-1;

      CurItem.FindData.nFileSizeLow = DecodedSize;

      m_Msg->GetSent( &CurItem.FindData.ftCreationTime );
      m_Msg->GetReceived( &CurItem.FindData.ftLastWriteTime );
      m_Msg->GetAccessed( &CurItem.FindData.ftLastAccessTime );

      CurItem.Owner = Owner;
    }

    fn.Empty();
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////
//
void CMessage::FreeFindData( FarPluginPanelItems *Items )
{
  far_assert( Items->Count() > 0 );
  delete (*Items)[ 0 ].Owner; // удаляем только первый элемент!!!
}

//////////////////////////////////////////////////////////////////////////
//
int CMessage::GetFiles( FarPluginPanelItems *Items, bool bMove, LPSTR DestPath, const int OpMode )
{
  if ( Items->Count() == 0 )
    return 1;

//#if (FARMANAGERVERSION <= MAKEFARVERSION(1,70,1376))
  if ( Items->Count() == 1 && !(*Items)[ 0 ].UserData )
  {
    DbgMsg( Far::GetMsg( MMailView ), "this feature allowed only in newer version of Far Manager" );
    return 0;
  }
//#pragma message ("CMessage::GetFiles - using UserData stub...")
//#endif

  FarSF::AddEndSlash( DestPath );

  int fnSize = 2048;
  LPSTR fn = create char[ fnSize ];

  FarCopyDlg * CopyDlg = NULL;

  /*  NeedMessage();
  */
  if ( ( OpMode == 0 || OpMode == OPM_TOPLEVEL ) )
  {
    FarDialog dlg( MdlgCopy_Copy, ":CopyFiles" );
    if ( Items->Count() != 1 )
      FarSF::snprintf( fn, fnSize, Far::GetMsg( MdlgCopy_CopyItems ), Items->Count() );
    else
      FarSF::snprintf( fn, fnSize, Far::GetMsg( MdlgCopy_CopyItem ), (*Items)[ 0 ].FindData.cFileName );
    dlg.AddText( fn );
    FarEditCtrl edtTo( &dlg, DestPath, 0, 66, HISTORY_COPYFILES );
    edtTo.SetNextY();

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
    FarSF::AddEndSlash( fn );

    int nTotalSize = 0;
    for ( int i = 0; i < Items->Count(); i ++ )
      nTotalSize += (*Items)[ i ].FindData.nFileSizeLow;

    CopyDlg = create FarCopyDlg( nTotalSize );
  }

  strcpy( fn, DestPath );
  CreateDirectory( fn );

  LPSTR pn = fn + strlen( fn );
  for ( int i = 0; i < Items->Count(); i ++ )
  {
    strcpy( pn, (*Items)[ i ].FindData.cFileName );

    PMsgPart part = (PMsgPart)(*Items)[ i ].UserData;
    far_assert( part != NULL );

    FarCopyDlg::TOperation op;
    if ( CopyDlg )
      op = CopyDlg->Copy( (*Items)[ i ].FindData.cFileName, fn,
        (*Items)[ i ].FindData.nFileSizeLow,
        (*Items)[ i ].FindData.ftLastWriteTime );
    else
      op = FarCopyDlg::opOverwrite;

    if ( op == FarCopyDlg::opOverwrite )
    {
      (*Items)[ i ].Flags &= ~PPIF_SELECTED;


      LPCSTR p = part->GetKludge( K_RFC_XContentFile );
      if ( *p )
      {
        CopyFile( p, fn, FALSE );
      }
      else
      {
        /*HANDLE hFile = CreateFileA( fn, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, 0 );
        if ( hFile != INVALID_HANDLE_VALUE )
        {
          part->DecodeAndWriteToFile( hFile );
          CloseHandle( hFile );
        }*/
        part->SaveToFile( fn );
      }
    }
    else if ( op == FarCopyDlg::opCancel )
    {
      if ( CopyDlg )
        delete CopyDlg;
      delete [] fn;
      return -1;
    }
  }

  if ( CopyDlg )
    delete CopyDlg;

  delete [] fn;
  return 1;
}

bool IsDangerousFileName( LPCSTR FileName )
{
  LPCSTR Ext = strrchr( FileName, '.' );
  if ( Ext )
  {
    char PathExt[ 0x1000 ];
    if ( GetEnvironmentVariable( "PATHEXT", PathExt, sizeof( PathExt ) ) == 0 )
      strcpy( PathExt, ".exe;.com;.bat;.cmd" );
    strcat( PathExt, ";.lnk;.pif" );

    for ( LPSTR tok = strtok( PathExt, "\x20,;" ); tok; tok = strtok( NULL, "\x20,;" ) )
    {
      if ( FarSF::LStricmp( Ext, tok ) == 0 )
        return true;
    }
  }

  far_assert( CMailView::m_Handle != NULL );

  return FarSF::CmpNameListEx( CMailView::m_Handle->GetDangerFilesMasks(), FileName, true );
}

long WINAPI AllowExecuteFileDlgProc( Far::HDIALOG hDlg, int Msg, int Param1, long Param2 )
{
  if ( Msg == DN_GOTFOCUS )
  {
    FarDialogItem Item;
    hDlg->getItem( Param1, &Item );
    Item.DefaultButton = TRUE;
    hDlg->setItem( Param1, &Item );
    return 0;
  }

  if ( Msg == DN_KILLFOCUS )
  {
    FarDialogItem Item;
    hDlg->getItem( Param1, &Item );
    Item.DefaultButton = FALSE;
    hDlg->setItem( Param1, &Item );
    return -1;
  }

  return hDlg->defDlgProc( Msg, Param1, Param2 );
}

bool CMessage::VirusCheckOk( PluginPanelItem * Item )
{
  far_assert( CMailView::m_Handle != NULL );

  int SeccessErrorCode = CMailView::m_Handle->GetVirusCheckerSuccessCode();

  FarFileName FileName = FarFileName::MakeTemp( "mvv" );

  {
    FarPluginPanelItems items( Item, 1 );
    if ( GetFiles( &items, false, FileName.GetBuffer(), OPM_SILENT ) != 1 )
      return false;
  }

  FileName.ReleaseBuffer();

  FileName.AddEndSlash() += Item->FindData.cFileName;

  FarString VirusChecker( CMailView::m_Handle->GetVirusChecker() );
  VirusChecker.Format( VirusChecker, FileName.c_str() );

  FarScreen Screen;

  int ExitCode = FarSF::Execute( this, VirusChecker, false, false, true, -1, false );

  Screen.Restore();

  if ( ExitCode != SeccessErrorCode )
  {
    FarString Msg;
    Msg.Format( "%s\n%s\n%s", Far::GetMsg( MWarning ),
      Far::GetMsg( MFileInfected ), Far::GetMsg( MOk ) );
    Msg.Format( Msg.c_str(), ExitCode );
    FarMessage::Show( FMSG_WARNING|FMSG_ALLINONE, NULL, (LPCSTR const*)Msg.c_str(), 0, 1 );
  }

  DeleteFile( FileName );
  FileName = FileName.GetPath();
  RemoveDirectory( FileName );

  return ExitCode == SeccessErrorCode;
}

bool CMessage::AllowExecuteFile( PluginPanelItem * Item )
{
  far_assert( CMailView::m_Handle != NULL );

  if ( !IsDangerousFileName( Item->FindData.cFileName ) )
    return true;

  int Rule = CMailView::m_Handle->GetDangerFilesWarnRule();
  if ( Rule == 0 )
    return false;
  if ( Rule == 1 )
    return true;
  if ( Rule == 2 )
    return VirusCheckOk( Item );

  int DefaultButton;
  if ( Rule == 3 )
    DefaultButton = 0;
  else if ( Rule == 4 )
    DefaultButton = 1;
  else
    DefaultButton = 2;

  FarStringTokenizer tokenizer( Far::GetMsg( MConfirmFileOpen ), '\n', false );
  FarStringArray Lines;
  while ( tokenizer.HasNext() )
    Lines.Add( tokenizer.NextToken() );

  int ItemsCount = Lines.Count() + 4;
  FarDialogItem * Items = create FarDialogItem[ ItemsCount ];
  memset( Items, 0, sizeof( FarDialogItem ) * ItemsCount );

  int Width = 0, Heigth = Lines.Count() + 3;
  Width = max( Width, (int)strlen( Far::GetMsg( MOk ) ) + (int)strlen( Far::GetMsg( MCancel ) + 6 ) );
  for ( int i = 0; i < Lines.Count(); i ++ )
    Width = max( Width, (int)strlen( Lines[ i ] ) + 2 );

  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo( GetStdHandle(STD_OUTPUT_HANDLE), &csbi );
  if ( Width > csbi.dwMaximumWindowSize.X - 12 )
    Width = csbi.dwMaximumWindowSize.X - 12;
  Width = min( Width, 512 );

  int i = 0;
  Items[ i ].Type = DI_DOUBLEBOX;
  Items[ i ].X1 = 3;
  Items[ i ].Y1 = 1;
  Items[ i ].X2 = Width + 3 + 1;
  Items[ i ].Y2 = Heigth;
  strncpy( Items[ i ].Data, Far::GetMsg( MWarning ), Width );
  i ++;
  for ( int j = 0; j < Lines.Count(); j ++, i ++ )
  {
    Items[ i ].Type  = DI_TEXT;
    Items[ i ].X1    = -1;
    Items[ i ].Y1    = j + 2;
    strncpy( Items[ i ].Data, Lines[ j ], Width - 2 );
  }

  Items[ i + 0 ].Type  = Items[ i + 1 ].Type  = Items[ i + 2 ].Type  = DI_BUTTON;
  Items[ i + 0 ].Y1    = Items[ i + 1 ].Y1    = Items[ i + 2 ].Y1    = i + 1;
  Items[ i + 0 ].Flags = Items[ i + 1 ].Flags = Items[ i + 2 ].Flags = DIF_CENTERGROUP|DIF_NOBRACKETS;

  FarSF::snprintf( Items[ i + 0 ].Data, sizeof( Items[ i + 0 ].Data ), " %s ", Far::GetMsg( MOk ) );
  FarSF::snprintf( Items[ i + 1 ].Data, sizeof( Items[ i + 1 ].Data ), " %s ", Far::GetMsg( MCheckAndOpen ) );
  FarSF::snprintf( Items[ i + 2 ].Data, sizeof( Items[ i + 2 ].Data ), " %s ", Far::GetMsg( MCancel ) );

  FarString VirusChecker = CMailView::m_Handle->GetVirusChecker();
  if ( VirusChecker.IsEmpty() )
  {
    Items[ i + 1 ].Flags |= DIF_DISABLE;
    if ( DefaultButton == 1 )
      DefaultButton = 2;
  }

  Items[ i + DefaultButton ].Focus = TRUE;
  Items[ i + DefaultButton ].DefaultButton = TRUE;

  int Result = Far::Dialog( -1, -1, Width + 8, Heigth + 2, NULL,
    Items, ItemsCount, FDLG_WARNING, AllowExecuteFileDlgProc );
  if ( Result >=0 )
    Result += 3 - ItemsCount;
  delete [] Items;

  if ( Result == 1 )
    return VirusCheckOk( Item );

  return Result == 0;
}
//////////////////////////////////////////////////////////////////////////
//
bool CMessage::ProcessKey( int Key, UINT ControlState )
{
  if ( Key == VK_RETURN && ( ControlState == PKF_SHIFT || ControlState == 0 ) )
  {
    PanelInfo info = getInfo();
    if ( info.ItemsNumber > 0 && info.PanelItems[ info.CurrentItem ].UserData )
    {
      return !AllowExecuteFile( &info.PanelItems[ info.CurrentItem ] );
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////
//
bool CMessage::ProcessHostFile( const FarPluginPanelItems *Items, int OpMode )
{
  FarMessage().SimpleMsg( FMSG_WARNING|FMSG_MB_OK, MMailView,
    "what you need here?", "please let me know about.", -1 );
  return false;
}
