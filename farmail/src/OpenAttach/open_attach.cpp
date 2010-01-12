#include "plugin.hpp"
#include "../FARMail/fmp.hpp"
#include "memory.h"

#if defined(__GNUC__)

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}
#endif

enum
{
  mName,
  mBottom,
};

struct PluginStartupInfo FarInfo;
FARSTANDARDFUNCTIONS FSF;
struct MailPluginStartupInfo FarMailInfo;

void *memset(void *dst, int val, size_t count)
{
  void *start = dst;

  while (count--)
  {
    *(char *)dst = (char)val;
    dst = (char *)dst + 1;
  }
  return(start);
}

void *memcpy(void * dst, const void * src, size_t count)
{
  void * ret = dst;

  while (count--)
  {
    *(char *)dst = *(char *)src;
    dst = (char *)dst + 1;
    src = (char *)src + 1;
  }
  return(ret);
}

char *GetMsg(int MsgNum,char *Str)
{
  FarMailInfo.GetMsg(FarMailInfo.MessageName,MsgNum,Str);
  return Str;
}

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct MailPluginStartupInfo *FarMailInfo)
{
  ::FarInfo=*FarInfo;
  FSF=*FarInfo->FSF;
  ::FarMailInfo=*FarMailInfo;
  return 0;
}

void WINAPI _export Exit(void)
{

}

int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData)
{
  (void)InData;
  switch(Msg)
  {
    case FMMSG_GETINFO:
      {
        GetInfoOutData *data=(GetInfoOutData *)OutData;
        GetMsg(mName,data->MenuString);
        lstrcpy(data->HotkeyID,"generic_open_attach");
        data->Flags=FMMSG_MENU;
      }
      return TRUE;
    case FMMSG_MENU:
      {
        char x_start[101],x_end[101];
        if(!(FarMailInfo.GetString(FMSTR_START,x_start,sizeof(x_start),NULL)&&FarMailInfo.GetString(FMSTR_END,x_end,sizeof(x_end),NULL)))
          return TRUE;
        EditorInfo ei;
        FarInfo.EditorControl(ECTL_GETINFO,&ei);
        int MaxLen = (ei.WindowSizeX<128?ei.WindowSizeX:128);
        if (MaxLen<15) MaxLen=15;
        EditorSetPosition esp;
        memset(&esp,0,sizeof(esp));
        if(!FarInfo.EditorControl(ECTL_SETPOSITION,&esp))
          return TRUE;
        EditorGetString egs;
        egs.StringNumber=-1;
        if(!FarInfo.EditorControl(ECTL_GETSTRING,&egs) || FSF.LStricmp(egs.StringText,x_start))
        {
          memcpy(&esp,&ei.CurLine,sizeof(esp));
          FarInfo.EditorControl(ECTL_SETPOSITION,&esp);
          return TRUE;
        }
        FarMenuItemEx *menu=NULL;
        int Items=0;
        do
        {
          if (++esp.CurLine>=ei.TotalLines)
            break;
          if(!FarInfo.EditorControl(ECTL_SETPOSITION,&esp))
            break;
          egs.StringNumber=-1;
          if(!FarInfo.EditorControl(ECTL_GETSTRING,&egs))
            break;
          const char *start=egs.StringText;
          while (*start==0x20 || *start==0x09) start++;
          long Len=lstrlen(start);
          if(Len && !FSF.LStrnicmp(start,"attach ",7))
          {
            Len-=7;
            char *buffer=(char *)malloc(Len+1);
            if(!buffer)
              break;
            lstrcpy(buffer,start+7);
            FSF.Unquote(FSF.Trim(buffer));
            EditorConvertText ect;
            ect.Text=FSF.Trim(buffer);
            ect.TextLength=Len;
            FarInfo.EditorControl(ECTL_EDITORTOOEM,&ect);
            FarMenuItemEx *newmenu=(FarMenuItemEx *)realloc(menu,(Items+1)*sizeof(*menu));
            if (!newmenu)
              break;
            menu=newmenu;
            memset(&menu[Items],0,sizeof(*menu));
            if (Len>=MaxLen)
            {
              lstrcpyn(menu[Items].Text.Text,buffer,10);
              lstrcat(menu[Items].Text.Text,"...");
              lstrcat(menu[Items].Text.Text,buffer+(Len-MaxLen+1+10+3));
            }
            else
              lstrcpy(menu[Items].Text.Text,buffer);
            menu[Items].UserData=(DWORD)buffer;
            DWORD attrib=GetFileAttributes(buffer);
            if (attrib==0xFFFFFFFF || attrib&FILE_ATTRIBUTE_DIRECTORY)
              menu[Items].Flags='?';
            Items++;
          }
        } while (FSF.LStricmp(egs.StringText,x_end));
        if (menu)
        {
          int oldret=0;
          while (true)
          {
            char msg1[128], msg2[128];
            int BreakCode;
            static const int BreakKeys[] =
            {
              VK_F3,
              VK_F4,
              0
            };

            int ret=FarInfo.Menu(FarInfo.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE|FMENU_USEEXT,GetMsg(mName,msg1),GetMsg(mBottom,msg2),NULL,BreakKeys,&BreakCode,(FarMenuItem *)menu,Items);
            if (ret>=0)
            {
              char *ptr=(char *)menu[ret].UserData;
              switch (BreakCode)
              {
                case 0:
                  FarInfo.Viewer(ptr,NULL,0,0,-1,-1,0);
                  break;
                case 1:
                  FarInfo.Editor(ptr,NULL,0,0,-1,-1,0,0,1);
                  break;
                default:
                {
                  SHELLEXECUTEINFO info;
                  memset(&info,0,sizeof(info));
                  info.cbSize=sizeof(info);
                  info.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_DDEWAIT;
                  OemToChar(ptr,ptr);
                  info.lpFile=ptr;
                  info.nShow=SW_SHOWNORMAL;
                  ShellExecuteEx(&info);
                  CharToOem(ptr,ptr);
                }
              }
            }
            else
              break;
            menu[oldret].Flags&=~MIF_SELECTED;
            menu[ret].Flags|=MIF_SELECTED;
            oldret=ret;
          }
          for (int i=0; i<Items; i++)
            free((char *)menu[i].UserData);
          free(menu);
        }
        memcpy(&esp,&ei.CurLine,sizeof(esp));
        FarInfo.EditorControl(ECTL_SETPOSITION,&esp);
      }
      return TRUE;
  }
  return FALSE;
}
