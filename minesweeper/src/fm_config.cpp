#include "plugin.hpp"
#include "farmine.hpp"
#include "far_settings.hpp"
#include "guid.h"

enum
{
  CONFIGDLG_BORDER=0,
  CONFIGDLG_CONFIGMENU,
  CONFIGDLG_PANELSMENU,
  CONFIGDLG_VIEWERMENU,
  CONFIGDLG_EDITORMENU,
  CONFIGDLG_DIALOGMENU,
  CONFIGDLG_SEP1,
  CONFIGDLG_LWON,
  CONFIGDLG_EWON,
  CONFIGDLG_LLOSE,
  CONFIGDLG_ELOSE,
  CONFIGDLG_SEP3,
  CONFIGDLG_SAVE,
  CONFIGDLG_CANCEL,
};

#define GetCheck(i) Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
#define GetData(i) ((const wchar_t*)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))

static INT_PTR WINAPI ConfigDialogProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

int Config(void)
{
  wchar_t won[32],lose[32];
  FSF.sprintf(won,L"%6ld",Opt.WonFreq);
  FSF.sprintf(lose,L"%6ld",Opt.LoseFreq);
  FarDialogItem DialogItems[]=
  {
    {DI_DOUBLEBOX, 3, 1,62,12,{0},                     NULL,NULL,      0,                                GetMsg(mName)             ,0,0},
    {DI_CHECKBOX,  5, 2, 0, 0,{(bool)Opt.ShowInConfig},NULL,NULL,      0,                                GetMsg(mConfigConfigMenu) ,0,0},
    {DI_CHECKBOX,  5, 3, 0, 0,{(bool)Opt.ShowInPanels},NULL,NULL,      0,                                GetMsg(mConfigPanelsMenu) ,0,0},
    {DI_CHECKBOX,  5, 4, 0, 0,{(bool)Opt.ShowInEditor},NULL,NULL,      0,                                GetMsg(mConfigViewerMenu) ,0,0},
    {DI_CHECKBOX,  5, 5, 0, 0,{(bool)Opt.ShowInViewer},NULL,NULL,      0,                                GetMsg(mConfigEditorMenu) ,0,0},
    {DI_CHECKBOX,  5, 6, 0, 0,{(bool)Opt.ShowInDialog},NULL,NULL,      0,                                GetMsg(mConfigDialogMenu) ,0,0},
    {DI_TEXT,     -1, 7, 0, 0,{0},                     NULL,NULL,      DIF_SEPARATOR,                    L""                       ,0,0},
    {DI_TEXT,      5, 8, 0, 0,{0},                     NULL,NULL,      0,                                GetMsg(mConfigWonFreq)    ,0,0},
    {DI_FIXEDIT,   0, 8, 0, 0,{0},                     NULL,L"#####9", DIF_MASKEDIT,                     won                       ,0,0},
    {DI_TEXT,      5, 9, 0, 0,{0},                     NULL,NULL,      0,                                GetMsg(mConfigLoseFreq)   ,0,0},
    {DI_FIXEDIT,   0, 9, 0, 0,{0},                     NULL,L"#####9", DIF_MASKEDIT,                     lose                      ,0,0},
    {DI_TEXT,     -1,10, 0, 0,{0},                     NULL,NULL,      DIF_SEPARATOR,                    L""                       ,0,0},
    {DI_BUTTON,    0,11, 0, 0,{0},                     NULL,NULL,      DIF_CENTERGROUP|DIF_DEFAULTBUTTON,GetMsg(mConfigSave)       ,0,0},
    {DI_BUTTON,    0,11, 0, 0,{0},                     NULL,NULL,      DIF_CENTERGROUP,                  GetMsg(mConfigCancel)     ,0,0},
  };
  size_t max=0;
  {
    int labels[]={CONFIGDLG_LWON,CONFIGDLG_LLOSE};
    for(unsigned int i=0;i<sizeofa(labels);i++)
    {
      size_t cur_len=wcslen(DialogItems[labels[i]].Data);
      if(max<cur_len) max=cur_len;
    }
  }
  DialogItems[CONFIGDLG_EWON].X1=DialogItems[CONFIGDLG_LWON].X1+(int)max; DialogItems[CONFIGDLG_EWON].X2=DialogItems[CONFIGDLG_EWON].X1+5;
  DialogItems[CONFIGDLG_ELOSE].X1=DialogItems[CONFIGDLG_LLOSE].X1+(int)max; DialogItems[CONFIGDLG_ELOSE].X2=DialogItems[CONFIGDLG_ELOSE].X1+5;

  int DlgCode=-1;
  HANDLE hDlg=Info.DialogInit(&MainGuid,&ConfigGuid,-1,-1,66,14,L"Config",DialogItems,sizeofa(DialogItems),0,0,ConfigDialogProc,0);
  if(hDlg!=INVALID_HANDLE_VALUE) DlgCode=Info.DialogRun(hDlg);
  if(DlgCode==CONFIGDLG_SAVE)
  {
    Opt.ShowInConfig=GetCheck(CONFIGDLG_CONFIGMENU);
    Opt.ShowInPanels=GetCheck(CONFIGDLG_PANELSMENU);
    Opt.ShowInEditor=GetCheck(CONFIGDLG_VIEWERMENU);
    Opt.ShowInViewer=GetCheck(CONFIGDLG_EDITORMENU);
    Opt.ShowInDialog=GetCheck(CONFIGDLG_DIALOGMENU);
    Opt.WonFreq=FSF.atoi(GetData(CONFIGDLG_EWON));
    Opt.LoseFreq=FSF.atoi(GetData(CONFIGDLG_ELOSE));
    {
      CFarSettings settings(MainGuid);
      settings.Set(L"ShowInConfig",Opt.ShowInConfig);
      settings.Set(L"ShowInPanels",Opt.ShowInPanels);
      settings.Set(L"ShowInEditor",Opt.ShowInEditor);
      settings.Set(L"ShowInViewer",Opt.ShowInViewer);
      settings.Set(L"ShowInDialog",Opt.ShowInDialog);
      settings.Set(L"WonFreq",Opt.WonFreq);
      settings.Set(L"LoseFreq",Opt.LoseFreq);
    }
  }
  if(hDlg!=INVALID_HANDLE_VALUE) Info.DialogFree(hDlg);
  return FALSE;
}
