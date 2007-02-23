//////////////////////////////////////////////////////////////////////////
// FarMenu.h
//
// (c) 2002 Dennis Trachuk <dennis.trachuk@nm.ru>
// (c) 2002 Dmitry Jemerov <yole@yole.ru>
//

#ifndef ___FarMenu_H___
#define ___FarMenu_H___

#if _MSC_VER >= 1000
#pragma once
#endif

#include "FarPlus.h"

class FarMenuStd
{
private:
	FarMenuStd(const FarMenuStd &rhs);
	const FarMenuStd &operator= (const FarMenuStd &rhs);

protected:
	FarString     fTitle;
	FarString     fHelpTopic;
	FarString     fBottom;
	int           fX;
	int           fY;
	UINT          fFlags;
	int         * fBreakKeys;
	int           fBreakCode;
	int           fMaxHeight;
	FarMenuItem * fItems;
	int           fItemsNumber;
	bool          fOwnsBreakKeys;

	int InternalAddItem( LPCSTR Text, int Selected, int Checked, int Separator );

public:
	FarMenuStd( LPCSTR TitleText, UINT Flags = FMENU_WRAPMODE, LPCSTR HelpTopic = NULL );
	FarMenuStd( int TitleLngIndex, UINT Flags = FMENU_WRAPMODE, LPCSTR HelpTopic = NULL );
	~FarMenuStd();

	void SetLocation( int X, int Y )
		{ fX = X; fY = Y; }
	void SetMaxHeight( int MaxHeight )
		{ fMaxHeight = MaxHeight; }
	void SetBottomLine( const FarString &Text )
		{ fBottom = Text; }
	void SetBottomLine(int LngIndex)
		{ fBottom = Far::GetMsg( LngIndex ); }
	void SetBreakKeys( int *BreakKeys )
		{ fOwnsBreakKeys = false; fBreakKeys = BreakKeys; }
	void SetBreakKeys( int aFirstKey, ... );

	// returns index of new item
	int AddItem( LPCSTR Text, bool Selected = false, int Checked = 0 );
	int AddItem( int LngIndex, bool Selected = false, int Checked = 0 );
	int AddSeparator();

	void ClearItems();
	void SelectItem( int index );

	// returns index of selected item
	int Show();
	int GetBreakCode() const
		{ return fBreakCode; }
};

class FarMenuExt
{
public:
	class TItem : private FarMenuItemEx
	{
		friend class FarMenuExt;
	private:
		DWORD m_ExtFlags;
		void SetSelected( bool bSelected = true );
		void SetSeparator( bool bSeparator = true );
	public:
		TItem();
		~TItem();
		bool GetSelected() const;
		bool GetChecked() const;
		void SetChecked( bool bChecked = true, char Mark = 0 );
		bool GetDisabled() const;
		void SetDisabled( bool bDisabled = true );
		bool GetEnabled() const;
		void SetEnabled( bool bEnabled = true );
		LPCSTR GetText() const;
		void SetText( LPCSTR szText );
		void SetText( int nMsgId );
		DWORD GetData() const;
		void SetData( DWORD dwData );
		DWORD GetAccelKey() const;
		void SetAccelKey( DWORD dwKey );
		FarMenuExt * GetSubMenu() const;
		void SetSubMenu( bool bSubMenu = true );
	};
	typedef TItem * PItem;
private:
	int       m_X;
	int       m_Y;
	int       m_MaxHeight;
	UINT      m_Flags;
	FarString m_Title;
	FarString m_Bottom;
	FarString m_HelpTopic;
	LPINT     m_BreakKeys;
	int       m_BreakCode;

	FarObjectArray<TItem> m_Items;

	int       m_ExtFlags;

public:
	int GetX() const;
	void SetX( int nX );
	int GetY() const;
	void SetY( int nY );
	void SetLocation( int nX, int nY );
	int GetMaxHeight() const;
	void SetMaxHeight( int nMaxHeight );
	FarString GetTitle() const;
	void SetTitle( const FarString& Title );
	void SetTitle( int nTitleId );
	FarString GetBottom() const;
	void SetBottom( const FarString& Bottom );
	void SetBottom( int nBottomId );
	FarString GetHelpTopic() const;
	void SetHelpTopic( const FarString& HelpTopic );

public:
	FarMenuExt();
	FarMenuExt( const FarString& Title );
	FarMenuExt( int nTitleId );
	~FarMenuExt();

	PItem AddItem( LPCSTR szText );
	PItem AddItem( int nTextId );
	PItem AddSeparator();
	PItem AddSeparator( LPCSTR szText );
	PItem AddSeparator( int nTextId );

	PItem Show();

	void SelectItem( PItem Item );

	void ClearItems();

	int GetItemsCount() const;
	PItem GetItem( int nIndex );
	int GetItemIndex( PItem Item );

	UINT GetFalgs() const;
	void SetFlags( UINT Flags );
};

#ifdef USE_FAR_170
typedef FarMenuExt FarMenu;
#else
typedef FarMenuStd FarMenu;
#endif

#endif //!defined(___FarMenu_H___)
