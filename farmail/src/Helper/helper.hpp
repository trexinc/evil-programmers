/*
    Helper sub-plugin for FARMail
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
#ifndef __HELPER_HPP__
#define __HELPER_HPP__
#include "registry.hpp"

#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

extern const char *NULLSTR;
extern struct PluginStartupInfo FarInfo;
extern FARSTANDARDFUNCTIONS FSF;
extern struct MailPluginStartupInfo FarMailInfo;
extern char msg1[128];

extern char *GetMsg(int MsgNum,char *Str);
extern bool open_file_dialog(const char *curr_dir,char *filename);

extern void *memset(void *s,int c,size_t n);
extern char *strchr(register const char *s,int c);
extern int get_line_count(void);
extern int get_current_line(void);
extern void set_current_line(int row,int col);
extern void add_blank(void);
extern void insert_string(const char *string,int row);
extern bool get_line(int lno,const char **str, int *len);

enum
{
  mName,
  //menu
  mAttach,
  mInsert,
  mPaste,
  mFARVer,
  mMailVer,
  mWinVer,
  //config
  mClipStart,
  mClipEnd,
  mFileStart,
  mFileEnd,
  //open file dialog
  mListBottomTitle,
  mOk,
  mCancel,
};

#endif
