/* $Header: /cvsroot/farplus/FARPlus/FARDialog.cpp,v 1.5 2002/09/05 06:32:38 yole Exp $
   FAR+Plus: dialog classes implementation
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
*/

#include "FARDialog.h"

// -- FarDialog --------------------------------------------------------------

FarDialog::FarDialog (const char *Title, const char *HelpTopic)
  : fHelpTopic       (HelpTopic),
    m_DefaultControl (NULL),
    m_FocusControl   (NULL),
    m_BorderX        (2),
    m_BorderY        (1)
{
	fControls.SetOwnsItems (false);
	fSpecialItems.SetOwnsItems (false);

    AddOwnedControl (create FarBoxCtrl (this, TRUE, 0, 0, 0, 0, Title));
}

FarDialog::FarDialog (int TitleLngIndex, const char *HelpTopic)
  : fHelpTopic       (HelpTopic),
    m_DefaultControl (NULL),
    m_FocusControl   (NULL),
    m_BorderX        (2),
    m_BorderY        (1)
{
	fControls.SetOwnsItems (false);
	fSpecialItems.SetOwnsItems (false);

    AddOwnedControl (create FarBoxCtrl (this, TRUE, 0, 0, 0, 0, TitleLngIndex));
}

FarDialog::~FarDialog()
{
    if (!fControls.OwnsItems())
	{
		for (int i=fControls.Count()-1; i >= 0; i--)
		{
			if (fControls [i]->IsOwned())
			{
				delete fControls [i];
				fControls [i] = NULL;
			}
		}
		for (int j=fSpecialItems.Count()-1; j >= 0; j--)
		{
			if (fSpecialItems [j]->IsOwned())
			{
				delete fSpecialItems [j];
				fSpecialItems [j] = NULL;
			}
		}
	}

	fControls.Clear();
	fSpecialItems.Clear();
}

void FarDialog::AddControl (FarControl *pCtrl)
{
	FarDialogItem newItem;   // not yet initialized
	fItems.Add (newItem);
	fControls.Add (pCtrl);

    for (int i=0; i<fItems.Count(); i++)
        fControls [i]->fItem = &fItems [i];
}

void FarDialog::AddSpecialItem (FarDlgItem *pItem)
{
	fSpecialItems.Add (pItem);
}

void FarDialog::AddOwnedControl (FarDlgItem *pCtrl)
{
	pCtrl->SetOwned();
}

int FarDialog::FindControl (FarControl *pCtrl)
{
	return fControls.IndexOf (pCtrl);
}

void FarDialog::AddText (const char *Text)
{
	AddOwnedControl (create FarTextCtrl (this, Text));
}

void FarDialog::AddText (int LngIndex)
{
	AddText (Far::GetMsg (LngIndex));
}

FarSeparatorCtrl * FarDialog::AddSeparator()
{
	FarSeparatorCtrl * ctl = create FarSeparatorCtrl( this );
	AddOwnedControl( ctl );
	return ctl;
}

FarSeparatorCtrl * FarDialog::AddSeparator( const char * text )
{
	FarSeparatorCtrl * ctl = create FarSeparatorCtrl( this, text );
	AddOwnedControl( ctl );
	return ctl;
}

FarSeparatorCtrl * FarDialog::AddSeparator( int lngIndex )
{
	FarSeparatorCtrl * ctl = create FarSeparatorCtrl( this, lngIndex );
	AddOwnedControl( ctl );
	return ctl;
}

void FarDialog::Layout()
{
    // The core loop. Automatic control layout, default/focus management
    // and so on.

    unsigned int LastY=m_BorderY;
    unsigned int MaxX2=0;
	unsigned int MaxY2 = 0;

    // m_Controls [0] is the dialog frame. It is handled specially.
    for (int i=1; i<fControls.Count(); i++)
    {
        FarDialogItem *pCurItem=fControls [i]->fItem;

		// Layout
        if (pCurItem->Y1 == -1)
        {
            if (pCurItem->X1 == -1) // next Y, default X
            {
                pCurItem->Y1 = ++LastY;
                if (!(pCurItem->Flags & DIF_SEPARATOR))
                {
                    pCurItem->X1 = m_BorderX+3;
                    if (pCurItem->X2)
                        pCurItem->X2 += pCurItem->X1-1;
                }
            }
            else if (pCurItem->X1 == DIF_CENTERGROUP) // special case for buttons
            {
                pCurItem->Flags |= DIF_CENTERGROUP;
				FarDialogItem * pPrevItem = fControls[ i - 1 ]->fItem;
                if (pPrevItem->Type == DI_BUTTON || pPrevItem->Type == DI_RADIOBUTTON )
                {
                    pCurItem->Y1 = pPrevItem->Y1;
                    pPrevItem->Flags |= DIF_CENTERGROUP;
                }
                else {
                    pCurItem->Y1 = ++LastY;
                }
                pCurItem->X1 = 0; // not important for centered groups
            }
            else
			{
                if (fControls [i]->m_flagsPlus & FCF_NEXTY)
                    pCurItem->Y1 = ++LastY;
                else
				{
					FarDialogItem * pPrevItem = fControls[ i - 1 ]->fItem;
					if ( pPrevItem->Flags & (DIF_SEPARATOR|DIF_SEPARATOR2 ) )
						LastY ++;
					pCurItem->Y1 = LastY;
				}
                pCurItem->X1 += m_BorderX+3;
                if (pCurItem->X2)
                    pCurItem->X2 += pCurItem->X1-1;
            }
			if (pCurItem->Type == DI_SINGLEBOX || pCurItem->Type == DI_DOUBLEBOX)
			{
				// convert height to bottom Y
				pCurItem->Y2 += pCurItem->Y1;
				MaxY2 = pCurItem->Y2;
			}
        }

        // Max width determination
        if (pCurItem->X2 > (int) MaxX2)
            MaxX2 = pCurItem->X2;
        else
        {
            int Width=0;
            if (pCurItem->Type == DI_TEXT && pCurItem->Data != NULL)
            {
                Width = strlen (pCurItem->Data);
            }
            else if (pCurItem->Type == DI_CHECKBOX || pCurItem->Type == DI_RADIOBUTTON)
                Width = strlen (pCurItem->Data)+4;

            int curX = pCurItem->X1;
            if (curX == -1 && pCurItem->Flags & DIF_SEPARATOR && pCurItem->Data != NULL)
            {
                curX = 3;
                Width++;
            }
            if (Width + curX > (int) MaxX2)
                MaxX2 = Width + curX;
        }

        // Default
        if (fControls [i] == m_DefaultControl)
            pCurItem->DefaultButton = TRUE;
        else if (m_DefaultControl == NULL && pCurItem->Type == DI_BUTTON)
        {
            m_DefaultControl = fControls [i];
            pCurItem->DefaultButton = TRUE;
        }
        else
            pCurItem->DefaultButton = FALSE;

    }
	UpdateFocus();

    // Set the size of the dialog frame
	if (MaxY2 > LastY)
		LastY = MaxY2;
    fItems [0].X1 = m_BorderX+1;
    fItems [0].Y1 = m_BorderY;
    fItems [0].X2 = fItems [0].X1+MaxX2-1;
    fItems [0].Y2 = LastY+1;
}

void FarDialog::UpdateFocus()
{
    for (int i=1; i<fControls.Count(); i++)
    {
		FarDialogItem *pCurItem=fControls [i]->fItem;
        if (fControls [i] == m_FocusControl)
            pCurItem->Focus = TRUE;
        else if (m_FocusControl == NULL &&
            pCurItem->Type != DI_SINGLEBOX && pCurItem->Type != DI_DOUBLEBOX &&
            pCurItem->Type != DI_TEXT && pCurItem->Type != DI_VTEXT)
        {
            m_FocusControl = fControls [i];
            pCurItem->Focus = TRUE;
        }
        else
            pCurItem->Focus = FALSE;
	}
}

FarControl *FarDialog::Show (bool SkipLayout)
{
    if (!SkipLayout)
        Layout();

    // Show dialog and validate data
    BOOL bValidData;
    int ExitCode;
    do {
		for (int k=0; k<fControls.Count(); k++)
			fControls [k]->BeforeShow();

        bValidData = TRUE;
        ExitCode=Far::m_Info.Dialog (Far::m_Info.ModuleNumber,
            -1, -1, fItems [0].X2+m_BorderX+2, fItems [0].Y2+m_BorderY+1,
            fHelpTopic,
            fItems.GetItems(),
            fItems.Count());
        if (ExitCode == -1)
            return NULL;
        if (!fControls [ExitCode]->m_flagsPlus & FCF_VALIDATE)
            break;

        for (int i=0; i<fControls.Count(); i++)
            if (!fControls [i]->Validate())
            {
				SetFocusControl (fControls [i]);
				UpdateFocus();
                bValidData = FALSE;
                break;
            }
    } while (!bValidData);

    return fControls [ExitCode];
}

// -- FarControl -------------------------------------------------------------

FarControl::FarControl (FarDialog *pDlg, unsigned char Type)
  : FarDlgItem  (pDlg),
    m_flagsPlus (0)
{
    InitControl (Type);
}

FarControl::FarControl (FarDialog *pDlg, unsigned char Type, const char *Text)
  : FarDlgItem  (pDlg),
    m_flagsPlus (0)
{
    InitControl (Type);
	SetData( Text );
}

FarControl::FarControl (FarDialog *pDlg, unsigned char Type, int LngIndex)
  : FarDlgItem  (pDlg),
    m_flagsPlus (0)
{
    InitControl (Type);
	SetData( LngIndex );
}

void FarControl::InitControl (unsigned char Type)
{
    fDlg->AddControl (this);
    memset (fItem, 0, sizeof (FarDialogItem));
    fItem->Type = Type;
    fItem->X1 = -1;
    fItem->Y1 = -1;
}

BOOL FarControl::Validate()
{
    return TRUE;
}

void FarControl::SetFlags (DWORD flags)
{
    fItem->Flags |= flags;
}

// -- FarTextCtrl ------------------------------------------------------------

FarTextCtrl::FarTextCtrl (FarDialog *pDlg, const char *Text, int X)
  : FarControl (pDlg, DI_TEXT, Text)
{
    fItem->X1 = X;
}

FarTextCtrl::FarTextCtrl (FarDialog *pDlg, int LngIndex, int X)
  : FarControl (pDlg, DI_TEXT, LngIndex)
{
    fItem->X1 = X;
}

// -- FarEditCtrl ------------------------------------------------------------

FarEditCtrl::FarEditCtrl (FarDialog *pDlg, const char *Text, int X, int Width,
						  const char *History)
  : FarControl    (pDlg, DI_EDIT, Text),
    fValidateFunc (NULL),
    fIntValuePtr  (NULL),
	fStringPtr    (NULL)
{
	fItem->X1 = X;
    fItem->X2 = Width;
    if (History)
    {
        fItem->Flags |= DIF_HISTORY;
        fItem->Selected = (int) History;
    }
}

FarEditCtrl::FarEditCtrl (FarDialog *pDlg,FarString *stringPtr, int X, int Width,
						  const char *History)
	: FarControl    (pDlg, DI_EDIT, *stringPtr),
	  fValidateFunc	(NULL),
	  fIntValuePtr  (NULL),
	  fStringPtr    (stringPtr)
{
    fItem->X1 = X;
    fItem->X2 = Width;
    if (History)
    {
        fItem->Flags |= DIF_HISTORY;
        fItem->Selected = (int) History;
    }
}

FarEditCtrl::FarEditCtrl (FarDialog *pDlg, int iValue, int X, int Width)
    : FarControl    (pDlg, DI_EDIT, ""),
      fValidateFunc (NULL),
      fIntValuePtr  (NULL),
	  fStringPtr    (NULL)
{
    fItem->X1 = X;
    fItem->X2 = Width;

#ifndef USE_FAR_170
    itoa (iValue, fItem->Data, 10);
#else
    FarSF::itoa(iValue, fItem->Data, 10);
#endif
}

FarEditCtrl::FarEditCtrl (FarDialog *pDlg, int *pIntValue, int X, int Width)
    : FarControl    (pDlg, DI_EDIT, ""),
      fValidateFunc (NULL),
      fIntValuePtr  (pIntValue),
	  fStringPtr    (NULL)
{
    fItem->X1 = X;
    fItem->X2 = Width;

    FarSF::itoa( *pIntValue, fItem->Data, 10 );
}

int FarEditCtrl::GetIntValue()
{
    return FarSF::atoi (fItem->Data);
}

BOOL FarEditCtrl::Validate()
{
    BOOL ret = TRUE;
    if (fValidateFunc != NULL)
        ret = fValidateFunc (fItem->Data, fValidateUserParam);

    if (ret)
	{
		if (fIntValuePtr)
			*fIntValuePtr = GetIntValue();
		if (fStringPtr)
			*fStringPtr = fItem->Data;
	}

    return ret;
}

// -- FarPswEditCtrl ---------------------------------------------------------

FarPswEditCtrl::FarPswEditCtrl (FarDialog *pDlg, const char *Text, int X, int Width)
  : FarEditCtrl (pDlg, Text, X, Width, NULL)
{
    fItem->Type = DI_PSWEDIT;
}

// -- FarFixEditCtrl ---------------------------------------------------------

FarFixEditCtrl::FarFixEditCtrl (FarDialog *pDlg, const char *Text, int X, int Width,
                                const char *History /* = NULL */)
    :  FarEditCtrl (pDlg, Text, X, Width, History)
{
    fItem->Type = DI_FIXEDIT;
}

FarFixEditCtrl::FarFixEditCtrl (FarDialog *pDlg, int iValue, int X, int Width)
    : FarEditCtrl (pDlg, iValue, X, Width)
{
    fItem->Type = DI_FIXEDIT;
}

FarFixEditCtrl::FarFixEditCtrl (FarDialog *pDlg, int *pIntValue, int X, int Width)
    : FarEditCtrl (pDlg, pIntValue, X, Width)
{
    fItem->Type = DI_FIXEDIT;
}

void FarFixEditCtrl::SetMask (const char *Mask)
{
    fItem->Mask = const_cast<char *> (Mask);
    fItem->Flags |= DIF_MASKEDIT;
}

// -- FarSeparatorCtrl -------------------------------------------------------

FarSeparatorCtrl::FarSeparatorCtrl( FarDialog * pDlg, FarSeparatorCtrl::Style style )
  : FarControl( pDlg, DI_TEXT )
{
	fItem->Flags |= style == Double ? DIF_SEPARATOR2 : DIF_SEPARATOR;
}

FarSeparatorCtrl::FarSeparatorCtrl( FarDialog * pDlg, const char * Text, FarSeparatorCtrl::Style style )
  : FarControl ( pDlg, DI_TEXT, Text )
{
	fItem->Flags |= style == Double ? DIF_SEPARATOR2 : DIF_SEPARATOR;
}


FarSeparatorCtrl::FarSeparatorCtrl( FarDialog * pDlg, int LngIndex, FarSeparatorCtrl::Style style )
  : FarControl( pDlg, DI_TEXT, LngIndex )
{
	fItem->Flags |= style == Double ? DIF_SEPARATOR2 : DIF_SEPARATOR;
}

void FarSeparatorCtrl::SetStyle( FarSeparatorCtrl::Style style )
{
	if ( style == Double )
	{
		fItem->Flags &= ~DIF_SEPARATOR;
		fItem->Flags |= DIF_SEPARATOR2;
	}
	else
	{
		fItem->Flags &= ~DIF_SEPARATOR2;
		fItem->Flags |= DIF_SEPARATOR;
	}
}

// -- FarCheckCtrl -----------------------------------------------------------

FarCheckCtrl::FarCheckCtrl (FarDialog *pDlg, const char *Text, int Selected)
    : FarControl  (pDlg, DI_CHECKBOX, Text),
      m_pSelected (NULL)
{
    fItem->Selected = Selected;
}

FarCheckCtrl::FarCheckCtrl (FarDialog *pDlg, int LngIndex, int Selected)
    : FarControl  (pDlg, DI_CHECKBOX, LngIndex),
      m_pSelected (NULL)
{
    fItem->Selected = Selected;
}

FarCheckCtrl::FarCheckCtrl (FarDialog *pDlg, const char *Text, bool *pSelected)
    : FarControl  (pDlg, DI_CHECKBOX, Text),
      m_pSelected (reinterpret_cast<int *> (pSelected))
{
    fItem->Selected = *pSelected;
}

FarCheckCtrl::FarCheckCtrl (FarDialog *pDlg, int LngIndex, bool *pSelected)
    : FarControl  (pDlg, DI_CHECKBOX, LngIndex),
      m_pSelected (reinterpret_cast<int *> (pSelected))
{
    fItem->Selected = *pSelected;
}

FarCheckCtrl::FarCheckCtrl (FarDialog *pDlg, const char *Text, int *pSelected)
    : FarControl  (pDlg, DI_CHECKBOX, Text),
      m_pSelected (pSelected)
{
    fItem->Selected = *pSelected;
}

FarCheckCtrl::FarCheckCtrl (FarDialog *pDlg, int LngIndex, int *pSelected)
    : FarControl  (pDlg, DI_CHECKBOX, LngIndex),
      m_pSelected (pSelected)
{
    fItem->Selected = *pSelected;
}

BOOL FarCheckCtrl::Validate()
{
    if (m_pSelected)
        *m_pSelected = GetSelected();
    return TRUE;
}

// -- FarRadioGroup ----------------------------------------------------------

FarRadioCtrl *FarRadioGroup::AddItem( const char * Text, int X )
{
	int index = fItems.Count();
	FarRadioCtrl * pCtrl = create FarRadioCtrl( fDlg, Text, X );
	pCtrl->SetOwned();     // the dialog will own it
	fItems.Add( pCtrl );
	if ( index == fSelectedIndex )
		pCtrl->fItem->Selected = 1;
	if ( index == 0 )
		pCtrl->fItem->Flags |= DIF_GROUP;
	return pCtrl;
}

FarRadioCtrl *FarRadioGroup::AddItem( int LngIndex, int X )
{
	return AddItem( Far::GetMsg( LngIndex ), X );
}

int FarRadioGroup::GetSelectedIndex()
{
	for (int i=0; i<fItems.Count(); i++)
		if (fItems [i]->fItem->Selected)
			return i;

	return -1;
}

// -- FarBoxCtrl -------------------------------------------------------------

FarBoxCtrl::FarBoxCtrl (FarDialog *pDlg, BOOL Double, int X1, int Y1,
                        int W, int H, const char *Title)
  : FarControl (pDlg, Double ? DI_DOUBLEBOX : DI_SINGLEBOX, Title)
{
    fItem->X1 = X1;
    fItem->Y1 = Y1;
	fItem->X2 = W;
    fItem->Y2 = H;
}

FarBoxCtrl::FarBoxCtrl (FarDialog *pDlg, BOOL Double, int X1, int Y1,
                        int W, int H, int LngIndex)
  : FarControl (pDlg, Double ? DI_DOUBLEBOX : DI_SINGLEBOX, LngIndex)
{
    fItem->X1 = X1;
    fItem->Y1 = Y1;
    fItem->X2 = W;
    fItem->Y2 = H;
}

// -- FarButtonCtrl ----------------------------------------------------------

FarButtonCtrl::FarButtonCtrl (FarDialog *pDlg, const char *Text, int X)
  : FarControl (pDlg, DI_BUTTON, Text)
{
    fItem->X1 = X;
}

FarButtonCtrl::FarButtonCtrl (FarDialog *pDlg, int LngIndex, int X)
  : FarControl (pDlg, DI_BUTTON, LngIndex)
{
    fItem->X1 = X;
}

#ifdef USE_FAR_170

// -- FarBaseListCtrl --------------------------------------------------------

FarBaseListCtrl::FarBaseListCtrl (FarDialog *pDlg, unsigned char Type, int X, int Width)
	: FarControl (pDlg, Type)
{
	fItem->X1 = X;
	fItem->X2 = Width;
}

void FarBaseListCtrl::AddItem (const char *Text, int Flags /* = 0 */)
{
	FarListItem newItem;
	memset (&newItem, 0, sizeof (newItem));
	strncpy (newItem.Text, Text, sizeof (newItem.Text)-1);
	newItem.Flags = Flags;
	fListItems.Add (newItem);
}

void FarBaseListCtrl::BeforeShow()
{
	fFarList.ItemsNumber = fListItems.Count();
	fFarList.Items = fListItems.GetItems();
	fItem->ListItems = &fFarList;
}

// -- FarComboBox ------------------------------------------------------------

BOOL FarComboBox::Validate()
{
	if (fStringPtr)
		*fStringPtr = fItem->Data;
	return TRUE;
}

#endif
