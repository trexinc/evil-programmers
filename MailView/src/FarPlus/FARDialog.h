/* $Header: /cvsroot/farplus/FARPlus/FARDialog.h,v 1.3 2002/04/15 05:40:56 yole Exp $
   FAR+Plus: dialog classes
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
*/

#ifndef __FARDIALOG_H
#define __FARDIALOG_H

#include "FARPlus.h"

#if _MSC_VER >= 1000
#pragma once
#endif

class FarControl;

/**
* Base class for both dialog controls and special items that are not controls.
*/

class FarDlgItem
{
protected:
	friend class FarDialog;
	
	FarDialog *fDlg;
	bool fOwned;
	
	FarDlgItem (FarDialog *pDlg)
		: fDlg (pDlg), fOwned (false) {};
	
	void SetOwned()
	{ fOwned = true; }
	bool IsOwned() const
	{ return fOwned; }
public:
	virtual ~FarDlgItem() {};
};

typedef BOOL (*ValidateFunc) (char *Text, void *UserParam);

class FarSeparatorCtrl;

// -- FarDialog  -------------------------------------------------------------

class FarDialog
{
protected:
    friend class FarControl;
    friend class FarEditCtrl;
    friend class FarRadioCtrl;

	FarDataArray<FarDialogItem> fItems;
    FarArray<FarControl> fControls;
	FarArray<FarDlgItem> fSpecialItems;
    FarString fHelpTopic;
    FarControl *m_DefaultControl;
    FarControl *m_FocusControl;
    int m_BorderX, m_BorderY;   // space to the left and to the top of the 
                                // dialog frame

    void AddControl (FarControl *pCtrl);
	void UpdateFocus();
public:
    FarDialog (const char *Title, const char *HelpTopic=NULL);
    FarDialog (int TitleLngIndex, const char *HelpTopic=NULL);
    ~FarDialog();
    void OwnsControls();
	void AddSpecialItem (FarDlgItem *pItem);
	void AddOwnedControl (FarDlgItem *pCtrl);
    void Layout();
    FarControl *Show (bool SkipLayout = false);

    void SetBorders (int X, int Y);
    void SetDefaultControl (FarControl *pCtrl);
    void SetFocusControl (FarControl *pCtrl);
    int FindControl (FarControl *pCtrl);  // returns index of given control or -1 if not found

	void AddText (const char *Text);
	void AddText (int LngIndex);

	FarSeparatorCtrl * AddSeparator();
	FarSeparatorCtrl * AddSeparator( const char * text );
	FarSeparatorCtrl * AddSeparator( int lngIndex );
};

inline void FarDialog::OwnsControls()
{
	fControls.SetOwnsItems (true);
}

inline void FarDialog::SetBorders (int X, int Y)
{
    m_BorderX = X;
    m_BorderY = Y;
}

inline void FarDialog::SetDefaultControl (FarControl *pCtrl)
{
    m_DefaultControl = pCtrl;
}

inline void FarDialog::SetFocusControl (FarControl *pCtrl)
{
    m_FocusControl = pCtrl;
}

// -- FarControl -------------------------------------------------------------

// additional flags for m_flagsPlus

enum { FCF_VALIDATE = 1, FCF_NEXTY = 2 };

class FarControl: public FarDlgItem
{
private:
    void InitControl (unsigned char Type);
protected:
    friend class FarDialog;

    FarDialogItem *fItem;
    DWORD m_flagsPlus;
    FarControl (FarDialog *pDlg, unsigned char Type);
    FarControl (FarDialog *pDlg, unsigned char Type, const char *Text);
    FarControl (FarDialog *pDlg, unsigned char Type, int LngIndex);
    virtual BOOL Validate();

	virtual void BeforeShow() {};

	void SetData( const char * data )
	{
		far_assert( data != NULL );
		strncpy( fItem->Data, data, sizeof( fItem->Data ) - 1 );
	}
	void SetData( int lngIndex )
	{
		SetData( Far::GetMsg( lngIndex ) );
	}
	void GetData( char * data, int maxLen ) const
	{
		strncpy( data, fItem->Data, maxLen - 1 );
	}
	FarString GetData() const
	{
		return fItem->Data;
	}
public:
    void SetFlags (DWORD flags);
    int GetSelected();
    void EnableValidate (BOOL bValidate = TRUE);
    void SetNextY() 
        { m_flagsPlus |= FCF_NEXTY; }
};

inline int FarControl::GetSelected()
{
    return fItem->Selected;
}

inline void FarControl::EnableValidate (BOOL bValidate)
{
    if (bValidate)
        m_flagsPlus |= FCF_VALIDATE;
    else
        m_flagsPlus &= ~FCF_VALIDATE;
}

// -- FarTextCtrl ------------------------------------------------------------

class FarTextCtrl : public FarControl
{
public:
    FarTextCtrl (FarDialog *pDlg, const char *Text, int X=-1);
    FarTextCtrl (FarDialog *pDlg, int LngIndex, int X=-1);
	void SetText( const char * text )
	{
		SetData( text );
	}
	void GetText( char * text, int maxLen ) const
	{
		GetData( text, maxLen );
	}
	FarString GetText() const
	{
		return GetData();
	}
};

class FarEditCtrl : public FarControl
{
protected:
    ValidateFunc fValidateFunc;
    int *fIntValuePtr;
	FarString *fStringPtr;
	void *fValidateUserParam;
    
    virtual BOOL Validate();
public:
    FarEditCtrl (FarDialog *pDlg, const char *Text, int X, int Width, 
        const char *History=NULL);
	FarEditCtrl (FarDialog *pDlg, FarString *stringPtr, int X, int Width,
		const char *History = NULL);
    FarEditCtrl (FarDialog *pDlg, int iValue, int X, int Width);
    FarEditCtrl (FarDialog *pDlg, int *pIntValue, int X, int Width);

    void GetText( char *Text, int MaxLength )
	{
		GetData( Text, MaxLength );
	}
	FarString GetText()
	{
		return GetData();
	}
    void SetText( const char * NewText )
	{
		SetData( NewText );
	}
    int GetIntValue();
    
	void SetValidateFunc (ValidateFunc func, void *UserParam = NULL)
	{ 
		fValidateFunc = func; 
		fValidateUserParam = UserParam;
	}
};

class FarPswEditCtrl: public FarEditCtrl
{
public:
    FarPswEditCtrl (FarDialog *pDlg, const char *Text, int X, int Width);
};

class FarFixEditCtrl: public FarEditCtrl
{
public:
    FarFixEditCtrl (FarDialog *pDlg, const char *Text, int X, int Width,
        const char *History = NULL);
    FarFixEditCtrl (FarDialog *pDlg, int iValue, int X, int Width);
    FarFixEditCtrl (FarDialog *pDlg, int *pIntValue, int X, int Width);
    
    void SetMask (const char *Mask);
};

class FarSeparatorCtrl : public FarControl
{
public:
	enum Style { Single, Double };
	
    FarSeparatorCtrl( FarDialog * pDlg, Style style = Single );
    FarSeparatorCtrl( FarDialog * pDlg, const char * Text, Style style = Single );
    FarSeparatorCtrl( FarDialog * pDlg, int LngIndex, Style style = Single );

	void SetStyle( FarSeparatorCtrl::Style style );
};

class FarCheckCtrl : public FarControl
{
protected:
    int *m_pSelected;
    virtual BOOL Validate();

public:
    FarCheckCtrl (FarDialog *pDlg, const char *Text, int Selected);
    FarCheckCtrl (FarDialog *pDlg, int LngIndex, int Selected);
    FarCheckCtrl (FarDialog *pDlg, const char *Text, bool *pSelected);
    FarCheckCtrl (FarDialog *pDlg, int LngIndex, bool *pSelected);
    FarCheckCtrl (FarDialog *pDlg, const char *Text, int *pSelected);
    FarCheckCtrl (FarDialog *pDlg, int LngIndex, int *pSelected);
	enum State { Unchecked = 0, Checked = 1, Undefined = 2 };
	State GetState() const
	{
		return (State)fItem->Selected;
	}
	void SetState( State newVal )
	{
		fItem->Selected = newVal;
		if ( m_pSelected )
			*m_pSelected = newVal;
	}
};

class FarRadioCtrl : public FarControl
{
private:
	friend class FarRadioGroup;

public:
    FarRadioCtrl( FarDialog * pDlg, const char * Text, int X = -1 )
		: FarControl( pDlg, DI_RADIOBUTTON, Text )
	{
		fItem->X1 = X;
	}
    FarRadioCtrl( FarDialog *pDlg, int LngIndex, int X = -1 )
		: FarControl( pDlg, DI_RADIOBUTTON, LngIndex )
	{
		fItem->X1 = X;
	}
	void SetSelected( bool newVal )
	{
		fItem->Selected = newVal ? 1 : 0;
	}

	FarString GetTitle() const
	{
		return GetData();
	}
};

class FarRadioGroup: public FarDlgItem
{
protected:
	int fSelectedIndex;
	FarArray<FarRadioCtrl> fItems;

public:
	FarRadioGroup (FarDialog *pDlg, int SelectedIndex = 0)
		: FarDlgItem (pDlg), fSelectedIndex (SelectedIndex) 
	{
		fDlg->AddSpecialItem (this);
		fItems.SetOwnsItems (false);
	}

	FarRadioCtrl * AddItem( const char * Text, int X = -1 );
	FarRadioCtrl * AddItem( int LngIndex, int X = -1 );
	int GetSelectedIndex();
};

class FarBoxCtrl: public FarControl
{
public:
    FarBoxCtrl (FarDialog *pDlg, BOOL Double, int X1, int Y1, int W, int H,
        const char *Title);
    FarBoxCtrl (FarDialog *pDlg, BOOL Double, int X1, int Y1, int W, int H,
        int LngIndex);
};

class FarButtonCtrl: public FarControl
{
public:
    // if X is equal to DIF_CENTERGROUP, the button gets this flag.
    // In this case, the previous button also gets this flag.
    FarButtonCtrl (FarDialog *pDlg, const char *Text, int X=-1);
    FarButtonCtrl (FarDialog *pDlg, int LngIndex, int X=-1);
};

#ifdef USE_FAR_170

// Base class for combo boxes and list boxes
class FarBaseListCtrl: public FarControl
{
protected:
	FarDataArray<FarListItem> fListItems;
	FarList fFarList;

	virtual void BeforeShow();

    FarBaseListCtrl (FarDialog *pDlg, unsigned char Type, int X, int Width);

public:
	void AddItem (const char *Text, int Flags = 0);

	int GetListPos() const
	{
		return fItem->ListPos;
	}
};

class FarComboBox: public FarBaseListCtrl
{
protected:
	FarString *fStringPtr;

	virtual BOOL Validate();

public:
	FarComboBox (FarDialog *pDlg, int X, int Width)
		: FarBaseListCtrl (pDlg, DI_COMBOBOX, X, Width), fStringPtr (NULL) {};

    FarComboBox (FarDialog *pDlg, FarString *pString, int X, int Width)
		: FarBaseListCtrl (pDlg, DI_COMBOBOX, X, Width), fStringPtr (pString) 
	{
		if ( pString )
			SetData( *pString );
	}
	void GetText (char *Text, int MaxLength)
	{
		GetData( Text, MaxLength );
	}
	FarString GetText()
	{
		return GetData();
	}
};

#endif

#endif
