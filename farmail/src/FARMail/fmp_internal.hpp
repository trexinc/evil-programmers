/*
    FARMail plugin for FAR Manager
    Copyright (C) 2002-2004 FARMail Group

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
#ifndef __FMP_INTERNAL_HPP__
#define __FMP_INTERNAL_HPP__

typedef int (WINAPI *PLUGINSTART)(const struct PluginStartupInfo *FarInfo,const struct MailPluginStartupInfo *FarMailInfo);
typedef void (WINAPI *PLUGINEXIT)(void);
typedef int (WINAPI *PLUGMESSAGE)(unsigned long Msg,void *InData,void *OutData);

struct PluginItem
{
  HMODULE hModule;
  unsigned long Flags;
  char MenuString[64];
  char HotkeyID[64];
  char Hotkey;
  PLUGINSTART pStart;
  PLUGINEXIT pExit;
  PLUGMESSAGE pMessage;
};

class PluginManager
{
  private:
    PluginItem *PluginsData;
    char *Language;
    int PluginsCount;
    static int WINAPI FindPlugin(const WIN32_FIND_DATA *fdata,const char *filename,PluginManager *self);
    void AddPlugin(const char *filename);
    int ShowMenu(unsigned long flag,int select=0,int *selected=NULL,void *InData=NULL, void *OutData=NULL);
    void SendBroadcastMessage(unsigned long Msg,void *InData,void *OutData);
    int SendMessage(int Index,unsigned long Msg,void *InData,void *OutData);
    void SetHotkey(int index);
    bool UpdateLanguage(void);
    void LoadPluginsData(void);
  public:
    PluginManager(char *plug_dir);
    ~PluginManager();
    int ShowEditorMenu(void);
    int ShowConfigMenu(void);
    int ShowAddressBookMenu(char *str);
    int ShowFilterMenu(HANDLE hPlugin,int index);
};

extern char PluginPluginsRootKey[100];

extern void WINAPI api_get_message(const char *file_name,int index,char *message);
extern int WINAPI api_get_header_field(const char *header,const char *field_name,char *field,size_t field_size);
extern int WINAPI api_get_free_number(char *dir);
extern void WINAPI api_init_boundary(char *bound);
extern long WINAPI api_show_help_dlg_proc(HANDLE hDlg, int Msg,int Param1,long Param2);
extern int WINAPI api_show_help_menu(const char *ModuleName, int PluginNumber, int X, int Y, int MaxHeight, unsigned int Flags, const char *Title, const char *Bottom, const char *HelpTopic, const int *BreakKeys, int *BreakCode, const struct FarMenuItem *Item, int ItemsNumber);
extern long WINAPI api_get_value(int type);
extern int WINAPI api_get_string(int type,char *str,size_t size,size_t *ret_size);
extern int WINAPI api_address_book(char *str);
extern void WINAPI api_encode_header(const char *in,char *out,size_t *out_size,const char *charset);

#endif
