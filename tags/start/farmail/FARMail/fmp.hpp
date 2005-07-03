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
#ifndef __FMP_HPP__
#define __FMP_HPP__

typedef void (WINAPI *PLUGINGETMESSAGE)(const char *file_name,int index,char *message);
typedef int (WINAPI *PLUGINGETHEADERFIELD)(const char *header,const char *field_name,char *field,size_t field_size);
typedef int (WINAPI *PLUGINGETFREENUMBER)(char *dir);
typedef void (WINAPI *PLUGININITBOUNDARY)(char *bound);
typedef int (WINAPI *PLUGINSHOWHELPMENU)(const char *ModuleName, int PluginNumber, int X, int Y, int MaxHeight, unsigned int Flags, const char *Title, const char *Bottom, const char *HelpTopic, const int *BreakKeys, int *BreakCode, const struct FarMenuItem *Item, int ItemsNumber);
typedef long (WINAPI *PLUGINSGETVALUE)(int type);
typedef int (WINAPI *PLUGINSGETSTRING)(int type,char *str,size_t size,size_t *ret_size);
typedef int (WINAPI *PLUGINSADDRESSBOOK)(char *str);
typedef void (WINAPI *PLUGINSENCODEHEADER)(const char *in,char *out,size_t *out_size,const char *charset);

struct MailPluginStartupInfo
{
  int StructSize;
  char ModuleName[MAX_PATH];
  char MessageName[MAX_PATH];
  const char *RootKey;
  PLUGINGETMESSAGE GetMsg;
  PLUGINGETHEADERFIELD GetHeaderField;
  PLUGINGETFREENUMBER GetFreeNumber;
  PLUGININITBOUNDARY InitBoundary;
  FARWINDOWPROC ShowHelpDlgProc;
  PLUGINSHOWHELPMENU ShowHelpMenu;
  PLUGINSGETVALUE GetValue;
  PLUGINSGETSTRING GetString;
  PLUGINSADDRESSBOOK AddressBook;
  PLUGINSENCODEHEADER EncodeHeader;
};

struct GetInfoOutData
{
  unsigned long StructSize;
  unsigned long Flags;
  char MenuString[64];
  char HotkeyID[64];
};

struct AddressOutData
{
  unsigned long StructSize;
  char *Email;
};

struct MsgPanelInData
{
  unsigned long StructSize;
  HANDLE hPlugin;
  int index;
};

#ifdef __cplusplus
extern "C" {
#endif
  int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct MailPluginStartupInfo *FarMailInfo);
  void WINAPI _export Exit(void);
  int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData);
#ifdef __cplusplus
}
#endif

#define FMMSG_GETINFO               1
#define FMMSG_MENU                  2
#define FMMSG_CONFIG                4
#define FMMSG_ADDRESSBOOK           8
#define FMMSG_FILTER               16

/*
#define FMMSG_PUTTOMAILBOX         xx
#define FMMSG_PROCCESSEDITORINPUT  xx
#define FMMSG_NEWMAIL              xx
*/

#define FMSTR_START                 0
#define FMSTR_END                   1
#define FMSTR_MSGEXT                2

#define FMVAL_VERSION_MAJOR         0
#define FMVAL_VERSION_MINOR         1
#define FMVAL_VERSION_BUILD         2

#endif
