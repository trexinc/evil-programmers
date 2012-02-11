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
  mConfigPreload,
  mConfigWonFreq,
  mConfigLoseFreq,
  mConfigSave,
  mConfigCancel,
};

#define FAR165_INFO_SIZE 336
#define DM_RESETBOARD DM_USER+1
#define DM_SHOWTIME DM_USER+2
#define DM_START_GAME DM_USER+3
#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

struct Options
{
  DWORD ShowInConfig;
  DWORD ShowInPanels;
  DWORD ShowInEditor;
  DWORD ShowInViewer;
  DWORD Preload;
  DWORD WonFreq;
  DWORD LoseFreq;
};

extern PluginStartupInfo Info;
extern FARSTANDARDFUNCTIONS FSF;
extern char PluginRootKey[80];
extern bool IsOldFAR;
extern Options Opt;

extern const char *GetMsg(int MsgId);
extern int Config(void);

#endif
