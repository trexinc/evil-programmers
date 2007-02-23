/* $Header: /cvsroot/farplus/FARPlus/FARPlus.cpp,v 1.4 2002/09/05 06:34:06 yole Exp $
   FAR+Plus: A FAR Plugin C++ class library: source file
   (C) 1998-2002 Dmitry Jemerov <yole@spb.cityline.ru>
*/

#include "FARPlus.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// -- plugin size optimization -----------------------------------------------

#ifdef USE_WINDOWS_HEAP

void *my_malloc(size_t size)
{
  return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

void *my_realloc(void *block, size_t size)
{
  if (block)
    return HeapReAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,block,size);
  else
    return HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, size);
}

void my_free(void *block)
{
  HeapFree(GetProcessHeap(),0,block);
}

void * cdecl operator new(size_t size)
{
  return my_malloc(size);
}

void cdecl operator delete(void *block)
{
  my_free(block);
}

#endif

// -- Far --------------------------------------------------------------------

PluginStartupInfo Far::m_Info;
FarStringArray *Far::fDiskMenuStrings;
FarIntArray    *Far::fDiskMenuNumbers;
FarStringArray *Far::fPluginMenuStrings;
FarStringArray *Far::fPluginConfigStrings;
DWORD  Far::m_PluginFlags = 0;
char  *Far::m_CommandPrefix = NULL;

bool Far::m_Init = false;

void Far::Init (const PluginStartupInfo *Info, DWORD PluginFlags)
{
    FillMemory (&m_Info, 0, sizeof (m_Info));
    MoveMemory (&m_Info, Info, Info->StructSize);

#ifdef USE_FAR_170
    FillMemory (&FarSF::m_FSF, 0, sizeof (FarSF::m_FSF));
    if (GetVersion() >= 0x170)
        MoveMemory (&FarSF::m_FSF, Info->FSF, Info->FSF->StructSize);
#endif

    m_PluginFlags = PluginFlags;

	fDiskMenuStrings     = create FarStringArray (4);
	fDiskMenuNumbers     = create FarIntArray (4);
	fPluginMenuStrings   = create FarStringArray (4);
	fPluginConfigStrings = create FarStringArray (4);

	m_Init = true;
}

void Far::Done()
{
    ClearDiskMenu();
    ClearPluginMenu();
    ClearPluginConfig();
	delete fDiskMenuStrings;
	delete fDiskMenuNumbers;
	delete fPluginMenuStrings;
	delete fPluginConfigStrings;

    if (m_CommandPrefix) delete m_CommandPrefix;
	FarString::FreeEmptyString();

	m_Init = false;
}

////
///  GetVersion()
//   Returns version of FAR (0x170 - 1.70 beta 2, 0x160 - 1.60, 0x152 - 1.52,
//   0x151 - 1.51, 0x150 - 1.50 release, 0x14F - 1.50 beta, 0x140 - 1.40)

int Far::GetVersion()
{
	if (m_Info.StructSize >= 372) return 0x170;  // FAR standard functions,
	                                             // ShowHelp, AdvControl,
	                                             // InputBox, DialogEx,
	                                             // SendDlgMessage, DefDlgProc
    if (m_Info.StructSize >= 336) return 0x160;  // EditorControl
    if (m_Info.StructSize >= 332) return 0x152;  // CharTable, Text
    if (m_Info.StructSize >= 324) return 0x151;  // CmpName
    if (m_Info.StructSize >= 320) return 0x150;  // Viewer, Editor
    //if (Info->StructSize >= 312) return 0x14F;
    return 0x140;
}

bool Far::RequireVersion (int MinVersion)
{
    if (GetVersion() < MinVersion)
    {
        FarMessage().SimpleMsg (FMSG_WARNING | FMSG_MB_OK, 0,
            "This plugin requires a later version of FAR", -1);
        return false;
    }
    return true;
}

const char *Far::GetRootKey()
{
	return m_Info.RootKey;
}

// -- GetPluginInfo() helpers ------------------------------------------------

void Far::ClearDiskMenu()
{
	fDiskMenuStrings->Clear();
	fDiskMenuNumbers->Clear();
}

void Far::ClearPluginMenu()
{
    fPluginMenuStrings->Clear();
}

void Far::ClearPluginConfig()
{
	fPluginConfigStrings->Clear();
}

void Far::AddDiskMenu (const char *Text, int Number)
{
	far_assert (Text != NULL);
	fDiskMenuStrings->Add (Text);
	fDiskMenuNumbers->Add (Number);
}

void Far::AddDiskMenu (int LngIndex, int Number)
{
    AddDiskMenu (GetMsg (LngIndex), Number);
}

void Far::AddPluginMenu (const char *Text)
{
	far_assert (Text != NULL);
	fPluginMenuStrings->Add (Text);
}

void Far::AddPluginMenu (int LngIndex)
{
    AddPluginMenu (GetMsg (LngIndex));
}

void Far::AddPluginConfig (const char *Text)
{
	far_assert (Text != NULL);
	fPluginConfigStrings->Add (Text);
}

void Far::AddPluginConfig (int LngIndex)
{
    AddPluginConfig (GetMsg (LngIndex));
}

void Far::SetCommandPrefix (const char *CommandPrefix)
{
    if (m_CommandPrefix) delete m_CommandPrefix;
    m_CommandPrefix = create char [strlen (CommandPrefix)+1];
    strcpy (m_CommandPrefix, CommandPrefix);
}

void Far::GetPluginInfo (PluginInfo *Info)
{
    Info->StructSize=sizeof(*Info);
    Info->Flags=m_PluginFlags;
    Info->DiskMenuStrings           = fDiskMenuStrings->GetItems();
    Info->DiskMenuNumbers           = fDiskMenuNumbers->GetItems();
    Info->DiskMenuStringsNumber     = fDiskMenuStrings->Count();
    Info->PluginMenuStrings         = fPluginMenuStrings->GetItems();
    Info->PluginMenuStringsNumber   = fPluginMenuStrings->Count();
    Info->PluginConfigStrings       = fPluginConfigStrings->GetItems();
    Info->PluginConfigStringsNumber = fPluginConfigStrings->Count();
    Info->CommandPrefix=m_CommandPrefix;
}

void Far::FreePanelItems (PluginPanelItem *PanelItem, int ItemsNumber)
{
    for (int i=0; i<ItemsNumber; i++)
    {
        if (PanelItem [i].CustomColumnData)
        {
            for (int col=0; col<PanelItem [i].CustomColumnNumber; col++)
                if (PanelItem [i].CustomColumnData [col])
                    delete PanelItem [i].CustomColumnData [col];
            delete PanelItem [i].CustomColumnData;
        }
        if (PanelItem [i].Owner)
            delete PanelItem [i].Owner;
        if (PanelItem [i].Description)
            delete PanelItem [i].Description;
        if ((PanelItem [i].Flags & PPIF_USERDATA) && PanelItem [i].UserData)
            free((void *)PanelItem [i].UserData);
    }
    delete PanelItem;
}

// -- FarMessage -------------------------------------------------------------

FarMessage::FarMessage (unsigned int Flags, const char *HelpTopic)
  : m_Flags         (Flags),
    m_HelpTopic     (HelpTopic),
    m_Items         (NULL),
    m_ItemsNumber   (0),
    m_ButtonsNumber (0)
{
}

FarMessage::~FarMessage()
{
    if (m_ItemsNumber > 0)
    {
        for (int i=0; i<m_ItemsNumber; i++)
            delete m_Items [i];
        free (m_Items);
    }
}

void FarMessage::AddLine (const char *Text)
{
    m_Items = (char **) realloc (m_Items, ++m_ItemsNumber * sizeof (char *));
    for (int i=1; i<=m_ButtonsNumber; i++)
        m_Items [m_ItemsNumber-i] = m_Items [m_ItemsNumber-i-1];
    m_Items [m_ItemsNumber-m_ButtonsNumber-1] = create char [strlen (Text)+1];
    strcpy (m_Items [m_ItemsNumber-m_ButtonsNumber-1], Text);
}

void FarMessage::AddLine (int LngIndex)
{
    AddLine (Far::GetMsg (LngIndex));
}

void FarMessage::AddFmt (const char *FmtText, ...)
{
    char buf[256];
    va_list va;

    va_start (va, FmtText);
	  FarSF::vsnprintf( buf, sizeof(buf), FmtText, va );
    va_end (va);

    AddLine (buf);
}

void FarMessage::AddFmt (int FmtLngIndex, ...)
{
    char buf[256];
    va_list va;

    va_start (va, FmtLngIndex);
	  FarSF::vsnprintf( buf, sizeof(buf), Far::GetMsg (FmtLngIndex), va );
    va_end (va);

    AddLine (buf);
}

void FarMessage::AddSeparator()
{
    AddLine ("\001");
}

int FarMessage::AddButton (const char *Text)
{
    m_Items = (char **) realloc (m_Items, ++m_ItemsNumber * sizeof (char *));
    m_Items [m_ItemsNumber-1] = create char [strlen (Text)+1];
    strcpy (m_Items [m_ItemsNumber-1], Text);
    m_ButtonsNumber++;
    return m_ButtonsNumber-1;
}

int FarMessage::AddButton (int LngIndex)
{
    return AddButton (Far::GetMsg (LngIndex));
}

int FarMessage::Show()
{
    return Far::m_Info.Message (Far::m_Info.ModuleNumber,
        m_Flags, m_HelpTopic, (const char **) m_Items, m_ItemsNumber, m_ButtonsNumber);
}

int FarMessage::SimpleMsg (unsigned int Flags, ...)
{
    m_Flags = Flags;

    va_list va;
    va_start (va, Flags);

    while (1)
    {
        int item = va_arg (va, int);
        if (item == -1)
            break;
        if (item < 2000)
            AddLine (Far::GetMsg (item));
        else
            AddLine ((const char *) item);
    }
    va_end (va);

#ifndef USE_FAR_170
    if (m_Flags & FMSG_MB_OK)
    {
        AddButton (1);   // use hard-coded LNG index for Ok
        m_Flags &= ~FMSG_MB_OK;
    }
#endif

    return Show();
}

int FarMessage::ErrMsg (const char *Text, int ExtraFlags)
{
    m_Flags = FMSG_WARNING | ExtraFlags;
    AddLine (0);
    AddLine (Text);
    AddButton (1);
    return Show();
}

int FarMessage::ErrMsg (int LngIndex, int ExtraFlags)
{
    return ErrMsg (Far::GetMsg (LngIndex), ExtraFlags);
}

// -- FarMenu ----------------------------------------------------------------
/*
FarMenu::FarMenu (const char *TitleText, unsigned int Flags,
                  const char *HelpTopic)
  : fTitle         (TitleText),
    fFlags         (Flags),
	fHelpTopic     (HelpTopic),
    fBottom        (NULL),
    fBreakKeys     (NULL),
    fBreakCode     (-1),
    fX             (-1),
    fY             (-1),
    fMaxHeight     (0),
    fItems         (NULL),
    fItemsNumber   (0),
    fOwnsBreakKeys (false)
{
}

FarMenu::FarMenu (int TitleLngIndex, unsigned int Flags,
                  const char *HelpTopic)
  : fTitle         (Far::GetMsg (TitleLngIndex)),
    fFlags         (Flags),
	fHelpTopic     (HelpTopic),
    fBottom        (NULL),
    fBreakKeys     (NULL),
    fBreakCode     (-1),
    fX             (-1),
    fY             (-1),
    fMaxHeight     (0),
    fItems         (NULL),
    fItemsNumber   (0),
    fOwnsBreakKeys (false)
{
}

FarMenu::~FarMenu()
{
    if (fItems) free (fItems);
    if (fOwnsBreakKeys && fBreakKeys) delete [] fBreakKeys;
}

int FarMenu::InternalAddItem (const char *Text, int Selected, int Checked, int Separator)
{
    fItems = (FarMenuItem *) realloc (fItems,
        ++fItemsNumber * sizeof (FarMenuItem));
    lstrcpyn (fItems [fItemsNumber-1].Text, Text, sizeof (fItems [fItemsNumber-1].Text)-1);
    fItems [fItemsNumber-1].Selected = Selected;
    fItems [fItemsNumber-1].Checked = Checked;
    fItems [fItemsNumber-1].Separator = Separator;
    return fItemsNumber-1;
}

int FarMenu::AddItem (const char *Text, bool Selected, int Checked)
{
    return InternalAddItem (Text, Selected, Checked, 0);
}

int FarMenu::AddItem (int LngIndex, bool Selected, int Checked)
{
    return InternalAddItem (Far::GetMsg (LngIndex), Selected, Checked, 0);
}

int FarMenu::AddSeparator()
{
    return InternalAddItem ("", 0, 0, 1);
}

void FarMenu::ClearItems()
{
    delete [] fItems;
    fItems = NULL;
    fItemsNumber = 0;
}

void FarMenu::SelectItem (int index)
{
    if (index >= 0 && index < fItemsNumber)
        for (int i=0; i < fItemsNumber; i++)
            fItems [i].Selected = (i == index);
}

void FarMenu::SetBreakKeys (int aFirstKey, ...)
{
    int keyCount = 0;
    int curKey = aFirstKey;
    va_list va;
    va_start (va, aFirstKey);
    while (curKey != NULL)
    {
        keyCount++;
        curKey = va_arg (va, int);
    }
    va_end (va);

    if (fOwnsBreakKeys && fBreakKeys)
        delete fBreakKeys;
    fOwnsBreakKeys = true;
    fBreakKeys = new int [keyCount+1];

    int *pKey = fBreakKeys;
    curKey = aFirstKey;
    va_start (va, aFirstKey);
    while (curKey != NULL)
    {
        *pKey++ = curKey;
        curKey = va_arg (va, int);
    }
    va_end (va);

    *pKey = curKey;  // put terminating NULL
}

int FarMenu::Show()
{
    return Far::m_Info.Menu (Far::m_Info.ModuleNumber,
        fX, fY, fMaxHeight, fFlags,
        fTitle, fBottom, fHelpTopic,
        fBreakKeys, &fBreakCode,
        fItems, fItemsNumber);
}
*/
