#include "../../plugin.hpp"
#include "farmine.hpp"

enum
{
  CONFIGDLG_BORDER=0,
  CONFIGDLG_CONFIGMENU,
  CONFIGDLG_PANELSMENU,
  CONFIGDLG_VIEWERMENU,
  CONFIGDLG_EDITORMENU,
  CONFIGDLG_SEP1,
  CONFIGDLG_PRELOAD,
  CONFIGDLG_SEP2,
  CONFIGDLG_LWON,
  CONFIGDLG_EWON,
  CONFIGDLG_LLOSE,
  CONFIGDLG_ELOSE,
  CONFIGDLG_SEP3,
  CONFIGDLG_SAVE,
  CONFIGDLG_CANCEL,
};

struct InitDialogItem
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  int Selected;
  unsigned int Flags;
  int DefaultButton;
  char *Data;
};

static void InitDialogItems(InitDialogItem *Init,FarDialogItem *Item,int ItemsNumber)
{
  for (int i=0;i<ItemsNumber;i++)
  {
    Item[i].Type=Init[i].Type;
    Item[i].X1=Init[i].X1;
    Item[i].Y1=Init[i].Y1;
    Item[i].X2=Init[i].X2;
    Item[i].Y2=Init[i].Y2;
    Item[i].Focus=Init[i].Focus;
    Item[i].Selected=Init[i].Selected;
    Item[i].Flags=Init[i].Flags;
    Item[i].DefaultButton=Init[i].DefaultButton;
    if ((unsigned)Init[i].Data<2000)
      strcpy(Item[i].Data,GetMsg((unsigned int)Init[i].Data));
    else
      strcpy(Item[i].Data,Init[i].Data);
  }
}

static long WINAPI ConfigDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

int Config(void)
{
  static struct InitDialogItem InitItems[]={
  /* 0*/  {DI_DOUBLEBOX,3,1,62,13,0,0,0,0,(char *)mName},
  /* 3*/  {DI_CHECKBOX,5,2,0,0,0,0,0,0,(char *)mConfigConfigMenu},
  /* 3*/  {DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)mConfigPanelsMenu},
  /* 4*/  {DI_CHECKBOX,5,4,0,0,0,0,0,0,(char *)mConfigViewerMenu},
  /* 5*/  {DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)mConfigEditorMenu},
  /* 6*/  {DI_TEXT,-1,6,0,0,0,0,DIF_SEPARATOR,0,""},
  /* 5*/  {DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)mConfigPreload},
  /* 6*/  {DI_TEXT,-1,8,0,0,0,0,DIF_SEPARATOR,0,""},
  /*05*/  {DI_TEXT,5,9,0,0,0,0,0,0,(char *)mConfigWonFreq},
  /*06*/  {DI_FIXEDIT,0,9,0,0,0,(int)"#####9",DIF_MASKEDIT,0,"  1000"},
  /*05*/  {DI_TEXT,5,10,0,0,0,0,0,0,(char *)mConfigLoseFreq},
  /*06*/  {DI_FIXEDIT,0,10,0,0,0,(int)"#####9",DIF_MASKEDIT,0,"   100"},
  /*18*/  {DI_TEXT,-1,11,0,0,0,0,DIF_SEPARATOR,0,""},
  /*19*/  {DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,1,(char *)mConfigSave},
  /*20*/  {DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,0,(char *)mConfigCancel},
  };
  struct FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));
  unsigned int max=0;
  {
    int labels[]={  CONFIGDLG_LWON,CONFIGDLG_LLOSE};
    for(unsigned int i=0;i<sizeofa(labels);i++)
    {
      unsigned int cur_len=strlen(DialogItems[labels[i]].Data);
      if(max<cur_len) max=cur_len;
    }
  }
  DialogItems[CONFIGDLG_EWON].X1=DialogItems[CONFIGDLG_LWON].X1+max; DialogItems[CONFIGDLG_EWON].X2=DialogItems[CONFIGDLG_EWON].X1+5;
  DialogItems[CONFIGDLG_ELOSE].X1=DialogItems[CONFIGDLG_LLOSE].X1+max; DialogItems[CONFIGDLG_ELOSE].X2=DialogItems[CONFIGDLG_ELOSE].X1+5;

  DialogItems[CONFIGDLG_CONFIGMENU].Selected=Opt.ShowInConfig;
  DialogItems[CONFIGDLG_PANELSMENU].Selected=Opt.ShowInPanels;
  DialogItems[CONFIGDLG_VIEWERMENU].Selected=Opt.ShowInEditor;
  DialogItems[CONFIGDLG_EDITORMENU].Selected=Opt.ShowInViewer;
  DialogItems[CONFIGDLG_PRELOAD].Selected=Opt.Preload;
  FSF.sprintf(DialogItems[CONFIGDLG_EWON].Data,"%6ld",Opt.WonFreq);
  FSF.sprintf(DialogItems[CONFIGDLG_ELOSE].Data,"%6ld",Opt.LoseFreq);

  int DlgCode=Info.DialogEx(Info.ModuleNumber,-1,-1,66,15,"Config",DialogItems,sizeofa(DialogItems),0,0,ConfigDialogProc,0);
  if(DlgCode==CONFIGDLG_SAVE)
  {
    Opt.ShowInConfig=DialogItems[CONFIGDLG_CONFIGMENU].Selected;
    Opt.ShowInPanels=DialogItems[CONFIGDLG_PANELSMENU].Selected;
    Opt.ShowInEditor=DialogItems[CONFIGDLG_VIEWERMENU].Selected;
    Opt.ShowInViewer=DialogItems[CONFIGDLG_EDITORMENU].Selected;
    Opt.Preload=DialogItems[CONFIGDLG_PRELOAD].Selected;
    Opt.WonFreq=atoi(DialogItems[CONFIGDLG_EWON].Data);
    Opt.LoseFreq=atoi(DialogItems[CONFIGDLG_ELOSE].Data);
    HKEY hKey;
    DWORD Disposition;
    if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition))==ERROR_SUCCESS)
    {
      RegSetValueEx(hKey,"ShowInConfig",0,REG_DWORD,(LPBYTE)&Opt.ShowInConfig,sizeof(Opt.ShowInConfig));
      RegSetValueEx(hKey,"ShowInPanels",0,REG_DWORD,(LPBYTE)&Opt.ShowInPanels,sizeof(Opt.ShowInPanels));
      RegSetValueEx(hKey,"ShowInEditor",0,REG_DWORD,(LPBYTE)&Opt.ShowInEditor,sizeof(Opt.ShowInEditor));
      RegSetValueEx(hKey,"ShowInViewer",0,REG_DWORD,(LPBYTE)&Opt.ShowInViewer,sizeof(Opt.ShowInViewer));
      RegSetValueEx(hKey,"Preload",0,REG_DWORD,(LPBYTE)&Opt.Preload,sizeof(Opt.Preload));
      RegSetValueEx(hKey,"WonFreq",0,REG_DWORD,(LPBYTE)&Opt.WonFreq,sizeof(Opt.WonFreq));
      RegSetValueEx(hKey,"LoseFreq",0,REG_DWORD,(LPBYTE)&Opt.LoseFreq,sizeof(Opt.LoseFreq));
      RegCloseKey(hKey);
    }
  }
  return FALSE;
}
