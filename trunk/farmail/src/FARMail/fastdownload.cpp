/*
    FARMail plugin for FAR Manager
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
#include "farmail.hpp"

const char FINBOX[] = "INBOX";

int FARMail::GetDialogEx(int Move, const char *dir, char *newdir, int *uniq, int *useinbox)
{
  int key;
  struct InitDialogItem InitItems[]=
  {
    // type, x1, y1, x2, y2, focus, selected, flags, default, data
    {DI_DOUBLEBOX,3,1,72,9,0,0,0,0, (char *)MesCopy_Title},
    {DI_TEXT, 5, 2, 0, 0, 0,0,0, 0, (char *)MesCopy_Message},
    {DI_RADIOBUTTON, 5, 3, 0, 0, 0,0,DIF_GROUP,0, NULLSTR},
    {DI_RADIOBUTTON, 5, 4, 0, 0, 0,0,0,0, NULLSTR},
    {DI_EDIT, 9, 4, 70,3,1,0,0,0, NULLSTR},
    {DI_TEXT, 3, 5, 0, 0, 0,0,DIF_SEPARATOR, 0, NULLSTR},
    {DI_CHECKBOX, 5, 6, 0, 0, 0,0,0,0, (char *)MesCopy_AssignUniqueNamesChkBx},
    {DI_TEXT, 3, 7, 0, 0, 0,0,DIF_SEPARATOR, 0, NULLSTR},
    {DI_BUTTON,0,8,0,0,0,0,DIF_CENTERGROUP,1,(char *)MesCopy_CopyBtn},
    {DI_BUTTON,0,8,0,0,0,0,DIF_CENTERGROUP,0,(char *)MesCancel}
  };
  enum
  {
    FD_TOINBOX=2,
    FD_TODIR,
    FD_EDIT,
    FD_TXT1,
    FD_UNIQ,
    FD_TXT2,
    FD_OK,
    FD_CANCEL
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems, DialogItems, sizeof(InitItems)/sizeof(InitItems[0]));

  if (Move)
  {
    lstrcpy(DialogItems[0].Data, GetMsg(MesMove_Title));
    lstrcpy(DialogItems[1].Data, GetMsg(MesMove_Message));
    lstrcpy(DialogItems[FD_OK].Data, GetMsg(MesMove_MoveBtn));
  }
  lstrcpy(DialogItems[FD_EDIT].Data, dir);
  DialogItems[FD_UNIQ].Selected = *uniq;
  if (Opt.UseInbox && *Opt.PathToInbox)
  {
    DialogItems[FD_TOINBOX].Selected = TRUE;
    DialogItems[FD_TODIR].Selected = FALSE;
    lstrcpy(DialogItems[FD_TOINBOX].Data, Opt.PathToInbox);
    FSF.TruncPathStr(DialogItems[FD_TOINBOX].Data, 70-5+1-4);
  }
  else
  {
    DialogItems[FD_TODIR].Selected = TRUE;
    DialogItems[FD_TOINBOX].Selected = FALSE;
    DialogItems[FD_TOINBOX].Flags |= DIF_DISABLE;
    lstrcpy(DialogItems[FD_TOINBOX].Data, GetMsg(MesCopy_NoInbox));
  }
  //if ( *DialogItems[FD_EDIT].Data && DialogItems[FD_EDIT].Data[lstrlen(DialogItems[FD_EDIT].Data)-1] != '\\' && DialogItems[FD_EDIT].Data[lstrlen(DialogItems[FD_EDIT].Data)-1] != ':' )
    //lstrcat( DialogItems[FD_EDIT].Data, BACKSLASH );

  key = _Info.Dialog(_Info.ModuleNumber,-1,-1,76,11, "FastDownload", DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));
  if (key == FD_OK)
  {
    lstrcpy(newdir, DialogItems[FD_EDIT].Data);
    if (DialogItems[FD_UNIQ].Selected )
      *uniq = 1;
    else
      *uniq = 0;
    if (DialogItems[FD_TOINBOX].Selected)
      *useinbox = 1;
    else
      *useinbox = 0;
  }

  return !(key == FD_OK);
}

int FARMail::FastDownload(struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, const char *DestPath, int OpMode)
{
#ifdef TDEBUG
try {
#endif
  char buf[MAX_PATH*2+100];
  const char *err[8];
  char NewDestPath[512];
  char filename[512];
  int newnum = 0;
  int UseInbox = Opt.UseInbox;
  bool silent = (OpMode&OPM_SILENT);
  BOOL res;

  lstrcpy(NewDestPath, DestPath);

  err[0] = GetMsg(MesOverwrite_Title);
  err[1] = NULLSTR;
  err[2] = GetMsg(MesOverwrite_Question);
  err[3] = GetMsg(MesOverwrite_YesBtn);  // yes
  err[4] = GetMsg(MesOverwrite_AllBtn);  // all
  err[5] = GetMsg(MesOverwrite_SkipBtn);  // skio
  err[6] = GetMsg(MesOverwrite_SkipAllBtn);  // skip all
  err[7] = GetMsg(MesCancel); // cancel

  int i, all = 0, skip_all = 0;

  if (!silent && GetDialogEx(Move, DestPath, NewDestPath, &Opt.Unique, &UseInbox))
    return -1;

  for (int j=0; j < ItemsNumber; j++)
  {
    if (clnt)
    {
      clnt->Disconnect();
      delete (MailClient *)clnt;
      clnt = NULL;
    }
    if (imap)
    {
      imap->Disconnect();
      delete (IMAP *)imap;
      imap = NULL;
    }
    current = NULL;

    for (i=0; i<ServerCount; i++)
    {
      if (!lstrcmp(PanelItem[j].FindData.cFileName, server[i].Name))
      {
        if (*server[i].Url && ((*server[i].User && *server[i].Pass) || !GetUser(server[i]._User,server[i]._Pass)))
        {
          current = &server[i];
          if (PanelItem[j].Flags&PPIF_SELECTED)
            PanelItem[j].Flags ^= PPIF_SELECTED;
        }
        break;
      }
    }
    if (!current)
      continue;
    if (current->Type == TYPE_POP3)
    {
      res = FALSE;
      clnt = new MailClient((Opt.DebugSession?TRUE:FALSE), Opt.LOGFILE, current->timeout*60);
      if (clnt)
      {
        clnt->FastDownload = TRUE;
        lstrcpy(clnt->ServerName,current->Name);
#ifdef FARMAIL_SSL
        res = clnt->Connect(current->Url, current->_User, current->_Pass, current->Port, (current->UsePOP3S?CON_SSL:CON_NORMAL));
#else
        res = clnt->Connect(current->Url, current->_User, current->_Pass, current->Port);
#endif
        if (res)
          res = clnt->Statistics();
        if ( !res /*&& !(OpMode & OPM_FIND)*/)
          SayError(clnt->GetErrorMessage());
      }
      if (!res)
        continue;
      if (clnt && clnt->connected)
      {
        for (i=1; i<=clnt->NumberMail; i++)
        {
          char dest[MAX_PATH*2];

          if (!UseInbox)
          {
            lstrcpy(dest, NewDestPath);
            if (*(dest+lstrlen(dest)-1) != '\\')
            {
              // a filename?
              DWORD aa = GetFileAttributes(dest);
              if ( aa!=0xFFFFFFFF && (aa&FILE_ATTRIBUTE_DIRECTORY) )
              {
                lstrcat(dest, BACKSLASH);
                // it's a directory!
                if (Opt.Unique || silent)
                  newnum = GetFreeNumber(dest);
                if (newnum > 0)
                {
                  char newname[MAX_PATH];
                  FSF.sprintf(newname, "%08ld.%s", newnum, Opt.EXT);
                  lstrcat(dest, newname);
                }
                else
                  lstrcat(dest, GenerateName(i, filename));
              } // else - a filename
            }
            else
            { // it's a directory name
              if (Opt.Unique || silent)
                newnum = GetFreeNumber(dest);

              if (newnum > 0)
              {
                char newname[MAX_PATH];
                FSF.sprintf(newname, "%08ld.%s", newnum, Opt.EXT);
                lstrcat(dest, newname);
              }
              else
                lstrcat(dest, GenerateName(i, filename));
            }

            if (GetFileAttributes(dest) != 0xFFFFFFFF && !all && !silent)
            {
              if (skip_all)
                continue;
              FSF.sprintf(buf, GetMsg(MesOverwrite_Filename), /* PanelItem[i].FindData.cFileName */ dest );
              err[1] = buf; //GetMsg(MesOverwrite_Filename);
              int answer = _Info.Message(_Info.ModuleNumber, FMSG_WARNING, NULL, err, 8, 5);
              if (answer == -1 || answer == 4)
                break;
              if (answer == 2)
                continue; // skip
              if (answer == 3)
              {
                skip_all = 1;
                continue;
              }
              if (answer == 1)
                all = 1;
            }
          }

          if (clnt->Retrieve(i, 0/*OpMode*/))
          {
            HANDLE fp;
            if(UseInbox)
            {
              char PathToInboxParsed[MAX_PATH];
              FSF.ExpandEnvironmentStr(Opt.PathToInbox,PathToInboxParsed,MAX_PATH);
              fp = CreateFile(PathToInboxParsed,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
              if (fp != INVALID_HANDLE_VALUE)
              {
                if (SetFilePointer(fp,0,NULL,FILE_END) == INVALID_SET_FILE_POINTER)
                {
                  CloseHandle(fp);
                  fp = INVALID_HANDLE_VALUE;
                }
              }
            }
            else
              fp = CreateFile(dest,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
            if (fp != INVALID_HANDLE_VALUE)
            {
              {
                DWORD written;
                char head[1024];
                FSF.sprintf(head,"From  %s\r\n",GetDateInSMTP());
                WriteFile(fp,head,lstrlen(head),&written,NULL);
              }
              int wrote = SaveToFile(fp,clnt->GetMsg(),UseInbox);
              CloseHandle(fp);
              if (wrote==1)
              {
                SayError(::GetMsg(MesErrWriteFile));
                break;
              }
            }
            else
            {
              SayError(::GetMsg(MesErrOpenFile));
              break;
            }
            if (Move && !clnt->Delete(i))
            {
              SayError(clnt->GetErrorMessage());
              break;
            }
          }
          else
          {
            SayError(clnt->GetErrorMessage());
            break;
          }
        }
      }
      else
        break;
    }
    else
    {
      res = TRUE;
      imap = new IMAP( (Opt.DebugSession?TRUE:FALSE) , Opt.LOGFILE3 , current->timeout*60 );
      if (imap)
      {
#ifdef FARMAIL_SSL
        res = imap->Connect(current->Url, current->Port, (current->UsePOP3S?CON_SSL:CON_NORMAL));
#else
        res = imap->Connect(current->Url, current->Port);
#endif
        if (res /*&& !(OpMode & OPM_FIND)*/)
        {
          SayError(imap->GetErrorMessage());
        }
        else if (!res)
        {
          res = imap->Login(current->_User, current->_Pass);
          if (!res)
          {
            res = imap->Capability();
            if (res /*&& !(OpMode & OPM_FIND)*/ )
            {
              SayError( imap->GetErrorMessage() );
            }
          }
          else /*if ( !(OpMode & OPM_FIND) )*/
          {
            SayError( imap->GetErrorMessage() );
          }
        }
      }
      if (res)
        continue;
      if (imap && imap->connected && !imap->Select(FINBOX))
      {
        ShortMessage *sm = new ShortMessage(MsgListPOP);
        imap->Noop();
        if (!imap->Status(FINBOX, BRACED_MESSAGES))
        {
          int line = 0;
          while (imap->GetRespString(line++))
          {
            if (imap->GetRespToken(0) && !lstrcmp(imap->RespString2, ASTERISK))
            {
              if (imap->GetRespToken(1) && !FSF.LStricmp(imap->RespString2, STATUS))
              {
                if (imap->GetRespToken(3))
                {
                  int token = 0;

                  lstrcpy(imap->RespString, imap->RespString2);
                  FSF.LStrupr(imap->RespString);

                  while (imap->GetRespToken(token++))
                  {
                    if (!lstrcmp(imap->RespString2, MESSAGES))
                    {
                      if (imap->GetRespToken(token++))
                        imap->MessageNumber = FSF.atoi(imap->RespString2);
                      else
                        break;
                    }
                  }
                }
              }
            }
          }
        }
        delete sm;

        for (i=0; i<imap->MessageNumber; i++)
        {
          char dest[MAX_PATH*2];

          if (!UseInbox)
          {
            lstrcpy(dest, NewDestPath);
            if (*(dest+lstrlen(dest)-1) != '\\')
            {
              // a filename?
              DWORD aa = GetFileAttributes(dest);
              if (aa!=0xFFFFFFFF && (aa&FILE_ATTRIBUTE_DIRECTORY))
              {
                // it's a directory!
                if (Opt.Unique || silent)
                  newnum = GetFreeNumber(dest);
                lstrcat (dest, BACKSLASH);
                if (newnum > 0)
                {
                  char newname[MAX_PATH];
                  FSF.sprintf(newname, "%08ld.%s", newnum, Opt.EXT);
                  lstrcat(dest, newname);
                }
                else
                  lstrcat(dest, GenerateName(i, filename));
              } // else - a filename
            }
            else
            { // it's a directory name
              if (Opt.Unique || silent)
                newnum = GetFreeNumber(dest);
              if (newnum > 0)
              {
                char newname[MAX_PATH];
                FSF.sprintf(newname, "%08ld.%s", newnum, Opt.EXT);
                lstrcat(dest, newname);
              }
              else
                lstrcat(dest, GenerateName(i, filename));
            }

            if (GetFileAttributes(dest) != 0xFFFFFFFF && !all && !silent)
            {
              if (skip_all)
                continue;
              FSF.sprintf(buf, GetMsg(MesOverwrite_Filename), dest);
              err[1] = buf;
              int answer = _Info.Message(_Info.ModuleNumber, FMSG_WARNING, NULL, err, 8, 5);
              if (answer == -1 || answer == 4)
                break;
              if (answer == 2)
                continue; // skip
              if (answer == 3)
              {
                skip_all = 1;
                continue;
              }
              if (answer == 1)
                all = 1;
            }
          }
          int size = 0;
          if (!imap->Fetch(i+1, RFC822SIZE ,0,0,NULL))
          {
            int line = 0;
            while (imap->GetRespString(line++))
            {
              if (imap->GetRespToken(0) && !lstrcmp(imap->RespString2, ASTERISK))
              {
                if (imap->GetRespToken(1) && i+1==FSF.atoi(imap->RespString2))
                {
                  if (imap->GetRespToken(2) && !lstrcmp(imap->RespString2, FETCH))
                  {
                    imap->GetRespToken(3);
                    lstrcpy(imap->RespString, imap->RespString2);
                    FSF.LStrupr( imap->RespString );
                    if (imap->GetRespToken(0) && !lstrcmp(imap->RespString2, RFC822SIZE))
                    {
                      if (imap->GetRespToken(1))
                        size = FSF.atoi(imap->RespString2);
                    }
                  }
                }
              }
            }
          }
          char x_of_y[120];
          FSF.sprintf(x_of_y,GetMsg(MesProgressReceiveFast_Title), current->Name, i+1, imap->MessageNumber);
          char *msgtxt = imap->GetMsgText(i+1, size, x_of_y);

          if (msgtxt != NULL)
          {
            HANDLE fp;
            if (UseInbox)
            {
              char PathToInboxParsed[MAX_PATH];
              FSF.ExpandEnvironmentStr(Opt.PathToInbox,PathToInboxParsed,MAX_PATH);
              fp = CreateFile(PathToInboxParsed,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
              if (fp != INVALID_HANDLE_VALUE)
              {
                if (SetFilePointer(fp,0,NULL,FILE_END) == INVALID_SET_FILE_POINTER)
                {
                  CloseHandle(fp);
                  fp = INVALID_HANDLE_VALUE;
                }
              }
            }
            else
              fp = CreateFile(dest,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
            if(fp==INVALID_HANDLE_VALUE)
            {
              SayError(::GetMsg(MesErrOpenFile));
              break;
            }

            char uid[41];
            *uid = '?';
            if (!imap->Fetch(i+1, UID ,0,0,NULL))
            {
              int line = 0;
              while (imap->GetRespString(line++))
              {
                if (imap->GetRespToken(0) && !lstrcmp(imap->RespString2, ASTERISK))
                {
                  if (imap->GetRespToken(1) && i+1==FSF.atoi(imap->RespString2))
                  {
                    if (imap->GetRespToken(2) && !lstrcmp(imap->RespString2, FETCH))
                    {
                      imap->GetRespToken(3);
                      lstrcpy(imap->RespString, imap->RespString2);
                      FSF.LStrupr(imap->RespString);
                      if (imap->GetRespToken(0) && !lstrcmp(imap->RespString2, UID))
                      {
                        if (imap->GetRespToken(1))
                          lstrcpy(uid, imap->RespString2 );
                      }
                    }
                  }
                }
              }
            }

            if (fp != INVALID_HANDLE_VALUE)
            {
              {
                DWORD written;
                char head[1024];
                FSF.sprintf(head,"From  %s\r\n",GetDateInSMTP());
                WriteFile(fp,head,lstrlen(head),&written,NULL);
              }
              char write_buf[128];
              DWORD written;
              bool wrote = true;
              if (imap->Partial)
              {
                FSF.sprintf(write_buf,XRESUMEDATA,atol(uid),imap->ReceivedMessageLen);
                wrote = wrote && (WriteFile(fp,write_buf,lstrlen(write_buf),&written,NULL)&&written);
              }
              if (UseInbox)
              {
                char *str = msgtxt;
                char *ptr = msgtxt;
                if (!strncmp(msgtxt,"From ",5))
                  wrote = wrote && (WriteFile(fp,">",1,&written,NULL)&&written);
                while (*ptr)
                {
                  while (*ptr && *ptr!='\n')
                    ptr++;
                  {
                    int len = ptr-str;
                    wrote = wrote && (WriteFile(fp,str,len,&written,NULL)&&(len?written:1));
                  }
                  str = ptr+1;
                  if (*ptr=='\n' && !strncmp(ptr+1,"From ",5))
                    wrote = wrote && (WriteFile(fp,">",1,&written,NULL)&&written);
                  if (*ptr)
                    ptr++;
                }
              }
              else
              {
                int len = lstrlen(msgtxt);
                wrote = wrote && (WriteFile(fp,msgtxt,len,&written,NULL)&&(len?written:1));
              }
              CloseHandle(fp);
              if (!wrote)
              {
                SayError(::GetMsg(MesErrWriteFile));
                break;
              }
            }
            if (Move && !imap->Partial && imap->DeleteUID(uid))
            {
              SayError(imap->GetErrorMessage());
              break;
            }
          }
          else
          {
            SayError(imap->GetErrorMessage());
            break;
          }
        }

        if (Move && imap->Expunge())
        {
          SayError(imap->GetErrorMessage());
          break;
        }
      }
      else
        break;
    }
  }
  return 1;

#ifdef TDEBUG
}
#ifdef __BORLANDC__
__except(1)
#else
catch(...)
#endif
{
 SayException("FastDownload");
 return -1;
}
#endif

}

int FARMail::FastExpunge(struct PluginPanelItem *PanelItem, int ItemsNumber, int OpMode)
{
#ifdef TDEBUG
try {
#endif
  BOOL res;
  const char *mes[4];
  int silent = (OpMode&OPM_SILENT);

  mes[0] = GetMsg(MesEmptyMailbox);
  mes[1] = GetMsg(MesEmptyMailboxConfirm);
  mes[2] = GetMsg(MesOk);
  mes[3] = GetMsg(MesCancel);

  if (!silent && _Info.Message(_Info.ModuleNumber, 0, "FastExpunge", mes, 4, 2) !=0)
    return -1;

  for (int j=0; j < ItemsNumber; j++)
  {
    if ( clnt ) { clnt->Disconnect(); delete (MailClient*)clnt; clnt = NULL; }
    if ( imap ) { imap->Disconnect(); delete (IMAP*)imap; imap = NULL; }
    current = NULL;

    for (int i=0; i<ServerCount; i++)
    {
      if ( !lstrcmp( PanelItem[j].FindData.cFileName, server[i].Name ) )
      {
        if (*server[i].Url && ((*server[i].User && *server[i].Pass) || !GetUser(server[i]._User,server[i]._Pass)))
        {
          current = &server[i];
          if (PanelItem[j].Flags&PPIF_SELECTED)
            PanelItem[j].Flags^=PPIF_SELECTED;
        }
        break;
      }
    }
    if (!current) continue;
    if ( current->Type == TYPE_POP3 )
    {
      res = FALSE;
      clnt = new MailClient( (Opt.DebugSession?TRUE:FALSE) , Opt.LOGFILE , current->timeout*60 );
      if ( clnt )
      {
        clnt->FastDelete = TRUE;
        //lstrcpy(clnt->ServerName,current->Name);
#ifdef FARMAIL_SSL
        res = clnt->Connect( current->Url, current->_User, current->_Pass, current->Port, (current->UsePOP3S?CON_SSL:CON_NORMAL) );
#else
        res = clnt->Connect( current->Url, current->_User, current->_Pass, current->Port);
#endif
        if (res)
          res = clnt->Statistics();
        if ( !res )
        {
          SayError( clnt->GetErrorMessage() );
        }
      }
      if ( !res ) continue;
      if ( clnt && clnt->connected )
      {

        Bar *bar = new Bar(clnt->NumberMail,GetMsg(MsgDelPOP),PROGRESS_LEN);
        for (int i=1;i<=clnt->NumberMail;i++)
        {

          if (!clnt->Delete(i))
          {
            SayError( clnt->GetErrorMessage() );
            break;
          }
          bar->UseBar(i);
        }
        delete bar;
      } else break;
    }
    else
    {
      res = TRUE;
      imap = new IMAP( (Opt.DebugSession?TRUE:FALSE) , Opt.LOGFILE3 , current->timeout*60 );
      if ( imap )
      {
#ifdef FARMAIL_SSL
        res = imap->Connect( current->Url, current->Port, (current->UsePOP3S?CON_SSL:CON_NORMAL) );
#else
        res = imap->Connect( current->Url, current->Port);
#endif
        if ( res )
        {
          SayError( imap->GetErrorMessage() );
        }
        else if (!res)
        {
          res = imap->Login( current->_User, current->_Pass  );
          if ( !res )
          {
            res = imap->Capability();
            if (res)
            {
              SayError( imap->GetErrorMessage() );
            }
          }
          else
          {
            SayError( imap->GetErrorMessage() );
          }
        }
      }
      if (res) continue;
      if ( imap && imap->connected )
      {
        ShortMessage *sm = new ShortMessage( MsgListPOP );
        //imap->Noop();
        if ( !imap->Status( FINBOX, BRACED_MESSAGES ) )
        {
          int line = 0;
          while ( imap->GetRespString( line++ ) )
          {
            if ( imap->GetRespToken(0) && !lstrcmp( imap->RespString2, ASTERISK ) )
            {
              if ( imap->GetRespToken(1) && !FSF.LStricmp( imap->RespString2, STATUS ) )
              {
                if ( imap->GetRespToken(3) )
                {
                  int token = 0;

                  lstrcpy( imap->RespString, imap->RespString2 );
                  FSF.LStrupr( imap->RespString );

                  while ( imap->GetRespToken( token++ ) )
                  {
                    if ( !lstrcmp( imap->RespString2, MESSAGES ) )
                    {
                      if ( imap->GetRespToken( token++ ) )
                        imap->MessageNumber = FSF.atoi(imap->RespString2);
                      else
                        break;
                    }
                  }
                }
              }
            }
          }
        }
        res = imap->Select( FINBOX );
        delete sm;
        if (res) continue;

        Bar *bar = new Bar(imap->MessageNumber,GetMsg(MsgDelPOP),PROGRESS_LEN);
        for (int i=1;i<=imap->MessageNumber;i++)
        {
          /*char uid[41];
          *uid = '?';
          if ( !imap->Fetch( i+1, UID ,0,0,NULL) )
          {
            int line = 0;
            while ( imap->GetRespString( line++) )
            {
              if ( imap->GetRespToken(0) && !lstrcmp( imap->RespString2, ASTERISK ) )
              {
                if ( imap->GetRespToken(1) && i+1==FSF.atoi(imap->RespString2) )
                {
                  if ( imap->GetRespToken(2) && !lstrcmp( imap->RespString2, FETCH ) )
                  {
                    imap->GetRespToken(3);
                    lstrcpy( imap->RespString, imap->RespString2 );
                    FSF.LStrupr( imap->RespString );
                    if ( imap->GetRespToken(0) && !lstrcmp( imap->RespString2, UID ) )
                    {
                      if ( imap->GetRespToken(1) )
                      {
                        lstrcpy(uid, imap->RespString2 );
                      }
                    }
                  }
                }
              }
            }
          }
          if ( imap->DeleteUID( uid ) )
          {
            SayError( imap->GetErrorMessage() );
            break;
          }*/
          //sm = new ShortMessage( MsgDelPOP );
          if ( imap->Delete( i ) )
          {
            SayError( imap->GetErrorMessage() );
            break;
          }
          bar->UseBar(i);
          //delete sm;
        }
        delete bar;
        if ( imap->Expunge() )
        {
          SayError( imap->GetErrorMessage() );
          break;
        }
      } else break;
    }
  }
  return 1;

#ifdef TDEBUG
}
#ifdef __BORLANDC__
__except(1)
#else
catch(...)
#endif
{
 SayException( "FastExpunge" );
 return -1;
}
#endif
}

struct imap4_folders
{
  char name[MAX_PATH];
  int messages, recent, unseen;
};

int FARMail::FastStatus(struct PluginPanelItem *PanelItem, int ItemsNumber, int OpMode)
{
#ifdef TDEBUG
try {
#endif
  BOOL res;
  int silent = (OpMode&OPM_SILENT);

  for (int j=0; j < ItemsNumber; j++)
  {
    if ( clnt ) { clnt->Disconnect(); delete (MailClient*)clnt; clnt = NULL; }
    if ( imap ) { imap->Disconnect(); delete (IMAP*)imap; imap = NULL; }
    current = NULL;

    for (int i=0; i<ServerCount; i++)
    {
      if ( !lstrcmp( PanelItem[j].FindData.cFileName, server[i].Name ) )
      {
        if (*server[i].Url && ((*server[i].User && *server[i].Pass) || !GetUser(server[i]._User,server[i]._Pass)))
        {
          current = &server[i];
          if (PanelItem[j].Flags&PPIF_SELECTED)
            PanelItem[j].Flags^=PPIF_SELECTED;
        }
        break;
      }
    }
    if (!current) continue;
    if ( current->Type == TYPE_POP3 )
    {
      res = FALSE;
      clnt = new MailClient( (Opt.DebugSession?TRUE:FALSE) , Opt.LOGFILE , current->timeout*60 );
      if ( clnt )
      {
        clnt->FastDownload = TRUE;
#ifdef FARMAIL_SSL
        res = clnt->Connect( current->Url, current->_User, current->_Pass, current->Port, (current->UsePOP3S?CON_SSL:CON_NORMAL) );
#else
        res = clnt->Connect( current->Url, current->_User, current->_Pass, current->Port);
#endif
        if (res)
          res = clnt->Statistics();
        if ( !res )
        {
          SayError( clnt->GetErrorMessage() );
        }
      }
      if (!res) continue;
      const char *mes[2];
      mes[0] = current->Name;
      char stat[128];
      FSF.sprintf(stat,GetMsg(MesFastStat_POP3),clnt->NumberMail,clnt->TotalSize);
      mes[1] = stat;
      if ( clnt ) { clnt->Disconnect(); delete (MailClient*)clnt; clnt = NULL; }
      if (_Info.Message(_Info.ModuleNumber,FMSG_MB_OK,NULL,mes,2,0)==-1)
        return FALSE;
    }
    else
    {
      res = TRUE;
      imap = new IMAP( (Opt.DebugSession?TRUE:FALSE) , Opt.LOGFILE3 , current->timeout*60 );
      if ( imap )
      {
#ifdef FARMAIL_SSL
        res = imap->Connect( current->Url, current->Port, (current->UsePOP3S?CON_SSL:CON_NORMAL) );
#else
        res = imap->Connect( current->Url, current->Port);
#endif
        if ( res )
        {
          SayError( imap->GetErrorMessage() );
        }
        else if (!res)
        {
          res = imap->Login( current->_User, current->_Pass  );
          if ( !res )
          {
            res = imap->Capability();
            if (res)
            {
              SayError( imap->GetErrorMessage() );
            }
          }
          else
          {
            SayError( imap->GetErrorMessage() );
          }
        }
      }
      if (res) continue;
      Cache.ClearCachedData();

      if ( imap && imap->connected )
      {
        int stat, i;
        ShortMessage *sm = new ShortMessage( MsgIMAPList );

        int folders = 0, old_folders, select_result;
        int tok = 0;
        const char *mb;
        struct imap4_folders *folder_info=NULL;

        while ( ( mb = get_token( Opt.IMAP_Inbox, tok++ , 1 ) ) != NULL && *mb )
        {
          stat = imap->List( "\"\"", mb );
          if ( !stat )
          {

            int line = 0;

            old_folders = folders;
            select_result = 0;

            while ( imap->GetRespString( line++ ) )
            {
              if ( imap->GetRespToken(0) && !lstrcmp( imap->RespString2, ASTERISK ) ) {
                if ( imap->GetRespToken(1) && !FSF.LStricmp( imap->RespString2, LIST ) ) {
                  if ( imap->GetRespToken(4) ) {
                    folders++;
                    select_result++;
                  }
                }
              }
            }

            struct imap4_folders *new_folder_info = (struct imap4_folders *) z_realloc(folder_info,folders*sizeof(struct imap4_folders));
            if (!new_folder_info)
            {
              z_free(folder_info);
              delete sm;
              return FALSE;
            }
            folder_info = new_folder_info;
            line = 0;
            i = old_folders;

            while ( imap->GetRespString( line++ ) )
            {
              if ( imap->GetRespToken(0) && !lstrcmp( imap->RespString2, ASTERISK ) ) {
                if ( imap->GetRespToken(1) && !FSF.LStricmp( imap->RespString2, LIST ) ) {
                  if ( imap->GetRespToken(4) ) {
                    imap->UnquotString();
                    lstrcpy(folder_info[i].name,imap->RespString2);
                    i++;
                  }
                }
              }
            }

            for ( i=0 ; i<select_result ; i++ )
            {
              if ( !imap->Status( folder_info[i+old_folders].name, "(MESSAGES RECENT UNSEEN)" ) )
              {
                line = 0;
                while ( imap->GetRespString( line++ ) ) {
                  if ( imap->GetRespToken(0) && !lstrcmp( imap->RespString2, ASTERISK ) ) {
                    if ( imap->GetRespToken(1) && !FSF.LStricmp( imap->RespString2, STATUS ) ) {
                      if ( imap->GetRespToken(3) ) {
                        int token = 0;

                        lstrcpy( imap->RespString, imap->RespString2 );
                        FSF.LStrupr( imap->RespString );

                        while ( imap->GetRespToken( token++ ) ) {
                          if ( !lstrcmp( imap->RespString2, MESSAGES ) ) {
                            if ( imap->GetRespToken( token++ ) )
                              FSF.sscanf( imap->RespString2,"%d",&folder_info[i+old_folders].messages);
                            else
                              break;
                          }

                          if ( !lstrcmp( imap->RespString2, RECENT ) ) {
                            if ( imap->GetRespToken( token++ ) )
                              FSF.sscanf( imap->RespString2,"%d",&folder_info[i+old_folders].recent);
                            else
                              break;
                          }

                          if ( !lstrcmp( imap->RespString2, UNSEEN ) ) {
                            if ( imap->GetRespToken( token++ ) )
                              FSF.sscanf( imap->RespString2,"%d",&folder_info[i+old_folders].unseen);
                            else
                              break;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          } else { if ( !silent ) SayError( imap->GetErrorMessage() ); }
        }
        delete sm;
        if ( imap ) { imap->Disconnect(); delete (IMAP*)imap; imap = NULL; }
        {
          int messages=0, recent=0, unseen=0;
          for (i=0; i<folders; i++)
          {
            messages+=folder_info[i].messages;
            recent+=folder_info[i].recent;
            unseen+=folder_info[i].unseen;
          }
          const char *mes[2];
          mes[0] = current->Name;
          char stat[128];
          FSF.sprintf(stat,GetMsg(MesFastStat_IMAP4),messages,recent,unseen);
          mes[1] = stat;
          if (_Info.Message(_Info.ModuleNumber,FMSG_MB_OK,NULL,mes,2,0)==-1)
            return FALSE;
        }
      }
    }
  }
  return 1;

#ifdef TDEBUG
}
#ifdef __BORLANDC__
__except(1)
#else
catch(...)
#endif
{
 SayException( "FastStatus" );
 return -1;
}
#endif
}
