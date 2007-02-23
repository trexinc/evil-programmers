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
#ifndef ___MailViewDlg_H___
#define ___MailViewDlg_H___

#include "FarDialogEx.h"
#include "MsgLib/MsgLib.h"

#ifndef FCT__INVALID
#define FCT__INVALID -1
#endif

//#define _USE_TTF_SUPPORT

void ShowConsoleCursor( CONST BOOL bVisible );

//////////////////////////////////////////////////////////////////////////
class CTextViewControl : public CFarUserControl
{
private:
  CFarDialog     * m_Dialog;
  FarStringArray * m_Strings;
  int  m_TopString;
  int  m_LeftColumn;
  bool m_NeedRedraw;
  bool m_WordWrap;
  long m_Encoding;
  long m_EncodingDef;
#ifdef _USE_TTF_SUPPORT
  bool m_isTTF;
#endif

  void Clear( int Size );
  void Clear();
protected:
  void Draw( int Row, int Col, int Color, LPCSTR Text );
  void Draw( int Row, int Col, int Color, LPCSTR Text, int TextLen );
  virtual void PaintLine( int Row, int LineNo );
private:
  void Paint();

  void UpdateScrollbar();

  void LineUp();
  void LineDown();
  void Top();
  void Bottom();
  void PageUp();
  void PageDown();
  void CharLeft();
  void CharRight();
  void Home();
  void End();
  void FileBegin();
  void FileEnd();

  virtual void Update();
  virtual bool Perform( int Msg, long Param, long * Result );
protected:
  FarStringArray * GetStrings()
  {
    return m_Strings;
  }
  FarString m_FileName;
  bool      m_bDeleteOnDestroy;
  virtual void Reload();
public:
  CTextViewControl( CFarDialog * Dlg ) : CFarUserControl(),
    m_Dialog( Dlg ),
    m_Strings( NULL ),
    m_TopString( 0 ),
    m_LeftColumn( 0 ),
    m_NeedRedraw( true ),
    m_WordWrap( false ),
    m_Encoding( FCT__INVALID ),
    m_EncodingDef( FCT__INVALID ),
    OnChangeEncodingFunc( NULL )
  {
    m_Strings = create FarStringArray( true );
#ifdef _USE_TTF_SUPPORT
    extern LPCSTR strstri( LPCSTR str1, LPCSTR str2 );
    m_isTTF = strstri( GetCommandLineA(), " /ttf" ) ? true : false;
#endif
  }
  virtual ~CTextViewControl()
  {
    if ( m_vBuf )
      delete [] m_vBuf;
    if ( m_Strings )
      delete m_Strings;

    if ( m_bDeleteOnDestroy )
    {
      SetFileAttributes( m_FileName, FILE_ATTRIBUTE_NORMAL );
      DeleteFile( m_FileName );
    }

    ShowConsoleCursor( TRUE );
  }
  void ForcePaint()
  {
    m_NeedRedraw = true;
    Paint();
  }
  bool GetWordWrap() const { return m_WordWrap; }
  void SetWordWrap( bool Value );

  void setDefaultEncoding( long encoding )
  {
    m_EncodingDef = encoding;
  }

  void SetEncoding( long FarTable, bool bReload = true )
  {
    if ( FarTable == m_Encoding )
      return;
    m_Encoding = FarTable;
    if ( m_EncodingDef == FCT__INVALID && m_Encoding != FCT__INVALID )
      m_EncodingDef = m_Encoding;
    if ( bReload )
      Reload();
    if ( OnChangeEncodingFunc )
      OnChangeEncodingFunc( m_Dialog );
  }
  long GetEncoding()
  {
    return m_Encoding;
  }
  void LoadFromFile( LPCSTR FileName, bool bDeleteOnDestoy )
  {
    m_FileName         = FileName;
    m_bDeleteOnDestroy = bDeleteOnDestoy;
    Reload();
  }

  void (__cdecl *OnChangeEncodingFunc)( CFarDialog * data );
};

class CMailViewControl : public CTextViewControl
{
private:
  int m_SignatureStart;
  int m_OriginLine;
  int m_TearlineLine;
  int m_TaglineLine;
  FarString m_XFTNOrigin, m_XFTNTearline;

  long m_defaultEncoding;

  bool IsSignature( int LineNo );
  bool IsOrigin( int LineNo );
  bool IsTearline( int LineNo );
  bool IsTagline( int LineNo );
  void StyleCodeHighlight( LPCSTR Text, int Row, int Col, int Color, bool dohide );
  virtual void Reload();
  virtual void PaintLine( int Row, int LineNo );
public:
  CMailViewControl( CFarDialog * Dlg, long defaultEncoding )
    : CTextViewControl( Dlg )
    , m_SignatureStart( -1 )
    , m_OriginLine( -1 )
    , m_TearlineLine( -1 )
    , m_defaultEncoding( defaultEncoding )
  {
  }
  virtual ~CMailViewControl()
  {
  }

  virtual void LoadStrings( PMessage pMsg );
};

class CFileViewDialog : public CFarDialog
{
private:
  bool m_bNeedReopen;
  bool m_FullWnd;
protected:
  COORD m_WndSize;
  far_msg bool OnKey( TDnKey & Item );

  CTextViewControl * m_View;

  CFileViewDialog( bool bFullWnd );

  virtual bool OnClose( int ItemId, LPBOOL bAllow )
  {
    return true;
  }

public:
  virtual ~CFileViewDialog();
  CFarDlgItem * Execute( bool * pbWordWrap, const int X = -1, const int Y = -1 );

FAR_BEGIN_MESSAGE_MAP
    FAR_MESSAGE_HANDLER( DN_KEY, TDnKey, OnKey )
FAR_END_MESSAGE_MAP( CFarDialog )
};

class CMailViewDialog : public CFileViewDialog
{
private:
  void CalcHeader( int * HeadersCount, int * MaxHeaderLen );
  CFarEdit * CreateHeaderControl( int Top, int HeaderId, int HeaderLen, const FarString& Text, LPCSTR DateText = NULL );

  bool     m_OrigDel;
  PMessage m_OrigMsg;

  CFarEdit * m_edtArea;
  CFarEdit * m_edtFrom;
  CFarEdit * m_edtTo;
  CFarEdit * m_edtSubject;
  CFarEdit * m_edtCc;

  void Create( LPCSTR FileName, long defaultEncoding );

  virtual bool OnDraw( LPBOOL result );

  static void __cdecl changeHeadersEncoding( CFarDialog * dlg );


//  far_msg bool OnCtlColorDlgItem( TDnColor & Item );
//FAR_BEGIN_MESSAGE_MAP
//  FAR_MESSAGE_HANDLER( DN_CTLCOLORDLGITEM, TDnColor, OnCtlColorDlgItem )
//FAR_END_MESSAGE_MAP( CFileViewDialog )
public:
  CMailViewDialog( PMessage Msg, bool bFullWnd, long defaultEncoding );
  CMailViewDialog( LPCSTR FileName, bool bFullWnd, long defaultEncoding );
  virtual ~CMailViewDialog();
};

#endif //!defined(___MailViewDlg_H___)
