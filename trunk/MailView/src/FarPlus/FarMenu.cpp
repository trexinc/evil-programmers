//////////////////////////////////////////////////////////////////////////
// FarMenu.cpp
//
// (c) 2002 Dennis Trachuk <dennis.trachuk@nm.ru>
// (c) 2002 Dmitry Jemerov <yole@yole.ru>
//

#include "FarMenu.h"

//////////////////////////////////////////////////////////////////////////
// FarMenuStd
//
FarMenuStd::FarMenuStd( LPCSTR TitleText, UINT Flags, LPCSTR HelpTopic )
  : fTitle         ( TitleText ),
  fHelpTopic     ( HelpTopic ),
  fBottom        ( NULL ),
  fX             ( -1 ),
  fY             ( -1 ),
  fFlags         ( Flags ),
  fBreakKeys     ( NULL ),
  fBreakCode     ( -1 ),
  fMaxHeight     ( 0 ),
  fItems         ( NULL ),
  fItemsNumber   ( 0 ),
  fOwnsBreakKeys ( false )
{
}

FarMenuStd::FarMenuStd( int TitleLngIndex, UINT Flags, LPCSTR HelpTopic )
  : fTitle         ( Far::GetMsg( TitleLngIndex ) ),
  fFlags         ( Flags ),
  fHelpTopic     ( HelpTopic ),
  fBottom        ( NULL ),
  fBreakKeys     ( NULL ),
  fBreakCode     ( -1 ),
  fX             ( -1 ),
  fY             ( -1 ),
  fMaxHeight     ( 0 ),
  fItems         ( NULL ),
  fItemsNumber   ( 0 ),
  fOwnsBreakKeys ( false )
{
}

FarMenuStd::~FarMenuStd()
{
    if ( fItems ) free ( fItems );
    if ( fOwnsBreakKeys && fBreakKeys ) delete [] fBreakKeys;
}

int FarMenuStd::InternalAddItem( LPCSTR Text, int Selected, int Checked, int Separator )
{
  fItems = (FarMenuItem*)realloc( fItems, ++fItemsNumber * sizeof( FarMenuItem ) );
  strncpy( fItems[ fItemsNumber - 1 ].Text, Text, sizeof( fItems[ fItemsNumber - 1 ].Text ) - 1 );

  fItems[ fItemsNumber - 1 ].Selected  = Selected;
  fItems[ fItemsNumber - 1 ].Checked   = Checked;
  fItems[ fItemsNumber - 1 ].Separator = Separator;

  return fItemsNumber - 1;
}

int FarMenuStd::AddItem( LPCSTR Text, bool Selected, int Checked )
{
  return InternalAddItem( Text, Selected, Checked, 0 );
}

int FarMenuStd::AddItem (int LngIndex, bool Selected, int Checked)
{
  return InternalAddItem( Far::GetMsg( LngIndex ), Selected, Checked, 0 );
}

int FarMenuStd::AddSeparator()
{
  return InternalAddItem( "", 0, 0, 1 );
}

void FarMenuStd::ClearItems()
{
  delete [] fItems;

  fItems       = NULL;
  fItemsNumber = 0;
}

void FarMenuStd::SelectItem (int index)
{
  if ( index >= 0 && index < fItemsNumber )
    for ( int i = 0; i < fItemsNumber; i++ )
      fItems[ i ].Selected = ( i == index );
}

void FarMenuStd::SetBreakKeys (int aFirstKey, ...)
{
  int keyCount = 0;
  int curKey   = aFirstKey;

  va_list va;
  va_start( va, aFirstKey );
  while ( curKey )
  {
    keyCount++;
    curKey = va_arg( va, int );
  }
  va_end ( va );

  if ( fOwnsBreakKeys && fBreakKeys )
    delete fBreakKeys;

  fOwnsBreakKeys = true;
  fBreakKeys     = create int[ keyCount + 1 ];

  int *pKey = fBreakKeys;
  curKey = aFirstKey;
  va_start (va, aFirstKey);
  while ( curKey )
  {
          *pKey++ = curKey;
    curKey = va_arg( va, int );
  }
  va_end( va );

  *pKey = curKey;  // put terminating NULL
}

int FarMenuStd::Show()
{
  return Far::m_Info.Menu( Far::m_Info.ModuleNumber,
    fX, fY, fMaxHeight, fFlags,
    fTitle, fBottom, fHelpTopic,
    fBreakKeys, &fBreakCode,
    fItems, fItemsNumber );
}


//////////////////////////////////////////////////////////////////////////
// FarMenuExt
//
#define MIF_EX_SUBMENU (1<<0)

FarMenuExt::TItem::TItem()
{
  ZeroMemory( this, sizeof( FarMenuExt::TItem ) );
}

FarMenuExt::TItem::~TItem()
{
  if ( m_ExtFlags & MIF_EX_SUBMENU )
  {
    far_assert( UserData != NULL );
    delete (FarMenuExt*)UserData;
  }
}

bool FarMenuExt::TItem::GetSelected() const
{
  return Flags & MIF_SELECTED != 0;
}

void FarMenuExt::TItem::SetSelected( bool bSelected )
{
  if ( bSelected )
    Flags |= MIF_SELECTED;
  else
    Flags &= ~MIF_SELECTED;
}

bool FarMenuExt::TItem::GetChecked() const
{
  return Flags & MIF_CHECKED != 0;
}

void FarMenuExt::TItem::SetChecked( bool bChecked, char Mark )
{
  if ( bChecked )
  {
    Flags |= MIF_CHECKED;
    Flags |= Mark;
  }
  else
  {
    Flags &= ~MIF_CHECKED;
    Flags &= 0xFFFF0000;
  }
}

bool FarMenuExt::TItem::GetDisabled() const
{
  return Flags & MIF_DISABLE != 0;
}

void FarMenuExt::TItem::SetDisabled( bool bDisabled )
{
  if ( bDisabled )
    Flags |= MIF_DISABLE;
  else
    Flags &= ~MIF_DISABLE;
}

bool FarMenuExt::TItem::GetEnabled() const
{
  return Flags & MIF_DISABLE == 0;
}

void FarMenuExt::TItem::SetEnabled( bool bEnabled )
{
  if ( bEnabled )
    Flags &= ~MIF_DISABLE;
  else
    Flags |= MIF_DISABLE;
}

LPCSTR FarMenuExt::TItem::GetText() const
{
  return Flags & MIF_USETEXTPTR ? Text.TextPtr : Text.Text;
}

void FarMenuExt::TItem::SetText( LPCSTR szText )
{
  Flags &= ~MIF_USETEXTPTR;
  strncpy( Text.Text, szText, 128 );
}

void FarMenuExt::TItem::SetText( int nMsgId )
{
  Flags |= MIF_USETEXTPTR;
  Text.TextPtr = Far::GetMsg( nMsgId );
}

DWORD FarMenuExt::TItem::GetData() const
{
  return UserData;
}

void FarMenuExt::TItem::SetData( DWORD dwData )
{
  if ( m_ExtFlags & MIF_EX_SUBMENU )
  {
    far_trace( "ERROR: FarMenuExt::TItem::SetData failed (SubMenu used).\n" );

    return;
  }
  UserData = dwData;
}

DWORD FarMenuExt::TItem::GetAccelKey() const
{
  return AccelKey;
}

void FarMenuExt::TItem::SetAccelKey( DWORD dwKey )
{
  AccelKey = dwKey;
}

/*bool FarMenuExt::TItem::Separator() const
{
  return Flags & MIF_SEPARATOR != 0;
}*/

void FarMenuExt::TItem::SetSeparator( bool bSeparator )
{
  if ( bSeparator )
    Flags |= MIF_SEPARATOR;
  else
    Flags &= ~MIF_SEPARATOR;
}

FarMenuExt * FarMenuExt::TItem::GetSubMenu() const
{
  return m_ExtFlags & MIF_EX_SUBMENU ? (FarMenuExt*)UserData : NULL;
}

void FarMenuExt::TItem::SetSubMenu( bool bSubMenu )
{
  if ( bSubMenu )
  {
    if ( m_ExtFlags & MIF_EX_SUBMENU )
      return;

    m_ExtFlags |= MIF_EX_SUBMENU;

    far_assert( UserData == NULL );

    UserData = (DWORD) new FarMenuExt();
  }
  else
  {
    if ( !(m_ExtFlags & MIF_EX_SUBMENU) )
      return;

    m_ExtFlags &= ~MIF_EX_SUBMENU;

    far_assert( UserData != NULL );

    delete (FarMenuExt*)UserData;

    UserData = 0;
  }
}

//////////////////////////////////////////////////////////////////////////

int FarMenuExt::GetX() const
{
  return m_X;
}

void FarMenuExt::SetX( int nX )
{
  m_X = nX;
}

int FarMenuExt::GetY() const
{
  return m_Y;
}

void FarMenuExt::SetY( int nY )
{
  m_Y = nY;
}

void FarMenuExt::SetLocation( int nX, int nY )
{
  m_X = nX;
  m_Y = nY;
}

int FarMenuExt::GetMaxHeight() const
{
  return m_MaxHeight;
}

void FarMenuExt::SetMaxHeight( int nMaxHeight )
{
  m_MaxHeight = nMaxHeight;
}

FarString FarMenuExt::GetTitle() const
{
  return m_Title;
}

void FarMenuExt::SetTitle( const FarString& Title )
{
  m_Title = Title;
}

void FarMenuExt::SetTitle( int nTitleId )
{
  m_Title = Far::GetMsg( nTitleId );
}

FarString FarMenuExt::GetBottom() const
{
  return m_Bottom;
}

void FarMenuExt::SetBottom( const FarString& Bottom )
{
  m_Bottom = Bottom;
}

void FarMenuExt::SetBottom( int nBottomId )
{
  m_Bottom = Far::GetMsg( nBottomId );
}

FarString FarMenuExt::GetHelpTopic() const
{
  return m_HelpTopic;
}

void FarMenuExt::SetHelpTopic( const FarString& HelpTopic )
{
  m_HelpTopic = HelpTopic;
}

FarMenuExt::FarMenuExt() : m_X( -1 ), m_Y( -1 ), m_MaxHeight( 0 ),
  m_Flags( FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT ), m_BreakKeys( NULL ), m_BreakCode( -1 )
{
}

FarMenuExt::FarMenuExt( const FarString& Title ) : m_Flags( FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT ),
  m_X( -1 ), m_Y( -1 ), m_MaxHeight( 0 ),
  m_BreakKeys( NULL ), m_BreakCode( -1 )
{
    m_Title = Title;
}

FarMenuExt::FarMenuExt( int nTitleId ) : m_Flags( FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT ),
  m_X( -1 ), m_Y( -1 ), m_MaxHeight( 0 ),
  m_BreakKeys( NULL ), m_BreakCode( -1 )
{
  m_Title = Far::GetMsg( nTitleId );
}

FarMenuExt::~FarMenuExt()
{
  if ( m_BreakKeys )
    delete [] m_BreakKeys;
}

FarMenuExt::PItem FarMenuExt::AddItem( LPCSTR szText )
{
  FarMenuExt::PItem Item = create FarMenuExt::TItem;
  Item->SetText( szText );
  m_Items.Add( Item );
  return Item;
}

FarMenuExt::PItem FarMenuExt::AddItem( int nTextId )
{
  FarMenuExt::PItem Item = create FarMenuExt::TItem;
  Item->SetText( nTextId );
  m_Items.Add( Item );
  return Item;
}

FarMenuExt::PItem FarMenuExt::AddSeparator()
{
  FarMenuExt::PItem Item = create FarMenuExt::TItem;
  Item->SetSeparator();
  m_Items.Add( Item );
  return Item;
}

FarMenuExt::PItem FarMenuExt::AddSeparator( LPCSTR szText )
{
  FarMenuExt::PItem Item = create FarMenuExt::TItem;
  Item->SetText( szText );
  Item->SetSeparator();
  m_Items.Add( Item );
  return Item;
}

FarMenuExt::PItem FarMenuExt::AddSeparator( int nTextId )
{
  FarMenuExt::PItem Item = create FarMenuExt::TItem;
  Item->SetText( nTextId );
  Item->SetSeparator();
  m_Items.Add( Item );
  return Item;
}

FarMenuExt::PItem FarMenuExt::Show()
{
  FarMenuItemEx * Items = create FarMenuItemEx[ m_Items.Count() ];
  int i, nMaxNameLen = 0;
  for ( i = 0; i < m_Items.Count(); i ++ )
  {
    FarMenuExt::PItem Item = m_Items[ i ];
    int nCurNameLen = strlen( Item->GetText() );
    if ( nCurNameLen > nMaxNameLen )
      nMaxNameLen = nCurNameLen;
  }
  nMaxNameLen += 1; // this one can be parametrized
  far_assert( nMaxNameLen < 122 );
  for ( i = 0; i < m_Items.Count(); i ++ )
  {
    FarMenuExt::PItem Item = m_Items[ i ];
    if ( m_Items[ i ]->GetSubMenu() )
    {
      LPSTR Text = Item->Text.Text;
      if ( Item->Flags & MIF_USETEXTPTR )
      {
        strncpy( Text, Item->Text.TextPtr, 128 );
        Item->Flags &= ~MIF_USETEXTPTR;
      }
      int nCurNameLen = strlen( Text );
      far_assert( nCurNameLen < 126 );
      int j;
      for ( j = nCurNameLen; j < nMaxNameLen; j ++ )
        Text[ j ] = '\x20';
      Text[ j ] = '\x10';
      j ++;
      Text[ j ] = '\x00';
    }
    memcpy( Items + i, Item, sizeof( FarMenuItemEx ) );
  }

  PItem result = NULL;

  while ( true )
  {
    i = Far::m_Info.Menu( Far::m_Info.ModuleNumber,
      m_X, m_Y, m_MaxHeight, m_Flags|FMENU_USEEXT,
      m_Title.data(), m_Bottom.data(), m_HelpTopic.data(),
      m_BreakKeys, &m_BreakCode,
      (FarMenuItem*)Items, m_Items.Count() );

    if ( i == -1 )
    {
      result = NULL;
      break;
    }

    FarMenuExt * subMenu = m_Items[ i ]->GetSubMenu();
    if ( subMenu == NULL )
    {
      result = m_Items[ i ];
      break;
    }

    result = subMenu->Show();

    if ( result != NULL )
      break;
  }

  delete [] Items;

  return result;
}

void FarMenuExt::SelectItem( FarMenuExt::PItem Item )
{
  far_assert( Item != NULL );
  for ( int i = 0; i < m_Items.Count(); i ++ )
    m_Items[ i ]->SetSelected( m_Items[ i ] == Item );
}

void FarMenuExt::ClearItems()
{
  m_Items.Clear();
}

UINT FarMenuExt::GetFalgs() const
{
  return m_Flags;
}

void FarMenuExt::SetFlags( UINT Flags )
{
  m_Flags /*|*/= Flags;
}

int FarMenuExt::GetItemsCount() const
{
  return m_Items.Count();
}

FarMenuExt::PItem FarMenuExt::GetItem( int nIndex )
{
  return m_Items[ nIndex ];
}

int FarMenuExt::GetItemIndex( FarMenuExt::PItem Item )
{
//  far_assert( Item != NULL );
  for ( int i = 0; i < m_Items.Count(); i ++ )
    if ( m_Items[ i ] == Item )
      return i;
  return -1;
}
