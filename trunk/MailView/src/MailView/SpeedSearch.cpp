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
#include "StdAfx.h"
#include "../farkeys.hpp"
#include "SpeedSearch.h"

#define _countof( a ) sizeof( a ) / sizeof( a[ 0 ] )

extern void DbgMsg( char const * const Title, char const * const Fmt, ... );
extern void DbgMsg( char const * const Fmt, ... );

#define EDT_ID 1

long WINAPI FarSpeedSearch::DlgProc( Far::HDIALOG dlg, int msg, int param1, long param2 )
{
  if ( msg != DN_CLOSE && dlg->getItemData( EDT_ID ) )
    dlg->close( EDT_ID );

  if ( msg == DM_DEACTIVATE || msg == DN_MOUSECLICK )
    return dlg->close( -2 );

  if ( msg == DN_KEY )
  {
    //*(int*)FarSendDlgMessage( hDlg, DM_GETDLGDATA, 0, 0 ) = Param2;

    if ( param2 == KEY_CTRLENTER )
      return dlg->close( 0 );

    if ( param2 >= KEY_F1 )
      return FALSE;

    if ( param2 & (~(0xFF|KEY_ALT|KEY_RALT|KEY_SHIFT)) || param2 == KEY_ENTER || param2 == KEY_TAB )
    {
      return dlg->close( -2 );
    }

    if ( param2 & (KEY_ALT|KEY_RALT|KEY_SHIFT) &&
      FarSF::LIsAlphanum( param2 & 0xFF ) )
    {
      param2 = FarSF::LLower( param2 & 0xFF );

      DWORD keys[] = { KEY_END, param2 };
      dlg->postKey( keys, _countof( keys ) );

      return dlg->close( EDT_ID );
    }

    dlg->setItemData( EDT_ID, 1 ); // draw char and exit

    return FALSE;
  }

  if ( msg == DN_INITDIALOG )
    dlg->setEditUnchanged( EDT_ID, false );

  return dlg->defDlgProc( msg, param1, param2 );
}

#define STR_SEARCH "Search"

FarSpeedSearch::FarSpeedSearch( PFarCustomPanelPlugin panel, TSelect selectFunc )
  : m_panel( panel )
  , m_title( STR_SEARCH )
  , select( selectFunc )
{
}

FarSpeedSearch::FarSpeedSearch( PFarCustomPanelPlugin panel, TSelect selectFunc, const FarString& title )
  : m_panel( panel )
  , m_title( title )
  , select( selectFunc )

{
  if ( m_title.IsEmpty() )
    m_title = STR_SEARCH;
}

FarSpeedSearch::FarSpeedSearch( PFarCustomPanelPlugin panel, TSelect selectFunc, int title )
  : m_panel( panel )
  , m_title( Far::GetMsg( title ) )
  , select( selectFunc )
{
  if ( m_title.IsEmpty() )
    m_title = STR_SEARCH;
}

FarSpeedSearch::~FarSpeedSearch()
{
}

void FarSpeedSearch::Execute( int firstKey )
{
  if ( select == NULL )
    return;

  PanelInfo PInfo = m_panel->getInfo();

  if ( PInfo.PanelType != PTYPE_FILEPANEL || PInfo.Plugin == 0 )
    return;

  FarDialogItem items[ 2 ];
  memset( items, 0, sizeof( items ) );

  items[ 0 ].Type = DI_DOUBLEBOX;
  items[ 0 ].X2   = 21;
  items[ 0 ].Y2   = 2;
  strcpy( items[ 0 ].Data, m_title );

  items[ EDT_ID ].Type = DI_EDIT;
  items[ EDT_ID ].X1   = 2;
  items[ EDT_ID ].Y1   = 1;
  items[ EDT_ID ].X2   = 19;


  /*items[ 1 ].Type =
  items[ 3 ].Type = DI_TEXT;
  items[ 1 ].Y1   =
  items[ 3 ].Y1   = 1;
  items[ 1 ].X1   = 1;
  items[ 3 ].X1   = 20;
  items[ 1 ].Data[ 0 ] =
  items[ 3 ].Data[ 0 ] = '\x20';

  items[ 2 ].Type = DI_EDIT;
  items[ 2 ].X1   = 2;
  items[ 2 ].Y1   = 1;
  items[ 2 ].X2   = 19;*/

  int lastKey = FarSF::LLower( firstKey & 0xFF );

  items[ EDT_ID ].Data[ 0 ] = lastKey;

  PanelInfo AInfo = m_panel->getAnotherInfo();

//  GetConsoleTitle( ConsoleTitle.GetBuffer( 1024 ), 1024 );
//  ConsoleTitle.ReleaseBuffer();

  for ( int res = 1; res >= 0; )
  {
    int currentItem = PInfo.CurrentItem;

    if ( res == 0 )
      currentItem ++;

    currentItem = select( items[ EDT_ID ].Data,
      PInfo.PanelItems, PInfo.ItemsNumber, currentItem );

    if ( currentItem == -1 )
    {
      items[ EDT_ID ].Data[ strlen( items[ EDT_ID ].Data ) - 1 ] = '\0';
    }
    else
    {
      PInfo.CurrentItem  = currentItem;
      PInfo.TopPanelItem = currentItem -
        ( PInfo.PanelRect.bottom - PInfo.PanelRect.top ) / 2;

      /*PanelRedrawInfo RInfo;
      RInfo.CurrentItem  = PInfo.CurrentItem;
      RInfo.TopPanelItem = PInfo.TopPanelItem;
      m_Ctrl.RedrawPanel( &RInfo );*/
      m_panel->redraw( (PanelRedrawInfo*)&PInfo.CurrentItem );

      if ( AInfo.PanelType == PTYPE_QVIEWPANEL || AInfo.PanelType == PTYPE_INFOPANEL )
        m_panel->updateAnother();
    }

    lastKey = 0;

    int X1 = PInfo.PanelRect.left + 9;
    int Y1 = PInfo.PanelRect.bottom;
    int X2 = X1 + 22 - 1;
    int Y2 = Y1 + 3 - 1;

    res = Far::Dialog( X1, Y1, X2, Y2, ":FastFind", items,
      _countof( items ), FDLG_SMALLDIALOG, DlgProc, &lastKey );
  }

  if ( lastKey != KEY_F10 )
  {
    KeySequence ks = { 0, 1, (LPDWORD)&lastKey };
    /*ks.Flags=0;ks.Count=1;ks.Sequence=(LPDWORD)&lastKey;*/
    Far::AdvControl( ACTL_POSTKEYSEQUENCE, &ks );
  }
}
