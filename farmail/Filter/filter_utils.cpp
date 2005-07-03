/*
    Filter sub-plugin for FARMail
    Copyright (C) 2002-2004 FARMail Group
    Copyright (C) 1999,2000 Serge Alexandrov

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
#include "filter.hpp"
#include "memory.hpp"

static char msg1[128],msg2[128],msg3[128];

enum
{
  BL_SELECT=0,
  BL_UNSELECT,
  BL_FOLDER,
};

typedef struct __BLOCKLIST
{
  char menu[128];
  char filename[MAX_PATH];
} BLOCKLIST;


typedef struct __BLOCKLINE
{
  char filter[1024];
  int  method;
} BLOCKLINE;

char *strchr(register const char *s,int c)
{
  do
  {
    if(*s==c)
    {
      return (char*)s;
    }
  } while (*s++);
  return (0);
}

static int GetName(char *file,char *name)
{
  HANDLE fp=CreateFile(file,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  char *ptr;
  if(fp!=INVALID_HANDLE_VALUE)
  {
    char str[128];
    while(TRUE)
    {
      if(!ReadLine(fp,str,sizeof(str))) break;
      ptr=str;
      while(*ptr)
      {
        if(*ptr=='\n'||*ptr=='\r') *ptr=0;
        ptr++;
      }
      if((ptr=strchr(str,'='))!=NULL)
      {
        *ptr=0;
        FSF.Trim(str);
        if(!FSF.LStricmp(str,"name"))
        {
          FSF.Trim(ptr+1);
          lstrcpy(name,ptr+1);
          break;
        }
      }
    }
    CloseHandle(fp);
  }
  else lstrcpy(name,"?");
  return 0;
}

static BLOCKLINE *GetBlockLines(char *file,int *total,int lwr)
{
  BLOCKLINE *ptr=NULL;
  int num=0;
  HANDLE fp=CreateFile(file,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if(fp!=INVALID_HANDLE_VALUE)
  {
    char str[1024];
    while(ReadLine(fp,str,sizeof(str)))
    {
      char *z=str;
      while(*z)
      {
        if(*z=='\n'||*z=='\r') *z=0;
        z++;
      }
      FSF.Trim(str);
      if(*str==';') continue;
      z = strchr( str, '=' );
      if(z)
      {
        *z = 0;
        FSF.Trim(str);
        FSF.Trim(z+1);
        if(!FSF.LStricmp(str,"select")||!FSF.LStricmp(str,"unselect"))
        {
          num++;
          ptr=(BLOCKLINE *)z_realloc(ptr,sizeof(BLOCKLINE)*num);
          if(!ptr) break;
          if(*str=='u'||*str=='U')
            ptr[num-1].method=BL_UNSELECT;
          else
            ptr[num-1].method=BL_SELECT;

          lstrcpy(ptr[num-1].filter,z+1);
          if(!lwr) FSF.LStrlwr(ptr[num-1].filter);

        }
      }
    }
    CloseHandle(fp);
  }
  *total=num;
  return ptr;
}

static int SayError(int s)
{
  static const char *err[3];
  err[0]=GetMsg(mError,msg1);
  err[1]=GetMsg(s,msg2);
  err[2]=GetMsg(mOk,msg3);
  FarInfo.Message(FarInfo.ModuleNumber,FMSG_WARNING,NULL,err,3,1);
  return 0;
}

static int GetFilter(char *filtername)
{
  char mask[MAX_PATH];
  HANDLE hf;
  WIN32_FIND_DATA fd;
  int found=0;
  BLOCKLIST *list;
  int retval=1,i;

  FSF.sprintf(mask,"%s*.fmf",DefFiltersDir);

  hf=FindFirstFile(mask,&fd);
  if(hf!=INVALID_HANDLE_VALUE)
  {
    while(1)
    {
      if(*fd.cFileName!='.'&&!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) found++;
      if(!FindNextFile(hf,&fd)) break;
    }
    FindClose(hf);
  }
  if(!found)
  {
    SayError(mNoLists);
    return 1;
  }
  list=(BLOCKLIST *)z_calloc(found,sizeof(BLOCKLIST));
  if(list)
  {
    found = 0;
    hf=FindFirstFile(mask,&fd);
    if(hf!=INVALID_HANDLE_VALUE)
    {
      while(1)
      {
        if(*fd.cFileName!='.'&&!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
        {
          char fullname[MAX_PATH];
          lstrcpy(fullname,DefFiltersDir);
          lstrcat(fullname,fd.cFileName);
          lstrcpy(list[found].filename,fullname);
          GetName(fullname,list[found].menu);
          found++;
        }
        if(!FindNextFile(hf,&fd)) break;
      }
      FindClose(hf);
    }
    if(found)
    {
      struct FarMenuItem *menu=(struct FarMenuItem *)z_calloc(found,sizeof(struct FarMenuItem));
      if(menu)
      {
        for(i=0;i<found;i++)
        {
          if(!i) menu[i].Selected=1;
          lstrcpy(menu[i].Text,list[i].menu);
        }
        //FIXME help
        int sel=FarInfo.Menu(FarInfo.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,GetMsg(mFiltersList,msg1),NULL,/*CONTENTS*/NULL,NULL,NULL,menu,found);
        z_free(menu);
        if(sel>=0)
        {
          retval=0;
          if(filtername) lstrcpy(filtername,list[sel].filename);
        }
      }
      else
      {
        SayError(mNoMem);
      }
    }
    else
    {
      SayError(mNoLists);
    }
    z_free(list);
  }
  else
  {
    SayError(mNoMem);
    return 1;
  }
  return retval;
}

static int CompareEStrings(char *str,char *mask,int SkipEndOfStr)
{
  while(*mask)
  {
    if(*mask!=*str&&*mask!='?') return 1;
    if(!*str) return 1;
    mask++;
    str++;
  }
  if(!(*str)||SkipEndOfStr) return 0;
  return 1;
}

static int ComparePattern(char *str,char *mask)
{
  int stat;
  char *ptr=strchr(mask,'*');
  if(ptr)
  {
    *ptr=0;
    stat=CompareEStrings(str,mask,1);
    *ptr='*';
    if(!stat)
    {
      if(*(ptr+1))
      {
        char *mptr=str+(ptr-mask);
        while(*mptr)
        {
          if(!ComparePattern(mptr,ptr+1)) return 0;
          mptr++;
        }
        return 1;
      }
      return 0;
    }
    else return stat;
  }
  else
  {
    return CompareEStrings(str,mask,0);
  }
}

static const char FilterHistory[]="FARMailFltHist";

int RunFilter(HANDLE hPlugin,int index)
{
  int stat;
  int total=0;
  BLOCKLINE *blocklist=NULL;

  static struct InitDialogItem InitItems[]=
  {
    // type,x1,y1,x2,y2,focus,selected,flags,default,data
    {DI_DOUBLEBOX,3,1,66,12,0,0,0,0,(char*)mFilter1},
    {DI_TEXT,5,3,0,0,0,0,0,0,(char*)mFilter2},
    {DI_EDIT,5,4,64,4,1,(DWORD)FilterHistory,DIF_HISTORY,0,NULLSTR},
    {DI_TEXT,3,6,0,0,0,0,DIF_SEPARATOR,0,NULLSTR},
    {DI_CHECKBOX,5,8,0,0,0,0,0,0,(char*)mFilter4},
    {DI_CHECKBOX,5,9,0,0,0,0,0,0,(char*)mFilter3},
    {DI_CHECKBOX,5,10,0,0,0,0,0,0,(char*)mFilter5},

    {DI_BUTTON,50,10,0,0,0,0,0,0,(char *)mUseFile},

    {DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,1,(char *)mOk},
    {DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,0,(char *)mCancel}
  };

  enum
  {
    C_EDIT1=2,
    C_TXT1,
    C_CHK1,
    C_CHK2,
    C_CHK3,
    C_USEFILE,
    C_OK,
    C_CANCEL
  };
  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  stat=FarInfo.DialogEx(FarInfo.ModuleNumber,-1,-1,70,15,"PatternFilter",DialogItems,sizeofa(DialogItems),0,0,FarMailInfo.ShowHelpDlgProc,(long)FarMailInfo.ModuleName);
  if(stat==C_OK||stat==C_USEFILE)
  {
    PanelInfo pis;
    char fname[MAX_PATH];
    if(stat==C_OK)
    {
      if(!DialogItems[C_CHK3].Selected) FSF.LStrlwr(DialogItems[C_EDIT1].Data);
    }
    else
    {
      if(GetFilter(fname)) return 0;
      blocklist=GetBlockLines(fname,&total,DialogItems[C_CHK3].Selected);
      if(!blocklist) return 0;
    }
    if(FarInfo.Control(hPlugin,FCTL_GETPANELINFO,(void*)&pis))
    {
      int i;
      for(i=0;i<pis.ItemsNumber;i++)
      {
        if(!(pis.PanelItems[i].Flags&PPIF_SELECTED)&&DialogItems[C_CHK1].Selected) continue;

        if(!lstrcmp(pis.PanelItems[i].FindData.cFileName,"..")) continue;
        if(!pis.PanelItems[i].CustomColumnData) continue;

        char *line;
        char *buff;

        int yup=0;

        line=pis.PanelItems[i].CustomColumnData[index];
        if(line)
        {
          int res=0;
          int match=0;

          buff=z_strdup(line);
          if(buff)
          {
            char *freethis=buff;
            *buff=0;
            while(*line)
            {
              if(*line=='\r'&&*(line+1)=='\n')
              {
                if(*(line+2)==32||*(line+2)==9)
                {
                  // skipping;
                  line+=2;
                }
                else
                {
                  // check for pattern
                  if(!DialogItems[C_CHK3].Selected) FSF.LStrlwr(freethis);
                  if(stat==C_OK)
                  {
                    if(ComparePattern(freethis,DialogItems[C_EDIT1].Data))
                    {
                      // no...
                      buff=freethis;
                      *buff=0;
                      line+=2;
                    }
                    else
                    {
                      yup=1;
                      break;
                    }
                  }
                  else
                  {
                    int k;
                    for(k=0;k<total;k++)
                    {
                      if(!ComparePattern(freethis,blocklist[k].filter)&&k>=match)
                      {
                        if(blocklist[k].method==BL_SELECT)
                          res=1;
                        else
                          res=2;
                        match=k;
                      }
                    }
                    buff=freethis;
                    *buff=0;
                    line+=2;
                  }
                }
              }
              else
              {
                *(buff++)=*(line++);
                *(buff)=0;
              }
            }
            z_free(freethis);
            if(res) yup=res;
            if(!yup) yup=2; // deselect
          }
        }
        if(yup)
        {
          if(yup==1)
            pis.PanelItems[i].Flags|=PPIF_SELECTED;
          else if(pis.PanelItems[i].Flags&PPIF_SELECTED)
            pis.PanelItems[i].Flags-=PPIF_SELECTED;
        }
        if(DialogItems[C_CHK2].Selected)
        {
          pis.PanelItems[i].Flags^=PPIF_SELECTED;
        }
      }
      FarInfo.Control(hPlugin,FCTL_SETSELECTION,(void*)&pis);
      FarInfo.Control(hPlugin,FCTL_REDRAWPANEL,NULL);
    }
  }
  if(blocklist)z_free(blocklist);
  return 0;
}
