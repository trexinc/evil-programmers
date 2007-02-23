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
#include "FarColorDialog.h"
#include "LangID.h"

/*
       000000000011111111112222222222333333333
       012345678901234567890123456789012345678
  ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ
  ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ
  ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ
 0ÛÛÛÛÛ                                       ÛÛÛÛÛ 0
 1ÛÛÛÛÛ   ÉÍÍÍÍÍÍÍÍÍÍÍÍ Color ÍÍÍÍÍÍÍÍÍÍÍÍ»   ±±ÛÛÛ 1
 2ÛÛÛÛÛ   º Ú Foreground ¿ Ú Background ¿ º   ±±ÛÛÛ 2
 3ÛÛÛÛÛ   º ³  °°°±±±²²²³ ³  °°°±±±²²²³ º   ±±ÛÛÛ 3
 4ÛÛÛÛÛ   º ³²²²ÛÛÛ°°°±±±³ ³²²²ÛÛÛ°°°±±±³ º   ±±ÛÛÛ 4
 5ÛÛÛÛÛ   º ³±±±²²²ÛÛÛ°°°³ ³±±±²²²ÛÛÛ°°°³ º   ±±ÛÛÛ 5
 6ÛÛÛÛÛ   º ³°°°±±±²²²ÛÛÛ³ ³°°°±±±²²²ÛÛÛ³ º   ±±ÛÛÛ 6
 7ÛÛÛÛÛ   º ÀÄÄÄÄÄÄÄÄÄÄÄÄÙ ÀÄÄÄÄÄÄÄÄÄÄÄÄÙ º   ±±ÛÛÛ 7
 8ÛÛÛÛÛ   º Text Text Text Text Text Text º   ±±ÛÛÛ 8
 9ÛÛÛÛÛ   º Text Text Text Text Text Text º   ±±ÛÛÛ 9
10ÛÛÛÛÛ   º Text Text Text Text Text Text º   ±±ÛÛÛ10
11ÛÛÛÛÛ   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   ±±ÛÛÛ11
12ÛÛÛÛÛ   º      [ Set ]  [ Cancel ]      º   ±±ÛÛÛ12
13ÛÛÛÛÛ   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   ±±ÛÛÛ13
14ÛÛÛÛÛ                                       ±±ÛÛÛ14
  ÛÛÛÛÛÛÛ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±ÛÛÛ
  ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ
  ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ
       000000000011111111112222222222333333333
       012345678901234567890123456789012345678
*/

int ToDlgColor( const int cl )
{
  int mat[ 16 ] =
  {
    0x0,0x4,0x8,0xC,
    0x1,0x5,0x9,0xD,
    0x2,0x6,0xA,0xE,
    0x3,0x7,0xB,0xF
  };
  return mat[ cl ];
}

CFarColorDialog::CFarColorDialog( const int Color )
: CFarDialog( Far::GetMsg( MColorDlg_Color ), 39, 15 ), m_Color( Color )
{
  CFarRadioGroup * Grp[ 2 ];

  Grp[ 0 ] = create CFarRadioGroup( bsSingle );
  Grp[ 1 ] = create CFarRadioGroup( bsSingle );

  Grp[ 0 ]->SetBounds(  1, 0, 14, 6 );
  Grp[ 1 ]->SetBounds( 16, 0, 14, 6 );

  Grp[ 0 ]->SetTitle( Far::GetMsg( MColorDlg_Foreground ) );
  Grp[ 1 ]->SetTitle( Far::GetMsg( MColorDlg_Background ) );

  int i;

  for ( i = 0; i < 2; i ++ )
  {
    Grp[ i ]->AddItem( 0, 0 )->SetColor( 0x07 );
    Grp[ i ]->AddItem( 0, 1 )->SetColor( 0x40 );
    Grp[ i ]->AddItem( 0, 2 )->SetColor( 0x87 );
    Grp[ i ]->AddItem( 0, 3 )->SetColor( 0xC7 );

    Grp[ i ]->AddItem( 3, 0 )->SetColor( 0x17 );
    Grp[ i ]->AddItem( 3, 1 )->SetColor( 0x50 );
    Grp[ i ]->AddItem( 3, 2 )->SetColor( 0x90 );
    Grp[ i ]->AddItem( 3, 3 )->SetColor( 0xD0 );

    Grp[ i ]->AddItem( 6, 0 )->SetColor( 0x20 );
    Grp[ i ]->AddItem( 6, 1 )->SetColor( 0x60 );
    Grp[ i ]->AddItem( 6, 2 )->SetColor( 0xA0 );
    Grp[ i ]->AddItem( 6, 3 )->SetColor( 0xE0 );

    Grp[ i ]->AddItem( 9, 0 )->SetColor( 0x30 );
    Grp[ i ]->AddItem( 9, 1 )->SetColor( 0x70 );
    Grp[ i ]->AddItem( 9, 2 )->SetColor( 0xB0 );
    Grp[ i ]->AddItem( 9, 3 )->SetColor( 0xF0 );

    for ( int j = 0; j < Grp[ i ]->GetControlCount(); j ++ )
      ((CFarRadioButton*)Grp[ i ]->GetControl( j ))->SetMoveSelect( true );

    InsertControl( Grp[ i ] );
  }

  m_SampleTextBox = create CFarBox( bsNone );
  for ( i = 6; i < 9; i ++ )
  {
    CFarText * SampleText = create CFarText;
    SampleText->SetLeft( 1 );
    SampleText->SetTop( i );
    SampleText->SetText( "Text Text Text Text Text Text" );
    SampleText->SetColor( m_Color );
    m_SampleTextBox->InsertControl( SampleText );
  }
  InsertControl( m_SampleTextBox );

  CFarSeparator * Separator = create CFarSeparator;
  Separator->SetTop( i++ );
  InsertControl( Separator );

  CFarGroupBox * ButtonSet = create CFarGroupBox;
  ButtonSet->SetCentered( true );
  ButtonSet->SetTop( i );
  InsertControl( ButtonSet );

  CFarButton * btnSet = create CFarButton;
  btnSet->SetCaption( Far::GetMsg( MColorDlg_Set ) );
  ButtonSet->AddItem( btnSet );

  CFarButton * btnCancel = create CFarButton;
  btnCancel->SetCaption( MCancel );
  ButtonSet->AddItem( btnCancel );

  SetDefaultControl( btnSet );

  // set color
  CFarRadioButton * btn;
  btn = (CFarRadioButton*)Grp[ 1 ]->GetControl( ToDlgColor((m_Color&0xF0)>>4) );
  btn->SetSelected( true );
  btn = (CFarRadioButton*)Grp[ 0 ]->GetControl( ToDlgColor((m_Color&0x0F)>>0) );
  btn->SetSelected( true );
  SetFocusControl( btn );
}
/*
bool CFarColorDialog::OnCtlColorDlgItem( TDnColor & Item )
{
  if ( Item.Id >= 35 && Item.Id <= 37 ) // Sample Text
  {
    FarDialogItem di;

    for ( int i = 0; i < 16; i ++ )
    {
      SendMessage( DM_GETDLGITEM, i + ID_Foreground_Start, (long)&di );
      if ( di.Selected )
        break;
    }

    Item.Result = (di.Flags&0xF0)>>4;

    for ( i = 0; i < 16; i ++ )
    {
      SendMessage( DM_GETDLGITEM, i + ID_Background_Start, (long)&di );
      if ( di.Selected )
        break;
    }

    Item.Result |= di.Flags&0xF0;
    Item.Result &= 0xFF;

    m_Color = Item.Result;
    return true;
  }

  //Item.Result = Item.Color;
  return false;
}
*/
#define ID_Foreground_Start  2
#define ID_Background_Start 19

bool CFarColorDialog::OnBtnClick( TDnBtnClick & Item )
{
  //FarDialogItem dItem; SendMessage( DM_GETDLGITEM, Item.Id, (long)&dItem );
  //if ( dItem.Type == DI_RADIOBUTTON )
  //  DbgMsg( "CFarColorDialog::OnBtnClick", "id:%d, state:%d", Item.Id, Item.State );

  if ( Item.State == 0 )
    return false;

  if ( Item.Id >= ID_Foreground_Start && Item.Id < ID_Foreground_Start + 16 )
  {
    m_Color &= 0xF0;
    m_Color |= ( GetRuntimeControl( Item.Id )->GetColor() & 0xF0 ) >> 4;
  }
  else
  if ( Item.Id >= ID_Background_Start && Item.Id < ID_Background_Start + 16 )
  {
    m_Color &= 0x0F;
    m_Color |= (GetRuntimeControl( Item.Id )->GetColor() & 0xF0 ) >> 0;
  }
  else
  return false;

  for ( int i = 0; i < m_SampleTextBox->GetControlCount(); i ++ )
    ((CFarDlgItem*)m_SampleTextBox->GetControl( i ))->SetColor( m_Color );

  return false;
}

int CFarColorDialog::GetColorValue() const
{
  return m_Color;
}

void CFarColorDialog::SetColorValue( const int Value )
{
  m_Color = Value;
}
