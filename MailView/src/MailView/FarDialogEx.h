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
#ifndef ___FarDialog_H___
#define ___FarDialog_H___

#if _MSC_VER >= 1000
#pragma once
#endif

#include "FarPlugin.h"
#include "FarMessages.h"

enum TBorderStyle { bsNone = 0, bsSingle = DI_SINGLEBOX, bsDouble = DI_DOUBLEBOX };
enum TTextStyle { tsHorizontal = DI_TEXT, tsVertical = DI_VTEXT };
enum TEditStyle { esNormal = DI_EDIT, esFixed = DI_FIXEDIT, esPassword = DI_PSWEDIT };
enum TAlign { alLeft = 0, alCenter = -1, alRight = -2, alTop = alLeft, alBottom = alRight };

class CFarDialog;
class CFarGroupBox;

//////////////////////////////////////////////////////////////////////////
// CFarControl
//
class CFarControl
{
  friend class CFarDialog;
  friend class CFarGroupBox;
private:
  int m_Left;
  int m_Top;
  int m_Width;
  int m_Height;
  CFarControl * m_Parent;
  FarArray<CFarControl> m_Controls;
protected:
  int InsertControl( CFarControl * Ctl )
  {
    far_assert( Ctl != NULL );
    Ctl->m_Parent = this;
    return m_Controls.Add( Ctl );
  }
  void RemoveControl( CFarControl * Ctl )
  {
    far_assert( Ctl != NULL );
    m_Controls.Remove( Ctl );
    Ctl->m_Parent = NULL;
  }
  void SetParent( CFarControl * Ctl )
  {
    far_assert( Ctl != NULL );
    if ( Ctl->m_Parent == this )
      return;
    if ( Ctl->m_Parent )
      Ctl->m_Parent->RemoveControl( Ctl );
    InsertControl( Ctl );
  }
  CFarControl();
public:
  virtual ~CFarControl();

  virtual int GetLeft() const;
  virtual int GetTop() const;
  virtual int GetWidth() const;
  virtual int GetHeight() const;
  virtual void SetLeft( int Value );
  virtual void SetTop( int Value );
  virtual void SetWidth( int Value );
  virtual void SetHeight( int Value );
  void SetBounds( int Left, int Top, int Width, int Height )
  {
    BeginUpdate();
    SetLeft( Left );
    SetTop( Top );
    SetWidth( Width );
    SetHeight( Height );
    EndUpdate();
  }
  CFarControl * GetParent() const
  {
    return m_Parent;
  }
  virtual void ClientToScreen( int * X, int * Y )
  {
    if ( m_Parent )
    {
      int BorderSize = m_Parent->GetBorderSize();
      if ( X )
        *X += m_Parent->GetLeft() + BorderSize;
      if ( Y )
        *Y += m_Parent->GetTop() + BorderSize;

      m_Parent->ClientToScreen( X, Y );
    }
  }
  virtual int GetControlCount() const
  {
    return m_Controls.Count();
  }
  virtual CFarControl * GetControl( int nIndex ) const
  {
    far_assert( nIndex >= 0 && nIndex < m_Controls.Count() );
    return m_Controls[ nIndex ];
  }
  virtual TBorderStyle GetBorderStyle() const
  {
    return bsNone;
  }
  virtual int GetBorderSize() const
  {
    return 0;
  }
  virtual void BeginUpdate()
  {
  }
  virtual void EndUpdate()
  {
  }
};

//////////////////////////////////////////////////////////////////////////
// CFarDlgItem
//
class CFarDlgItem : public CFarControl
{
  friend class CFarDialog;
  friend class CFarGroupBox;
private:
protected:
  DWORD m_FarFlags;
  union
  {
    int         m_Selected;
    char      * m_History;
    char      * m_Mask;
    FarList   * m_ListItems;
    CHAR_INFO * m_vBuf;
  };
  long m_Color;
  virtual void GetData( char Data[512] );
  CFarDlgItem();
public:
  virtual ~CFarDlgItem();

  virtual DialogItemTypes GetType() = 0;

  virtual bool GetVisible() const;
  virtual void SetVisible( bool Value );

  virtual bool GetEnabled() const;
  virtual void SetEnabled( bool Value );

  virtual bool GetTabStop() const;
  virtual void SetTabStop( bool Value );

  int GetColor() const
  {
    //return m_FarFlags & DIF_SETCOLOR ? m_FarFlags & 0xFF : -1;
    return m_Color&0xFF;
  }
  void SetColor( int Value )
  {
    /*if ( Value == -1 )
      m_FarFlags &= ~DIF_SETCOLOR;
    else
      m_FarFlags |= DIF_SETCOLOR | ( Value & 0xFF );
      */
    m_Color = MAKELONG(
      MAKEWORD( Value, Value ),
      MAKEWORD( Value, Value ) );
  }
  void SetColor( BYTE LoLo, BYTE LoHi, BYTE HiLo = 0, BYTE HiHi = 0 )
  {
    m_Color = MAKELONG( MAKEWORD( LoLo, LoHi ), MAKEWORD( HiLo, HiHi ) );
  }
  virtual bool Perform( int Msg, long Param, long * Result )
  {
    if ( Msg == DN_CTLCOLORDLGITEM && m_Color != -1 )
    {
      *Result = m_Color;
      return true;
    }
    return false;
  }
};

//////////////////////////////////////////////////////////////////////////
// Элемент управления, определяемый программистом.
//
class CFarUserControl : public CFarDlgItem
{
private:
  int m_UpdateCount;
  int m_Color;
protected:
  virtual void Update()
  {
  }
public:
  CFarUserControl() : CFarDlgItem(), m_UpdateCount( 0 ),
    m_Color( MAKE_COLOR( clBlack, clLtGray ) )
  {
  }
  virtual ~CFarUserControl()
  {
  }
  virtual DialogItemTypes GetType()
  {
    return DI_USERCONTROL;
  }
  virtual void BeginUpdate()
  {
    m_UpdateCount ++;
  }
  virtual void EndUpdate()
  {
    m_UpdateCount --;
    if ( m_UpdateCount <= 0 )
    {
      m_UpdateCount = 0;
      Update();
    }
  }
};

//////////////////////////////////////////////////////////////////////////
// Кнопка (Push Button)
//
class CFarButton : public CFarDlgItem
{
protected:
  FarString m_Data;
  virtual void GetData( char Data[512] )
  {
    strncpy( Data, m_Data, 512 );
  }
public:
  CFarButton() : CFarDlgItem()
  {
  }
  CFarButton( const FarString& Caption ) : CFarDlgItem()
  {
    SetCaption( Caption );
  }
  CFarButton( int Caption ) : CFarDlgItem()
  {
    SetCaption( Caption );
  }
  virtual ~CFarButton()
  {
  }
  virtual DialogItemTypes GetType()
  {
    return DI_BUTTON;
  }
  FarString GetCaption() const
  {
    return m_Data;
  }
  void SetCaption( const FarString& Value )
  {
    m_Data = Value;
  }
  void SetCaption( int Value )
  {
    m_Data = Far::GetMsg( Value );
  }
  bool GetSelected() const
  {
    return m_Selected == TRUE;
  }
  void SetSelected( bool Value )
  {
    m_Selected = Value ? TRUE : FALSE;
  }
};

//////////////////////////////////////////////////////////////////////////
// Контрольный переключатель (Check Box).
//
class CFarCheckBox : public CFarButton
{
public:
  CFarCheckBox() : CFarButton()
  {
  }
  CFarCheckBox( const FarString& Caption ) : CFarButton( Caption )
  {
  }
  CFarCheckBox( int Caption ) : CFarButton( Caption )
  {
  }
  virtual ~CFarCheckBox()
  {
  }
  virtual DialogItemTypes GetType()
  {
    return DI_CHECKBOX;
  }

  bool Get3State() const
  {
    return (m_FarFlags & DIF_3STATE) != FALSE;
  }
  void Set3State( bool newVal )
  {
    if ( newVal )
      m_FarFlags |= DIF_3STATE;
    else
      m_FarFlags &= ~DIF_3STATE;
  }

  enum CheckState { csUnchecked = 0, csChecked = 1, csUndefined = 2 };

  CheckState GetState() const
  {
    return (CheckState)m_Selected;
  }
  void SetState( CheckState newState )
  {
    m_Selected = newState;
  }
};

//////////////////////////////////////////////////////////////////////////
// Селекторная кнопка (Radio Button).
//
class CFarRadioButton : public CFarCheckBox
{
  friend class CFarRadioGroup;
public:
  CFarRadioButton() : CFarCheckBox()
  {
  }
  virtual ~CFarRadioButton()
  {
  }
  virtual DialogItemTypes GetType()
  {
    return DI_RADIOBUTTON;
  }
  bool GetMoveSelect() const
  {
    return (m_FarFlags & DIF_MOVESELECT) == 0;
  }
  void SetMoveSelect( bool Value )
  {
    if ( Value )
      m_FarFlags |= DIF_MOVESELECT;
    else
      m_FarFlags &= ~DIF_MOVESELECT;
  }
};

//////////////////////////////////////////////////////////////////////////
// Текстовая строка
//
class CFarText : public CFarDlgItem
{
private:
  TTextStyle m_Style;
protected:
  FarString m_Data;
  virtual void GetData( char Data[512] )
  {
    strncpy( Data, m_Data, 512 );
  }
public:
  CFarText( TTextStyle Style = tsHorizontal )
    : CFarDlgItem(), m_Style( Style )
  {
  }
  virtual ~CFarText()
  {
  }
  virtual DialogItemTypes GetType()
  {
    return (DialogItemTypes)m_Style;
  }
  FarString GetText() const
  {
    return m_Data;
  }
  void SetText( const FarString& Value )
  {
    m_Data = Value;
  }
  TTextStyle GetStyle() const
  {
    return m_Style;
  }
  virtual int GetLeft() const
  {
    int Left = CFarDlgItem::GetLeft();
    if ( Left == alRight )
    {
      far_assert( GetParent() != NULL );
      Left = GetParent()->GetWidth() - m_Data.Length() - GetParent()->GetBorderSize() * 2;
    }
    else if ( Left == alCenter )
    {
      far_assert( GetParent() != NULL );
      Left = ( GetParent()->GetWidth() - m_Data.Length() ) / 2 - GetParent()->GetBorderSize() * 2;
    }
    return Left;
  }
  virtual int GetTop() const
  {
    int Top = CFarDlgItem::GetTop();
    if ( Top == alBottom )
    {
      far_assert( GetParent() != NULL );
      Top = GetParent()->GetHeight() - m_Data.Length() - GetParent()->GetBorderSize() * 2;
    }
    else if ( Top == alCenter )
    {
      far_assert( GetParent() != NULL );
      Top = ( GetParent()->GetHeight() - m_Data.Length() ) / 2 - GetParent()->GetBorderSize() * 2;
    }
    return Top;
  }
};

//////////////////////////////////////////////////////////////////////////
// Поле ввода
//
class CFarEdit : public CFarText
{
private:
  TEditStyle m_Style;
protected:
public:
  CFarEdit( TEditStyle Style = esNormal ) : CFarText(), m_Style( Style )
  {
  }
  virtual ~CFarEdit()
  {
  }
  virtual DialogItemTypes GetType()
  {
    return (DialogItemTypes)m_Style;
  }
/*  FarString GetText() const
  {
    return m_Data;
  }
  void SetText( const FarString& Value )
  {
    m_Data = Value;
  }
*/  bool GetReadOnly() const
  {
    return ( m_FarFlags & DIF_READONLY ) != 0;
  }
  void SetReadOnly( const bool Value )
  {
    if ( Value && Far::GetBuildNumber() >= 687 ) // beta 3 doesn't handle DIF_READONLY correctly
      m_FarFlags |= DIF_READONLY;
    else
      m_FarFlags &= ~DIF_READONLY;
  }
  bool GetSelectOnEntry() const
  {
    return ( m_FarFlags & DIF_SELECTONENTRY ) != 0;
  }
  void SetSelectOnEntry( const bool Value )
  {
    if ( Value )
      m_FarFlags |= DIF_SELECTONENTRY;
    else
      m_FarFlags &= ~DIF_SELECTONENTRY;
  }
};


//////////////////////////////////////////////////////////////////////////
// Окно списка.
//
class CFarListBox : public CFarDlgItem
{
protected:
  FarString m_Data;
  virtual void GetData( char Data[512] )
  {
    strncpy( Data, m_Data, 512 );
  }
public:
  CFarListBox() : CFarDlgItem()
  {
  }
  virtual ~CFarListBox()
  {
  }
  virtual DialogItemTypes GetType()
  {
    return DI_LISTBOX;
  }
  FarString GetText() const
  {
    return m_Data;
  }
  void SetText( const FarString& Value )
  {
    m_Data = Value;
  }
};

//////////////////////////////////////////////////////////////////////////
// Комбинированный список.
//
class CFarComboBox : public CFarListBox
{
protected:
  virtual int GetHeight();
  virtual void SetHeight( int Value );
public:
  CFarComboBox() : CFarListBox()
  {
  }
  virtual ~CFarComboBox()
  {
  }
  virtual DialogItemTypes GetType()
  {
    return DI_COMBOBOX;
  }
};

//////////////////////////////////////////////////////////////////////////
// CFarSeparator
//
class CFarSeparator : public CFarText
{
protected:
  virtual void GetData( char Data[512] );
public:
  CFarSeparator( TTextStyle Style = tsHorizontal ) : CFarText( Style )
  {
    m_FarFlags |= DIF_SEPARATOR;
  }
  virtual ~CFarSeparator()
  {
  }
  virtual int GetLeft() const
  {
    far_assert( GetParent() != NULL );
    return GetStyle() != tsHorizontal ? CFarText::GetLeft() :  - GetParent()->GetBorderSize();
  }
  virtual int GetTop() const
  {
    far_assert( GetParent() != NULL );
    return GetStyle() != tsVertical ? CFarText::GetTop() :  - GetParent()->GetBorderSize();
  }
};

//////////////////////////////////////////////////////////////////////////
// CFarSeparator
//
class CFarScrollBar : public CFarText
{
protected:
  virtual void GetData( char Data[512] );
public:
  CFarScrollBar( TTextStyle Style ) : CFarText( Style )
  {
  }
  virtual ~CFarScrollBar()
  {
  }
};
//////////////////////////////////////////////////////////////////////////
// Рамка
//
class CFarBox : public CFarDlgItem
{
private:
  TBorderStyle m_Style;
protected:
  FarString m_Data;
  virtual void GetData( char Data[512] )
  {
    strncpy( Data, m_Data, 512 );
  }
public:
  CFarBox( TBorderStyle Style = bsDouble ) : CFarDlgItem(),
    m_Style( Style )
  {
  }
  virtual ~CFarBox()
  {
  }
  virtual DialogItemTypes GetType()
  {
    return (DialogItemTypes)m_Style;
  }
  FarString GetTitle() const
  {
    return m_Data;
  }
  int InsertControl( CFarDlgItem * Item )
  {
    return CFarControl::InsertControl( Item );
  }
  void SetTitle( const FarString& Value )
  {
    m_Data = Value;
  }
  void SetBorderStyle( TBorderStyle Style )
  {
    m_Style = Style;
  }
  virtual TBorderStyle GetBorderStyle() const
  {
    return m_Style;
  }
  virtual int GetBorderSize() const
  {
    return ( m_Style != bsNone ) ? 1 : 0;
  }
  virtual int GetWidth() const
  {
    int Width = CFarDlgItem::GetWidth();
    if ( Width > 0 )
      return Width;
    far_assert( GetParent() );
    return GetParent()->GetWidth() - GetLeft() * 2;
  }
  virtual int GetHeight() const
  {
    int Height = CFarDlgItem::GetHeight();
    if ( Height > 0 )
      return Height;
    far_assert( GetParent() );
    return GetParent()->GetHeight() - GetTop() * 2;
  }
};

//////////////////////////////////////////////////////////////////////////
// CFarGroupControl
//
class CFarGroupBox : public CFarBox
{
private:
  bool m_Centered;
protected:
public:
  CFarGroupBox( TBorderStyle BorderStyle = bsNone )
    : CFarBox( BorderStyle ), m_Centered( false )
  {
  }
  virtual ~CFarGroupBox()
  {
  }
  CFarDlgItem * AddItem( CFarDlgItem * Item )
  {
    if ( m_Centered )
      Item->m_FarFlags |= DIF_CENTERGROUP;
    InsertControl( Item );
    return Item;
  }
  bool GetCentered()
  {
    return m_Centered;
  }
  void SetCentered( bool Value )
  {
    if ( m_Centered != Value )
    {
      m_Centered = Value;

      if ( m_Centered )
      {
        for ( int i = 0; i < m_Controls.Count(); i ++ )
        {
          CFarDlgItem * Item = (CFarDlgItem*)m_Controls[ i ];
          Item->m_FarFlags |= DIF_CENTERGROUP;
        }
      }
      else
      {
        for ( int i = 0; i < m_Controls.Count(); i ++ )
        {
          CFarDlgItem * Item = (CFarDlgItem*)m_Controls[ i ];
          Item->m_FarFlags &= ~DIF_CENTERGROUP;
        }
      }
    }
  }
};

//////////////////////////////////////////////////////////////////////////
// CFarRadioGroup
//
class CFarRadioGroup : public CFarGroupBox
{
public:
  CFarRadioGroup( TBorderStyle Style = bsSingle ) : CFarGroupBox( Style )
  {
  }
  virtual ~CFarRadioGroup()
  {
  }
  CFarRadioButton * AddItem( const FarString& Text, int Left, int Top )
  {
    CFarRadioButton * btn = AddItem( Left, Top );
    btn->SetCaption( Text );
    return btn;
  }
  CFarRadioButton * AddItem( int Left, int Top )
  {
    CFarControl * Parent = /*GetBorderStyle() != bsNone ?*/ this/* : GetParent()*/;

    CFarDlgItem * Last;
    if ( GetControlCount() > 0 )
      Last = (CFarDlgItem*)GetControl( GetControlCount() - 1 );
    else
      Last = NULL;


    if ( Top == -1 )
    {
      if ( Left != -1 )
      {
        if ( Last )
          Top = Last->GetTop();
        else
          Top = Parent->GetTop() + 1;
      }
      else
      {
        if ( Last )
          Top = Last->GetTop() + Last->GetHeight();
        else
          Top = Parent->GetTop() + Parent->GetHeight();
        Top ++;
      }
    }
    if ( Left == -1 )
    {
      if ( Last )
        Left = Last->GetLeft() + Last->GetWidth();
      else
        Left = Parent->GetLeft() + Parent->GetWidth();
      Left ++;
    }

    CFarRadioButton * btn = create CFarRadioButton();

    btn->SetLeft( Left );
    btn->SetTop( Top );

    if ( GetControlCount() == 0 )
      btn->m_FarFlags |= DIF_GROUP;

    CFarGroupBox::AddItem( btn );

    return btn;
  }
};

//////////////////////////////////////////////////////////////////////////
// CFarEditorControl
//
class CFarEditorControl : public CFarGroupBox
{
public:
  CFarEditorControl() : CFarGroupBox( bsNone )
  {
  }
  virtual ~CFarEditorControl()
  {
  }
};



#define FAR_BEGIN_MESSAGE_MAP virtual bool Dispatch( TMessage& Msg ) \
{                                                                    \
    switch( Msg.Msg )                                            \
    {                                                            \

#define FAR_MESSAGE_HANDLER( msg, type, meth )                       \
      case msg: return meth( *((type*)&Msg) );                 \

#define FAR_END_MESSAGE_MAP( base )                                  \
    }                                                            \
  return base::Dispatch( Msg );                                    \
}                                                                    \

#define far_msg

//////////////////////////////////////////////////////////////////////////
// CFarDialog
//
class CFarDialog : public CFarControl
{
  friend class CFarDlgItem;
private:
  static long WINAPI DialogProc( Far::HDIALOG hDlg, int Msg, int Param1, long Param2 );
  Far::HDIALOG  m_Handle;
  FarString     m_HelpTopic;

  CFarDlgItem * m_DefaultControl;
  CFarDlgItem * m_FocusControl;

  int           m_Color;

  CFarBox * GetBorder() const
  {
    return (CFarBox*)CFarControl::GetControl( 0 );
  }

  FarArray<CFarDlgItem> * m__Items;
  void MakeDlgItem( CFarDlgItem * Src, FarDialogItem * Dst );
  void MakeDlgItems( CFarDlgItem * Group, FarDataArray<FarDialogItem> & DlgItems );

protected:

  virtual int GetLeft();
  virtual int GetTop();
  virtual int GetWidth();
  virtual int GetHeight();

  virtual void SetLeft( int Value );
  virtual void SetTop( int Value );
  virtual void SetWidth( int Value );
  virtual void SetHeight( int Value );

  long SendMessage( int msg, int param1, long param2 );

  virtual bool Dispatch( TMessage & Msg );

  void SetFocusControl( CFarDlgItem * Value );
  void SetDefaultControl( CFarDlgItem * Value );

  virtual void InsertControl( CFarControl * Ctl, bool bAuto = false )
  {
    CFarControl * Border = CFarControl::GetControl( 0 );
    int n = Border->InsertControl( Ctl );
    if ( n > 0 && bAuto && Ctl->GetTop() == 0 )
    {
      CFarControl * Prev = Border->GetControl( n - 1 );
      far_assert( Prev != NULL );
      Ctl->SetTop( Prev->GetTop() + 1 );
      Ctl->SetLeft( Prev->GetLeft() );
    }
  }
  virtual void RemoveControl( CFarControl * Ctl )
  {
    CFarControl::GetControl( 0 )->RemoveControl( Ctl );
  }
  virtual int GetControlCount()
  {
    return CFarControl::GetControl( 0 )->GetControlCount();
  }
  CFarControl * GetControl( int nIndex )
  {
    return CFarControl::GetControl( 0 )->GetControl( nIndex );
  }
  CFarDlgItem * GetRuntimeControl( int nRuntimeIndex )
  {
    far_assert( m__Items != NULL );
    return m__Items->At( nRuntimeIndex );
  }
  int GetBorderSize()
  {
    return GetBorder()->GetBorderSize();
  }

  virtual bool OnCreate( int ItemId )
  {
    return false;
  }
  virtual bool OnClose( int ItemId, LPBOOL bAllow )
  {
    return false;
  }
  virtual bool OnDraw( LPBOOL bShow )
  {
    return false;
  }
  virtual bool OnEnterIdle()
  {
    return false;
  }

  DWORD m_Flags;

  CFarDialog();
  CFarDialog( const int Width, const int Height );
  CFarDialog( const FarString& Title, const int Width, const int Height );
public:
  virtual ~CFarDialog();

  CFarDlgItem * Execute( const int X = -1, const int Y = -1 );

  FarString GetHelpTopic();
  void SetHelpTopic( const FarString& Value );

  virtual void SetBorderStyle( TBorderStyle Value )
  {
    GetBorder()->SetBorderStyle( Value );
  }
  virtual TBorderStyle GetBorderStyle() const
  {
    return GetBorder()->GetBorderStyle();
  }
  void SetBorderOrigin( const int X, const int Y )
  {
    far_assert( X >= 0 && Y >= 0 );
    GetBorder()->SetLeft( X );
    GetBorder()->SetTop( Y );
  }
  void SetTitle( const FarString& Value )
  {
    GetBorder()->SetTitle( Value );
  }
  int GetColor() const
  {
    return m_Color;
  }
  void SetColor( const int Value )
  {
    m_Color = Value;
    GetBorder()->SetColor( Value );
  }

  void Redraw()
  {
    SendMessage( DM_SETREDRAW, 0, 0 );
  }

  void UpdateControl( CFarDlgItem * item );
};

inline void CFarDialog::SetFocusControl( CFarDlgItem * Value )
{
  m_FocusControl = Value;
}

inline void CFarDialog::SetDefaultControl( CFarDlgItem * Value )
{
  m_DefaultControl = Value;
}

inline FarString CFarDialog::GetHelpTopic()
{
  return m_HelpTopic;
}

inline void CFarDialog::SetHelpTopic( const FarString& Value )
{
  m_HelpTopic = Value;
}

#endif //!defined(___FarDialog_H___)
