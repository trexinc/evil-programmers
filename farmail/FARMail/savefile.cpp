/*
    FARMail plugin for FAR Manager
    Copyright (C) 2002-2005 FARMail Group
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
#include "farmail.hpp"

int GetFreeNumber( char *dir )
{

  HANDLE hff;
  WIN32_FIND_DATA fd;
  int maxval = 0;

  char buf[MAX_PATH];

  if ( *dir && dir[lstrlen(dir)-1] != '\\' )
    FSF.sprintf( buf, "%s\\*.%s", dir, Opt.EXT );
  else
    FSF.sprintf( buf, "%s*.%s", dir, Opt.EXT );

  hff = FindFirstFile( buf, &fd );
  if ( hff != INVALID_HANDLE_VALUE )
  {
    do
    {
      int i = FSF.atoi(fd.cFileName );
      if ( !i ) i = FSF.atoi(fd.cAlternateFileName );

      if ( i>maxval ) maxval = i;

    } while (FindNextFile(hff, &fd));
    FindClose( hff );
  }

  return maxval+1;
}

int FARMail::SaveOutgoingMessage( int how, const char *str )
{

  if(!Opt.SaveMessages) return 0;

  DWORD transferred;
  switch(how)
  {
    case SAVE_OPEN:
    {
      char SaveDirParsed[MAX_PATH];
      FSF.ExpandEnvironmentStr(Opt.SaveDir,SaveDirParsed,MAX_PATH);

      int num=GetFreeNumber(SaveDirParsed);
      savefp=INVALID_HANDLE_VALUE;
      tempfp=INVALID_HANDLE_VALUE;
      if ( num > 0 )
      {
        if ( *SaveDirParsed && SaveDirParsed[lstrlen(SaveDirParsed)-1] != '\\' )
          FSF.sprintf( savefp_name, "%s\\%08d.%s", SaveDirParsed, num, Opt.EXT );
        else
          FSF.sprintf( savefp_name, "%s%08d.%s", SaveDirParsed, num, Opt.EXT );
        if (Opt.SaveMessageID && FSF.MkTemp(tempfp_name,"FARMail"))
        {
          tempfp=CreateFile(tempfp_name,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
        }
        if (Opt.UseOutbox && *(Opt.PathToOutbox))
        {
          char PathToOutboxParsed[MAX_PATH];
          FSF.ExpandEnvironmentStr(Opt.PathToOutbox,PathToOutboxParsed,MAX_PATH);
          savefp=CreateFile(PathToOutboxParsed,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
          if(savefp!=INVALID_HANDLE_VALUE)
            if(SetFilePointer(savefp,0,NULL,FILE_END)==INVALID_SET_FILE_POINTER)
            {
              CloseHandle(savefp);
              savefp=INVALID_HANDLE_VALUE;
            }
          if(savefp!=INVALID_HANDLE_VALUE)
          {
            char head[1024];
            FSF.sprintf(head,"From  %s\r\n",GetDateInSMTP());
            WriteFile(savefp,head,lstrlen(head),&transferred,NULL);
          }
        }
        else
        {
          savefp=CreateFile(savefp_name,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
          if(savefp!=INVALID_HANDLE_VALUE)
          {
            char head[1024];
            FSF.sprintf(head,"From  %s\r\n",GetDateInSMTP());
            WriteFile(savefp,head,lstrlen(head),&transferred,NULL);
          }
        }
      }
      break;
    }
    case SAVE_STR:
      if(tempfp!=INVALID_HANDLE_VALUE)
      {
        if(str[0]=='.'&&str[1]=='.')
          WriteFile(tempfp,str+1,lstrlen(str+1),&transferred,NULL);
        else if(str[0]!='.')
        {
          if ((Opt.UseOutbox && *(Opt.PathToOutbox)) && !strncmp(str,"From ",5))
            WriteFile(tempfp,">",1,&transferred,NULL);
          WriteFile(tempfp,str,lstrlen(str),&transferred,NULL);
        }
      }
      else if(savefp!=INVALID_HANDLE_VALUE)
      {
        if(str[0]=='.'&&str[1]=='.')
          WriteFile(savefp,str+1,lstrlen(str+1),&transferred,NULL);
        else if(str[0]!='.')
        {
          if ((Opt.UseOutbox && *(Opt.PathToOutbox)) && !strncmp(str,"From ",5))
            WriteFile(savefp,">",1,&transferred,NULL);
          WriteFile(savefp,str,lstrlen(str),&transferred,NULL);
        }
      }
      break;
    case SAVE_CLOSE:
      if((tempfp!=INVALID_HANDLE_VALUE)&&(savefp!=INVALID_HANDLE_VALUE))
      {
        WriteFile(savefp,"Message-ID: <",13,&transferred,NULL);
        WriteFile(savefp,str,lstrlen(str),&transferred,NULL);
        WriteFile(savefp,">\r\n",3,&transferred,NULL);
        SetFilePointer(tempfp,0,NULL,FILE_BEGIN);
        DWORD s;
        do
        {
          char buf[1024*10];
          ReadFile(tempfp,buf,sizeof(buf),&s,NULL);
          WriteFile(savefp,buf,s,&transferred,NULL);
        } while (s);
      }
      if(savefp!=INVALID_HANDLE_VALUE) CloseHandle(savefp);
      savefp=INVALID_HANDLE_VALUE;
      if(tempfp!=INVALID_HANDLE_VALUE)
      {
        CloseHandle(tempfp);
        DeleteFile(tempfp_name);
      }
      tempfp=INVALID_HANDLE_VALUE;
      *tempfp_name=0;
      *savefp_name=0;
      break;
    case SAVE_KILL:
      if(tempfp!=INVALID_HANDLE_VALUE)
      {
        CloseHandle(tempfp);
        DeleteFile(tempfp_name);
      }
      if(savefp!=INVALID_HANDLE_VALUE)
      {
        CloseHandle(savefp);
        if(!(Opt.UseOutbox&&*(Opt.PathToOutbox)))
          DeleteFile(savefp_name);
      }
      tempfp=INVALID_HANDLE_VALUE;
      savefp=INVALID_HANDLE_VALUE;
      *tempfp_name=0;
      *savefp_name=0;
      break;
  }
  return 0;
}

int FARMail::InvWin( const char *name, const char *prompt, const char *def, char *res )
{
  int key;

  static struct InitDialogItem InitItems[]=
  {
    // type, x1, y1, x2, y2, focus, selected, flags, default, data
    { DI_DOUBLEBOX,3,1,65,6,0,0,0,0,name },
    { DI_TEXT, 5, 3, 0, 0, 0,0,0, 0,prompt },
    { DI_EDIT, 5, 4, 63,5,1,0,0,0, def },
    { DI_BUTTON,3,7,0,0,0,0,0,1,(char *)MesOk },
    { DI_BUTTON,11,7,0,0,0,0,0,0,(char *)MesCancel }
  };

  enum
  {
    D_EDIT = 2,
    D_OK,
    D_CANCEL
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  key = _Info.Dialog( _Info.ModuleNumber,-1,-1,69,9, NULL, DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));

  if ( key == D_OK )
  {
    lstrcpy( res, DialogItems[D_EDIT].Data );
  }

  return !( key == D_OK );
}
