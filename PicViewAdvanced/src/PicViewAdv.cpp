/*
    PicView Advanced plugin for FAR Manager
    Copyright (C) 2003-2011 FARMail Group

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
//#include "crt.hpp"
#include <ntdef.h>
#include <initguid.h>

// {9F25A250-45D2-45a0-90A3-5686B2A048FA}
DEFINE_GUID(MainGuid,0x9f25a250, 0x45d2, 0x45a0, 0x90, 0xa3, 0x56, 0x86, 0xb2, 0xa0, 0x48, 0xfa);
// {A59072DC-49BA-4598-BBFB-F7906A7A821D}
DEFINE_GUID(MenuGuid,0xa59072dc, 0x49ba, 0x4598, 0xbb, 0xfb, 0xf7, 0x90, 0x6a, 0x7a, 0x82, 0x1d);
// {66ffe4cf-ee00-4b45-b91a-163f7c57d084}
DEFINE_GUID(DlgGuid,0x66ffe4cf, 0xee00, 0x4b45, 0xb9, 0x1a, 0x16, 0x3f, 0x7c, 0x57, 0xd0, 0x84);
// {CADF6A67-7D6D-4072-8C88-BD604D1C3CC7}
DEFINE_GUID(CfgDlgGuid,0xcadf6a67, 0x7d6d, 0x4072, 0x8c, 0x88, 0xbd, 0x60, 0x4d, 0x1c, 0x3c, 0xc7);

///
#define ControlKeyAllMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED|RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED)
#define ControlKeyAltMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED)
#define ControlKeyNonAltMask (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED)
#define ControlKeyCtrlMask (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED)
#define ControlKeyNonCtrlMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED|SHIFT_PRESSED)
#define IsShift(rec) (((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyAllMask)==SHIFT_PRESSED)
#define IsAlt(rec) (((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyAltMask)&&!((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyNonAltMask))
#define IsCtrl(rec) (((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyCtrlMask)&&!((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyNonCtrlMask))
#define IsNone(rec) (((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyAllMask)==0)

struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;
struct Options
{
  int AutomaticInViewer;
  int AutomaticInQuickView;
  int BilinearResizeInViewer;
  int BilinearResizeInQuickView;
  int Override;
  int IgnoreReadError;
  int ShowInViewer;
} Opt;

wchar_t **Exts=NULL;
DWORD ExtsNum=0;

enum
{
  MTitle,
  MAutomaticInViewer,
  MAutomaticInQuickView,
  MBilinearResizeInViewer,
  MBilinearResizeInQuickView,
  MOverride,
  MIgnoreReadError,
  MShowInViewer,
  MImageInfo,
};

const wchar_t *GetMsg(int MsgId)
{
  return(Info.GetMsg(&MainGuid,MsgId));
}

bool GetValue(HANDLE Handle,int Root,const TCHAR* Name,bool Default)
{
  bool result=Default;
  FarSettingsItem item={sizeof(FarSettingsItem),Root,Name,FST_QWORD};
  if(Info.SettingsControl(Handle,SCTL_GET,0,&item))
  {
    result=item.Number?true:false;
  }
  return result;
}

void SetValue(HANDLE Handle,int Root,const TCHAR* Name,__int64 Value)
{
  FarSettingsItem item={sizeof(FarSettingsItem),Root,Name,FST_QWORD};
  item.Number=Value;
  Info.SettingsControl(Handle,SCTL_SET,0,&item);
}

void GetDIBFromBitmap(GFL_BITMAP *bitmap,BITMAPINFOHEADER *bitmap_info,unsigned char **data)
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

RECT RangingPic(RECT DCRect,GFL_BITMAP *RawPicture)
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

bool CheckName(const wchar_t *AFileName)
{
  int flen=lstrlen(AFileName);
  for(DWORD i=0;i<ExtsNum&&Exts;i++)
    if(Exts[i]&&lstrlen(Exts[i])<flen)
      if(!FSF.LStricmp(AFileName+flen-lstrlen(Exts[i]),Exts[i])&&AFileName[flen-lstrlen(Exts[i])-1]==L'.') return true;
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
  wchar_t FileName[32768];
  bool Redraw;
  bool SelfKeys;
  bool CurPanel;
  bool Loaded;
  bool FirstRun;
  INPUT_RECORD ResKey;
  BITMAPINFOHEADER *BmpHeader;
  unsigned char *DibData;
  GFL_FILE_INFORMATION *pic_info;
  int Align;
  int ShowingIn;
  int Page;
  int Rotate;
};


bool DrawImage(DialogData *data)
{
  bool result=false;
  GFL_BITMAP *RawPicture=NULL;
  data->DibData=NULL;

  RECT rect;
  CONSOLE_SCREEN_BUFFER_INFO info;
  GetClientRect(data->FarWindow,&rect);
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&info);

  int dx=rect.right/(info.srWindow.Right-info.srWindow.Left);
  int dy=rect.bottom/(info.srWindow.Bottom-info.srWindow.Top);

  RECT DCRect;
  DCRect.left=dx*(data->DrawRect.left-info.srWindow.Left);
  DCRect.right=dx*(data->DrawRect.right+1-info.srWindow.Left);
  DCRect.top=dy*(data->DrawRect.top/*-info.srWindow.Top*/);          //костыль для запуска far.exe /w
  DCRect.bottom=dy*(data->DrawRect.bottom+1/*-info.srWindow.Top*/);  //костыль для запуска far.exe /w

  RECT RangedRect;
  {
    GFL_LOAD_PARAMS load_params;
    gflGetDefaultLoadParams(&load_params);
    load_params.Flags|=GFL_LOAD_SKIP_ALPHA;
    // обнаружилось, что новая библиотека gfl 3.11 некоторые файлы не хочет грузить, говорит- GFL_ERROR:2
    // старая АНСИ gfl 2.20 грузит эти же файлы без проблем
    // выставим GFL_LOAD_IGNORE_READ_ERROR :)
    if(Opt.IgnoreReadError)
      load_params.Flags|=GFL_LOAD_IGNORE_READ_ERROR;
    load_params.Origin=GFL_BOTTOM_LEFT;
    load_params.LinePadding=4;
    load_params.ImageWanted=data->Page-1;
    GFL_ERROR res=gflLoadBitmapW(data->FileName,&RawPicture,&load_params,data->pic_info);
    if(res)
    {
      RawPicture=NULL;
    }
    if(RawPicture)
    {
      if(!gflChangeColorDepth(RawPicture,NULL,GFL_MODE_TO_BGR,GFL_MODE_NO_DITHER) && !gflRotate(RawPicture,NULL,data->Rotate,0))
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
        gflResize(RawPicture,&pic,RangedRect.right,RangedRect.bottom,
                    (data->ShowingIn==VIEWER?(Opt.BilinearResizeInViewer?GFL_RESIZE_BILINEAR:0):(Opt.BilinearResizeInQuickView?GFL_RESIZE_BILINEAR:0)),0);
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

bool UpdateImage(DialogData *data, bool CheckOnly=false)
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
  HDC hDC=GetDC(data->FarWindow);
  StretchDIBits(hDC,data->GDIRect.left,data->GDIRect.top,data->GDIRect.right,data->GDIRect.bottom,0,0,data->GDIRect.right,data->GDIRect.bottom,data->DibData,(BITMAPINFO *)data->BmpHeader,DIB_RGB_COLORS,SRCCOPY);
  ReleaseDC(data->FarWindow,hDC);
  return true;
}

void FreeImage(DialogData *data)
{
  if(data->DibData)
  {
    HeapFree(GetProcessHeap(),0,data->DibData);
    data->DibData=NULL;
  }
  gflFreeFileInformation(data->pic_info);
}

void UpdateInfoText(HANDLE hDlg, DialogData *data)
{
  wchar_t string[512];
  const wchar_t* types[]={L"RGB",L"GREY",L"CMY",L"CMYK",L"YCBCR",L"YUV16",L"LAB",L"LOGLUV",L"LOGL"};
  FSF.sprintf(string,GetMsg(MImageInfo),data->pic_info->Width,data->pic_info->Height,data->GDIRect.right,data->GDIRect.bottom,data->pic_info->Xdpi,data->pic_info->Ydpi,data->Page,data->pic_info->NumberOfImages,types[data->pic_info->ColorModel]);
  Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,2,string);
  COORD coord = {0,0};
  Info.SendDlgMessage(hDlg,DM_SETCURSORPOS,2,&coord);
}

intptr_t WINAPI PicDialogProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void *Param2)
{
  DialogData *DlgParams=(DialogData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);

  switch(Msg)
  {
    case DN_INITDIALOG:
      Info.SendDlgMessage(hDlg,DM_SETDLGDATA,0,Param2);
      break;
    case DN_CTLCOLORDLGITEM:
      if(Param1==0)
      {
        FarColor Color;
        struct FarDialogItemColors *Colors=(FarDialogItemColors*)Param2;
        if(DlgParams->ShowingIn==VIEWER)
        {
          Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_VIEWERSTATUS,&Color);
          Colors->Colors[0] = Colors->Colors[2] = Colors->Colors[3] = Color;
          Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_VIEWERTEXT,&Color);
          Colors->Colors[1] = Color;
        }
        else
        {
          Info.AdvControl(&MainGuid,ACTL_GETCOLOR,(DlgParams->SelfKeys||DlgParams->CurPanel?COL_PANELSELECTEDTITLE:COL_PANELTITLE),&Color);
          Colors->Colors[0] = Color;
          Info.AdvControl(&MainGuid,ACTL_GETCOLOR,(DlgParams->SelfKeys?COL_PANELSELECTEDTITLE:COL_PANELTITLE),&Color);
          Colors->Colors[1] = Color;
          Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_PANELTEXT,&Color);
          Colors->Colors[2] = Color;
        }
      }
      if(Param1==2)
      {
        DlgParams->Redraw=true;
        FarColor Color;
        struct FarDialogItemColors *Colors=(FarDialogItemColors*)Param2;
        if(DlgParams->ShowingIn==VIEWER)
        {
          Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_VIEWERSTATUS,&Color);
          Colors->Colors[0] = Colors->Colors[2] = Colors->Colors[3] = Color;
          Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_VIEWERTEXT,&Color);
          Colors->Colors[1] = Color;
        }
        else
        {
          Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_PANELTEXT,&Color);
          Colors->Colors[0] = Colors->Colors[2] = Colors->Colors[3] = Color;
          Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_PANELCURSOR,&Color);
          Colors->Colors[1] = Color;
        }
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
    case DN_CONTROLINPUT:
      if(!DlgParams->SelfKeys)
      {
        const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
        if (record->EventType==KEY_EVENT && record->Event.KeyEvent.bKeyDown)
        {
          WORD vk=record->Event.KeyEvent.wVirtualKeyCode;

          switch(vk)
          {
            case 0x52: //VK_R
            {
              if (IsCtrl(record))
              {
                UpdateImage(DlgParams);
                return TRUE;
              }
              break;
            }
            case 0x44: //VK_D
            case 0x53: //VK_S
            case 0x45: //VK_E
            {
              if (IsCtrl(record))
              {
                FreeImage(DlgParams);
                if(vk==0x44) DlgParams->Rotate-=90;
                else if (vk==0x53) DlgParams->Rotate+=90;
                else DlgParams->Rotate+=180;
                DlgParams->Loaded=false;
                UpdateImage(DlgParams);
                UpdateInfoText(hDlg,DlgParams);
                return TRUE;
              }
              break;
            }
            case VK_TAB:
            {
              if (IsNone(record))
              {
                DlgParams->SelfKeys=true;
                Info.SendDlgMessage(hDlg,DM_SETFOCUS,2,0);
                return TRUE;
              }
              break;
            }
            case VK_BACK:
            case VK_SPACE:
              if (IsNone(record) && DlgParams->ShowingIn==VIEWER)
                vk=(vk==VK_BACK?VK_SUBTRACT:VK_ADD);
            default:
              if (IsNone(record) && DlgParams->ShowingIn==QUICKVIEW && vk==VK_DELETE)
                vk=VK_F8;
              DlgParams->ResKey=*record;
              DlgParams->ResKey.Event.KeyEvent.wVirtualKeyCode=vk;
              Info.SendDlgMessage(hDlg,DM_CLOSE,-1,0);
              return TRUE;
          }
        }
      }
      else
      {
        const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
        if (record->EventType==KEY_EVENT && record->Event.KeyEvent.bKeyDown)
        {
          WORD vk=record->Event.KeyEvent.wVirtualKeyCode;
          switch(vk)
          {
            case VK_TAB:
            {
              if (IsNone(record))
              {
                DlgParams->SelfKeys=false;
                Info.SendDlgMessage(hDlg,DM_SETFOCUS,1,0);
                return TRUE;
              }
              break;
            }
            case VK_ADD:
            case VK_SUBTRACT:
              if (IsNone(record) && DlgParams->DibData)
              {
                int Pages=DlgParams->pic_info->NumberOfImages;
                FreeImage(DlgParams);
                DlgParams->Loaded=false;
                if(vk==VK_ADD) DlgParams->Page++;
                else DlgParams->Page--;
                if(DlgParams->Page<1) DlgParams->Page=Pages;
                if(DlgParams->Page>Pages) DlgParams->Page=1;
                UpdateImage(DlgParams);
                UpdateInfoText(hDlg,DlgParams);
              }
              return TRUE;
          }
        }
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void GetJiggyWithIt(HANDLE XPanelInfo,bool Override, bool Force)
{
  ViewerInfo info;
  info.StructSize=sizeof(ViewerInfo);
  if(Info.ViewerControl(-1,VCTL_GETINFO,0,&info))
  {
    DialogData data;
    PanelInfo PInfo; PInfo.StructSize=sizeof(PanelInfo);
    Info.PanelControl(XPanelInfo,FCTL_GETPANELINFO,0,&PInfo);

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
    if (PInfo.Flags&PFLAGS_FOCUS)
      data.CurPanel=true;
    else
      data.CurPanel=false;

    intptr_t FileNameSize=Info.ViewerControl(-1,VCTL_GETFILENAME,0,0);
    wchar_t* FileName=(wchar_t*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,FileNameSize*sizeof(wchar_t));
    if(FileName&&Info.ViewerControl(-1,VCTL_GETFILENAME,FileNameSize,FileName)&&(CheckName(FileName)||Override))
    {
      RECT ViewerRect;
      if(data.ShowingIn==VIEWER)
      {
        ViewerRect.left=0;
        ViewerRect.top=0;
        ViewerRect.right=info.WindowSizeX-1;
        ViewerRect.bottom=info.WindowSizeY+1;
      }
      data.FarWindow=(HWND)Info.AdvControl(&MainGuid,ACTL_GETFARHWND,0,0);
      size_t Size=FSF.ConvertPath(CPM_NATIVE,FileName,NULL,0);
      FSF.ConvertPath(CPM_NATIVE,FileName,data.FileName,Size>=32768?32767:Size);

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
      memset(&data.ResKey,0,sizeof(data.ResKey));
      BITMAPINFOHEADER BmpHeader;
      data.BmpHeader=&BmpHeader;
      data.DibData=NULL;
      GFL_FILE_INFORMATION pic_info;
      data.pic_info=&pic_info;
      data.Page=1;
      data.Rotate=0;

      //HANDLE hs=Info.SaveScreen(0,0,-1,-1);
      if(UpdateImage(&data,true))
      {
        FarDialogItem DialogItems[3];
        memset(DialogItems,0,sizeof(DialogItems));
        unsigned int VBufSize;
        FarColor Color;
        if(data.ShowingIn==VIEWER)
        {
          DialogItems[0].Type=DI_EDIT;
          DialogItems[0].X1=0; DialogItems[0].X2=info.WindowSizeX-1;
          DialogItems[0].Y1=0; DialogItems[0].Y2=0;
          DialogItems[0].Data=FileName;
          DialogItems[1].Type=DI_USERCONTROL; DialogItems[1].Flags=DIF_FOCUS;
          DialogItems[1].X1=0; DialogItems[1].X2=info.WindowSizeX-1;
          DialogItems[1].Y1=1; DialogItems[1].Y2=info.WindowSizeY;
          DialogItems[2].Type=DI_EDIT;
          DialogItems[2].X1=0; DialogItems[2].X2=info.WindowSizeX-1;
          DialogItems[2].Y1=info.WindowSizeY+1;
          DialogItems[2].Flags=DIF_READONLY;
          VBufSize=(info.WindowSizeY)*(info.WindowSizeX);
          Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_VIEWERTEXT,&Color);
        }
        else
        {
          DialogItems[0].Type=DI_DOUBLEBOX;
          DialogItems[0].X1=0; DialogItems[0].X2=PInfo.PanelRect.right-PInfo.PanelRect.left;
          DialogItems[0].Y1=0; DialogItems[0].Y2=PInfo.PanelRect.bottom-PInfo.PanelRect.top;
          DialogItems[0].Data=FSF.PointToName(FileName);
          DialogItems[1].Type=DI_USERCONTROL; DialogItems[1].Flags=DIF_FOCUS;
          DialogItems[1].X1=1; DialogItems[1].X2=PInfo.PanelRect.right-PInfo.PanelRect.left-1;
          DialogItems[1].Y1=1; DialogItems[1].Y2=PInfo.PanelRect.bottom-PInfo.PanelRect.top-2;
          DialogItems[2].Type=DI_EDIT;
          DialogItems[2].X1=1; DialogItems[2].X2=PInfo.PanelRect.right-PInfo.PanelRect.left-1;
          DialogItems[2].Y1=PInfo.PanelRect.bottom-PInfo.PanelRect.top-1;
          DialogItems[2].Flags=DIF_READONLY;
          VBufSize=(PInfo.PanelRect.right-PInfo.PanelRect.left-1)*(PInfo.PanelRect.bottom-PInfo.PanelRect.top-2);
          Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_PANELTEXT,&Color);
        }
        FAR_CHAR_INFO *VirtualBuffer;
        VirtualBuffer=(FAR_CHAR_INFO *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,VBufSize*sizeof(FAR_CHAR_INFO));

        if (VirtualBuffer)
        {
          DialogItems[1].VBuf=VirtualBuffer;
          for(unsigned int i=0;i<VBufSize;i++)
          {
            VirtualBuffer[i].Char=L' ';
            VirtualBuffer[i].Attributes=Color;
          }

          HANDLE hDlg=Info.DialogInit(&MainGuid,&DlgGuid,ViewerRect.left,ViewerRect.top,ViewerRect.right,ViewerRect.bottom,NULL,DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]),0,FDLG_SMALLDIALOG|FDLG_NODRAWSHADOW,PicDialogProc,&data);
          if (hDlg != INVALID_HANDLE_VALUE)
          {
            Info.DialogRun(hDlg);
            Info.DialogFree(hDlg);
          }

          HeapFree(GetProcessHeap(),0,VirtualBuffer);
        }

        FreeImage(&data);

        if(!data.SelfKeys)
        {
          struct MacroSendMacroText macro;
          macro.StructSize=sizeof(MacroSendMacroText);
          macro.Flags=KMFLAGS_DISABLEOUTPUT;
          macro.AKey=data.ResKey;
          macro.SequenceText=L"";
          wchar_t Seq[256];

          if (data.ResKey.Event.KeyEvent.wVirtualKeyCode==VK_F3 && data.ShowingIn==VIEWER)
          {
            macro.SequenceText=L"Keys([==[CtrlF10 Esc]==])";
          }
          else
          {
            wchar_t Key[128];
            if(FSF.FarInputRecordToName(&data.ResKey,Key,128))
            {
              wsprintf(Seq,L"Keys([==[%ls]==])",Key);
              macro.SequenceText=Seq;
            }
          }
          Info.MacroControl(0,MCTL_SENDSTRING,MSSC_POST,&macro);
        }
      }
      else
      {
        //Info.RestoreScreen(NULL);
      }
      //Info.RestoreScreen(hs);
    }
    if(FileName) HeapFree(GetProcessHeap(),0,FileName);
  }
}

void SetDefaultExtentions()
{
  int number=gflGetNumberOfFormat();
  GFL_FORMAT_INFORMATION finfo;
  for(int i=0;i<number;i++)
  {
    gflGetFormatInformationByIndex(i,&finfo);
    for(DWORD j=0;j<finfo.NumberOfExtension;j++)
    {
      Exts=(wchar_t **)(((Exts)?HeapReAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,Exts,(ExtsNum+1)*sizeof(*Exts)):HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(ExtsNum+1)*sizeof(*Exts))));
      int size = MultiByteToWideChar(CP_ACP,0,finfo.Extension[j],-1,0,0);
      Exts[ExtsNum]=(wchar_t *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size*sizeof(wchar_t));
      MultiByteToWideChar(CP_ACP,0,finfo.Extension[j],-1,Exts[ExtsNum],size-1);
      ExtsNum++;
    }
  }
}

intptr_t WINAPI ProcessViewerEventW(const struct ProcessViewerEventInfo *pveInfo)
{
  if(pveInfo->Event==VE_READ)
  {
    HANDLE XPanelInfo=PANEL_PASSIVE;
    struct WindowType wi; wi.StructSize=sizeof(WindowType);
    if (Info.AdvControl(&MainGuid,ACTL_GETWINDOWTYPE,0,(void *)&wi) && wi.Type==WTYPE_PANELS)
    {
      struct PanelInfo pi; pi.StructSize=sizeof(PanelInfo);
      Info.PanelControl(PANEL_PASSIVE,FCTL_GETPANELINFO,0,&pi);
      if (pi.PanelType==PTYPE_INFOPANEL)
        return 0;
      pi.StructSize=sizeof(PanelInfo);
      Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELINFO,0,&pi);
      if (pi.PanelType==PTYPE_QVIEWPANEL)
        XPanelInfo=PANEL_ACTIVE;
      else if (pi.PanelType==PTYPE_INFOPANEL)
        return 0;
      // только после проверки! во избежание рекурсии в инфопанели
      Info.PanelControl(PANEL_PASSIVE,FCTL_REDRAWPANEL,0,0);
      Info.PanelControl(PANEL_ACTIVE,FCTL_REDRAWPANEL,0,0);
    }
    GetJiggyWithIt(XPanelInfo,Opt.Override?true:false,false);
  }
  return 0;
}

HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
  GetJiggyWithIt(PANEL_ACTIVE,true,true);
  return NULL;
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=MAKEFARVERSION(3,0,0,8,VS_RC);
  Info->Guid=MainGuid;
  Info->Title=L"PicView Advanced";
  Info->Description=L"PicView Advanced plugin for Far Manager v3.0";
  Info->Author=L"FARMail Group & Alexey Samlyukov";
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  if (Info->StructSize >= sizeof(PluginStartupInfo))
  {
    FSF = *Info->FSF;
    ::Info.FSF = &FSF;
    wchar_t folder[32768];
    lstrcpy(folder, Info->ModuleName);
    *(wchar_t*)(FSF.PointToName(folder)) = 0;
    gflSetPluginsPathnameW(folder);
    gflLibraryInit();
    gflEnableLZW(GFL_TRUE);
    SetDefaultExtentions();

    FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
    if(::Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
    {
      int root=0; // корень ключа
      Opt.AutomaticInViewer=GetValue(settings.Handle,root,L"AutomaticInViewer",0);
      Opt.AutomaticInQuickView=GetValue(settings.Handle,root,L"AutomaticInQuickView",1);
      Opt.BilinearResizeInViewer=GetValue(settings.Handle,root,L"BilinearResizeInViewer",1);
      Opt.BilinearResizeInQuickView=GetValue(settings.Handle,root,L"BilinearResizeInQuickView",0);
      Opt.Override=GetValue(settings.Handle,root,L"Override",0);
      Opt.IgnoreReadError=GetValue(settings.Handle,root,L"IgnoreReadError",1);
      Opt.ShowInViewer=GetValue(settings.Handle,root,L"ShowInViewer",1);
      ::Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
    }
    else
    {
      Opt.AutomaticInViewer=0;
      Opt.AutomaticInQuickView=1;
      Opt.BilinearResizeInViewer=1;
      Opt.BilinearResizeInQuickView=0;
      Opt.Override=0;
      Opt.IgnoreReadError=1;
      Opt.ShowInViewer=1;
    }
  }
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
  static const wchar_t *MenuStrings[1];
  MenuStrings[0]=GetMsg(MTitle);

  Info->StructSize=sizeof(PluginInfo);
  Info->Flags=PF_DISABLEPANELS|PF_VIEWER;
  Info->PluginConfig.Guids=&MenuGuid;
  Info->PluginConfig.Strings=MenuStrings;
  Info->PluginConfig.Count=sizeof(MenuStrings)/sizeof(MenuStrings[0]);
  if (Opt.ShowInViewer)
  {
    Info->PluginMenu.Guids=&MenuGuid;
    Info->PluginMenu.Strings=MenuStrings;
    Info->PluginMenu.Count=sizeof(MenuStrings)/sizeof(MenuStrings[0]);
  }
}

void WINAPI ExitFARW(const struct ExitInfo *Info)
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

intptr_t WINAPI ConfigureW(const struct ConfigureInfo *cfgInfo)
{
  FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
  if (Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
  {
    int root=0; // корень ключа
    Opt.AutomaticInViewer=GetValue(settings.Handle,root,L"AutomaticInViewer",0);
    Opt.AutomaticInQuickView=GetValue(settings.Handle,root,L"AutomaticInQuickView",1);
    Opt.BilinearResizeInViewer=GetValue(settings.Handle,root,L"BilinearResizeInViewer",1);
    Opt.BilinearResizeInQuickView=GetValue(settings.Handle,root,L"BilinearResizeInQuickView",0);
    Opt.Override=GetValue(settings.Handle,root,L"Override",0);
    Opt.IgnoreReadError=GetValue(settings.Handle,root,L"IgnoreReadError",1);
    Opt.ShowInViewer=GetValue(settings.Handle,root,L"ShowInViewer",1);
    Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
  }
  FarDialogItem DialogItems[8];
  memset(DialogItems,0,sizeof(DialogItems));

  DialogItems[0].Type=DI_DOUBLEBOX;
  DialogItems[0].X1=3; DialogItems[0].X2=50;
  DialogItems[0].Y1=1; DialogItems[0].Y2=9;
  DialogItems[0].Data=GetMsg(MTitle);

  DialogItems[1].Type=DI_CHECKBOX;
  DialogItems[1].X1=5;
  DialogItems[1].Y1=2;
  DialogItems[1].Flags=DIF_FOCUS;
  DialogItems[1].Flags|=DIF_DEFAULTBUTTON;
  DialogItems[1].Selected=Opt.AutomaticInViewer;
  DialogItems[1].Data=GetMsg(MAutomaticInViewer);

  DialogItems[2].Type=DI_CHECKBOX;
  DialogItems[2].X1=5;
  DialogItems[2].Y1=3;
  DialogItems[2].Selected=Opt.AutomaticInQuickView;
  DialogItems[2].Data=GetMsg(MAutomaticInQuickView);

  DialogItems[3].Type=DI_CHECKBOX;
  DialogItems[3].X1=5;
  DialogItems[3].Y1=4;
  DialogItems[3].Selected=Opt.BilinearResizeInViewer;
  DialogItems[3].Data=GetMsg(MBilinearResizeInViewer);

  DialogItems[4].Type=DI_CHECKBOX;
  DialogItems[4].X1=5;
  DialogItems[4].Y1=5;
  DialogItems[4].Selected=Opt.BilinearResizeInQuickView;
  DialogItems[4].Data=GetMsg(MBilinearResizeInQuickView);

  DialogItems[5].Type=DI_CHECKBOX;
  DialogItems[5].X1=5;
  DialogItems[5].Y1=6;
  DialogItems[5].Selected=Opt.Override;
  DialogItems[5].Data=GetMsg(MOverride);

  DialogItems[6].Type=DI_CHECKBOX;
  DialogItems[6].X1=5;
  DialogItems[6].Y1=7;
  DialogItems[6].Selected=Opt.IgnoreReadError;
  DialogItems[6].Data=GetMsg(MIgnoreReadError);

  DialogItems[7].Type=DI_CHECKBOX;
  DialogItems[7].X1=5;
  DialogItems[7].Y1=8;
  DialogItems[7].Selected=Opt.ShowInViewer;
  DialogItems[7].Data=GetMsg(MShowInViewer);

  HANDLE hDlg=Info.DialogInit(&MainGuid,&CfgDlgGuid,-1,-1,53,11,NULL,DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]),0,0,0,0);

  if (hDlg != INVALID_HANDLE_VALUE)
  {
    if (Info.DialogRun(hDlg) != -1)
    {
      Opt.AutomaticInViewer=Info.SendDlgMessage(hDlg,DM_GETCHECK,1,0);
      Opt.AutomaticInQuickView=Info.SendDlgMessage(hDlg,DM_GETCHECK,2,0);
      Opt.BilinearResizeInViewer=Info.SendDlgMessage(hDlg,DM_GETCHECK,3,0);
      Opt.BilinearResizeInQuickView=Info.SendDlgMessage(hDlg,DM_GETCHECK,4,0);
      Opt.Override=Info.SendDlgMessage(hDlg,DM_GETCHECK,5,0);
      Opt.IgnoreReadError=Info.SendDlgMessage(hDlg,DM_GETCHECK,6,0);
      Opt.ShowInViewer=Info.SendDlgMessage(hDlg,DM_GETCHECK,7,0);

      FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
      if (Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
      {
        int root=0;
        SetValue(settings.Handle,root,L"AutomaticInViewer",Opt.AutomaticInViewer);
        SetValue(settings.Handle,root,L"AutomaticInQuickView",Opt.AutomaticInQuickView);
        SetValue(settings.Handle,root,L"BilinearResizeInViewer",Opt.BilinearResizeInViewer);
        SetValue(settings.Handle,root,L"BilinearResizeInQuickView",Opt.BilinearResizeInQuickView);
        SetValue(settings.Handle,root,L"Override",Opt.Override);
        SetValue(settings.Handle,root,L"IgnoreReadError",Opt.IgnoreReadError);
        SetValue(settings.Handle,root,L"ShowInViewer",Opt.ShowInViewer);
        Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
      }

      Info.DialogFree(hDlg);
      return TRUE;
    }
    Info.DialogFree(hDlg);
  }
  return FALSE;
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
