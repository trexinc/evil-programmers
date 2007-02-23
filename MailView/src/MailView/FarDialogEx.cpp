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
#include "FarPlugin.h"
#include "FarDialogEx.h"

long WINAPI CFarDialog::DialogProc( Far::HDIALOG hDlg, int Msg, int Param1, long Param2 )
{
  CFarDialog * dlg = (CFarDialog*)hDlg->getData();

  far_assert( dlg != NULL );

  TMessage m = { (FarMessagesProc)Msg, Param1, Param2, 0 };

  if ( Msg == DN_INITDIALOG )
  {
    //dlg = (CFarDialog*)Param2;
    dlg->m_Handle = hDlg;
    m.Param2 = 0;
    for ( int i = 0; i < dlg->m__Items->Count(); i ++ )
    {
      hDlg->setItemData( i, dlg->m__Items->At( i ) );
    }
    if ( !dlg->OnCreate( Param1 ) )
      return hDlg->defDlgProc( Msg, Param1, Param2 );
  }

  if ( !dlg->Dispatch( m ) )
    return hDlg->defDlgProc( Msg, Param1, Param2 );

  return m.Result;
}
//#include <FarLog.h>
bool CFarDialog::Dispatch( TMessage & Msg )
{
/*  FarFileName lfn(Far::GetModuleName());
  lfn.SetExt( ".log" );
  FarLog log( lfn, 3 );
  log.Message( "Msg=0x%08X, Param1=0x%08X, Param2=0x%08X\n", Msg.Msg, Msg.Param1, Msg.Param2 );
*/
  switch ( Msg.Msg )
  {
  case DN_CTLCOLORDIALOG:                                         // new in 1.70 beta 1
    if ( m_Color == -1 )
      return false;
    Msg.Result = m_Color;
    return true;

  case DN_CLOSE:                                                  // new in 1.70 beta 1
    Msg.Result = TRUE;
    return OnClose( Msg.Param1, (LPBOOL)&Msg.Result );

  case DN_DRAWDIALOG:                                             // new in 1.70 beta 1
    Msg.Result = TRUE;
    return OnDraw( (LPBOOL)&Msg.Result );

  case DN_ENTERIDLE:                                              // new in 1.70 beta 1
    return OnEnterIdle();

  case DN_CTLCOLORDLGLIST:                                        // new in 1.70 beta 1
    return false;

  case DN_BTNCLICK:                                               // new in 1.70 beta 1
  case DN_DRAWDLGITEM:                                            // new in 1.70 beta 1
  case DN_EDITCHANGE:                                             // new in 1.70 beta 1
  case DN_HOTKEY:                                                 // new in 1.70 beta 1
  case DN_LISTCHANGE:                                             // new in 1.70 beta 1
    Msg.Result = TRUE;
    break;

  case DN_CTLCOLORDLGITEM:                                        // new in 1.70 beta 1
    Msg.Result = Msg.Param2;
    break;

  case DN_HELP:                                                   // new in 1.70 beta 1
    Msg.Result = 0;
    break;

  case DN_KEY:                                                    // new in 1.70 beta 1
    Msg.Result = FALSE;
    break;

  case DN_MOUSECLICK:                                             // new in 1.70 beta 1
    Msg.Result = FALSE;
    if ( Msg.Param1 = -1 )
      return false;
    break;

  case DN_GOTFOCUS:                                               // new in 1.70 beta 1
    Msg.Result = TRUE;
    break;

  case DN_KILLFOCUS:                                              // new in 1.70 beta 1
    Msg.Result = -1;
    break;

  case DN_DRAGGED:                                                // new in 1.70 beta 4
    return false;
  case DN_MOUSEEVENT:                                             // new in 1.70 beta 4
    return false;
  case DN_RESIZECONSOLE:                                          // new in 1.70 beta 4
    return false;


  default:
    return false;
  }

#if (FARMANAGERVERSION <= MAKEFARVERSION(1,70,1282))
#pragma message ("CFarDialog::Dispatch - using checking stub...")
  if ( Msg.Param1 < 0 || Msg.Param1 >= m__Items->Count() ) // fixed in build >= 1351
    return false;
#endif
  CFarDlgItem * Item = (CFarDlgItem*)SendMessage( DM_GETITEMDATA, Msg.Param1, 0 );
  return Item ? Item->Perform( Msg.Msg, Msg.Param2, &Msg.Result ) : NULL;
}

CFarDialog::CFarDialog() : CFarControl(),
  m_Handle( NULL ),
  m_DefaultControl( NULL ),
  m_FocusControl( NULL ),
  m_Color( -1 ),
  m_Flags( 0 ),
  m__Items( NULL )
{
  BeginUpdate();
  CFarControl::InsertControl( create CFarGroupBox( bsDouble ) );
  SetBorderOrigin( 3, 1 );
  SetWidth( 80 );
  SetHeight( 25 );
  EndUpdate();
}

CFarDialog::CFarDialog( const int Width, const int Height ) : CFarControl(),
  m_Handle( NULL ),
  m_DefaultControl( NULL ),
  m_FocusControl( NULL ),
  m_Color( -1 ),
  m_Flags( 0 ),
  m__Items( NULL )
{
  BeginUpdate();
  CFarControl::InsertControl( create CFarGroupBox( bsDouble ) );
  SetBorderOrigin( 3, 1 );
  SetWidth( Width );
  SetHeight( Height );
  EndUpdate();
}

CFarDialog::CFarDialog( const FarString& Title,  const int Width, const int Height )
  : CFarControl(),
  m_Handle( NULL ),
  m_DefaultControl( NULL ),
  m_FocusControl( NULL ),
  m_Color( -1 ),
  m_Flags( 0 ),
  m__Items( NULL )
{
  BeginUpdate();
  CFarControl::InsertControl( create CFarGroupBox( bsDouble ) );
  SetBorderOrigin( 3, 1 );
  SetWidth( Width );
  SetHeight( Height );
  SetTitle( Title );
  EndUpdate();
}

CFarDialog::~CFarDialog()
{
}

int CFarDialog::GetLeft()
{
  if ( m_Handle )
  {
    SMALL_RECT R;
    SendMessage( DM_GETDLGRECT, 0, (long)&R );
    return R.Left;
  }
  return 0;
}

int CFarDialog::GetTop()
{
  if ( m_Handle )
  {
    SMALL_RECT R;
    SendMessage( DM_GETDLGRECT, 0, (long)&R );
    return R.Top;
  }
  return 0;
}

int CFarDialog::GetWidth()
{
  if ( m_Handle )
  {
    SMALL_RECT R;
    SendMessage( DM_GETDLGRECT, 0, (long)&R );
    return R.Right - R.Left;
  }
  return CFarControl::GetWidth();
}

int CFarDialog::GetHeight()
{
  if ( m_Handle )
  {
    SMALL_RECT R;
    SendMessage( DM_GETDLGRECT, 0, (long)&R );
    return R.Bottom - R.Top;
  }
  return CFarControl::GetHeight();
}

void CFarDialog::SetLeft( const int Value )
{
  if ( m_Handle )
  {
    COORD c = { Value, GetTop() };
    SendMessage( DM_MOVEDIALOG, TRUE, (long)&c );
    CFarControl::SetLeft( Value );
  }
}

void CFarDialog::SetTop( const int Value )
{
  if ( m_Handle )
  {
    COORD c = { GetLeft(), Value };
    SendMessage( DM_MOVEDIALOG, TRUE, (long)&c );
    CFarControl::SetTop( Value );
  }
}

void CFarDialog::SetWidth( const int Value )
{
  if ( m_Handle )
  {
    far_assert( CFarDialog::SetWidth != NULL );
  }
  CFarControl::SetWidth( Value );
}

void CFarDialog::SetHeight( const int Value )
{
  if ( m_Handle )
  {
    far_assert( CFarDialog::SetHeight != NULL );
  }
  CFarControl::SetHeight( Value );
}

void CFarDialog::MakeDlgItem( CFarDlgItem * Src, FarDialogItem * Dst )
{
  far_assert( Src != NULL && Dst != NULL );

  Dst->Type          = Src->GetType();
  Dst->X1            = Src->GetLeft();
  Dst->Y1            = Src->GetTop();
  Dst->X2            = Dst->X1 + Src->GetWidth() - 1;
  Dst->Y2            = Dst->Y1 + Src->GetHeight() - 1;
  Dst->Focus         = Src == m_FocusControl ? TRUE : FALSE;
  Dst->Selected      = Src->m_Selected;
  Dst->Flags         = Src->m_FarFlags;
  Dst->DefaultButton = Src == m_DefaultControl ? TRUE : FALSE;
  Src->GetData( Dst->Data );

  Src->ClientToScreen( &Dst->X1, &Dst->Y1 );
  Src->ClientToScreen( &Dst->X2, &Dst->Y2 );
}

void CFarDialog::MakeDlgItems( CFarDlgItem * Group, FarDataArray<FarDialogItem> & DlgItems )
{
  far_assert( Group != NULL );

  FarDialogItem Item;
  MakeDlgItem( Group, &Item );

  DlgItems.Add( Item );
  m__Items->Add( Group );

  for ( int i = 0; i < Group->GetControlCount(); i ++ )
    MakeDlgItems( (CFarDlgItem*)Group->GetControl( i ), DlgItems );
}

CFarDlgItem * CFarDialog::Execute( const int X, const int Y )
{
  FarDataArray<FarDialogItem> Items;
  FarArray    < CFarDlgItem > ItemsData;
  ItemsData.SetOwnsItems( false );

  m__Items = &ItemsData;

  MakeDlgItems( GetBorder(), Items );

  int X2 = GetWidth (); if ( X != -1 ) X2 += X;
  int Y2 = GetHeight(); if ( Y != -1 ) Y2 += Y;

  int res = Far::Dialog( X, Y, X2, Y2, m_HelpTopic.data(),
    Items.GetItems(), Items.Count(), m_Flags, DialogProc, this );

  m_Handle = NULL;
  m__Items = NULL;

  return res != -1 ? ItemsData[ res ] : NULL;
}

long CFarDialog::SendMessage( int Msg, int Param1, long Param2 )
{
  far_assert( m_Handle != NULL );
  if ( Msg == DM_SETDLGDATA || Msg == DM_GETDLGDATA )
  {
    return 0;
  }
  return m_Handle->sendMessage( Msg, Param1, Param2 );
}

CFarControl::CFarControl() : m_Parent( NULL ),
  m_Left( 0 ), m_Top( 0 ), m_Width( 0 ), m_Height( 0 )

{
}

CFarControl::~CFarControl()
{
}

int CFarControl::GetLeft() const
{
  return m_Left;
}

int CFarControl::GetTop() const
{
  return m_Top;
}

int CFarControl::GetWidth() const
{
  return m_Width;
}

int CFarControl::GetHeight() const
{
  return m_Height;
}

void CFarControl::SetLeft( int Value )
{
//  far_assert( Value >= 0 );
  m_Left = Value;
}

void CFarControl::SetTop( int Value )
{
//  far_assert( Value >= 0 );
  m_Top = Value;
}

void CFarControl::SetWidth( int Value )
{
  far_assert( Value >= 0 );
  m_Width = Value;
}

void CFarControl::SetHeight( int Value )
{
  far_assert( Value >= 0 );
  m_Height = Value;
}

CFarDlgItem::CFarDlgItem() : CFarControl(),
  m_FarFlags( 0 ),
  m_Selected( 0 ),
  m_Color( -1 )
{
}

CFarDlgItem::~CFarDlgItem()
{
}

void CFarDlgItem::GetData( char Data[512] )
{
  *Data = '\0';
}

bool CFarDlgItem::GetVisible() const
{
  return (m_FarFlags & DIF_HIDDEN) != 0;
}

void CFarDlgItem::SetVisible( bool Value )
{
  if ( Value )
    m_FarFlags &= ~DIF_HIDDEN;
  else
    m_FarFlags |= DIF_HIDDEN;
}

bool CFarDlgItem::GetEnabled() const
{
  return (m_FarFlags & DIF_DISABLE) != 0;
}

void CFarDlgItem::SetEnabled( bool Value )
{
  if ( Value )
    m_FarFlags &= ~DIF_DISABLE;
  else
    m_FarFlags |= DIF_DISABLE;
}

bool CFarDlgItem::GetTabStop() const
{
  return (m_FarFlags & DIF_NOFOCUS) != 0;
}

void CFarDlgItem::SetTabStop( bool Value )
{
  if ( Value )
    m_FarFlags &= ~DIF_NOFOCUS;
  else
    m_FarFlags |= DIF_NOFOCUS;
}

void CFarSeparator::GetData( char Data[512] )
{
  const char SEP_DATA[] =
  {
    '\xC4', '\xC4', '\xC4', // h none
    '\xC3', '\xB4', '\xC4', // h single
    '\xC7', '\xB6', '\xC4', // h double
    '\xB3', '\xB3', '\xB3', // v none
    '\xC2', '\xC1', '\xB3', // v single
    '\xD1', '\xCF', '\xB3', // v double
  };

  far_assert( GetParent() != NULL );

  int Len, DataStart;

  switch ( GetParent()->GetBorderStyle() )
  {
  case bsDouble:
    DataStart = 6;
    break;
  case bsSingle:
    DataStart = 3;
    break;
  default:
    DataStart = 0;
    break;
  }

  if ( GetStyle() == tsVertical )
  {
    Len = GetParent()->GetHeight();
    DataStart += 9;
  }
  else
    Len = GetParent()->GetWidth();

  if ( Len > 511 )
    Len = 511;

  Data[ Len-- ] = 0;

  Data[  0  ] = SEP_DATA[ DataStart++ ];
  Data[ Len ] = SEP_DATA[ DataStart++ ];

  memset( Data + 1, SEP_DATA[ DataStart ], Len - 1 );

  if ( m_Data.Length() > 0 )
  {
    if ( GetStyle() == tsVertical )
      DataStart = CFarText::GetTop();
    else
      DataStart = CFarText::GetLeft();

    memcpy( Data + DataStart, m_Data.c_str(), min( Len - DataStart, m_Data.Length() ) );
  }
}

void CFarScrollBar::GetData( char Data[512] )
{
  const char BAR_DATA[] =
  {
    '\x11', '\x10', // h
    '\x1E', '\x1F', // v
  };

  int Len, DataStart;

  if ( GetStyle() == tsVertical )
  {
    Len = GetHeight();
    DataStart = 2;
  }
  else
  {
    Len = GetWidth();
    DataStart = 0;
  }

  if ( Len > 511 )
    Len = 511;
  else if ( Len < 3 )
    Len = 3;

  Data[ Len-- ] = 0;

  Data[  0  ] = BAR_DATA[ DataStart + 0 ];
  Data[ Len ] = BAR_DATA[ DataStart + 1 ];

  memset( Data + 1, '\xB1', Len - 1 );
}

void CFarDialog::UpdateControl( CFarDlgItem * item )
{
  if ( m__Items == NULL || m_Handle == NULL )
  {
    return;
  }
  // найдем контрол
  int id = 0;
  for ( ; id < m__Items->Count(); id ++ )
  {
    CFarDlgItem * find = (CFarDlgItem*)m_Handle->getItemData( id );
    if ( find == item )
      break;
  }

  if ( id < m__Items->Count() )
  {
    FarDialogItem dest;

    MakeDlgItem( item, &dest );

    m_Handle->setItem( id, &dest );
  }
}
