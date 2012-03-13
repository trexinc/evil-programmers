/*
    dt.hpp
    Copyright (C) 2008 zg

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

#ifndef __DT_HPP__
#define __DT_HPP__

#include "far_helper.h"

#define FAR165_INFO_SIZE 336

enum
{
  mNameCase,
  mNameFile,
  mNamePaste,
  mNamePwd,
  mNameSearch,
  mNameReplace,
  mNameUndo,
  mNameEdit,
  mLower,
  mUpper,
  mTitle,
  mToggle,
  mCyclic,
  mProcessWholeLine,
  mPwd,
  mOk,
  mCancel,
  mBottomTitle,
  mUndo,
  mRedo,
  mSearchCaption,
  mReplaceCaption,
  mSearch,
  mReplace,
  mCase,
  mCurPos,
  mSelect
};

struct InitDialogItem
{
  int Type;
  int X1, Y1, X2, Y2;
  int Selected;
  unsigned __int64 Flags;
  const TCHAR *Data;
};

extern PluginStartupInfo Info;
extern FARSTANDARDFUNCTIONS FSF;
extern TCHAR PluginRootKey[];
extern const TCHAR* GetMsg(int MsgId);
extern void FinishCase(void);
extern void DoCase(HANDLE aDlg);
extern void InitCase(void);
extern void DoPwd(HANDLE aDlg);
extern void DoOpenFile(HANDLE aDlg);
extern void FinishUndo(void);
extern void DoUndo(HANDLE aDlg);
extern void FilterUndo(HANDLE aDlg,int aMsg,int aParam1,void* aParam2);
extern void DoPaste(HANDLE aDlg);
extern void DoSearch(HANDLE aDlg);
extern void DoReplace(HANDLE aDlg);
extern void DoEdit(HANDLE aDlg);

#endif
