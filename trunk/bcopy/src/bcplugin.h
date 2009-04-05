/*
    bcplugin.h
    Copyright (C) 2000-2009 zg

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

#include "bcCommon.h"

enum {
  mName,
  //main menu items
  mMenuCopy,
  mMenuMove,
  mMenuDelete,
  mMenuAttr,
  mMenuWipe,
  mMenuRun,
  mMenuSep1,
  mMenuView,
  mMenuEdit,
  mMenuName,
  mMenuSep2,
  mMenuEject,
  mMenuReject,
  mMenuRefreshSCSI,
  mMenuSep3,
  mMenuConfig,
  mMenuInfo,
  //copy dialog
  mCpyDlgCopyTitle,
  mCpyDlgMoveTitle,
  mCpyDlgOkCopy,
  mCpyDlgOkMove,
  mCpyDlgCancel,
  mCpyDlgCopyTo1,
  mCpyDlgCopyTox1,
  mCpyDlgCopyTo234,
  mCpyDlgCopyToN,
  mCpyDlgMoveTo1,
  mCpyDlgMoveTox1,
  mCpyDlgMoveTo234,
  mCpyDlgMoveToN,
  mCpyDlgOpt1,
  mCpyDlgOpt2,
  mCpyDlgOpt3,
  mCpyDlgOpt4,
  mCpyDlgAsk,
  mCpyDlgChk1,
  mCpyDlgChk2,
  mCpyDlgChk3,
  mCpyDlgChk4,
  mCpyDlgChkAccess,
  mCpyDlgChkLink,
  //delete dialog
  mDelDlgTitle,
  mDelDlgOk,
  mDelDlgCancel,
  mDelDlgDelete1,
  mDelDlgDeletex1,
  mDelDlgDelete234,
  mDelDlgDeleteN,
  mDelDlgChk1,
  mDelDlgChk2,
  //wipe dialog
  mWpeDlgTitle,
  mWpeDlgOk,
  mWpeDlgCancel,
  mWpeDlgWipe1,
  mWpeDlgWipex1,
  mWpeDlgWipe234,
  mWpeDlgWipeN,
  mWpeDlgChk,
  //attributes dialog
  mAttrDlgTitle,
  mAttrDlgTitle2,
  mAttrDlgChange1,
  mAttrDlgChangex1,
  mAttrDlgChange234,
  mAttrDlgChangeN,
  mAttrDlgReadOnly,
  mAttrDlgArchive,
  mAttrDlgHidden,
  mAttrDlgSystem,
  mAttrDlgCompressed,
  mAttrDlgSet,
  mAttrDlgCancel,
  //eject
  mEjectEject,
  mEjectLoad,
  mEjectRetry,
  mEjectIgnore,
  mEjectCancel,
  mEjectCurrent,
  //info
  mInfoCopy,
  mInfoMove,
  mInfoWipe,
  mInfoDel,
  mInfoAttr,
  mInfoSep,
  mInfoBottom,
  mInfoDlgOk,
  mInfoDlgStop,
  mInfoDlgPause,
  mInfoDlgContinue,

  mInfoDlgCopy,
  mInfoDlgMove,
  mInfoDlgWipe,
  mInfoDlgDelete,
  mInfoDlgAttr,
  mInfoDlgTo,
  mInfoDlgStatus1,
  mInfoDlgStatus1a,
  mInfoDlgStatus2,
  mInfoDlgStatus2a,
  mInfoDlgStatus3,
  mInfoDlgStatus3a,
  mInfoDlgStatus3b,
  mInfoDlgStatus4,
  mInfoDlgFill,
  mInfoDlgWait,
  //config
  mConfigMenu1,
  mConfigMenu2,
  mConfigMenu3,
  mConfigSave,
  mConfigCancel,
  mConfigHistory,
  mConfigFullInfo,
  mConfigDelAbort,
  mConfigWpeAbort,
  mConfigAutostart,
  mConfigConfigMenu,
  mConfigViewerMenu,
  mConfigEditorMenu,
  mConfigDialogMenu,
  mConfigInfo,
  mConfigInfoEmpty,
  mConfigInfoOnly,
  mComfigAutoShowInfo,
  mConfigExpandVars,
  mConfigPassive,
  mConfigFormatSize,
  mConfigConfirmAbort,
  mConfigCurrentTime,
  mConfigAutoRefresh,
  mConfigResolveDestination,
  mConfigMaxErrors,
  mConfigThreadCount,
  mConfigQueueSize,
  mConfigWork,
  mConfigHear,
  mConfigAllowNetwork,
  mConfigSetPassword,
  mConfigClearPassword,
  //priority
  mPriorityIdle,
  mPriorityLowest,
  mPriorityBelowNormal,
  mPriorityNormal,
  mPriorityAboveNormal,
  mPriorityHighest,
  mPriorityTimeCritical,
  //color
  mColorMain,
  mColorForeground,
  mColorBackground,
  mColorTest1,
  mColorTest2,
  mColorTest3,
  mColorSet,
  mColorCancel,
  //units
  mUnits1,
  mUnits2,
  mUnits3,
  mUnits4,
  mUnits5,
  mUnits6,
  //pwd
  mPwdTitle,
  mPwdSet,
  mPwdCancel,
  mPwdCleared,
  mPwdSeted,
  //overwrite
  mExistSource,
  mExistDestination,
  mExistDestAlreadyExists,
  mExistAcceptChoice,
  mExistOverwrite,
  mExistSkip,
  mExistAppend,
  mExistAbort,
  mExistClose,
  mExistIgnore,
  mExistRetry,
  mExistSkipAll,
  mExistCopy,
  mExistLink,
  mExistCannotProcess,
  mExistLinkFound,
  //errors
  mError,
  mCpyErr1,
  mCpyErr2,
  mCpyErr3,
  mErrorNoPipe,
  mNoWork,
  mActualize,
  mAbortText,
  mErrorList,
  mErrorMemory,
  mErrorWritePipe,
  mErrorReadPipe,
  //scsi
  mSCSIRefresh,
  //common buttons
  mOk,
  mYes,
  mNo,
};

#define FAR165_INFO_SIZE 336

struct CopyDialogData
{
  TCHAR CopyMessage[512];
  TCHAR MoveMessage[512];
  BOOL SrcRO;
  BOOL DstRO;
  BOOL Macro;
};

struct CommonDialogData
{
  BOOL Macro;
};

struct InitDialogItem
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  DWORD_PTR Selected;
  unsigned int Flags;
  int DefaultButton;
  const TCHAR *Data;
};

struct Options
{
  int  CopyType;
  BOOL CopyHistory;
  BOOL CopyROSrc;
  BOOL CopyRODest;
  DWORD CopyAbort;
  BOOL CopyFullInfo;
  BOOL CopyAccess;
  DWORD CopyLink;
  BOOL DeleteRO;
  DWORD DeleteAbort;
  DWORD WipeAbort;
};

struct PlugOptions
{
  DWORD AutoStart;
  DWORD ShowMenu;
  DWORD InfoMenu;
  DWORD ExpandVars;
  BOOL CheckPassive;
  BOOL FormatSize;
  BOOL AutoShowInfo;
  BOOL ConfirmAbort;
  BOOL CurrentTime;
  BOOL AutoRefresh;
  BOOL ForceEject;
  DWORD ErrorColor;
  BOOL IgnoreButton;
  BOOL ResolveDestination;
  BOOL ShowUnicode;
  BOOL RefreshInterval;
  BOOL Preload;
};

extern Options Opt;
extern PlugOptions PlgOpt;

extern PluginStartupInfo Info;
extern FARSTANDARDFUNCTIONS FSF;

extern void InitDialogItems(InitDialogItem *Init,FarDialogItem *Item,int ItemsNumber);
extern const TCHAR *GetMsg(int MsgId);
extern void UNCPath(TCHAR *path);
extern void NormalizeName(int width,int msg,TCHAR *filename,TCHAR *dest);
extern void NormalizeNameW(int width,int msg,wchar_t *filename,wchar_t *dest);
extern wchar_t *TruncPathStrW(wchar_t *Str,int MaxLength);
extern void ShowError(int Message,bool SysError);

extern void ShowInfoDialog(SmallInfoRec *receive);
extern void SendCommand(DWORD ThreadId,DWORD Command);
extern bool GetInfo(DWORD ThreadId,InfoRec *receive);
extern void AbortThread(DWORD ThreadId);

#ifdef __cplusplus
extern "C"{
#endif
extern void WINAPI _export ShowInfoMenu(void);
#ifdef __cplusplus
};
#endif

extern bool SelectColor(int *fg,int *bg);
extern void ShowName(const wchar_t *Name);

extern void load_macros_2(const TCHAR *registry);
extern void free_macros_2(void);
extern bool run_macro_2(int index,DWORD Key);

#define MACRO_COPY           0
#define MACRO_MOVE           1
#define MACRO_DELETE         2
#define MACRO_ATTR           3
#define MACRO_WIPE           4
#define MACRO_OVERWRITE      5
#define MACRO_RETRY          6
#define MACRO_INFO           7
#define MACRO_LINK           8
#define MACRO_INFO_MENU      9
#define MACRO_COUNT         10

#define BCOPY_REN_COPY    0
#define BCOPY_NOREAL_COPY 1
#define BCOPY_MOVE        2
#define BCOPY_FLAG_COUNT  3

#define SHOW_IN_VIEWER 1
#define SHOW_IN_EDITOR 2
#define SHOW_IN_CONFIG 4
#define SHOW_IN_DIALOG 8

#define INFO_MENU_ALT_0 1
#define INFO_MENU_ALT_1 2
