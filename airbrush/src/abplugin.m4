include(`ab_ver.m4')dnl
/*
    abplugin.h
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

#ifndef __ABPLUGIN_HPP__
#define __ABPLUGIN_HPP__

`#define AB_VERSION' eval(MAJOR*1000000+MINOR*10000+BUILD)
`#define AB_API' API

typedef int (WINAPI *COLORIZECALLBACK)(int from,int row,int col,void* param);

struct ColorizeParams
{
  int size;
  int eid;
  int startline;
  int startcolumn;
  int endline;
  int topline;
  unsigned long data_size;
  unsigned char* data;
  HANDLE LocalHeap;
  int flags;
  COLORIZECALLBACK callback;
  void *param;
};

enum ColorizePriority
{
  EPriorityNormal,
  EPriorityBrackets
};

#define AB_OPAQUE(x) ((x)|0xff000000)
#define ABCF_4BIT (0x0000000000000003ULL)

struct ABColor
{
  __int64 Flags;
  COLORREF ForegroundColor;
  COLORREF BackgroundColor;
  void* Reserved;
  bool ForegroundDefault;
  bool BackgroundDefault;
};

typedef void (WINAPI* PLUGINADDCOLOR)(int lno,int start,int len,const struct ABColor* color,enum ColorizePriority priority);
typedef const TCHAR* (WINAPI *PLUGINGETLINE)(int lno,int* len);
typedef bool (WINAPI* PLUGINADDSTATE)(int eid,int pos,unsigned long size,unsigned char* data);
typedef void (WINAPI* PLUGINGETCURSOR)(int* row,int* col);
typedef void (WINAPI* PLUGINCALLPARSER)(const TCHAR* parser,struct ColorizeParams* params);

struct ColorizeInfo
{
  int size;
  int version;
  int api;
  int cachestr;
  TCHAR folder[MAX_PATH];
  void* reserved;
  PLUGINADDCOLOR pAddColor;
  PLUGINGETLINE pGetLine;
  PLUGINADDSTATE pAddState;
  PLUGINGETCURSOR pGetCursor;
  PLUGINCALLPARSER pCallParser;
};

#ifdef __cplusplus
extern "C" {
#endif
  unsigned long WINAPI LoadSyntaxModule(char* ModuleName,void* Info);
  int WINAPI SetColorizeInfo(struct ColorizeInfo* AInfo);
  void WINAPI Colorize(int index,struct ColorizeParams* params);
  int WINAPI Input(const INPUT_RECORD* rec);
  unsigned long WINAPI GetSyntaxCount(void);
  void WINAPI Exit(void);
  int WINAPI GetParams(int index,int command,const char** param);
#ifdef __cplusplus
}
#endif

#define PAR_GET_NAME              0
#define PAR_GET_PARAMS            1
#define PAR_GET_MASK              2
#define PAR_CHECK_MASK            3
#define PAR_GET_FILESTART         4
#define PAR_CHECK_FILESTART       5
#define PAR_GET_COLOR_COUNT       6
#define PAR_GET_COLOR_NAME        7
#define PAR_GET_COLOR             8
#define PAR_CONFIGURE1            9

#define PAR_MASK_STORE            1
#define PAR_MASK_CACHE            2
#define PAR_FILESTART_STORE       4
#define PAR_FILESTART_CACHE       8
#define PAR_COLORS_STORE         16
#define PAR_COLORS_CACHE         32 //not used
#define PAR_SHOW_IN_LIST         64

#endif
