/*
    PicView Advanced plugin for FAR Manager
    Copyright (C) 2003-2005 FARMail Group

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
#include "libgfl.h"
#include "farcolor.hpp"
#include "plugin.hpp"
#include "farkeys.hpp"
#include "crt.hpp"

static PluginStartupInfo Info;
static FARSTANDARDFUNCTIONS FSF;
static char PluginRootKey[80];
static struct Options
{
  int AutomaticInViewer;
  int AutomaticInQuickView;
  int Override;
  int ShowInViewer;
} Opt;

static char **Exts=NULL;
static DWORD ExtsNum=0;
static OSVERSIONINFO Version;

enum
{
  MTitle,
  MAutomaticInViewer,
  MAutomaticInQuickView,
  MOverride,
  MShowInViewer,
  MImageInfo,
};

const char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

HKEY CreateRegKey(void)
{
  HKEY hKey;
  DWORD Disposition;
  RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition);
  return(hKey);
}

HKEY OpenRegKey(void)
{
  HKEY hKey;
  if (RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
    return(NULL);
  return(hKey);
}

void SetRegKey(const char *ValueName,DWORD ValueData)
{
  HKEY hKey=CreateRegKey();
  RegSetValueEx(hKey,ValueName,0,REG_DWORD,(BYTE *)&ValueData,sizeof(ValueData));
  RegCloseKey(hKey);
}

int GetRegKey(const char *ValueName,int *ValueData,DWORD Default)
{
  HKEY hKey=OpenRegKey();
  DWORD Type,Size=sizeof(*ValueData);
  int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(BYTE *)ValueData,&Size);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    *ValueData=Default;
    return(FALSE);
  }
  return(TRUE);
}

static void GetDIBFromBitmap(GFL_BITMAP *bitmap,BITMAPINFOHEADER *bitmap_info,unsigned char **data)
{
  int bytes_per_line;

  *data=NULL;
  memset(bitmap_info,0,sizeof(BITMAPINFOHEADER));

  bitmap_info->biSize=sizeof(BITMAPINFOHEADER);
  bitmap_info->biWidth=bitmap->Width;
  bitmap_info->biHeight=bitmap->Height;
  bitmap_info->biPlanes=1;

  bytes_per_line=(bitmap->Width*3+3)&-4;
  bitmap_info->biClrUsed=0;
  bitmap_info->biBitCount=24;
  bitmap_info->biCompression=BI_RGB;
  bitmap_info->biSizeImage=bytes_per_line*bitmap->Height;
  bitmap_info->biClrImportant=0;

  *data=(unsigned char*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,bitmap_info->biSizeImage);

  if(*data)
    memcpy(*data,bitmap->Data,bitmap_info->biSizeImage);

  return;
}

#if defined(__BORLANDC__)
static RECT RangingPic(RECT DCRect,GFL_BITMAP *RawPicture)
{
  int x1=(DCRect.right-DCRect.left);
  int y1=(DCRect.bottom-DCRect.top);
  int x2=RawPicture->Width;
  int y2=RawPicture->Height;
  volatile short int cw, cwtmp=0x0F7F;
  volatile int dst_w = min(x1,x2);
  volatile int dst_h = min(y1,y2);

  asm {
    fnstcw cw
    fldcw cwtmp
    fild x2
    fild y2
    fdiv
    fild x1
    fild y1
    fdiv
    fcomp
    fstsw ax
    sahf
    setb al
    cmp al, 0
    jnz adj_height
    jmp adj_width
adj_height:
    fild dst_w
    fdivr
    fistp dst_h
    jmp finish
adj_width:
    fild dst_h
    fmul
    fistp dst_w
finish:
    fldcw  cw
  }

  RECT dest={DCRect.left,DCRect.top,dst_w,dst_h};
  return dest;
}
#else
static RECT RangingPic(RECT DCRect,GFL_BITMAP *RawPicture)
{
  float asp_dst=(float)(DCRect.right-DCRect.left)/(float)(DCRect.bottom-DCRect.top);
  float asp_src=(float)RawPicture->Width/(float)RawPicture->Height;

  int dst_w;
  int dst_h;

  if(asp_dst<asp_src)
  {
    dst_w=min(DCRect.right-DCRect.left,RawPicture->Width);
    dst_h=(int)(dst_w/asp_src);
  }
  else
  {
    dst_h=min(DCRect.bottom-DCRect.top,RawPicture->Height);
    dst_w=(int)(asp_src*dst_h);
  }

  RECT dest={DCRect.left,DCRect.top,dst_w,dst_h};
  return dest;
}
#endif

static bool CheckName(const char *AFileName)
{
  int flen=lstrlen(AFileName);
  for(DWORD i=0;i<ExtsNum&&Exts;i++)
    if(Exts[i]&&lstrlen(Exts[i])<flen)
      if(!FSF.LStricmp(AFileName+flen-lstrlen(Exts[i]),Exts[i])&&AFileName[flen-lstrlen(Exts[i])-1]=='.') return true;
  return false;
}

enum
{
  LEFT,
  RIGHT,
  CENTER
};

enum
{
  QUICKVIEW,
  VIEWER
};

struct DialogData
{
  HWND FarWindow;
  RECT DrawRect;
  RECT GDIRect;
  char FileName[MAX_PATH];
  bool Redraw;
  bool SelfKeys;
  bool CurPanel;
  bool Loaded;
  bool FirstRun;
  long ResKey;
  BITMAPINFOHEADER *BmpHeader;
  unsigned char *DibData;
  GFL_FILE_INFORMATION *pic_info;
  int Align;
  int ShowingIn;
  int Page;
};

static bool DrawImage(DialogData *data)
{
  bool result=false;
  RECT DCRect,RangedRect;
  GFL_BITMAP *RawPicture=NULL;
  data->DibData=NULL;
  {
    RECT rect;
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetClientRect(data->FarWindow,&rect);
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&info);

    int dx=rect.right/(info.srWindow.Right-info.srWindow.Left);
    int dy=rect.bottom/(info.srWindow.Bottom-info.srWindow.Top);

    DCRect.left=dx*(data->DrawRect.left-info.srWindow.Left);
    DCRect.right=dx*(data->DrawRect.right+1-info.srWindow.Left);
    DCRect.top=dy*(data->DrawRect.top-info.srWindow.Top);
    DCRect.bottom=dy*(data->DrawRect.bottom+1-info.srWindow.Top);
  }
  {
    GFL_LOAD_PARAMS load_params;
    gflGetDefaultLoadParams(&load_params);
    load_params.Flags|=GFL_LOAD_SKIP_ALPHA;
    load_params.Origin=GFL_BOTTOM_LEFT;
    load_params.LinePadding=4;
    load_params.ImageWanted=data->Page-1;
    GFL_ERROR res=gflLoadBitmap(data->FileName,&RawPicture,&load_params,data->pic_info);
    if(res) RawPicture=NULL;
    if(RawPicture)
    {
      if(!gflChangeColorDepth(RawPicture,NULL,GFL_MODE_TO_BGR,GFL_MODE_NO_DITHER))
      {
        GFL_BITMAP *pic=NULL;
        RangedRect=RangingPic(DCRect,RawPicture);
        if(data->Align==RIGHT)
        {
          RangedRect.left=DCRect.right-RangedRect.right;
        }
        else if(data->Align==CENTER)
        {
          RangedRect.left+=(DCRect.right-DCRect.left-RangedRect.right)/2;
          RangedRect.top+=(DCRect.bottom-DCRect.top-RangedRect.bottom)/2;
        }
        gflResize(RawPicture,&pic,RangedRect.right,RangedRect.bottom,0,0);
        if(pic)
        {
          GetDIBFromBitmap(pic,data->BmpHeader,&data->DibData);
          gflFreeBitmap(pic);
        }
      }
    }
  }
  if(RawPicture&&data->DibData)
  {
    result=true;
    data->GDIRect=RangedRect;
  }
  if(RawPicture)
    gflFreeBitmap(RawPicture);
  return result;
}

static bool UpdateImage(DialogData *data, bool CheckOnly=false)
{
  if(!data->DibData&&!data->Loaded)
  {
    if(DrawImage(data))
    {
      data->Loaded=true;
      if ((!(data->FirstRun))&&(!CheckOnly))
        InvalidateRect(data->FarWindow,NULL,TRUE);
    }
  }
  if(!data->DibData||!data->Loaded)
    return false;
  if(CheckOnly)
    return true;
  if(Version.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
    Sleep(100);
  HDC hDC=GetDC(data->FarWindow);
  StretchDIBits(hDC,data->GDIRect.left,data->GDIRect.top,data->GDIRect.right,data->GDIRect.bottom,0,0,data->GDIRect.right,data->GDIRect.bottom,data->DibData,(BITMAPINFO *)data->BmpHeader,DIB_RGB_COLORS,SRCCOPY);
  ReleaseDC(data->FarWindow,hDC);
  return true;
}

static void FreeImage(DialogData *data)
{
  if(data->DibData)
  {
    HeapFree(GetProcessHeap(),0,data->DibData);
    data->DibData=NULL;
  }
  gflFreeFileInformation(data->pic_info);
}

static void UpdateInfoText(HANDLE hDlg, DialogData *data)
{
  char string[512];
  char *types[]={"RGB","GREY","CMY","CMYK","YCBCR","YUV16","LAB","LOGLUV","LOGL"};
  FSF.sprintf(string,GetMsg(MImageInfo),data->pic_info->Width,data->pic_info->Height,data->pic_info->Xdpi,data->pic_info->Ydpi,data->Page,data->pic_info->NumberOfImages,types[data->pic_info->ColorModel],data->pic_info->CompressionDescription);
  Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(long)string);
  COORD coord = {0,0};
  Info.SendDlgMessage(hDlg,DM_SETCURSORPOS,2,(long)&coord);
}

static long WINAPI PicDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  DialogData *DlgParams=(DialogData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  //PrintDialogMessages(hDlg,Msg,Param1,Param2);

  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_SETDLGDATA,0,Param2);
      break;
    case DN_CTLCOLORDLGITEM:
      if(Param1==0)
      {
        if(DlgParams->ShowingIn==VIEWER)
          return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)COL_VIEWERSTATUS)<<24)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)COL_VIEWERSTATUS)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)COL_VIEWERTEXT)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)COL_VIEWERSTATUS));
        else
          return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)COL_PANELTEXT)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)(DlgParams->SelfKeys?COL_PANELSELECTEDTITLE:COL_PANELTITLE))<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)(DlgParams->SelfKeys||DlgParams->CurPanel?COL_PANELSELECTEDTITLE:COL_PANELTITLE)));
      }
      if(Param1==2)
      {
        DlgParams->Redraw=true;
        if(DlgParams->ShowingIn==VIEWER)
          return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)COL_VIEWERSTATUS)<<24)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)COL_VIEWERSTATUS)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)COL_VIEWERTEXT)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)COL_VIEWERSTATUS));
        else
          return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)COL_PANELTEXT)<<24)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)COL_PANELTEXT)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)COL_PANELCURSOR)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)COL_PANELTEXT));
      }
      break;
    case DN_DRAWDLGITEM:
      DlgParams->Redraw=true;
      break;
    case DN_ENTERIDLE:
      if(DlgParams->Redraw)
      {
        DlgParams->Redraw=false;
        UpdateImage(DlgParams);
        if(DlgParams->FirstRun)
        {
          DlgParams->FirstRun=false;
          UpdateInfoText(hDlg,DlgParams);
        }
      }
      break;
    case 0x3FFF:
      if(Param1)
        UpdateImage(DlgParams);
      break;
    case DN_GOTFOCUS:
      if(DlgParams->SelfKeys)
        Info.SendDlgMessage(hDlg,DM_SETFOCUS,2,0);
      break;
    case DN_KEY:
      if(!DlgParams->SelfKeys)
      {
        if((Param2&(KEY_CTRL|KEY_ALT|KEY_SHIFT|KEY_RCTRL|KEY_RALT))==Param2) break;
        switch(Param2)
        {
          case KEY_CTRLR:
            UpdateImage(DlgParams);
            return TRUE;
          case KEY_TAB:
            DlgParams->SelfKeys=true;
            Info.SendDlgMessage(hDlg,DM_SETFOCUS,2,0);
            return TRUE;
          case KEY_BS:
          case KEY_SPACE:
            if(DlgParams->ShowingIn==VIEWER)
              Param2=Param2==KEY_BS?KEY_SUBTRACT:KEY_ADD;
          default:
            if (DlgParams->ShowingIn==VIEWER && Param2==KEY_F3)
              Param2=KEY_ESC;
            DlgParams->ResKey=Param2;
            Info.SendDlgMessage(hDlg,DM_CLOSE,-1,0);
            return TRUE;
        }
      }
      else
      {
        switch(Param2)
        {
          case KEY_TAB:
            DlgParams->SelfKeys=false;
            Info.SendDlgMessage(hDlg,DM_SETFOCUS,1,0);
            return TRUE;
          case KEY_ADD:
          case KEY_SUBTRACT:
            if(DlgParams->DibData)
            {
              int Pages=DlgParams->pic_info->NumberOfImages;
              FreeImage(DlgParams);
              DlgParams->Loaded=false;
              if(Param2==KEY_ADD) DlgParams->Page++;
              else DlgParams->Page--;
              if(DlgParams->Page<1) DlgParams->Page=Pages;
              if(DlgParams->Page>Pages) DlgParams->Page=1;
              UpdateImage(DlgParams);
              UpdateInfoText(hDlg,DlgParams);
            }
            return TRUE;
        }
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void GetJiggyWithIt(int XPanelInfo,bool Override, bool Force)
{
  ViewerInfo info;
  info.StructSize=sizeof(info);
  if(Info.ViewerControl(VCTL_GETINFO,&info))
  {
    DialogData data;
    PanelInfo PInfo;
    Info.Control(INVALID_HANDLE_VALUE,XPanelInfo,&PInfo);
    if(info.WindowSizeX==(PInfo.PanelRect.right-PInfo.PanelRect.left-1)&&PInfo.PanelType==PTYPE_QVIEWPANEL)
    {
      if(!Opt.AutomaticInQuickView && !Force)
        return;
      data.ShowingIn=QUICKVIEW;
    }
    else
    {
      if(!Opt.AutomaticInViewer && !Force)
        return;
      data.ShowingIn=VIEWER;
    }
    if (PInfo.Focus)
      data.CurPanel=true;
    else
      data.CurPanel=false;
    if(Info.AdvControl(Info.ModuleNumber,ACTL_CONSOLEMODE,(void*)FAR_CONSOLE_GET_MODE)!=FAR_CONSOLE_FULLSCREEN&&(CheckName(info.FileName)||Override))
    {
      RECT ViewerRect;
      if(data.ShowingIn==VIEWER)
      {
        ViewerRect.left=0;
        ViewerRect.top=0;
        ViewerRect.right=info.WindowSizeX-1;
        ViewerRect.bottom=info.WindowSizeY+1;
      }
      if(Version.dwPlatformId==VER_PLATFORM_WIN32_NT)
        data.FarWindow=(HWND)Info.AdvControl(Info.ModuleNumber,ACTL_GETFARHWND,0);
      else
        data.FarWindow=FindWindowEx(FindWindow("tty",0),NULL,"ttyGrab",NULL);
      lstrcpy(data.FileName,info.FileName);
      if(data.ShowingIn == VIEWER)
      {
        data.DrawRect=ViewerRect;
        data.DrawRect.top++;
        data.DrawRect.bottom--;
        data.Align=CENTER;
      }
      else
      {
        ViewerRect=data.DrawRect=PInfo.PanelRect;
        data.DrawRect.left++;
        data.DrawRect.top++;
        data.DrawRect.right--;
        data.DrawRect.bottom-=2;
        data.Align=CENTER;//(PInfo.PanelRect.left>0)?RIGHT:LEFT;
      }
      data.FirstRun=true;
      data.Redraw=false;
      data.SelfKeys=false;
      data.Loaded=false;
      data.ResKey=0;
      BITMAPINFOHEADER BmpHeader;
      data.BmpHeader=&BmpHeader;
      data.DibData=NULL;
      GFL_FILE_INFORMATION pic_info;
      data.pic_info=&pic_info;
      data.Page=1;

      //HANDLE hs=Info.SaveScreen(0,0,-1,-1);
      if(UpdateImage(&data,true))
      {
        FarDialogItem DialogItems[3];
        memset(DialogItems,0,sizeof(DialogItems));
        unsigned int VBufSize; int color;
        if(data.ShowingIn==VIEWER)
        {
          DialogItems[0].Type=DI_EDIT;
          DialogItems[0].X1=0; DialogItems[0].X2=info.WindowSizeX-1;
          DialogItems[0].Y1=0; DialogItems[0].Y2=0;
          lstrcpy(DialogItems[0].Data,info.FileName);
          DialogItems[1].Type=DI_USERCONTROL; DialogItems[1].Focus=TRUE;
          DialogItems[1].X1=0; DialogItems[1].X2=info.WindowSizeX-1;
          DialogItems[1].Y1=1; DialogItems[1].Y2=info.WindowSizeY;
          DialogItems[2].Type=DI_EDIT;
          DialogItems[2].X1=0; DialogItems[2].X2=info.WindowSizeX-1;
          DialogItems[2].Y1=info.WindowSizeY+1;
          DialogItems[2].Flags=DIF_READONLY;
          VBufSize=(info.WindowSizeY)*(info.WindowSizeX);
          color=Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)COL_VIEWERTEXT);
        }
        else
        {
          DialogItems[0].Type=DI_DOUBLEBOX;
          DialogItems[0].X1=0; DialogItems[0].X2=PInfo.PanelRect.right-PInfo.PanelRect.left;
          DialogItems[0].Y1=0; DialogItems[0].Y2=PInfo.PanelRect.bottom-PInfo.PanelRect.top;
          lstrcpy(DialogItems[0].Data,FSF.PointToName(info.FileName));
          DialogItems[1].Type=DI_USERCONTROL; DialogItems[1].Focus=TRUE;
          DialogItems[1].X1=1; DialogItems[1].X2=PInfo.PanelRect.right-PInfo.PanelRect.left-1;
          DialogItems[1].Y1=1; DialogItems[1].Y2=PInfo.PanelRect.bottom-PInfo.PanelRect.top-2;
          DialogItems[2].Type=DI_EDIT;
          DialogItems[2].X1=1; DialogItems[2].X2=PInfo.PanelRect.right-PInfo.PanelRect.left-1;
          DialogItems[2].Y1=PInfo.PanelRect.bottom-PInfo.PanelRect.top-1;
          DialogItems[2].Flags=DIF_READONLY;
          VBufSize=(PInfo.PanelRect.right-PInfo.PanelRect.left-1)*(PInfo.PanelRect.bottom-PInfo.PanelRect.top-2);
          color=Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void*)COL_PANELTEXT);
        }

        color=color&0xF0;
        color=color|(color>>4);

        CHAR_INFO *VirtualBuffer;
        VirtualBuffer=(CHAR_INFO *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,VBufSize*sizeof(CHAR_INFO));

        if (VirtualBuffer)
        {
          DialogItems[1].VBuf=VirtualBuffer;
          for(unsigned int i=0;i<VBufSize;i++)
          {
            VirtualBuffer[i].Char.AsciiChar='.';
            VirtualBuffer[i].Attributes=color;
          }

          Info.DialogEx(Info.ModuleNumber,ViewerRect.left,ViewerRect.top,ViewerRect.right,ViewerRect.bottom,NULL,DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]),0,FDLG_SMALLDIALOG,PicDialogProc,(long)&data);

          HeapFree(GetProcessHeap(),0,VirtualBuffer);
        }

        FreeImage(&data);

        if(data.ResKey)
        {
          KeySequence key;
          key.Flags=KSFLAGS_DISABLEOUTPUT;
          key.Count=1;
          key.Sequence=(DWORD *)&data.ResKey;
          Info.AdvControl(Info.ModuleNumber,ACTL_POSTKEYSEQUENCE,&key);
        }
      }
      else
      {
        //Info.RestoreScreen(NULL);
      }
      //Info.RestoreScreen(hs);
    }
  }
}

int WINAPI ProcessViewerEvent(int Event,void *Param)
{
  (void)Param;
  if(Event==VE_READ)
  {
    int XPanelInfo=FCTL_GETANOTHERPANELSHORTINFO;
    struct WindowInfo wi;
    wi.Pos=-1;
    Info.AdvControl(Info.ModuleNumber,ACTL_GETSHORTWINDOWINFO,(void *)&wi);
    if (wi.Type==WTYPE_PANELS)
    {
      struct PanelInfo pi;
      Info.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELSHORTINFO,(void *)&pi);
      if (pi.PanelType==PTYPE_QVIEWPANEL)
        XPanelInfo=FCTL_GETPANELSHORTINFO;
    }
    GetJiggyWithIt(XPanelInfo,Opt.Override?true:false,false);
  }
  return 0;
}

HANDLE WINAPI OpenPlugin(int OpenFrom,int Item)
{
  (void)OpenFrom;
  (void)Item;
  GetJiggyWithIt(FCTL_GETPANELSHORTINFO,true,true);
  return INVALID_HANDLE_VALUE;
}

void WINAPI GetPluginInfo(struct PluginInfo *Info)
{
  static const char *MenuStrings[1];
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_DISABLEPANELS|PF_VIEWER;
  MenuStrings[0]=GetMsg(MTitle);
  Info->PluginMenuStrings=MenuStrings;
  Info->PluginMenuStringsNumber=Opt.ShowInViewer?1:0;
  Info->PluginConfigStrings=MenuStrings;
  Info->PluginConfigStringsNumber=1;
}

static void SetDefaultExtentions()
{
  int number=gflGetNumberOfFormat();
  GFL_FORMAT_INFORMATION finfo;
  for(int i=0;i<number;i++)
  {
    gflGetFormatInformationByIndex(i,&finfo);
    for(DWORD j=0;j<finfo.NumberOfExtension;j++)
    {
      Exts=(char **)(((Exts)?HeapReAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,Exts,(ExtsNum+1)*sizeof(*Exts)):HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(ExtsNum+1)*sizeof(*Exts))));
      Exts[ExtsNum]=(char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,lstrlen(finfo.Extension[j])+1);
      lstrcpy(Exts[ExtsNum],finfo.Extension[j]);
      ExtsNum++;
    }
  }
}

void WINAPI SetStartupInfo(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;
  FSF.sprintf(PluginRootKey,"%s\\PicViewAdv",::Info.RootKey);
  GetRegKey("AutomaticInViewer",&Opt.AutomaticInViewer,0);
  GetRegKey("AutomaticInQuickView",&Opt.AutomaticInQuickView,1);
  GetRegKey("ShowInViewer",&Opt.ShowInViewer,1);
  GetRegKey("Override",&Opt.Override,0);
  char folder[NM];
  lstrcpy(folder, Info->ModuleName);
  *FSF.PointToName(folder) = 0;
  gflSetPluginsPathname(folder);
  gflLibraryInit();
  gflEnableLZW(GFL_TRUE);
  SetDefaultExtentions();
  Version.dwOSVersionInfoSize=sizeof(Version);
  GetVersionEx(&Version);
}

void WINAPI ExitFAR()
{
  gflLibraryExit();
  if(Exts)
  {
    for(DWORD i=0;i<ExtsNum;i++)
      if(Exts[i])
        HeapFree(GetProcessHeap(),0,Exts[i]);
    HeapFree(GetProcessHeap(),0,Exts);
  }
  Exts=NULL;
  ExtsNum=0;
}

int WINAPI Configure(int ItemNumber)
{
  (void)ItemNumber;
  FarDialogItem DialogItems[5];

  memset(DialogItems,0,sizeof(DialogItems));
  DialogItems[0].Type=DI_DOUBLEBOX;
  DialogItems[0].X1=3; DialogItems[0].X2=50;
  DialogItems[0].Y1=1; DialogItems[0].Y2=6;
  lstrcpy(DialogItems[0].Data,GetMsg(MTitle));
  DialogItems[1].Type=DI_CHECKBOX;
  DialogItems[1].X1=5; //DialogItems[1].X2=48;
  DialogItems[1].Y1=2; //DialogItems[1].Y2=2;
  DialogItems[1].Focus=TRUE;
  GetRegKey("AutomaticInViewer",&Opt.AutomaticInViewer,0);
  DialogItems[1].Selected=Opt.AutomaticInViewer;
  DialogItems[1].DefaultButton = 1;
  lstrcpy(DialogItems[1].Data,GetMsg(MAutomaticInViewer));
  DialogItems[2].Type=DI_CHECKBOX;
  DialogItems[2].X1=5; //DialogItems[2].X2=48;
  DialogItems[2].Y1=3; //DialogItems[2].Y2=3;
  GetRegKey("AutomaticInQuickView",&Opt.AutomaticInQuickView,1);
  DialogItems[2].Selected=Opt.AutomaticInQuickView;
  lstrcpy(DialogItems[2].Data,GetMsg(MAutomaticInQuickView));
  DialogItems[3].Type=DI_CHECKBOX;
  DialogItems[3].X1=5; //DialogItems[3].X2=48;
  DialogItems[3].Y1=4; //DialogItems[3].Y2=4;
  GetRegKey("Override",&Opt.Override,0);
  DialogItems[3].Selected=Opt.Override;
  lstrcpy(DialogItems[3].Data,GetMsg(MOverride));
  DialogItems[4].Type=DI_CHECKBOX;
  DialogItems[4].X1=5; //DialogItems[4].X2=48;
  DialogItems[4].Y1=5; //DialogItems[4].Y2=5;
  GetRegKey("ShowInViewer",&Opt.ShowInViewer,1);
  DialogItems[4].Selected=Opt.ShowInViewer;
  lstrcpy(DialogItems[4].Data,GetMsg(MShowInViewer));

  if(Info.Dialog(Info.ModuleNumber,-1,-1,53,8,NULL,DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0])) == -1)
    return FALSE;

  Opt.AutomaticInViewer=DialogItems[1].Selected;
  Opt.AutomaticInQuickView=DialogItems[2].Selected;
  Opt.Override=DialogItems[3].Selected;
  Opt.ShowInViewer=DialogItems[4].Selected;
  SetRegKey("AutomaticInViewer",Opt.AutomaticInViewer);
  SetRegKey("AutomaticInQuickView",Opt.AutomaticInQuickView);
  SetRegKey("ShowInViewer",Opt.ShowInViewer);
  SetRegKey("Override",Opt.Override);
  return TRUE;
}

#if defined(__GNUC__)

#ifdef __cplusplus
extern "C"{
#endif
  bool WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

bool WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void)hDll;
  (void)dwReason;
  (void)lpReserved;
  return true;
}

#endif
