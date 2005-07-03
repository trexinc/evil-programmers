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
#include "progress.hpp"

static const char EERROR[] = "-ERR";

#define TARGET_RESOLUTION 1000

DWORD WINAPI ThreadProc2(LPVOID arg)
{
   MailClient * clnt = (MailClient*)arg;
   long tick = 0;

   while ( !clnt->EndThread ) {

      Sleep( 1000 );
      tick++;

      if ( tick > clnt->Interval ) {

         clnt->Noop();

         tick = 0;
      }

   }
   return 0;
}

MailClient::MailClient( BOOL _log , char *file , int _interval )
{
 log = _log;
 ResponseBufferLen = BUFFER_SIZE*2;
 ResponseBuffer = (char*)z_calloc( 1, ResponseBufferLen );
 if ( !ResponseBuffer ) ResponseBufferLen = 0;
 if ( log && file && *file ) fplog = CreateFile( file, GENERIC_WRITE,  FILE_SHARE_READ, NULL, OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL, NULL );
 else       fplog = INVALID_HANDLE_VALUE;
 if ( fplog != INVALID_HANDLE_VALUE ) {
    SetFilePointer( fplog, 0, 0, FILE_END );
 }
 MessageLens=NULL;
 MessageNums=NULL;
 MessageUidls=NULL;
 NumberMail = 0;
 TotalSize  = 0;
 DownloadedSize = 0;
 connected = 0;
 AddLog("Starting MailClient..\n");
 lstrcpy( ErrMessage , ::GetMsg(MesErrorBreak) );
 hTransferSemaphore = CreateSemaphore( NULL, 1, 1, NULL ); //"POP3SessionSemaphore"
 FastDownload = FALSE;
 FastDelete = FALSE;
 *ServerName = '\0';

 EndThread = 0;
 bkThread = INVALID_HANDLE_VALUE;

 if ( _interval ) {

    DWORD ID;

    Interval = _interval;

    bkThread=CreateThread(NULL,0,ThreadProc2,(void*)this,0,&ID);
    if(!bkThread) bkThread=INVALID_HANDLE_VALUE;
 }
}





MailClient::~MailClient()
{
  if(ResponseBuffer) z_free(ResponseBuffer);
  ResponseBuffer=NULL;
  ResponseBufferLen=0;
  AddLog("Closing MailClient..\n");
  if(fplog!=INVALID_HANDLE_VALUE) CloseHandle(fplog);
  fplog=INVALID_HANDLE_VALUE;
  if(MessageLens) z_free(MessageLens);
  if(MessageNums) z_free(MessageNums);
  if(MessageUidls)
  {
    for(int i=0;i<NumberMail;i++)
      if(MessageUidls[i]) z_free(MessageUidls[i]);
    z_free(MessageUidls);
  }

  if(Interval&&bkThread!=INVALID_HANDLE_VALUE)
  {
    EndThread=1;
    WaitForSingleObject(bkThread,INFINITE);
    CloseHandle(bkThread);
  }
  CloseHandle(hTransferSemaphore);
}


const char * MailClient::GetMsg()
{
 char *i;
 if ( ResponseBuffer ) {
    if ( (i = strchr( ResponseBuffer, '\n' ) ) != NULL )
       return i+1;
    else
       return ResponseBuffer;
 }
 else
     return NULLSTR;
}


int MailClient::AddLog( const char *s )
{
 if ( log && fplog != INVALID_HANDLE_VALUE ) {
    SYSTEMTIME tm;
    GetLocalTime( &tm );

    char *z = (char*)z_calloc( 1, lstrlen(s) + 30 );
    if ( z ) {
       DWORD res;

       FSF.sprintf( z, "%02d:%02d:%02d %s", (int)tm.wHour, (int)tm.wMinute, (int)tm.wSecond , s );
       if ( !lstrlen(s) || ( s[ lstrlen(s)-1 ] != '\n' && s[ lstrlen(s)-1 ] != '\r' ))
          lstrcat( z, CRLF );

       DWORD l1 = lstrlen(z);
       WriteFile( fplog, z, l1, &res, NULL );

       z_free(z);
    }

 }
 return 0;
}



BOOL MailClient::IsError( char *buf )
{
 if (FSF.LStrnicmp( buf, EERROR , 4) == 0) return TRUE;
 return FALSE;
}

BOOL MailClient::GetResponseBuffer(char *InitBuf,int initsize,char *endstr,char *progressname,long tsize,long isize)
{
  char buf[BUFFER_SIZE];
  char *start_check; long endlstrlen=lstrlen(endstr),step,threshold;
  int m,n=initsize;
  Progress *p=NULL;


  step=tsize/8;
  if(step<BUFFER_SIZE) step=BUFFER_SIZE;
  else step=(step/BUFFER_SIZE+1)*BUFFER_SIZE;
  threshold=((tsize+step)/BUFFER_SIZE+1)*BUFFER_SIZE;

  if(!ResponseBuffer)
  {
    ResponseBuffer=(char*)z_calloc(1,ResponseBufferLen);
    if(!ResponseBuffer)
    {
      lstrcpy(ErrMessage,::GetMsg(MesNoMem));
      return FALSE;
    }
  }

  memset(ResponseBuffer,0,ResponseBufferLen);
  memcpy(ResponseBuffer,InitBuf,initsize);

  if(progressname&&*progressname)
  {
    p=new Progress(tsize,progressname);
    p->UseProgress(initsize,initsize+isize);
  }
  start_check=ResponseBuffer;
  while(!strstr(start_check,endstr))
  {
    memset(buf,0,BUFFER_SIZE);
    m=PopServer.Receive(buf,sizeof(buf),Opt.Timeout*1000);
    if(m==SOCKET_ERROR)
    {
      lstrcpy(ErrMessage,::GetMsg(MesErrWinsock));
      if(p) delete p;
      return FALSE;
    }
    for(int i=0;i<m;i++)
    {
      if(!buf[i]) buf[i]=' '; //FIXME
    }
    if((n+m)>=ResponseBufferLen)
    {
      if(ResponseBufferLen<threshold) ResponseBufferLen=threshold;
      else ResponseBufferLen+=step;
      ResponseBuffer=(char*)z_realloc(ResponseBuffer,ResponseBufferLen); //FIXME: memory leak, if realloc return NULL
      if(!ResponseBuffer)
      {
        ResponseBufferLen=0;
        lstrcpy(ErrMessage,::GetMsg(MesNoMem));
        if(p) delete p;
        return FALSE;
      }
      memset(ResponseBuffer+n,0,ResponseBufferLen-n);
    }
    memcpy(ResponseBuffer+n,buf,m);
    if((n-endlstrlen+1)>0) start_check=ResponseBuffer+n-endlstrlen+1;
    n+=m;
    if(p) p->UseProgress(n,n+isize);
  }
  DownloadedSize+=n;
  if(p) delete p;
  return TRUE;
}


int MailClient::GetMsgSize( int num )
{
 int i;

 for ( i=0 ; i<NumberMail; i++ )
    if ( MessageNums[i] == num )
       return MessageLens[i];

 return 1;
}


BOOL MailClient::AddMessage( int n, int num, int len )
{
 if ( n > NumberMail || !MessageLens ) {
    lstrcpy( ErrMessage, ::GetMsg(MesNoMem) );
    return FALSE;
 }
 MessageLens[n-1] = len;
 MessageNums[n-1] = num;
 return TRUE;
}

//Uidl must be call after this function.
BOOL MailClient::CorrectList(void)
{
  int *ML,*MN;
  int nm=0;

  for (int i=0;i<NumberMail;i++)
    if (MessageLens[i]!=0) nm++;

  ML=(int *)z_calloc(nm,sizeof(int));
  MN=(int *)z_calloc(nm,sizeof(int));
  if ((ML && MN)||!nm)
  {
    for (int i=0,j=0;i<NumberMail && nm;i++)
    {
      if (MessageLens[i]!=0)
      {
        ML[j]=MessageLens[i];
        MN[j]=MessageNums[i];
        j++;
      }
    }
    z_free(MessageLens);
    z_free(MessageNums);
    MessageNums=MN;
    MessageLens=ML;
    NumberMail=nm;
    return TRUE;
  }
  z_free(ML);
  z_free(MN);
  lstrcpy( ErrMessage, ::GetMsg(MesNoMem) );
  return FALSE;
}


BOOL MailClient::CheckResponse(int ResponseType)
{
   char  buf [BUFFER_SIZE*2];
   int n,num,len;

   for (int i=0;i<BUFFER_SIZE*2;i++)
      buf[i]='\0';

   n = PopServer.Receive(buf, BUFFER_SIZE, Opt.Timeout*1000);
   while ( n>=0 && !strstr( buf, CRLF ) ) {
      char buf2[BUFFER_SIZE];
      for (int i=0;i<BUFFER_SIZE;i++)
         buf2[i]='\0';
      int n2  = PopServer.Receive(buf2, sizeof(buf2), Opt.Timeout*1000);
      if ( n2 == SOCKET_ERROR ) {
         lstrcpy( ErrMessage, ::GetMsg(MesErrWinsock) );
         return FALSE;
      }else {
         lstrcat( buf, buf2 );
         n=lstrlen(buf);
      }
   }

   if ( n == SOCKET_ERROR ) {
       lstrcpy( ErrMessage, ::GetMsg(MesErrWinsock) );
       return FALSE;
   }

   switch (ResponseType)
   {
      case CONNECTION_CHECK:
         AddLog( buf );
         if ( IsError( buf ) )
         {
            lstrcpy( ErrMessage , ::GetMsg(MesErrConnection) );
            return FALSE;
         }
         break;

      case USER_CHECK:
         AddLog( buf );
         if ( IsError( buf ) )
         {
            lstrcpy( ErrMessage , ::GetMsg(MesErrUser) );
            return FALSE;
         }
         break;
      case PASSWORD_CHECK:
         AddLog( buf );
         if ( IsError( buf ) )
         {
            lstrcpy( ErrMessage , ::GetMsg(MesErrPass) );
            return FALSE;
         }
         break;
      case QUIT_CHECK:
         AddLog( buf );
         if ( IsError( buf ) )
         {
            lstrcpy( ErrMessage , ::GetMsg(MesErrQuit) );
            return FALSE;
         }
         break;
      case DELETE_CHECK:
         AddLog( buf );
         if ( IsError( buf ) )
         {
            lstrcpy( ErrMessage , ::GetMsg(MesErrDel) );
            return FALSE;
         }
         break;
      case RSET_CHECK:
         AddLog( buf );
         if ( IsError( buf ) )
         {
            lstrcpy( ErrMessage , ::GetMsg(MesErrReset) );
            return FALSE;
         }
         break;
      case STAT_CHECK:
         AddLog( buf );
         if ( IsError( buf ) )
         {
            lstrcpy( ErrMessage , ::GetMsg(MesErrStat) );
            return FALSE;
         }
         else
         {
            BOOL EmailNumber = TRUE;
            NumberMail = 0;
            if(MessageLens) { z_free(MessageLens); MessageLens=NULL; }
            if(MessageNums) { z_free(MessageNums); MessageNums=NULL; }
            if(MessageUidls)
            {
              for(int i=0;i<NumberMail;i++)
                if(MessageUidls[i]) z_free(MessageUidls[i]);
              z_free(MessageUidls);
              MessageUidls=NULL;
            }
            for (char *p = buf; *p != '\0'; p++)
            {
               if (*p == '\t' || *p == ' ')
               {
                  if(EmailNumber == TRUE)
                  {
                     NumberMail = FSF.atoi(p);
                     if ( NumberMail ) {
                        MessageLens=(int *)z_calloc(NumberMail,sizeof(int));
                        MessageNums=(int *)z_calloc(NumberMail,sizeof(int));
                        MessageUidls=(char **)z_calloc(NumberMail,sizeof(char *));
                        if(!MessageLens||!MessageNums||!MessageUidls) //FIXME: free resourses
                        {
                          lstrcpy( ErrMessage, ::GetMsg(MesNoMem) );
                          return FALSE;
                        }
                     }
                     EmailNumber = FALSE;
                  }
                  else
                  {
                     TotalSize = FSF.atoi(p);
                     return TRUE;
                  }
               }
            }
         }
         break;
      case NOOP_CHECK:
         AddLog( buf );
         if ( IsError( buf ) )
         {
            lstrcpy( ErrMessage , &buf[5] );
            return FALSE;
         }
         break;

      case LIST_CHECK:
         if ( IsError( buf ) )
         {
            AddLog( buf );
            lstrcpy( ErrMessage , ::GetMsg(MesErrList) );
            return FALSE;
         }
         else
         {
            char *p;
            int nmes = 1;

            if ( !GetResponseBuffer( buf, n , "\r\n.\r\n" , NULL, 0 ) )
               return FALSE;
            AddLog( ResponseBuffer );
            p = strstr( ResponseBuffer, CRLF );
            if ( !p ) {
               lstrcpy( ErrMessage, ::GetMsg(MesErrData) );
               return FALSE;
            }
            p+=2;
            while ( *p != '.' ) {
               num = FSF.atoi(p);
               while ( isdigit ( *p ) ) p++;
               while ( !isdigit( *p ) ) p++;
               len = FSF.atoi(p);
               while ( *p != '\n' && *p != '.' ) p++;
               if ( *p == '\n' ) p++;

               if ( !AddMessage( nmes++, num, len ) ) return FALSE;
            }
         }
         break;
      case RETR_CHECK:
         if ( IsError( buf ) )
         {
            AddLog( buf );
            lstrcpy( ErrMessage , ::GetMsg(MesErrRetr) );
            return FALSE;
         }
         else
         {
            if (FastDownload)
            {
              //_Size = 0;
              //FSF.sscanf(buf,"+OK %d",&_Size);
              if ( !GetResponseBuffer( buf, n , "\r\n.\r\n" , _Name, TotalSize, DownloadedSize ) )
                return FALSE;
            }
            else
              if ( !GetResponseBuffer( buf, n , "\r\n.\r\n" , _Name, _Size ) )
                return FALSE;
            AddLog( ResponseBuffer );
         }
         break;
      case TOP_CHECK:
         if ( IsError( buf ) )
         {
            AddLog( buf );
            lstrcpy( ErrMessage , ::GetMsg(MesErrRetr) );
            return FALSE;
         }
         else
         {
            if ( !GetResponseBuffer( buf, n , "\r\n.\r\n" , NULL, 0 ) )
               return FALSE;
            AddLog( ResponseBuffer );
         }
         break;
      case UIDL_CHECK:
        if(IsError(buf))
        {
          AddLog(buf);
          lstrcpy(ErrMessage,::GetMsg(MesErrRetr));
          return FALSE;
        }
        else
        {
          if(!GetResponseBuffer(buf,n,"\r\n.\r\n",NULL,0)) return FALSE;
          AddLog(ResponseBuffer);
          if(MessageUidls)
          {
            char *p=strstr(ResponseBuffer,CRLF),*q;
            if(!p)
            {
              lstrcpy(ErrMessage,::GetMsg(MesErrData));
              return FALSE;
            }
            p+=2;
            while(*p!='.')
            {
              num=FSF.atoi(p);
              while(isdigit(*p)) p++;
              while(*p==' ') p++;
              q=p;
              while(*q!='\n'&&*q!='\r') q++;
              if(q-p)
              {
                for(int i=0;i<NumberMail;i++)
                  if(MessageNums[i]==num)
                  {
                    if(MessageUidls[i]) z_free(MessageUidls[i]);
                    MessageUidls[i]=(char *)z_calloc(sizeof(char),q-p+1);
                    if(MessageUidls[i])
                    {
                      memcpy(MessageUidls[i],p,q-p);
                      MessageUidls[i][q-p]=0;
                    }
                    break;
                  }
              }
              p=q;
              while(*p=='\n'||*p=='\r') p++;
            }
          }
        }
        break;
   }
   return TRUE;
}





#ifdef FARMAIL_SSL
BOOL MailClient::Connect( char * Host, char * User, char * Password, int port, int type )
#else
BOOL MailClient::Connect( char * Host, char * User, char * Password, int port)
#endif
{
   char  buf [BUFFER_SIZE];
   char  buf2 [BUFFER_SIZE];

   WaitForSingleObject( hTransferSemaphore, INFINITE );

   ShortMessage *sm = new ShortMessage( MsgConnectPOP );

#ifdef FARMAIL_SSL
   if (PopServer.Connect( Host, port , Opt.Timeout*1000, type ) )
#else
   if (PopServer.Connect( Host, port , Opt.Timeout*1000) )
#endif
   {
      lstrcpy( ErrMessage , ::GetMsg(MesErrConnection) );
      delete sm;
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return FALSE;
   }
   else
   {
      if( CheckResponse(CONNECTION_CHECK)==FALSE) {
         delete sm;
         ReleaseSemaphore( hTransferSemaphore, 1, NULL );
         return FALSE;
      }

      FSF.sprintf (buf, "USER %s\r\n", User);
      AddLog( buf );
      if ( PopServer.Send( buf, lstrlen (buf) , Opt.Timeout*1000 ) == SOCKET_ERROR ) {
         delete sm;
         ReleaseSemaphore( hTransferSemaphore, 1, NULL );
         return FALSE;
      }
      if(CheckResponse(USER_CHECK)==FALSE) {
         delete sm;
         ReleaseSemaphore( hTransferSemaphore, 1, NULL );
         return FALSE;
      }

      FSF.sprintf (buf, "PASS %s\r\n", Password);
      FSF.sprintf (buf2, "PASS *\r\n");
      AddLog( buf2 );
      if ( PopServer.Send( buf, lstrlen (buf) , Opt.Timeout*1000 ) == SOCKET_ERROR ) {
         delete sm;
         ReleaseSemaphore( hTransferSemaphore, 1, NULL );
         return FALSE;
      }
      if (CheckResponse(PASSWORD_CHECK)==FALSE) {
         delete sm;
         ReleaseSemaphore( hTransferSemaphore, 1, NULL );
         return FALSE;
      }
      connected = 1;
      delete sm;
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return TRUE;
   }

}



BOOL MailClient::Delete(int MsgNumber)
{
  char buf[BUFFER_SIZE];

  if (!connected) return FALSE;

  WaitForSingleObject( hTransferSemaphore, INFINITE );
  ShortMessage *sm;

  if (!FastDelete)
    sm = new ShortMessage( MsgDelPOP );

  FSF.sprintf(buf, "DELE %d\r\n",MsgNumber );
  AddLog( buf );
  if ( PopServer.Send(buf, lstrlen(buf), Opt.Timeout*1000 ) == SOCKET_ERROR )
  {
    if (!FastDelete)
      delete sm;
    ReleaseSemaphore( hTransferSemaphore, 1, NULL );
    return FALSE;
  }
  if (CheckResponse(DELETE_CHECK)==FALSE)
  {
    if (!FastDelete)
      delete sm;
    ReleaseSemaphore( hTransferSemaphore, 1, NULL );
    return FALSE;
  }
  if (!FastDelete)
    delete sm;
  ReleaseSemaphore( hTransferSemaphore, 1, NULL );
  return TRUE;
}



BOOL MailClient::Disconnect()
{
   char  buf [BUFFER_SIZE];

   if ( !connected ) return TRUE;

   WaitForSingleObject( hTransferSemaphore, INFINITE );

   ShortMessage *sm = new ShortMessage( MsgQuitPOP );

   FSF.sprintf (buf, "QUIT\r\n");
   AddLog( buf );
   if ( PopServer.Send(buf, lstrlen (buf), Opt.Timeout*1000 ) == SOCKET_ERROR ) {
      delete sm;
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return FALSE;
   }
   if (CheckResponse(QUIT_CHECK)==FALSE) {
      delete sm;
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return FALSE;
   }
   delete sm;
   ReleaseSemaphore( hTransferSemaphore, 1, NULL );
   return TRUE;
}



BOOL MailClient::Noop()
{
   char  buf [BUFFER_SIZE];

   if ( !connected ) return FALSE;

   if ( WaitForSingleObject( hTransferSemaphore, 0 ) == WAIT_TIMEOUT ) return TRUE;

   FSF.sprintf (buf, "NOOP\r\n");
   AddLog( buf );
   if ( PopServer.Send(buf, lstrlen (buf), Opt.Timeout*1000 ) == SOCKET_ERROR ) {
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return FALSE;
   }
   if (CheckResponse(NOOP_CHECK)==FALSE) {
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return FALSE;
   }
   else {
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return TRUE;
   }
}



BOOL MailClient::Reset()
{
   char  buf [BUFFER_SIZE];

   if ( !connected ) return FALSE;

   WaitForSingleObject( hTransferSemaphore, INFINITE );

   ShortMessage *sm = new ShortMessage( MsgResetPOP );

   FSF.sprintf (buf, "RSET\r\n");
   AddLog( buf );
   if ( PopServer.Send(buf, lstrlen (buf), Opt.Timeout*1000 ) == SOCKET_ERROR ) {
      delete sm;
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return FALSE;
   }
   if (CheckResponse(RSET_CHECK)==FALSE) {
      delete sm;
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return FALSE;
   }
   delete sm;
   ReleaseSemaphore( hTransferSemaphore, 1, NULL );
   return TRUE;
}


BOOL MailClient::Retrieve(int  MsgNumber, int OpMode)
{
   char  buf [BUFFER_SIZE];

   if ( !connected ) return FALSE;

   WaitForSingleObject( hTransferSemaphore, INFINITE );

   FSF.sprintf (buf, "RETR %d\r\n",MsgNumber );
   AddLog( buf );
   if ( PopServer.Send(buf, lstrlen (buf), Opt.Timeout*1000 ) == SOCKET_ERROR ) {
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return FALSE;
   }
   _Size = GetMsgSize( MsgNumber );
   if ( OpMode & OPM_SILENT ) {
     *_Name = 0;
   }
   else
   {
     if (FastDownload)
       FSF.sprintf( _Name , ::GetMsg(MesProgressReceiveFast_Title) , ServerName, MsgNumber, NumberMail );
     else
       FSF.sprintf( _Name , ::GetMsg(MesProgressReceive_Title) , MsgNumber );
   }
   if (CheckResponse(RETR_CHECK)==FALSE) {
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return FALSE;
   }
   ReleaseSemaphore( hTransferSemaphore, 1, NULL );
   return TRUE;
}


BOOL MailClient::Statistics()
{
   char  buf [BUFFER_SIZE];

   if ( !connected ) return FALSE;

   WaitForSingleObject( hTransferSemaphore, INFINITE );

   FSF.sprintf (buf, "STAT\r\n");
   AddLog( buf );
   if ( PopServer.Send(buf, lstrlen (buf), Opt.Timeout*1000 ) == SOCKET_ERROR ) {
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return FALSE;
   }
   if (CheckResponse(STAT_CHECK)==FALSE) {
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return FALSE;
   }
   else        {
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return TRUE;
   }
}



char * MailClient::GetErrorMessage()
{
   char * i = ErrMessage;
   while ( *i != '\0' ) {
     if ( *i == '\r' || *i == '\n' ) *i = ' ';
     i++;
   }
   return ErrMessage;
}




BOOL MailClient::List()
{
   char  buf [BUFFER_SIZE];

   if ( !connected ) return FALSE;

   WaitForSingleObject( hTransferSemaphore, INFINITE );

   FSF.sprintf (buf, "LIST\r\n");
   AddLog( buf );
   if ( PopServer.Send(buf, lstrlen (buf), Opt.Timeout*1000 ) == SOCKET_ERROR ) {
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return FALSE;
   }
   if (CheckResponse(LIST_CHECK)==FALSE) {
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return FALSE;
   }
   else {
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return TRUE;
   }

}


BOOL MailClient::Top( int Num , int lines )
{
   char  buf [BUFFER_SIZE];

   if ( !connected ) return FALSE;

   WaitForSingleObject( hTransferSemaphore, INFINITE );

   FSF.sprintf(buf, "TOP %d %d\r\n", Num, lines );
   AddLog( buf );
   if ( PopServer.Send(buf, lstrlen (buf), Opt.Timeout*1000 ) == SOCKET_ERROR ) {
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return FALSE;
   }
   if (CheckResponse(TOP_CHECK)==FALSE) {
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return FALSE;
   }
   else {
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return TRUE;
   }

}

BOOL MailClient::Uidl(void)
{
  char buf[BUFFER_SIZE];
  if(!connected) return FALSE;
  WaitForSingleObject(hTransferSemaphore,INFINITE);
  FSF.sprintf(buf,"UIDL\r\n");
  AddLog(buf);
  if(PopServer.Send(buf,lstrlen(buf),Opt.Timeout*1000)==SOCKET_ERROR)
  {
    ReleaseSemaphore(hTransferSemaphore,1,NULL);
    return FALSE;
  }
  if(CheckResponse(UIDL_CHECK)==FALSE)
  {
    ReleaseSemaphore(hTransferSemaphore,1,NULL);
    return FALSE;
  }
  else
  {
    ReleaseSemaphore(hTransferSemaphore,1,NULL);
    return TRUE;
  }
}
