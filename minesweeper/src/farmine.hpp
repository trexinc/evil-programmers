#ifndef __FARMINE_HPP__
#define __FARMINE_HPP__

enum {
  mName,
  mStart,
  mGame,
  mWon,
  mLose,
  mLev1,
  mLev2,
  mLev3,
  mTime,
  mHighscore,
  mHighscoreTitle,
  mHighscoreFormat,
  mOk,
  mConfigConfigMenu,
  mConfigPanelsMenu,
  mConfigViewerMenu,
  mConfigEditorMenu,
  mConfigDialogMenu,
  mConfigWonFreq,
  mConfigLoseFreq,
  mConfigSave,
  mConfigCancel,
};

#define DM_RESETBOARD DM_USER+1
#define DM_SHOWTIME DM_USER+2
#define DM_START_GAME DM_USER+3
#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

struct Options
{
  __int64 ShowInConfig;
  __int64 ShowInPanels;
  __int64 ShowInEditor;
  __int64 ShowInViewer;
  __int64 ShowInDialog;
  __int64 WonFreq;
  __int64 LoseFreq;
};

extern PluginStartupInfo Info;
extern FARSTANDARDFUNCTIONS FSF;
extern Options Opt;

extern const wchar_t* GetMsg(int MsgId);
extern intptr_t Config(void);

#define ControlKeyAllMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED|RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED)
#define IsNone(rec) static_cast<bool>(((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyAllMask)==0)

#endif
