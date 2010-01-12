/*
    FARMail plugin for FAR Manager
    Copyright (C) 2002-2004 FARMail Group
    Copyright (C) 1999,2000 Serge Alexandrov

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
#include "farmail.hpp"

#define Dialog_Enable(Index) _Info.SendDlgMessage(hDlg,DM_ENABLE,Index,TRUE)
#define Dialog_Disable(Index) _Info.SendDlgMessage(hDlg,DM_ENABLE,Index,FALSE)
#define Dialog_IsEnabled(Index) _Info.SendDlgMessage(hDlg,DM_ENABLE,Index,-1)
#define Dialog_Focus(Index) _Info.SendDlgMessage(hDlg,DM_SETFOCUS,Index,0)
#define Dialog_GetText(Index,Text) _Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,Index,(long)Text)
#define Dialog_Focus(Index) _Info.SendDlgMessage(hDlg,DM_SETFOCUS,Index,0)
#define Dialog_SetState(Index,State) _Info.SendDlgMessage(hDlg,DM_SETCHECK,Index,State?BSTATE_CHECKED:BSTATE_UNCHECKED)
#define Dialog_GetState(Index) _Info.SendDlgMessage(hDlg,DM_GETCHECK,Index,0)

  enum
  {
    C_TITLE,
    C_LINEHORZ1,
    C_TXT1,
    C_TXT2,
    C_TXT3,
    C_TXT4,
    C_TXT5,
    C_FRQCSET1,
    C_FRQTBL1,
    C_FRQCSET2,
    C_FRQTBL2,
    C_ADDCSET1,
    C_ADDTBL1,
    C_ADDCSET2,
    C_ADDTBL2,
    C_ADDCSET3,
    C_ADDTBL3,
    C_RB1_1,
    C_RB1_2,
    C_RB1_3,
    C_RB1_4,
    C_RB1_5,
    C_RB1_6,
    C_RB2_1,
    C_RB2_2,
    C_RB2_3,
    C_RB2_4,
    C_RB2_5,
    C_RB2_6,
    C_H1_1,
    C_H1_2,
    C_H1_3,
    C_H1_4,
    C_H1_5,
    C_H1_6,
    C_H2_1,
    C_H2_2,
    C_H2_3,
    C_H2_4,
    C_H2_5,
    C_H2_6,
    C_LINEVERT1,
    C_LINEVERT2,
    C_LINEVERT3,
    C_LINEHORZ2,
    C_LINEHORZ3,
    C_LINEHORZ4,
    C_TXT6,
    C_TXT7,
    C_TXT8,
    C_OK,
    C_CANCEL,
  };

CHARSET_TABLE *CharsetTable;

void ReadRegistryEncodings(void)
{
  HKEY hRoot = HKEY_CURRENT_USER;
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, DEFAULTCHARSET,    Opt.DefCharset, GetMsg(MesDefKOICharset), 20 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, DEFAULTOUTCHARSET, Opt.DefOutCharset, GetMsg(MesDefKOICharset), 20 );
}

BOOL GenerateFARTableList(struct FarList *List, int ListsNumber)
{
 struct CharTableSet cts;

 for (int i=0; i<ListsNumber; i++)
 {
   List[i].ItemsNumber = 0;
   List[i].Items = NULL;
 }

 for (int i=0; _Info.CharTable(i, (char *)&cts, sizeof(struct CharTableSet)) != -1; i++)
 {
   for (int j=0; j<ListsNumber; j++)
   {
     List[j].ItemsNumber++;
     List[j].Items = (struct FarListItem *)z_realloc(List[j].Items, (i+1)*sizeof(struct FarListItem));
     if (!List[j].Items)
     {
       List[j].ItemsNumber = 0;
       return FALSE;
     }
     lstrcpy(List[j].Items[i].Text,cts.TableName);
     List[j].Items[i].Reserved[0] = List[j].Items[i].Reserved[1] = List[j].Items[i].Reserved[2] = 0;
     if ((*CharsetTable[j+2].table)&&!FSF.LStrnicmp(cts.TableName, CharsetTable[j+2].table, lstrlen(CharsetTable[j+2].table)))
       List[j].Items[i].Flags = LIF_SELECTED;
     else
       List[j].Items[i].Flags = 0;
   }
 }
 return TRUE;
}


static long WINAPI Settings_EncodingsDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  static int *changed_items;
  switch(Msg)
  {
    case DN_INITDIALOG:
      changed_items = (int *) Param2;
      return FALSE;
    case DN_EDITCHANGE:
      switch (Param1)
      {
        case C_FRQTBL1:
        case C_FRQTBL2:
        case C_ADDTBL1:
        case C_ADDTBL2:
        case C_ADDTBL3:
          if (Dialog_GetText(Param1-C_FRQTBL1+C_FRQCSET1,NULL))
          {
            Dialog_Enable((Param1-C_FRQTBL1)/2+C_RB1_2);
            Dialog_Enable((Param1-C_FRQTBL1)/2+C_RB2_2);
          }
          break;
        case C_FRQCSET1:
        case C_FRQCSET2:
        case C_ADDCSET1:
        case C_ADDCSET2:
        case C_ADDCSET3:
          if (Dialog_GetText(Param1,NULL))
          {
            if (Dialog_GetText(Param1-C_FRQCSET1+C_FRQTBL1,NULL) == 1)
            {
              char tmp[2];
              Dialog_GetText(Param1-C_FRQCSET1+C_FRQTBL1,tmp);
              if (tmp[0] == 0x20)
                break;
            }
            Dialog_Enable((Param1-C_FRQCSET1)/2+C_RB1_2);
            Dialog_Enable((Param1-C_FRQCSET1)/2+C_RB2_2);
          }
          else
          {
            if (Dialog_GetState((Param1-C_FRQCSET1)/2+C_RB1_2))
            {
              Dialog_SetState((Param1-C_FRQCSET1)/2+C_RB1_2,false);
              Dialog_SetState(C_RB1_1,true);
            }
            if (Dialog_GetState((Param1-C_FRQCSET1)/2+C_RB2_2))
            {
              Dialog_SetState((Param1-C_FRQCSET1)/2+C_RB2_2,false);
              Dialog_SetState(C_RB2_1,true);
            }
            Dialog_Disable((Param1-C_FRQCSET1)/2+C_RB1_2);
            Dialog_Disable((Param1-C_FRQCSET1)/2+C_RB2_2);
          }
          break;
      }
      break;
    case DN_HOTKEY:
      switch(Param1)
      {
        case C_H1_1:
        case C_H1_2:
        case C_H1_3:
        case C_H1_4:
        case C_H1_5:
        case C_H1_6:
        {
          if (!Dialog_IsEnabled(Param1-C_H1_1+C_RB1_1))
            return FALSE;
          Dialog_Focus(Param1-C_H1_1+C_RB1_1);
          Dialog_SetState(Param1-C_H1_1+C_RB1_1,true);
          return FALSE;
        }
        case C_H2_1:
        case C_H2_2:
        case C_H2_3:
        case C_H2_4:
        case C_H2_5:
        case C_H2_6:
        {
          if (!Dialog_IsEnabled(Param1-C_H2_1+C_RB2_1))
            return FALSE;
          Dialog_Focus(Param1-C_H2_1+C_RB2_1);
          Dialog_SetState(Param1-C_H2_1+C_RB2_1,true);
          return FALSE;
        }
      }
      break;
  }
  return _Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void Settings_Encodings(const char *txt)
{
  static struct InitDialogItem InitItems[] =
  {
    // type, x1, y1, x2, y2, focus, selected, flags, default, data
    { DI_DOUBLEBOX,3,1,74,19,0,0,0,0,(char*)MesConfig_MainTitle },

    { DI_TEXT, 15, 2, 0,0,0,0,0,0, (char *)MesConfig_Charset },
    { DI_TEXT, 4, 3, 0,0,0,0,0,0, "ÚÄÄÄÄÂÄÄÄÄÂÄÄÄ" },
    { DI_TEXT, 17, 4, 0,0,0,0,0,0, (char *)MesConfig_UndefinedCharset},

    { DI_TEXT, 16, 6, 0,0,0,0,0,0, (char *)MesConfig_Frequent},
    { DI_TEXT, 38, 6, 0,0,0,0,0,0, (char *)MesConfig_FARTable},
    { DI_TEXT, 16, 10, 0,0,0,0,0,0, (char *)MesConfig_Aditional},
    { DI_EDIT, 17, 7, 37, 0, 1, 0, 0, 0, NULLSTR },
    { DI_COMBOBOX, 39, 7, 72, 0, 0, 0, DIF_DROPDOWNLIST, 0, NULLSTR },

    { DI_EDIT, 17, 8, 37, 0, 0, 0, 0, 0, NULLSTR },
    { DI_COMBOBOX, 39, 8, 72, 0, 0, 0, DIF_DROPDOWNLIST, 0, NULLSTR },

    { DI_EDIT, 17, 11, 37, 0, 0, 0, 0, 0, NULLSTR },
    { DI_COMBOBOX, 39, 11, 72, 0, 0, 0, DIF_DROPDOWNLIST, 0, NULLSTR },

    { DI_EDIT, 17, 12, 37, 0, 0, 0, 0, 0, NULLSTR },
    { DI_COMBOBOX, 39, 12, 72, 0, 0, 0, DIF_DROPDOWNLIST, 0, NULLSTR },

    { DI_EDIT, 17, 13, 37, 0, 0, 0, 0, 0, NULLSTR },
    { DI_COMBOBOX, 39, 13, 72, 0, 0, 0, DIF_DROPDOWNLIST, 0, NULLSTR },

    { DI_RADIOBUTTON, 6, 4, 0, 0, 0, 0, DIF_GROUP, 0, NULLSTR },
    { DI_RADIOBUTTON, 6, 7, 0, 0, 0, 0, 0, 0, NULLSTR },
    { DI_RADIOBUTTON, 6, 8, 0, 0, 0, 0, 0, 0, NULLSTR },
    { DI_RADIOBUTTON, 6, 11,0, 0, 0, 0, 0, 0, NULLSTR },
    { DI_RADIOBUTTON, 6, 12,0, 0, 0, 0, 0, 0, NULLSTR },
    { DI_RADIOBUTTON, 6, 13,0, 0, 0, 0, 0, 0, NULLSTR },

    { DI_RADIOBUTTON, 11, 4, 0, 0, 0, 0, DIF_GROUP, 0, NULLSTR },
    { DI_RADIOBUTTON, 11, 7, 0, 0, 0, 0, 0, 0, NULLSTR },
    { DI_RADIOBUTTON, 11, 8, 0, 0, 0, 0, 0, 0, NULLSTR },
    { DI_RADIOBUTTON, 11, 11,0, 0, 0, 0, 0, 0, NULLSTR },
    { DI_RADIOBUTTON, 11, 12,0, 0, 0, 0, 0, 0, NULLSTR },
    { DI_RADIOBUTTON, 11, 13,0, 0, 0, 0, 0, 0, NULLSTR },

    { DI_TEXT, 5, 4, 0, 0, 0, 0, 0, 0, (char*)MesHotkey1 },
    { DI_TEXT, 5, 7, 0, 0, 0, 0, 0, 0, (char*)MesHotkey2 },
    { DI_TEXT, 5, 8, 0, 0, 0, 0, 0, 0, (char*)MesHotkey3 },
    { DI_TEXT, 5, 11,0, 0, 0, 0, 0, 0, (char*)MesHotkey4 },
    { DI_TEXT, 5, 12,0, 0, 0, 0, 0, 0, (char*)MesHotkey5 },
    { DI_TEXT, 5, 13,0, 0, 0, 0, 0, 0, (char*)MesHotkey6 },

    { DI_TEXT, 10, 4, 0, 0, 0, 0, 0, 0, (char *)MesHotkeyA },
    { DI_TEXT, 10, 7, 0, 0, 0, 0, 0, 0, (char *)MesHotkeyB },
    { DI_TEXT, 10, 8, 0, 0, 0, 0, 0, 0, (char *)MesHotkeyC },
    { DI_TEXT, 10, 11,0, 0, 0, 0, 0, 0, (char *)MesHotkeyD },
    { DI_TEXT, 10, 12,0, 0, 0, 0, 0, 0, (char *)MesHotkeyE },
    { DI_TEXT, 10, 13,0, 0, 0, 0, 0, 0, (char *)MesHotkeyF },

    { DI_VTEXT,4, 4, 0,0,0,0,0,0, "³³³³³³³³³³" },
    { DI_VTEXT,9,4, 0,0,0,0,0,0, "³³³³³³³³³³" },
    { DI_VTEXT,14,4, 0,0,0,0,0,0, "³³³³³³³³³³" },
    { DI_TEXT, 4, 14, 0,0,0,0,0,0, "ÀÄÂÄÄÁÄÂÄÄÁÄÄÄ" },
    { DI_TEXT, 4, 15, 0,0,0,0,0,0, "  ³    ÀÄÄÄÄ" },
    { DI_TEXT, 4, 16, 0,0,0,0,0,0, "  ÀÄÄÄÄÄÄÄÄÄ" },
    { DI_TEXT, 17, 15, 0,0,0,0,0,0, (char *)MesConfig_Outgoing},
    { DI_TEXT, 17, 16, 0,0,0,0,0,0, (char *)MesConfig_Incoming},
    {DI_TEXT,3,17,0,0,0,0,DIF_SEPARATOR,0,NULLSTR},

    { DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,1,(char *)MesOk },
    { DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MesCancel },
  };
  struct FarDialogItem DialogItems[sizeofa(InitItems)];

  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));

  ReadRegistryEncodings();

  CharsetTable = *(GetCharsetTable());
  ReadCharsetTable(CharsetTable);
  InitCharset(&CharsetTable);

  {
    int i = FindCharset(Opt.DefCharset, &CharsetTable);
    if (i>1)
      DialogItems[C_RB1_2+i-2].Selected = TRUE;
    else
      DialogItems[C_RB1_1].Selected = TRUE;

    i = FindCharset(Opt.DefOutCharset, &CharsetTable);
    if (i>1)
      DialogItems[C_RB2_2+i-2].Selected = TRUE;
    else
      DialogItems[C_RB2_1].Selected = TRUE;
  }

  struct FarList List[5];
  if (!GenerateFARTableList(List,5))
  {
    SayError(GetMsg(MesNoMem));
    for (int i=0; i<5; i++)
    {
      z_free(List[i].Items);
    }
    return;
  }
  for (int i=0; i<5; i++)
  {
    DialogItems[C_FRQTBL1+i*2].ListItems = &List[i];
    if (!*(CharsetTable[i+2].table))
      lstrcpy( DialogItems[C_FRQTBL1+i*2].Data , " " );
    else
      lstrcpy( DialogItems[C_FRQCSET1+i*2].Data, CharsetTable[i+2].charset);

    if (CharsetTable[i+2].num == UNKNOWN_CHARSET_TABLE)
    {
      DialogItems[C_RB1_2+i].Flags |= DIF_DISABLE;
      DialogItems[C_RB2_2+i].Flags |= DIF_DISABLE;
    }
  }

  int l = lstrlen(DialogItems[0].Data);
  lstrcat( DialogItems[0].Data, ": " );
  lstrcat( DialogItems[0].Data, txt );
  FSF.LStrlwr( DialogItems[0].Data+l );


  if ( _Info.DialogEx(_Info.ModuleNumber,-1,-1,78,21,"Encodings",DialogItems,sizeofa(DialogItems),0,0,Settings_EncodingsDialogProc,0) == C_OK )
  {
     for (int i=0; i<5; i++)
     {
       lstrcpyn(  CharsetTable[i+2].charset , DialogItems[C_FRQCSET1+i*2].Data , 20 );
       lstrcpy(CharsetTable[i+2].table,DialogItems[C_FRQTBL1+i*2].Data);
       if (!lstrcmp(CharsetTable[i+2].table," ") || !*(CharsetTable[i+2].charset))
         *(CharsetTable[i+2].table) = 0;

       if (DialogItems[C_RB1_2+i].Selected)
          lstrcpy( Opt.DefCharset , CharsetTable[i+2].charset );
       if (DialogItems[C_RB2_2+i].Selected)
          lstrcpy( Opt.DefOutCharset , CharsetTable[i+2].charset );
     }
     if (DialogItems[C_RB1_1].Selected)
       *(Opt.DefCharset) = 0;
     if (DialogItems[C_RB2_1].Selected)
       *(Opt.DefOutCharset) = 0;

     HKEY hRoot = HKEY_CURRENT_USER;

     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, DEFAULTCHARSET,    Opt.DefCharset );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, DEFAULTOUTCHARSET, Opt.DefOutCharset );

     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, MAINTABLE1,   CharsetTable[2].table );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, MAINTABLE2,   CharsetTable[3].table );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, MAINCHARSET1, CharsetTable[2].charset );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, MAINCHARSET2, CharsetTable[3].charset );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, CHARSET1,     CharsetTable[4].charset );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, CHARSET2,     CharsetTable[5].charset );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, CHARSET3,     CharsetTable[6].charset );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, TABLE1,       CharsetTable[4].table );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, TABLE2,       CharsetTable[5].table );
     SetRegKey2( hRoot, PluginCommonKey, NULLSTR, TABLE3,       CharsetTable[6].table );
  }

  for (int i=0; i<5; i++)
  {
    z_free(List[i].Items);
  }
}
