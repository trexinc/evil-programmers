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
// DlgTest.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdio.h>

#include "Decoder.h"
#include "MailViewDlg.h"
#include "MsgLib/MimeContent.h"
#include "Kludges.h"
#include "MailView.h"
#include "FarInetMessage.h"
//#include "FarFidoMessage.h"
#include "WordWrap.h"
#include "File.h"
#include "DateTime.h"

#include <FarPlus.h>
#include <FarMenu.h>
#include <ctype.h>

#define FileTimeToString(ft) ((FileTime)ft).toString(ViewCfg()->getDateFormat(), ViewCfg()->getTimeFormat(), LCID_DateTime)

extern PMessage CreateFarMessage( LPCSTR fileName, long encoding, bool headOnly );

#define MVColor( Index ) ColorCfg()->GetColor( Index )

void ShowConsoleCursor( CONST BOOL bVisible )
{
  CONSOLE_CURSOR_INFO Info;
  HANDLE hOutput = GetStdHandle( STD_OUTPUT_HANDLE );
  GetConsoleCursorInfo( hOutput, &Info );
  Info.bVisible = bVisible;
  SetConsoleCursorInfo( hOutput, &Info );
}

/*bool CMailViewDialog::OnCtlColorDlgItem( TDnColor & Item )
{
  if ( Item.Id == 0 )
  {
    //LO(LO=Title,HI=HiText), HI(LO=Box,0)
    Item.Result = MAKELONG(
      MAKEWORD( clText, 0x00 ),
      MAKEWORD( clText, 0x00 ) );
    return true;
  }
  return false;
}
*/

CFarDlgItem * CFileViewDialog::Execute( bool * pbWordWrap, const int X, const int Y )
{
  m_bNeedReopen = false;
  if ( m_View && pbWordWrap )
    m_View->SetWordWrap( *pbWordWrap );
  CFarDialog::Execute( X, Y );
  if ( m_View && pbWordWrap )
    *pbWordWrap = m_View->GetWordWrap();
  return m_bNeedReopen ? NULL : (CFarDlgItem*)-1;
}

bool CMailViewDialog::OnDraw( LPBOOL result )
{
  *result = TRUE;

  SendMessage( DM_GETFOCUS, 0, 0 );

  COORD coord = { 0, 0 };

  //DWORD keys[] = { KEY_HOME };

  for ( int i = 0; i < 256; i ++ )
  {
    FarDialogItem item;
    if ( !SendMessage( DM_GETDLGITEM, i, (long)&item ) )
      break;

    if ( item.Type == DI_EDIT && (int)strlen( item.Data ) > (item.X2 - item.X1) )
    {
      SendMessage( DM_SETCURSORPOS, i, (long)&coord );
      //SendMessage( DM_SETFOCUS, i, 0 );
      //SendMessage( DM_KEY, sizeof( keys ) / sizeof( keys[ 0 ] ), (long)&keys );
    }
  }

  //SendMessage( DM_SETFOCUS, itemId, 0 );

  return true;
}

bool CFileViewDialog::OnKey( TDnKey & Item )
{
  if ( Item.Key == KEY_F5 )
  {
    if ( Far::GetBuildNumber() >= 1446 )
    {
      m_bNeedReopen = true;
      Item.Result = SendMessage( DM_CLOSE, -1, 0 );
    }
    return true;
  }
#ifdef _DEBUG
  if ( Item.Key == KEY_CTRLF12 )
  {
    int Count = Far::AdvControl( ACTL_GETWINDOWCOUNT, NULL );

    FarMenu mnu( "Screens" );

    LPSTR Tmp = create char[ 0x1000 ];

    WindowInfo Info;
    for ( int i = 0; i < Count; i++ )
    {
      Info.Pos = i;
      Far::AdvControl( ACTL_GETWINDOWINFO, &Info );
      FarSF::snprintf( Tmp, 0x1000, "%c. %s %25s", i<10?i+'0':i+'A'-10, Info.TypeName, Info.Name );
      mnu.AddItem( Tmp )->SetData( i );
    }

    FarMenu::PItem res = mnu.Show();
    if ( res )
    {
      Far::AdvControl( ACTL_SETCURRENTWINDOW, (LPVOID)res->GetData() );
      Far::AdvControl( ACTL_COMMIT, NULL );
      KeySequence ks;
      ks.Flags = 0;
      ks.Count = 1;
      DWORD keys[] = { KEY_TAB };
      ks.Sequence = keys;
      Far::AdvControl( ACTL_POSTKEYSEQUENCE, &ks );
      Far::AdvControl( ACTL_POSTKEYSEQUENCE, &ks );
    }

    delete Tmp;
    return true;
  }
#endif

  if ( m_View == NULL )
    return false;

  if ( Item.Key == KEY_F2 )
  {
    m_View->SetWordWrap( !m_View->GetWordWrap() );
  }
  else if ( Item.Key == KEY_F8 && m_View )
  {
    long Table = SelectCharacterTableMenu( m_View->GetEncoding() );
    if ( Table != FCT__INVALID )
      m_View->SetEncoding( Table );
  }
  else if ( Item.Key == KEY_F9 )
  {
    CharTableSet ctSet;
    int Table = m_View->GetEncoding();
    if ( Table == FCT__INVALID || Table & FCT_DETECT )
      Table = Far::GetCharTable( 0, &ctSet );
    else
    {
      Table = Far::GetCharTable( Table + 1, &ctSet );
      if ( Table == FCT__INVALID )
        Table = Far::GetCharTable( 0, &ctSet );
    }

    if ( Table != FCT__INVALID )
      m_View->SetEncoding( Table );
  }
  else if ( Item.Key == KEY_SHIFTF9 )
  {
    CharTableSet ctSet;
    int Table = m_View->GetEncoding();
    if ( Table == FCT__INVALID || Table & FCT_DETECT )
    {
      Table = FCT__INVALID;
      while ( Far::GetCharTable( Table + 1, &ctSet ) != FCT__INVALID )
        Table ++;
    }
    else
    {
      Table = Far::GetCharTable( Table - 1, &ctSet );
      if ( Table == FCT__INVALID )
      {
        Table = FCT__INVALID;
        while ( Far::GetCharTable( Table + 1, &ctSet ) != FCT__INVALID )
          Table ++;
      }
    }

    if ( Table != FCT__INVALID )
      m_View->SetEncoding( Table );

  }
  else if ( Item.Key == KEY_ALTSHIFTF9 )
  {
    ViewCfg()->Configure();

    CMailView::m_Handle->SaveSettings();

    far_assert( m_View != NULL );

    m_View->ForcePaint();
  }
  else if ( Item.Key == KEY_ALTF2 )
  {
    ViewCfg()->SetHideStylePunct( !ViewCfg()->GetHideStylePunct() );

    CMailView::m_Handle->SaveSettings();

    far_assert( m_View != NULL );

    m_View->ForcePaint();
  }

  Redraw();

  Item.Result = TRUE;

  return CFarDialog::Dispatch( *(TMessage*)&Item );
}

CFileViewDialog::CFileViewDialog( bool bFullWnd )
  : CFarDialog(), m_FullWnd( bFullWnd ), m_View( NULL )
{
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &csbi );

  //m_WndSize = csbi.dwSize;
  m_WndSize = csbi.dwMaximumWindowSize;

//  DbgMsg( "m_wndSize.x = %d, m_wndSize.y = %d", m_WndSize.X, m_WndSize.Y );

  if ( m_FullWnd )
    SetWidth( m_WndSize.X ), SetHeight( m_WndSize.Y );
  else
    SetWidth( m_WndSize.X - 8 ), SetHeight( m_WndSize.Y - 3 );
}

CFileViewDialog::~CFileViewDialog()
{
}

CMailViewDialog::CMailViewDialog( PMessage Msg, bool bFullWnd, long defaultEncoding )
  : CFileViewDialog( bFullWnd )
  , m_OrigDel( false )
  , m_OrigMsg( Msg )
  , m_edtArea( NULL )
  , m_edtFrom( NULL )
  , m_edtTo( NULL )
  , m_edtSubject( NULL )
  , m_edtCc( NULL )
{
  SetHelpTopic( "MsgView" );
  far_assert( Msg != NULL );
  Create( NULL, defaultEncoding );
}

CMailViewDialog::CMailViewDialog( LPCSTR FileName, bool bFullWnd, long defaultEncoding )
  : CFileViewDialog( bFullWnd )
  , m_OrigDel( true )
  , m_OrigMsg( CreateFarMessage( FileName, FCT_DEFAULT, false ) )
  , m_edtArea( NULL )
  , m_edtFrom( NULL )
  , m_edtTo( NULL )
  , m_edtSubject( NULL )
  , m_edtCc( NULL )
{

  SetHelpTopic( "MsgView" );
  Create( FileName, defaultEncoding );
}

CMailViewDialog::~CMailViewDialog()
{
  if ( m_OrigDel )
    delete m_OrigMsg;
}

void CMailViewDialog::changeHeadersEncoding( CFarDialog * fdlg )
{
  CMailViewDialog * dlg = (CMailViewDialog*)fdlg;

  dlg->m_OrigMsg->SetEncoding( dlg->m_View->GetEncoding() );

  if ( dlg->m_edtArea )
  {
    FarString Area = dlg->m_OrigMsg->GetArea();
    dlg->m_edtArea->SetText( Area.IsEmpty() ? dlg->m_OrigMsg->GetNewsgroups() : Area );
    dlg->UpdateControl( dlg->m_edtArea );
  }
  if ( dlg->m_edtFrom )
  {
    dlg->m_edtFrom->SetText( dlg->m_OrigMsg->GetFrom()->GetMailboxName() );
    dlg->UpdateControl( dlg->m_edtFrom );
  }
  if ( dlg->m_edtSubject )
  {
    dlg->m_edtSubject->SetText( dlg->m_OrigMsg->GetSubject() );
    dlg->UpdateControl( dlg->m_edtSubject );
  }
  if ( dlg->m_edtTo )
  {
    dlg->m_edtTo->SetText( dlg->m_OrigMsg->GetTo()->getAddresses() );
    dlg->UpdateControl( dlg->m_edtFrom );
  }
  if ( dlg->m_edtCc )
  {
    dlg->m_edtCc->SetText( dlg->m_OrigMsg->GetCc()->getAddresses() );
    dlg->UpdateControl( dlg->m_edtCc );
  }
}

void CMailViewDialog::CalcHeader( int * HeadersCount, int * MaxHeaderLen )
{
  far_assert( HeadersCount != NULL );
  far_assert( MaxHeaderLen != NULL );

  *HeadersCount = 0;
  *MaxHeaderLen = 0;

  if ( !m_OrigMsg->GetArea().IsEmpty() )
  {
    (*HeadersCount) ++;
    (*MaxHeaderLen) = max( *MaxHeaderLen, (int)strlen( Far::GetMsg( MArea ) ) );
  }
  else if ( !m_OrigMsg->GetNewsgroups().IsEmpty() )
  {
    (*HeadersCount) ++;
    (*MaxHeaderLen) = max( *MaxHeaderLen, (int)strlen( Far::GetMsg( MNewsgroups ) ) );
  }

  if ( !m_OrigMsg->GetFrom()->GetMailboxName().IsEmpty() )
  {
    (*HeadersCount) ++;
    (*MaxHeaderLen) = max( *MaxHeaderLen, (int)strlen( Far::GetMsg( MFrom ) ) );
  }

  if ( !m_OrigMsg->GetTo()->GetMailboxName().IsEmpty() )
  {
    (*HeadersCount) ++;
    (*MaxHeaderLen) = max( *MaxHeaderLen, (int)strlen( Far::GetMsg( MTo ) ) );
  }

  if ( !m_OrigMsg->GetCc()->GetMailboxName().IsEmpty() )
  {
    (*HeadersCount) ++;
    (*MaxHeaderLen) = max( *MaxHeaderLen, (int)strlen( Far::GetMsg( MCc ) ) );
  }

  if ( !m_OrigMsg->GetSubject().IsEmpty() )
  {
    (*HeadersCount) ++;
    (*MaxHeaderLen) = max( *MaxHeaderLen, (int)strlen( Far::GetMsg( MSubject ) ) );
  }
}

CFarEdit * CMailViewDialog::CreateHeaderControl( int Top, int HeaderId, int HeaderLen, const FarString& EditText, LPCSTR DateText )
{
  CFarText * Text = create CFarText;
  InsertControl( Text );
  Text->SetLeft( 1 );
  Text->SetTop( Top );
  Text->SetColor( MVColor( CColorCfg::iclHeaderLine ) );
  FarString Hdr = FarString( '\x20',
    HeaderLen - strlen( Far::GetMsg( HeaderId ) ) ) + Far::GetMsg( HeaderId );
  Text->SetText( Hdr );

  CFarEdit * Edit = create CFarEdit;
  InsertControl( Edit );
  Edit->SetLeft( HeaderLen + 2 );
  Edit->SetTop( Top );
  Edit->SetWidth( GetWidth() - Edit->GetLeft() - GetBorderSize() * 2 - 1  );
  Edit->SetReadOnly( true );
  Edit->SetText( EditText );
  Edit->SetColor( MVColor( CColorCfg::iclHeaderEdit ),
    MVColor( CColorCfg::iclHeaderEditSel ), MVColor( CColorCfg::iclHeaderEdit ) );
  Edit->SetSelectOnEntry( true );

  if ( DateText && *DateText )
  {
    int DateLen = strlen( DateText );

    Edit->SetWidth( Edit->GetWidth() - DateLen - 1 );
    Text = create CFarText;
    InsertControl( Text );
    Text->SetLeft( Edit->GetLeft() + Edit->GetWidth() + 1 );
    Text->SetTop( Top );
    Text->SetText( DateText );
    Text->SetColor( MVColor( CColorCfg::iclDateTime ) );
  }
  return Edit;
}

void CMailViewDialog::Create( LPCSTR FileName, long defaultEncoding )
{
  SetColor( MVColor( CColorCfg::iclBorder ) );
  SetBorderStyle( bsDouble );
  SetBorderOrigin( 0, 0 );

  FarString Tmp;

  if ( FileName )
  {
    Tmp = FarSF::PointToName( FileName );
    FarSF::TruncStr( Tmp.GetBuffer(), GetWidth() - 6 );
    Tmp.ReleaseBuffer();
    SetTitle( Tmp );
  }

  int HeadersCount;
  int MaxHeaderLen;
  CalcHeader( &HeadersCount, &MaxHeaderLen );

  int ViewStart = HeadersCount;
  if ( HeadersCount > 0 )
  {
    ViewStart ++;
    //////////////////////////////////////////////////////////////////////////
    // Separator
    CFarSeparator * Separator = create CFarSeparator;

    char flags[] = "[ \x20 ]";

    if ( m_OrigMsg->GetAttchmentsCount() > 0 )
      flags[ 2 ] = '@';
    /*if ( m_OrigMsg->GetFlags() & EMF_FLAGGED )
      flags[ 3 ] = '!';
    if ( m_OrigMsg->GetFlags() & EMF_REPLIED )
      flags[ 4 ] = 'R';
    if ( m_OrigMsg->GetFlags() & EMF_FORWDED )
      flags[ 5 ] = 'F';*/

    if ( flags[ 2 ] != '\x20' )
      Separator->SetText( flags );

    InsertControl( Separator );

    Separator->SetTop( HeadersCount );
    Separator->SetLeft( alRight );
    Separator->SetColor( MVColor( CColorCfg::iclBorder ) );
  }

  //////////////////////////////////////////////////////////////////////////
  // User Control
  m_View = create CMailViewControl( this, defaultEncoding );
  m_View->SetColor( MVColor( CColorCfg::iclNormal ) );
  m_View->SetBounds( 0, ViewStart, GetWidth() - 2, GetHeight() - ViewStart - 2 );
  InsertControl( m_View );
  SetFocusControl( m_View );

  //////////////////////////////////////////////////////////////////////////
  // Vertical Scrollbar
  /*CFarText * VScroll = create CFarScrollBar( tsVertical );
  VScroll->SetTop( ViewStart );
  VScroll->SetLeft( View->GetWidth() );
  VScroll->SetHeight( View->GetHeight() );
  InsertControl( VScroll );
*/

  ViewStart = 0;
  if ( !(Tmp = m_OrigMsg->GetArea()).IsEmpty() )
  {
    m_edtArea = CreateHeaderControl( ViewStart++, MArea, MaxHeaderLen, Tmp );
  }
  else if ( !(Tmp = m_OrigMsg->GetNewsgroups()).IsEmpty() )
  {
    m_edtArea = CreateHeaderControl( ViewStart++, MNewsgroups, MaxHeaderLen, Tmp );
  }
  else
    m_edtArea = NULL;

  if ( !(Tmp = m_OrigMsg->GetFrom()->GetMailboxName()).IsEmpty() )
  {
    FILETIME ft;
    m_OrigMsg->GetSent( &ft );
    m_edtFrom = CreateHeaderControl( ViewStart++, MFrom, MaxHeaderLen, Tmp, FileTimeToString( ft ) );
  }

  if ( !(Tmp = m_OrigMsg->GetTo()->getAddresses()).IsEmpty() )
  {
    FILETIME ft;
    m_OrigMsg->GetReceived( &ft );
    m_edtTo = CreateHeaderControl( ViewStart++, MTo, MaxHeaderLen, Tmp, FileTimeToString( ft ) );
  }
  else
    m_edtTo = NULL;

  if ( !(Tmp = m_OrigMsg->GetCc()->getAddresses()).IsEmpty() )
    m_edtCc = CreateHeaderControl( ViewStart++, MCc, MaxHeaderLen, Tmp );
  else
    m_edtCc = NULL;

  if ( !(Tmp = m_OrigMsg->GetSubject()).IsEmpty() )
    m_edtSubject = CreateHeaderControl( ViewStart++, MSubject, MaxHeaderLen, Tmp );
  else
    m_edtSubject = NULL;


  m_View->OnChangeEncodingFunc = changeHeadersEncoding;

  ((CMailViewControl*)m_View)->LoadStrings( m_OrigMsg );

  SetFocusControl( m_View );
}

//////////////////////////////////////////////////////////////////////////
void CTextViewControl::Reload()
{
  GetStrings()->Clear();

  FarTextFile TxtFile;
  if ( TxtFile.OpenForRead( m_FileName ) )
  {
    CharTableSet ctSet;
    int Enc = m_Encoding;
    if ( Enc == FCT_DETECT )
    {
      Enc = Far::DetectCharTable( (LPCSTR)TxtFile.GetMemory(), TxtFile.GetSize() );
      if ( Enc != FCT__INVALID )
        m_Encoding = Enc;
    }
    else if ( Enc == FCT_DEFAULT )
    {
      m_Encoding = m_EncodingDef;
    }

    if ( Enc != FCT__INVALID && Enc != FCT_OEM && Enc != FCT_ANSI )
      Enc = Far::GetCharTable( m_Encoding, &ctSet );

    while ( TxtFile.GetPosition() < TxtFile.GetSize() )
      GetStrings()->Add( ToOEMString( TxtFile.ReadLine(), m_Encoding ) );
  }
  TxtFile.Close();

  m_NeedRedraw = true;
}

//////////////////////////////////////////////////////////////////////////
extern long GetMsgEncoding( PMessage Msg, PMsgPart TextPart, long defaultEncoding );

void CMailViewControl::LoadStrings( PMessage pMsg )
{
  FarFileName TmpFileName = FarFileName::MakeTemp( "mv" );
  // TODO: chaeck File
  File::mkdirs( TmpFileName.GetPath() );

  FarString Tmp;

  typedef HANDLE (__cdecl *THV_Document_Create)(PluginStartupInfo*,LPCSTR);
  typedef void   (__cdecl *THV_Document_Destroy)(HANDLE);
  typedef LPCSTR (__cdecl *THV_Document_MakeTxtFile)(HANDLE,LPCSTR,COORD);
  THV_Document_Create      docCreate      = NULL;
  THV_Document_Destroy     docDestroy     = NULL;
  THV_Document_MakeTxtFile docMakeTxtFile = NULL;
  HANDLE  hDoc = NULL;
  HMODULE hMod = NULL;

  /*!!!pMsg->SaveBodyToFile( TmpFileName );*/
  PMsgPart TextPart = pMsg->GetTextPart();
  if ( TextPart )
  {
    TextPart->SaveToFile( TmpFileName );

    CMimeContent mc;
    mc.assign( TextPart->GetKludge(K_RFC_ContentType));

    LPCSTR charset = mc.getDataValue("charset");
    if (charset)
      setDefaultEncoding(getCharacterTable(charset));


    if (FarSF::LStricmp( mc.getType(), "text" ) == 0 && FarSF::LStricmp( mc.getSubType(), "html" ) == 0)
    {
      hMod = LoadLibrary( ViewCfg()->GetHVLib() );
      if ( hMod )
      {
        typedef HANDLE (__cdecl *THV_Document_Create)(PluginStartupInfo*,LPCSTR);
        typedef void   (__cdecl *THV_Document_Destroy)(HANDLE);
        typedef LPCSTR (__cdecl *THV_Document_MakeTxtFile)(HANDLE,LPCSTR,COORD);
        docCreate      = (THV_Document_Create)GetProcAddress( hMod, "HV_Document_Create" );
        docDestroy     = (THV_Document_Destroy)GetProcAddress( hMod, "HV_Document_Destroy" );
        docMakeTxtFile = (THV_Document_MakeTxtFile)GetProcAddress( hMod, "HV_Document_MakeTxtFile" );
        if ( docCreate && docDestroy && docMakeTxtFile )
        {
          hDoc = docCreate( Far::GetPluginStartupInfo(), ViewCfg()->GetHVCfg().data() );
          if ( hDoc )
          {
            COORD c = { GetWidth(), GetHeight() };
            LPCSTR TxtFileName = docMakeTxtFile( hDoc, TmpFileName, c );
            if ( TxtFileName )
            {
              SetFileAttributes( m_FileName, FILE_ATTRIBUTE_NORMAL );
              DeleteFile( TmpFileName );
              TmpFileName = TxtFileName;
            }
            docDestroy( hDoc );
          }
        }
        FreeLibrary( hMod );
      }

    }
  }
  m_XFTNOrigin   = pMsg->GetKludge( K_RFC_XFTNOrigin );
  m_XFTNTearline = pMsg->GetKludge( K_RFC_XFTNTearline );
  SetEncoding( GetMsgEncoding( pMsg, TextPart, m_defaultEncoding ), false );
  LoadFromFile( TmpFileName, true );
}
//////////////////////////////////////////////////////////////////////////

char * strxcpy( char * d, const char * s, int n )
{
  if ( n )
    strncpy( d, s, n - 1 ), d[ n - 1 ] = 0;
  else
    *d = 0;
  return d;
}

inline bool isstylechar( char c )
{
  return (c == '*') || (c == '/') || (c == '_') || (c == '#');
}

void CTextViewControl::SetWordWrap( bool Value )
{
  if ( m_WordWrap == Value )
    return;

  Reload();

  m_WordWrap = Value;

  if ( Value == false )
    return;

  FarStringArray * ss = WordWrapLines( *m_Strings, GetWidth(), true );
  delete m_Strings;
  m_Strings = ss;
}

void CTextViewControl::FileBegin()
{
  BeginUpdate();
  Home();
  Top();
  EndUpdate();
}

void CTextViewControl::FileEnd()
{
  BeginUpdate();
  Home();
  Bottom();
  EndUpdate();
}

void CTextViewControl::Top()
{
  int H = GetHeight() - 1;
  if ( m_Strings->Count() > H && m_TopString != 0 )
  {
    m_TopString = 0;
    m_NeedRedraw = true;
    m_Dialog->Redraw();
  }
}

void CTextViewControl::Bottom()
{
  int H = GetHeight() - 1;
  int C = m_Strings->Count();
  if ( C > H  )
  {
    m_TopString = C - H;
    m_NeedRedraw = true;
    m_Dialog->Redraw();
  }
}

void CTextViewControl::PageUp()
{
  int H = GetHeight() - 1;
  int C = m_Strings->Count();
  if ( C > H  )
  {
    m_TopString -= H - 1;
    if ( m_TopString < 0 )
      m_TopString = 0;
    m_NeedRedraw = true;
    m_Dialog->Redraw();
  }
}

void CTextViewControl::PageDown()
{
  int H = GetHeight() - 1;
  int C = m_Strings->Count();
  if ( C > H )
  {
    m_TopString += H-1;
    if ( m_TopString >= C - H )
      m_TopString = C - H - 1;
    m_NeedRedraw = true;
    m_Dialog->Redraw();
  }
}

void CTextViewControl::LineUp()
{
  if ( m_TopString > 0 )
  {
    m_TopString --;

    int W = GetWidth();

    memmove( m_vBuf + W, m_vBuf, sizeof( CHAR_INFO ) * W * ( GetHeight() - 1 ) );

    PaintLine( 0, m_TopString );

    m_Dialog->Redraw();
  }
}

void CTextViewControl::LineDown()
{
  int H = GetHeight();
  if ( m_TopString < m_Strings->Count() - H )
  {
    m_TopString ++;

    int W = GetWidth();

    memmove( m_vBuf, m_vBuf + W, sizeof( CHAR_INFO ) * W * (--H) );

    PaintLine( H, m_TopString + H );

    m_Dialog->Redraw();
  }
}

void CTextViewControl::CharLeft()
{
  if ( m_LeftColumn > 0 )
  {
    m_LeftColumn --;
    m_NeedRedraw = true;
    m_Dialog->Redraw();
  }
}

void CTextViewControl::CharRight()
{
  if ( m_LeftColumn < 256 )
  {
    m_LeftColumn ++;
    m_NeedRedraw = true;
    m_Dialog->Redraw();
  }
}

void CTextViewControl::Home()
{
  if ( m_LeftColumn != 0 )
  {
    m_LeftColumn = 0;
    m_NeedRedraw = true;
    m_Dialog->Redraw();
  }
}

void CTextViewControl::End()
{
}

bool CTextViewControl::Perform( int Msg, long Param, long * Result )
{
  if ( Msg == DN_DRAWDLGITEM && m_NeedRedraw )
  {
    Paint();
    return true;
  }

  if ( Msg == DN_KEY )
  {
    switch ( Param )
    {
    case KEY_UP:
      LineUp();
      break;
    case KEY_DOWN:
      LineDown();
      break;
    case KEY_PGUP:
      PageUp();
      break;
    case KEY_PGDN:
    case KEY_SPACE:
      PageDown();
      break;
    case KEY_LEFT:
      CharLeft();
      break;
    case KEY_RIGHT:
      CharRight();
      break;
    case KEY_HOME:
      Home();
      break;
    case KEY_END:
      End();
      break;
    case KEY_CTRLPGUP:
      Top();
      break;
    case KEY_CTRLPGDN:
      Bottom();
      break;
    case KEY_CTRLHOME:
      FileBegin();
      break;
    case KEY_CTRLEND:
      FileEnd();
      break;
    default:
      return false;
    }

    return true;
  }

/*  if ( Msg == DN_GOTFOCUS )
  {
    ShowConsoleCursor( FALSE );
    return true;
  }

  if ( Msg == DN_KILLFOCUS )
  {
    ShowConsoleCursor( TRUE );
    return true;
  }
*/
  return CFarUserControl::Perform( Msg, Param, Result );
}

void CTextViewControl::UpdateScrollbar()
{
}

void CTextViewControl::Update()
{
  if ( m_vBuf )
    delete [] m_vBuf;

  int Size = GetWidth() * GetHeight();
  m_vBuf = create CHAR_INFO[ Size ];

  Clear( Size );
}

extern LPCSTR strstri( LPCSTR str1, LPCSTR str2 );

void CTextViewControl::Draw( int Row, int Col, int Color, LPCSTR Text, int TextLen )
{
  int W = GetWidth();
  int H = GetHeight();

  if ( Row >= H ) return;

  PCHAR_INFO vBuf = m_vBuf + Row * W;

  int Start = m_LeftColumn - Col > 0 ? Start = m_LeftColumn - Col : 0;

  if ( TextLen > W - Col + m_LeftColumn )
    TextLen = W - Col + m_LeftColumn;

  for ( int j = Start; j < TextLen; j ++ )
  {
    if ( Text[ j ] == '\t' )
      vBuf[ j + Col - m_LeftColumn ].Char.UnicodeChar = '\x20';
    else
    {
#ifdef _USE_TTF_SUPPORT
      if ( m_isTTF )
      {
        WCHAR ub[ 1 ];
        MultiByteToWideChar( CP_OEMCP, 0, Text + j, 1, ub, 1 );
        vBuf[ j + Col - m_LeftColumn ].Char.UnicodeChar = ub[ 0 ];
      }
      else
#endif
      {
        vBuf[ j + Col - m_LeftColumn ].Char.AsciiChar = Text[ j ];
      }
    }
    vBuf[ j + Col - m_LeftColumn ].Attributes = Color;
  }
}

void CTextViewControl::Draw( int Row, int Col, int Color, LPCSTR Text )
{
  Draw( Row, Col, Color, Text, strlen( Text ) );
}

void CTextViewControl::PaintLine( int Row, int LineNo )
{
  LPCSTR Txt = m_Strings->At( LineNo );
  int    Len = strlen( Txt );
  int    Clr = GetColor();

  Draw( Row, 0, Clr, Txt, Len );

  int W = GetWidth();

  PCHAR_INFO vBuf = m_vBuf + W * Row;
  while ( Len < W )
  {
    vBuf[ Len ].Attributes       = Clr;
    vBuf[ Len ].Char.UnicodeChar = '\x20';
    Len ++;
  }
}

void CTextViewControl::Paint()
{
  m_NeedRedraw = false;

  int H = GetHeight();
  int W = GetWidth();

  Clear( H * W );

  for ( int Row = 0, i = m_TopString; Row < H; Row ++, i ++ )
    if ( i < m_Strings->Count() )
      PaintLine( Row, i );

  UpdateScrollbar();
}

void CTextViewControl::Clear( int Size )
{
  if ( m_vBuf == NULL )
  {
    DbgMsg( "CTextViewControl::Clear", "Fatal :: Please report about!!!" );
    return;
  }

  if ( GetColor() != -1 )
  {
    WORD Attr = GetColor();
    while ( Size )
    {
      --Size;
      m_vBuf[ Size ].Attributes       = Attr;
      m_vBuf[ Size ].Char.UnicodeChar = '\x20';
    }
  }
}

void CTextViewControl::Clear()
{
  Clear( GetWidth() * GetHeight() );
}

#define STR_OriginStr      " * Origin: "
#define STR_TaglineStr     "... "
#define STR_TearlineStr    "--- "
#define STR_SignatureStr   "-- "

bool CMailViewControl::IsSignature( int LineNo )
{
  FarStringArray * ss = GetStrings();

  // проверим вычислялим мы уже начало сигнатуры или нет
  if ( m_SignatureStart == -1 )
  {
    bool bLastSignature = ViewCfg()->GetUseLastSignature();
    // сигратурой считается все что встретится после последних символов сигнатуры
    if ( bLastSignature )
    {
      for ( m_SignatureStart = ss->Count() - 1; m_SignatureStart >= 0; m_SignatureStart -- )
      {
        if ( strcmp( ss->At( m_SignatureStart ), STR_SignatureStr ) == 0 )
          break;
      }
      if ( m_SignatureStart <= 0 )
        m_SignatureStart = ss->Count();
    }
    // сигратурой считается все, что встретится после первых символов сигнатуры
    else
    {
      for ( m_SignatureStart = 0; m_SignatureStart < ss->Count(); m_SignatureStart ++ )
      {
        if ( strcmp( ss->At( m_SignatureStart ), STR_SignatureStr ) == 0 )
          break;
      }
    }
  }

  return m_SignatureStart < ss->Count() && LineNo >= m_SignatureStart;
}

bool CMailViewControl::IsOrigin( int LineNo )
{
  FarStringArray * ss = GetStrings();

  if ( m_OriginLine == -1 )
  {
    for ( m_OriginLine = ss->Count() - 1; m_OriginLine >= 0; m_OriginLine -- )
    {
      if ( strncmp( ss->At( m_OriginLine ), STR_OriginStr, strlen( STR_OriginStr ) ) == 0 )
        break;
    }
    if ( m_OriginLine <= 0 )
      m_OriginLine = ss->Count();
  }

  return m_OriginLine < ss->Count() && LineNo == m_OriginLine;
}

bool CMailViewControl::IsTearline( int LineNo )
{
  FarStringArray * ss = GetStrings();

  if ( m_TearlineLine == -1 )
  {
    for ( m_TearlineLine = ss->Count() - 1; m_TearlineLine >= 0; m_TearlineLine -- )
    {
      if ( strncmp( ss->At( m_TearlineLine ), STR_TearlineStr, strlen( STR_TearlineStr ) ) == 0 )
        break;
    }
    if ( m_TearlineLine <= 0 )
      m_TearlineLine = ss->Count();
  }

  return m_TearlineLine < ss->Count() && LineNo == m_TearlineLine;
}

bool CMailViewControl::IsTagline( int LineNo )
{
  FarStringArray * ss = GetStrings();

  if ( m_TaglineLine == -1 )
  {
    for ( m_TaglineLine = ss->Count() - 1; m_TaglineLine >= 0; m_TaglineLine -- )
    {
      if ( strncmp( ss->At( m_TaglineLine ), STR_TaglineStr, strlen( STR_TaglineStr ) ) == 0 )
        break;
    }
    if ( m_TaglineLine <= 0 )
      m_TaglineLine = ss->Count();
  }

  return m_TaglineLine < ss->Count() && LineNo == m_TaglineLine;
}

void CMailViewControl::Reload()
{
  m_SignatureStart = -1;
  m_OriginLine     = -1;
  m_TearlineLine   = -1;
  m_TaglineLine    = -1;
  CTextViewControl::Reload();
  FarStringArray * ss = GetStrings();
  if ( !m_XFTNTearline.IsEmpty() || !m_XFTNOrigin.IsEmpty() )
  {
    if ( ss->Count() > 0 && *ss->At( ss->Count() - 1 ) != '\0' )
      ss->Add( STR_EmptyStr );
    if ( !m_XFTNTearline.IsEmpty() )
      ss->Add( STR_TearlineStr + ToOEMString( m_XFTNTearline, GetEncoding() ) );
    if ( !m_XFTNOrigin.IsEmpty() )
      ss->Add( STR_OriginStr + ToOEMString( m_XFTNOrigin, GetEncoding() ) );
  }
}

void CMailViewControl::PaintLine( int Row, int LineNo )
{
  LPCSTR Txt = GetStrings()->At( LineNo );
  int    Len = strlen( Txt );
  int    Clr;

  if ( IsOrigin( LineNo ) )
    Clr = MVColor( CColorCfg::iclOrigin );
  else if ( IsTearline( LineNo ) )
    Clr = MVColor( CColorCfg::iclTearline );
  else if ( IsTagline( LineNo ) )
    Clr = MVColor( CColorCfg::iclTagline );
  else if ( IsSignature( LineNo ) )
    Clr = MVColor( CColorCfg::iclSignature );
  else
  {
    int j = 0;

    Clr = GetColor();

    // пропустим все пробелы
    while ( j < Len && isspace( (unsigned char)Txt[ j ] ) ) j ++;

    const char * QuoteStr = ">|";

    char QuoteChar = 0;
    // определим есть ли квотинг
    for ( int k = 0; j < Len; j ++ )
    {
      // инициалы это или нет
      if ( !FarSF::LIsAlpha( (BYTE)Txt[ j ] ) )
      {
        // определим является ли этот символ символом квотинга
        if ( strchr( QuoteStr, Txt[ j ] ) )
        {
          QuoteChar = Txt[ j ];

          // подсчитаем сколько символов квотинга
          while ( j < Len && Txt[ j ] == QuoteChar )
          {
            j ++;
            k ++;
          }

          // зададим разный цвет для четного и нечетного
          if ( k > 0 )
            Clr = MVColor( k % 2 ? CColorCfg::iclQuotesOdd : CColorCfg::iclQuotesEven );
        }

        // больше делать нечего
        break;
      }
    }
  }

  StyleCodeHighlight( Txt, Row, 0, Clr, ViewCfg()->GetHideStylePunct() );

  int W = GetWidth();
  PCHAR_INFO vBuf = m_vBuf + W * Row;
  while ( Len < W )
  {
    vBuf[ Len ].Attributes       = Clr;
    vBuf[ Len ].Char.UnicodeChar = '\x20';
    Len ++;
  }
}

void CMailViewControl::StyleCodeHighlight(LPCSTR Text, int Row, int Col, int Color, bool dohide)
{
  UINT   sclen          = 0;
  LPCSTR txptr          = Text;
  LPCSTR ptr            = Text;
  LPCSTR stylemargins   = " -|\\"; // we probably have to make a keyword for it
  char   prevchar       = '\x20';

  LPCSTR styleCodePunct = ViewCfg()->getStyleCodePunct();
  LPCSTR styleCodeStops = ViewCfg()->getStyleCodeStops();

  while (*ptr)
  {
    if (isstylechar(*ptr) && strchr(styleCodePunct, prevchar))
    {
      LPCSTR beginstyle = ptr;

      int bb = 0, bi = 0, bu = 0, br = 0;
      while (isstylechar(*ptr))
      {
        switch (*ptr)
        {
          case '*': bb++; break;
          case '/': bi++; break;
          case '_': bu++; break;
          case '#': br++; break;
        }

        ptr++;
      }

      if (bb <= 1 && bi <= 1 && br <= 1 && bu <= 1 && *ptr)
      {
        LPCSTR beginword = ptr;                       //  /_*>another*_/
        char   endchar   = '\0';
        LPSTR  end       = (LPSTR)ptr;
        do
        {
          end = strpbrk(++end, styleCodePunct);
        } while (end && !isstylechar(*(end-1)));

        if (end)
          endchar = *end;
        else
          end = (LPSTR)ptr + strlen(ptr);
        *end = '\0';
        LPCSTR endstyle = end - 1;                   //  /_*another*>_/
        if (isstylechar(*endstyle) && !strchr(stylemargins, *beginword))
        {
          LPSTR endword = (LPSTR)endstyle;
          int eb = 0, ei = 0, eu = 0, er = 0;
          while (isstylechar(*endword))
          {
            switch ( *endword )
            {
              case '*': eb++; break;
              case '/': ei++; break;
              case '_': eu++; break;
              case '#': er++; break;
            }
            endword--;
          }                                          //  /_*anothe>r*_/
          if ( endword >= beginword && !strchr( stylemargins, *endword ) )
          {
            if (bb == eb && bi == ei && bu == eu && br == er)
            {
              char endwordchar = *endword;
              *endword = '\0';
              bool style_stops_present = strpbrk(beginword, styleCodeStops) != NULL;
              *endword = endwordchar;
              if (!style_stops_present) // style stops does not present
              {
                int colorindex = (bb ? 1 : 0) | (bi ? 2 : 0) | (bu ? 4 : 0) | (br ? 8 : 0);
                /* пробуем избавиться от переменной buf
                strxcpy( buf, txptr, (UINT)(beginstyle-txptr) + 1 );
                Draw( Row, Col + sclen, Color, buf );
                sclen += strlen( buf );
                */
                int DrawLen = beginstyle - txptr;
                Draw( Row, Col + sclen, Color, txptr, DrawLen );
                sclen += DrawLen;
                /* пробуем избавиться от переменной buf
                if( dohide )
                  strxcpy( buf, beginword, (UINT)(endword-beginword) + 2 );
                else
                  strxcpy( buf, beginstyle, (UINT)(endstyle-beginstyle) + 2 );
                Draw( Row, Col + sclen, MVColor( colorindex ), buf );
                sclen += strlen( buf );
                */
                if ( dohide )
                {
                  DrawLen = endword - beginword + 1;
                  Draw(Row, Col + sclen, MVColor(colorindex), beginword, DrawLen);
                }
                else
                {
                  DrawLen = endstyle - beginstyle + 1;
                  Draw(Row, Col + sclen, MVColor(colorindex), beginstyle);
                }
                sclen += DrawLen;
                txptr = end;
              }
            }
          }
        }

        *end = endchar;
        ptr = end - 1;
      }
    }
    else
    {
      const char * HyperlinkPrefixes[] =
      {
        "http://", "ftp://", "https://", "gopher://", "news://", "file://",
        "mailto:", "msgid:", "mid:",
        "www.", "ftp."
      };

      // пробежим по всем префиксам
      for (int i = 0; i < sizeof(HyperlinkPrefixes)/sizeof(HyperlinkPrefixes[0]); i ++)
      {
        int HyperlinkPrefixesLen = strlen( HyperlinkPrefixes[ i ] );
        // проверим теущий префикс
        if (FarSF::LStrnicmp(HyperlinkPrefixes[ i ], ptr, HyperlinkPrefixesLen) == 0)
        {
          // нарисуем текст до префикса
          int DrawLen = ptr-txptr;
          Draw( Row, Col + sclen, Color, txptr, DrawLen );
          sclen += DrawLen;
          txptr += DrawLen;

          // высчитаем длинну гиперссылки
          const char * end = ptr + HyperlinkPrefixesLen;
          while ( *end && (FarSF::LIsAlphanum(*end) ||
            *end == '/' || *end == '%' || *end == '~' ||
            *end == '.' || *end == '_' || *end == '@' ||
            *end == '-' || *end == '+' || *end == '#' ||
            *end == '?' || *end == '=' || *end == '&' ||
            *end == ':' ) ) end ++;

          DrawLen = end-ptr;

          // если попал только префикс, тогда ничего делать не будем
          if (DrawLen - HyperlinkPrefixesLen > 0)
          {
            // всё в ажуре, рисуем гиперссылку
            Draw(Row, Col + sclen, MVColor(CColorCfg::iclHyperlink), ptr, DrawLen);
            sclen += DrawLen;

            ptr = txptr = end;
          }

          break;
        }
      }
    }

    if (*ptr)
      prevchar = *ptr++;
  }

  // дорисуем остатки текста
  if ( *txptr )
  {
    Draw( Row, Col + sclen, Color, txptr );
    sclen += strlen( txptr );
  }

/* Draw() должен сам справиться с проблемой прорисовки последних пробелов
  UINT splen = strlen( Text ) - sclen;
  if ( splen )
  {
    memset( buf, '\x20', splen ); buf[ splen ] = '\0';
    Draw( Row, Col + sclen, Color, buf );
  }
*/
}
