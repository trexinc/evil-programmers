/*
    ab_main.h
    Copyright (C) 2000-2008 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __AB_MAIN_HPP__
#define __AB_MAIN_HPP__

// strings in lng file
enum {
  mName,
  mUnknown,
  mDefault,
  mLoading,
  mConfigMenu1,
  mConfigMenu2,
  mConfigMenu3,
  mConfigMenu4,
  mConfigDialogActive,
  mConfigDialogColorizeAll,
  mConfigDialogMaxLines,
  mConfigSave,
  mConfigCancel,
  mStopQuestion,
  mFatalLine1,
  mFatalLine2,
  mButtonOk,
  mButtonYes,
  mButtonNo,
  mError,
};

const TCHAR* GetMsg(int MsgId);
TCHAR* GetCommaWord(TCHAR* Src,TCHAR* Word);

typedef unsigned long (WINAPI *PLUGINLOADSYNTAXMODULE)(TCHAR* ModuleName,void *Info);
typedef int (WINAPI *PLUGINSETCOLORIZEINFO)(struct ColorizeInfo *AInfo);
typedef int (WINAPI *PLUGINGETPARAMS)(int index,int command,const char **param);
typedef void (WINAPI *PLUGINCOLORIZE)(int index,struct ColorizeParams *params);
typedef int (WINAPI *PLUGININPUT)(const INPUT_RECORD *rec);
typedef unsigned long (WINAPI *PLUGINGETSYNTAXCOUNT)(void);
typedef void (WINAPI *PLUGINEXIT)(void);

struct PluginItem
{
  HMODULE hModule;
  int Index;
  int Type;
  const TCHAR* Name;
  TCHAR* Mask;
  TCHAR* Start;
  int Params;
  PLUGINLOADSYNTAXMODULE pLoadSyntaxModule;
  PLUGINSETCOLORIZEINFO pSetColorizeInfo;
  PLUGINGETPARAMS pGetParams;
  PLUGINCOLORIZE pColorize;
  PLUGININPUT pInput;
  PLUGINGETSYNTAXCOUNT pGetSyntaxCount;
  PLUGINEXIT pExit;
};

extern struct PluginItem *PluginsData;
extern int PluginsCount;

struct StateCache
{
  unsigned long data_size;
  unsigned char *data;
  StateCache *next;
};

typedef struct CEditFile *PEditFile;

struct CEditFile
{
  int id;
  bool main;
  PEditFile next, prev;
//
  StateCache *cache;
  int cachesize;
  int type;
  int topline;
};

//struct CEditFile methods
extern PEditFile ef_create(bool m = false);
extern void ef_free(PEditFile value);
extern PEditFile ef_getfile(int fid);
extern PEditFile ef_addfile(int fid);
extern bool ef_deletefile(int fid);

extern PEditFile editfiles;

extern PEditFile loadfile(int eid,int type);

extern void OnLoad(void);
extern void OnExit(void);
extern void OnConfigure(void);
extern int OnEditorEvent(int event, void *param,int editorid);
extern int OnEditorInput(const INPUT_RECORD *Rec);

#define PARSER_CACHESTR 50
#define FAR165_INFO_SIZE 336

extern PluginStartupInfo Info;
extern FARSTANDARDFUNCTIONS FSF;
extern HANDLE Mutex;
#define PLUGIN_MASK_KEY _T("masks")
#define PLUGIN_COLOR_KEY _T("colors")
#define PLUGIN_START_KEY _T("starts")

extern void LoadPlugs(const TCHAR* ModuleName);
extern void UnloadPlugs(void);

extern void ConvertColor(const ABColor& Color,FarColor& NewColor);
extern void SaveColors(const wchar_t* Name,const ABColor* Colors,const size_t Count);
extern void LoadColors(const wchar_t* Name,ABColor* Colors,size_t Count);

struct Options
{
  bool Active;
  __int64 MaxLines;
  bool ColorizeAll;
};

extern struct Options Opt;
extern bool fatal;

#define FG_MASK (~FCF_BG_4BIT)
#define BG_MASK (FCF_BG_4BIT)

#endif
