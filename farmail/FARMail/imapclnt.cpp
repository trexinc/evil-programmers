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

static const char IMAP_OK[] = "OK";
static const char IMAP_NO[] = "NO";
static const char IMAP_BAD[] = "BAD";
const char FETCH            [] = "FETCH";
const char UID              [] = "UID";
const char MESSAGES         [] = "MESSAGES";
const char BRACED_MESSAGES  [] = "(MESSAGES)";
const char STATUS           [] = "STATUS";
const char LIST             [] = "LIST";
const char RECENT           [] = "RECENT";
const char UNSEEN           [] = "UNSEEN";
const char RFC822HEADER     [] = "RFC822.HEADER";
const char RFC822SIZE       [] = "RFC822.SIZE";
//const char [];
//const char [];

#define TARGET_RESOLUTION 1000

enum ERRORS
{
  ERR_NO = 0,
  ERR_RESPONSE_OK  = 0,
  ERR_SOCKETERROR  = 1,
  ERR_NOMEM,
  ERR_RESPONSE_NO  = 100,
  ERR_RESPONSE_BAD = 101,
  ERR_NOCAPABILITY = 102,
};


DWORD WINAPI ThreadProc(LPVOID arg)
{
   IMAP * clnt = (IMAP*)arg;
   long tick = 0;

   while ( !clnt->EndThread ) {

      Sleep( 1000 );
      tick++;

      if ( tick > clnt->Interval ) {

         clnt->Noop2();

         tick = 0;
      }

   }
   return 0;
}


char * IMAP::GetErrorMessage()
{
   char * i = ErrMessage;
   while ( *i != '\0' ) {
     if ( *i == '\r' || *i == '\n' ) *i = ' ';
     i++;
   }
   return ErrMessage;
}



char * IMAP::GetRespString( int num )
{
 int i;
 char *ptr = ResponseBuffer;
 char *str = RespString;

 *RespString = '\0';

 for ( i=0 ; i<num; i++ ) {
    ptr = strstr( ptr , CRLF );
    if ( !ptr ) return NULL;
    ptr+=2;
    if ( ! (*ptr) ) return NULL;
 }

 RespStringPtr = ptr;
 RespStringLen = 0;

 while ( *ptr == 32  || *ptr == 9 )  { ptr++; RespStringLen++; }

 while ( *ptr && *ptr != '\r' && *ptr != '\n' ) {
    *(str++) = *(ptr++);
    RespStringLen++;
 }
 *str = '\0';
 RespStringLen+=2;

 return RespString;
}



char * IMAP::GetRespToken( int num )
{
 int i;
 char *ptr = RespString2;
 char *str = RespString;

 *RespString2 = '\0';

 while ( *str == 32 || *str == 9 ) str++;

 for ( i=0 ; i<num; i++ ) {

    if ( *str == '(' ) {

       while ( *str != ')' && *str ) str++;
       if ( *str ) {
          str++;
          while ( *str == 32 || *str == 9 ) str++;
       } else break;

    } else if (*str == '"') {

       str++;
       while ( *str != '"' && *str ) str++;
       if ( *str ) {
          str++;
          while ( *str == 32 || *str == 9 ) str++;
       } else break;

    } else {
       // skip token
       while ( *str != 32 && *str != 9 && *str != 0 && *str != '\r' && *str != '\n' ) str++;
       if ( *str == 0 || *str == '\r' || *str == '\n' ) return NULL;

       while ( *str == 32 || *str == 9 ) str++;
    }
 }

 if ( *str == '(' ) {
    str++;
    while ( *str != ')' && *str )
       *(ptr++) = *(str++);
 } else if ( *str == '"' ) {
//    str++;
    *(ptr++) = *(str++);
    while ( *str != '"' && *str )
       *(ptr++) = *(str++);
    if ( *str )
       *(ptr++) = *(str++);

 } else while ( *str != 32 && *str != 9 && *str != 0 && *str != '\r' && *str != '\n' ) {
    *(ptr++) = *(str++);
 }
 *ptr = '\0';

 if ( lstrlen(RespString2) || ( *str ) ) return RespString2;

 return NULL;
}



void IMAP::UnquotString()
{
 if ( ! *RespString2 ) return;

 char *str = z_strdup( RespString2 );
 char *ptr = str;

 if ( !str ) return;

 while ( *ptr == '\"' || *ptr == '\'' ) ptr++;

 lstrcpy( RespString2 , ptr );
 z_free( str );

 ptr = RespString2 + lstrlen(RespString2) - 1;

 while ( ptr >= RespString2 && ( *ptr == '\"' || *ptr == '\'' ) ) ptr--;
 *(++ptr) = '\0';
 return;
}


void IMAP::IncreaseTag( void )
{
 TagCounter++;
 FSF.sprintf( Tag , "FML%ld", TagCounter );
}



int IMAP::SendCommand( const char *str )
{
 char buf[BUFFER_SIZE];

 if ( !connected ) return ERR_SOCKETERROR;
 FSF.sprintf ( buf, "%s %s\r\n", Tag, str );
 if ( strncmp( str, "LOGIN", 5) )
    AddLog( buf , 0 );
 else {
    char q[BUFFER_SIZE];
    FSF.sprintf ( q, "%s LOGIN * *\r\n", Tag );
    AddLog( q , 0 );
 }
 if ( Socket.Send( buf, lstrlen (buf) , Opt.Timeout*1000 ) >= 0/*== lstrlen(buf)*/ )
    return 0;
 lstrcpy( ErrMessage, ::GetMsg(MesErrWinsock) );
 return ERR_SOCKETERROR;
}




int IMAP::CheckTag( char *buffer, int len /*, char *tagpos*/ )
{
 char *ptr = buffer, *eptr ;

 while ( ptr < buffer+len && ( eptr = strstr( ptr, CRLF ) ) != NULL ) {

    if ( !FSF.LStrnicmp( ptr, Tag, lstrlen(Tag) ) ) {
       if ( strstr( ptr, CRLF ) )
          return 0;
    }
    if ( *(eptr-1) == '}' ) {

       char *sptr = eptr-1;

       while ( *sptr != '{' && sptr>ptr ) sptr--;
       if ( *sptr == '{' ) {

          int wlen = FSF.atoi( sptr+1 );

          ptr = eptr+2+wlen;
          continue;

       }
    }
    ptr = eptr+2;
 }
 return 0;
}




int IMAP::ReceiveResponse( int tagflag , long _size, long _startsize, const char *_name )
{
  char *ptr=NULL;
  Progress *p=NULL;

  if(!ResponseBufferLen)
  {
    ResponseBufferLen=BUFFER_SIZE;
  }

  if(!ResponseBuffer)
  {
    ResponseBuffer = (char*)z_calloc( 1, ResponseBufferLen );
    if ( !ResponseBuffer )
    {
      ResponseBufferLen = 0;
      lstrcpy ( ErrMessage, ::GetMsg(MesNoMem) );
      return ERR_NOMEM;
    }
  }
  RealBufferLen = 0;
  memset( ResponseBuffer, 0,  ResponseBufferLen );

  if ( _name && *_name )
  {
    p = new Progress( _size , _name );
    p->UseProgress( _startsize );
  }

  long endlstrlen,start_check=0,step,threshold;
  if(!tagflag) endlstrlen=2;
  else endlstrlen=lstrlen(Tag);
  step=_size/8;
  if(step<BUFFER_SIZE) step=BUFFER_SIZE;
  else step=(step/BUFFER_SIZE+1)*BUFFER_SIZE;
  threshold=((_size+step)/BUFFER_SIZE+1)*BUFFER_SIZE;
  if(!step) step=BUFFER_SIZE*16;

  while(1)
  {
    char buf[ BUFFER_SIZE ], *ptr2;

    int m = Socket.Receive( buf, sizeof(buf) , Opt.Timeout*1000 );

    if ( m == SOCKET_ERROR )
    {
      lstrcpy( ErrMessage, ::GetMsg(MesErrWinsock) );
      if ( p ) delete p;
      return ERR_SOCKETERROR;
    }
    if ( RealBufferLen + m >= ResponseBufferLen )
    {
      if(ResponseBufferLen<threshold) ResponseBufferLen=threshold;
      else ResponseBufferLen+=step;
      ResponseBuffer = (char*)z_realloc( ResponseBuffer, ResponseBufferLen );
      if ( !ResponseBuffer )
      {
        ResponseBufferLen = 0;
        lstrcpy( ErrMessage, ::GetMsg(MesNoMem) );
        if ( p ) delete p;
        return ERR_NOMEM;
      }
    }
    memset( ResponseBuffer + RealBufferLen, 0, m+1 );
    memcpy( ResponseBuffer + RealBufferLen, buf, m );
    RealBufferLen += m;

    if ( p ) p->UseProgress( RealBufferLen+_startsize );

    if ( !tagflag ) {
      if ( strstr( ResponseBuffer+start_check , CRLF ) ) break;
    } else
    if ( ( ptr = strstr( ResponseBuffer+start_check , Tag ) ) != NULL ) { // tag found
       if ( ( ( ptr2 = strstr( ptr , IMAP_OK ) ) != NULL ) ||
            ( ( ptr2 = strstr( ptr , IMAP_NO ) ) != NULL ) ||
            ( ( ptr2 = strstr( ptr , IMAP_BAD ) ) != NULL ) ) { // responce found
          if ( strstr( ptr2 , CRLF ) ) { // end of line received

             if ( ptr == ResponseBuffer || *(ptr-1) == '\r' || *(ptr-1) == '\n' ) {
                if ( !CheckTag(ResponseBuffer, ResponseBufferLen /*, ptr*/) )
                   break; // end of transmission
             }
          }
       }
    }
    if((RealBufferLen-endlstrlen+1)>0) start_check=RealBufferLen-endlstrlen+1;
  }
  AddLog( ResponseBuffer , 1 );

  if ( p ) delete p;

  if ( !tagflag )
     return 0;
  else {
     if ( strstr( ptr , IMAP_OK  ) ) return ERR_RESPONSE_OK;
     if ( strstr( ptr , IMAP_NO  ) ) return ERR_RESPONSE_NO;
     if ( strstr( ptr , IMAP_BAD ) ) return ERR_RESPONSE_BAD;
  }

  lstrcpy( ErrMessage, ::GetMsg(MesErrWinsock) );
  return ERR_SOCKETERROR;
}




int IMAP::ReceiveResponse2( int tagflag )
{
 char *ptr = NULL;

 if ( !ResponseBufferLen2 ) {
    ResponseBufferLen2 = BUFFER_SIZE;
 }

 if ( !ResponseBuffer2 ) {
    ResponseBuffer2 = (char*)z_calloc( 1, ResponseBufferLen2 );
    if ( !ResponseBuffer2 ) {
       ResponseBufferLen2 = 0;
       lstrcpy ( ErrMessage, ::GetMsg(MesNoMem) );
       return ERR_NOMEM;
    }
 }
 RealBufferLen2 = 0;
 memset( ResponseBuffer2, 0,  ResponseBufferLen2 );

 while (1) {

    char buf[ BUFFER_SIZE ], *ptr2;

    int m = Socket.Receive( buf, sizeof(buf) , Opt.Timeout*1000 );

    if ( m == SOCKET_ERROR ) {
       lstrcpy( ErrMessage, ::GetMsg(MesErrWinsock) );
       return ERR_SOCKETERROR;
    }
    if ( RealBufferLen2 + m >= ResponseBufferLen2 ) {
       ResponseBufferLen2 += BUFFER_SIZE;
       ResponseBuffer2 = (char*)z_realloc( ResponseBuffer2, ResponseBufferLen2 );
       if ( !ResponseBuffer2 ) {
          ResponseBufferLen2 = 0;
          lstrcpy( ErrMessage, ::GetMsg(MesNoMem) );
          return ERR_NOMEM;
       }
    }
    memset( ResponseBuffer2 + RealBufferLen2, 0, m+1 );
    memcpy( ResponseBuffer2 + RealBufferLen2, buf, m );
    RealBufferLen2 += m;

    if ( !tagflag ) {
      if ( strstr( ResponseBuffer2 , CRLF ) ) break;
    } else
    if ( ( ptr = strstr( ResponseBuffer2 , Tag ) ) != NULL ) { // tag found
       if ( ( ( ptr2 = strstr( ptr , IMAP_OK ) ) != NULL ) ||
            ( ( ptr2 = strstr( ptr , IMAP_NO ) ) != NULL ) ||
            ( ( ptr2 = strstr( ptr , IMAP_BAD ) ) != NULL ) ) { // responce found
          if ( strstr( ptr2 , CRLF ) ) { // end of line received

             if ( ptr == ResponseBuffer2 || *(ptr-1) == '\r' || *(ptr-1) == '\n' ) {
                if ( !CheckTag( ResponseBuffer2, ResponseBufferLen2 /*, ptr*/) )
                   break; // end of transmission
             }
          }
       }
    }
 }
 AddLog( ResponseBuffer , 1 );

 if ( !tagflag )
    return 0;
 else {
    if ( strstr( ptr , IMAP_OK  ) ) return ERR_RESPONSE_OK;
    if ( strstr( ptr , IMAP_NO  ) ) return ERR_RESPONSE_NO;
    if ( strstr( ptr , IMAP_BAD ) ) return ERR_RESPONSE_BAD;
 }

 lstrcpy( ErrMessage, ::GetMsg(MesErrWinsock) );
 return ERR_SOCKETERROR;
}






int IMAP::ExecCommand( const char * str , long _size, long _startsize, const char *_name )
{
 int stat;

 if ( !connected ) return 0;

 WaitForSingleObject( hTransferSemaphore, INFINITE );
 IncreaseTag();
 if ( (stat = SendCommand( str ) ) == 0 ) {
    stat = ReceiveResponse(1, _size, _startsize, _name );
 }
 ReleaseSemaphore( hTransferSemaphore, 1, NULL );

 return stat;
}



int IMAP::ExecCommand2( const char * str )
{
 int stat;

 if ( !connected ) return 0;

 if ( WaitForSingleObject( hTransferSemaphore, 0 ) == WAIT_TIMEOUT ) return 0;

 IncreaseTag();
 if ( (stat = SendCommand( str ) ) == 0 ) {
    stat = ReceiveResponse2(1);
 }
 ReleaseSemaphore( hTransferSemaphore, 1, NULL );

 return stat;
}





#ifdef FARMAIL_SSL
int IMAP::Connect( char * Host, int port, int type )
#else
int IMAP::Connect( char * Host, int port)
#endif
{

   WaitForSingleObject( hTransferSemaphore, INFINITE );

   ShortMessage *sm = new ShortMessage( MsgConnectIMAP );

#ifdef FARMAIL_SSL
   if (Socket.Connect( Host, port , Opt.Timeout*1000, type) )
#else
   if (Socket.Connect( Host, port , Opt.Timeout*1000) )
#endif
   {
      lstrcpy( ErrMessage , ::GetMsg(MesErrIMAP_Connect) );
      delete sm;
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return ERR_SOCKETERROR;
   }
   else
   {
      if( ReceiveResponse(0,0,0,NULL) ) {
         delete sm;
         lstrcpy( ErrMessage , ::GetMsg(MesErrIMAP_Connect) );
         ReleaseSemaphore( hTransferSemaphore, 1, NULL );
         return ERR_SOCKETERROR;
      }

      connected = 1;
      delete sm;
      ReleaseSemaphore( hTransferSemaphore, 1, NULL );
      return 0;
   }

}




int IMAP::Disconnect( void )
{
 const char *command = "LOGOUT";

 int stat = ExecCommand( command , 0 , 0 , NULL );

 switch ( stat ) {
    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , command );
       lstrcat( ErrMessage , ::GetMsg(MesErrIMAP_NO_BAD) );
       break;

 }
 return stat;
}



int IMAP::Noop()
{
 const char *command = "NOOP";

 int stat = ExecCommand( command , 0 , 0 , NULL );
 switch ( stat ) {
       case 0:
       {
          int line = 0;

          while ( GetRespString( line++ ) ) {
             if ( GetRespToken(0)  && !lstrcmp( RespString2 , ASTERISK ) ) {
                if ( GetRespToken(2) && !FSF.LStricmp( RespString2 , "EXISTS" ) ) {
                   if ( GetRespToken(1) ) {
                      MessageNumber  = FSF.atoi( RespString2 );
                      break;
                   }
                }
             }
          }

       }
       break;
    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , command );
       lstrcat( ErrMessage , ::GetMsg(MesErrIMAP_NO_BAD) );
       break;

 }
 return stat;
}



int IMAP::Noop2()
{
 return ExecCommand2( "NOOP" );
}


int IMAP::Capability()
{
 const char *command = "CAPABILITY";
 int stat = ExecCommand( command , 0 , 0 , NULL );
 switch ( stat ) {
    case 0:
       {
          int line = 0;
          stat = ERR_NOCAPABILITY;
          while ( stat && GetRespString( line++ ) ) {
             if ( GetRespToken(0)  && !lstrcmp( RespString2 , ASTERISK ) ) {
                if ( GetRespToken(1) && !lstrcmp( RespString2 , command ) ) {
                   int token = 2;
                   while ( stat && GetRespToken(token++) ) {
                      if ( !FSF.LStrnicmp( RespString2, "IMAP4", 5 ) ) { stat = 0; break; }
                   }
                }
             }
          }
          if ( stat )
             lstrcpy( ErrMessage , ::GetMsg(MesErrIMAP_Cap) );

       }
       break;
    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , command );
       lstrcat( ErrMessage , ::GetMsg(MesErrIMAP_NO_BAD) );
       break;

 }
 return stat;
}



int IMAP::Login( char *user , char *pass )
{
 char buf[BUFFER_SIZE];
 const char *command = "LOGIN";
 int stat;

 FSF.sprintf( buf, "%s %s %s", command, user, pass );
 stat = ExecCommand( buf , 0 , 0 , NULL );
 switch ( stat ) {
    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , ::GetMsg(MesErrIMAPAuth) );
       break;

 }
 return stat;
}


int IMAP::List( const char *reference, const char *mailbox )
{
 char buf[BUFFER_SIZE];

 FSF.sprintf( buf, "%s %s %s", LIST, reference, mailbox );

 int stat = ExecCommand( buf , 0 , 0 , NULL );
 switch ( stat ) {
    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , LIST );
       lstrcat( ErrMessage , ::GetMsg(MesErrIMAP_NO_BAD) );
       break;

 }
 return stat;
}


int IMAP::Select( const char *mailbox )
{
 char buf[BUFFER_SIZE];
 const char *command = "SELECT";

 FSF.sprintf( buf, "%s \"%s\"", command, mailbox );

 int stat = ExecCommand( buf , 0 , 0 , NULL );
 switch ( stat ) {
       case 0:
       {
          int line = 0;
          MessageNumber = 0;

          while ( GetRespString( line++ ) ) {
             if ( GetRespToken(0)  && !lstrcmp( RespString2 , ASTERISK ) ) {
                if ( GetRespToken(2) && !FSF.LStricmp( RespString2 , "EXISTS" ) ) {
                   if ( GetRespToken(1) ) {
                      MessageNumber  = FSF.atoi( RespString2 );
                      break;
                   }
                }
             }
          }

       }
       break;
    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , command );
       lstrcat( ErrMessage , ::GetMsg(MesErrIMAP_NO_BAD) );
       break;

 }
 return stat;
}



int IMAP::Close( void )
{
 const char *command = "CLOSE";

 int stat = ExecCommand( command , 0 , 0 , NULL );

 switch ( stat ) {
    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , command );
       lstrcat( ErrMessage , ::GetMsg(MesErrIMAP_NO_BAD) );
       break;

 }
 return stat;
}



int IMAP::Fetch( int num, const char *type , long _size, long _startsize, const char *_name )
{
 char buf[BUFFER_SIZE];

 FSF.sprintf( buf, "%s %d %s", FETCH, num, type );

 int stat = ExecCommand( buf , _size, _startsize, _name );

 switch ( stat ) {

    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , FETCH );
       lstrcat( ErrMessage , ::GetMsg(MesErrIMAP_NO_BAD) );
       break;

 }
 return stat;
}


int IMAP::Status( const char *box, const char *type )
{
 char buf[BUFFER_SIZE];

 FSF.sprintf( buf, "%s \"%s\" %s", STATUS, box, type );

 int stat = ExecCommand( buf , 0 , 0 , NULL );

 switch ( stat ) {

    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , STATUS );
       lstrcat( ErrMessage , ::GetMsg(MesErrIMAP_NO_BAD) );
       break;

 }
 return stat;
}



int IMAP::Delete( int num )
{
 char buf[BUFFER_SIZE];
 const char *command = "STORE";

 FSF.sprintf( buf, "%s %d +FLAGS.SILENT (\\Deleted)", command, num );

 int stat = ExecCommand( buf , 0 , 0 , NULL );

 switch ( stat ) {

    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , command );
       lstrcat( ErrMessage , ::GetMsg(MesErrIMAP_NO_BAD) );
       break;

 }
 return stat;
}



int IMAP::DeleteUID( char * num )
{
 char buf[BUFFER_SIZE];
 const char *command = "UID STORE";

 FSF.sprintf( buf, "%s %s +FLAGS.SILENT (\\Deleted)", command, num );

 int stat = ExecCommand( buf , 0 , 0 , NULL );

 switch ( stat ) {

    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , command );
       lstrcat( ErrMessage , ::GetMsg(MesErrIMAP_NO_BAD) );
       break;

 }
 return stat;
}



int IMAP::Expunge( void )
{
 const char *cmd = "EXPUNGE";

 int stat = ExecCommand( cmd , 0, 0, NULL );

 switch ( stat ) {

    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , cmd );
       lstrcat( ErrMessage , ::GetMsg(MesErrIMAP_NO_BAD) );
       break;

 }
 if( !stat ) {
    int line = 0;
    while ( GetRespString( line++ ) ) {
       if ( GetRespToken(0)  && !lstrcmp( RespString2 , ASTERISK ) ) {
          if ( GetRespToken(2) && !FSF.LStricmp( RespString2 , cmd ) ) {
                MessageNumber --;
          }
       }

    }
 }
 return stat;
}





int IMAP::AddLog( const char *s , int type )
{
 char *z = (char*)z_calloc( 1, lstrlen(s) + 40 );
 if ( !z ) return 1;
 DWORD res;

 if ( log && fplog != INVALID_HANDLE_VALUE ) {
    if ( type == 0 ) {
       SYSTEMTIME tm;
       GetLocalTime( &tm );

       FSF.sprintf( z, " > Sending (%02d:%02d:%02d): %s", (int)tm.wHour, (int)tm.wMinute, (int)tm.wSecond , s );
       if ( !lstrlen(s) || s[ lstrlen(s)-1 ] != '\n' )
          lstrcat( z, CRLF );

       WriteFile( fplog, z, lstrlen(z), &res, NULL );

    } else if ( type == 1 ) {
       FSF.sprintf( z, "\n%s" , s);
       if ( !lstrlen(s) || s[ lstrlen(s)-1 ] != '\n' )
          lstrcat( z, CRLF );
       lstrcat( z, CRLF );
       WriteFile( fplog, z, lstrlen(z), &res, NULL );
    } else if ( type == 2 ) {
       lstrcpy( z, s );
       WriteFile( fplog, z, lstrlen(z), &res, NULL );
    }
 }
 z_free(z);
 return 0;
}




IMAP::IMAP( BOOL _log , char *file , int _interval )
{
 TagCounter = 0;

 log = _log;

 MessageBuffer = NULL;
 MessageLen = 0;

 ResponseBufferLen = 0;
 ResponseBuffer = NULL;

 ResponseBufferLen2 = 0;
 ResponseBuffer2 = NULL;

 Partial = 0;

 if ( log && file && *file ) fplog = CreateFile( file, GENERIC_WRITE,  FILE_SHARE_READ , NULL, OPEN_ALWAYS , 0, NULL );
 else       fplog = INVALID_HANDLE_VALUE;
 if ( fplog != INVALID_HANDLE_VALUE ) {
    SetFilePointer( fplog, 0, 0, FILE_END );
 }

 MessageNumber = 0;
 connected = 0;

 AddLog("--- Starting IMAP4 session..\n", 2);

 hTransferSemaphore = CreateSemaphore( NULL, 1, 1, NULL ); //"IMAP4SessionSemaphore"
 bkThread = INVALID_HANDLE_VALUE;
 EndThread = 0;

 if ( _interval ) {

    DWORD ID;
    Interval = _interval;

    bkThread=CreateThread(NULL,0,ThreadProc,(void*)this,0,&ID);
    if(!bkThread) bkThread=INVALID_HANDLE_VALUE;
 }
 lstrcpy( ErrMessage , ::GetMsg(MesErrorBreak) );
}





IMAP::~IMAP()
{
 if ( ResponseBuffer ) z_free( ResponseBuffer );
 if ( ResponseBuffer2 ) z_free( ResponseBuffer2 );
 ResponseBuffer = NULL;
 ResponseBufferLen = 0;
 ResponseBuffer2 = NULL;
 ResponseBufferLen2 = 0;

 if ( MessageBuffer ) z_free( MessageBuffer );
 MessageLen = 0;

 AddLog("--- Closing IMAP4 session..\n", 2);
 if ( fplog != INVALID_HANDLE_VALUE ) CloseHandle(fplog);
 fplog = INVALID_HANDLE_VALUE;

 if ( Interval  && bkThread != INVALID_HANDLE_VALUE ) {
    EndThread = 1;
    WaitForSingleObject( bkThread, INFINITE );
    CloseHandle(bkThread);
 }
 CloseHandle( hTransferSemaphore );

}



const char * IMAP::GetMsg()
{
 if ( ResponseBuffer )
 {
   return ResponseBuffer;
 }
 else
   return NULLSTR;
}


char * IMAP::GetMsgText( int num , int _size, const char * _name )
{
 int stat;
 if ( MessageBuffer ) z_free(MessageBuffer);
 MessageBuffer = NULL;
 MessageLen = 0;

 if ( ( Fetch( num, RFC822HEADER , 0 , 0 , NULL ) ) == 0 ) {
    int line = 0;

    while ( GetRespString(line++) ) {

       if ( GetRespToken(0)  && !lstrcmp( RespString2 , ASTERISK ) ) {
          if ( GetRespToken(1) && num == FSF.atoi(RespString2) ) {
             if ( GetRespToken(2) && !FSF.LStricmp( RespString2 , FETCH ) ) {
                if ( RespString[ lstrlen(RespString)-1 ] == '}' ) {
                   char *ptr = RespString + lstrlen(RespString)-1;
                   while ( *ptr != '{' && ptr>=RespString ) ptr--;
                   if ( ptr>RespString ) {
                      ptr++;
                      MessageLen = FSF.atoi(ptr);
                      MessageBuffer = (char*)z_calloc( 1, MessageLen+1 );
                      if ( MessageBuffer )memcpy( MessageBuffer, RespStringPtr + RespStringLen , MessageLen );
                   }
                }
                break;
             }
          }
       }

    }

 } else return NULL;

 if ( ( stat = Fetch( num, "RFC822.TEXT" , _size, 0 , _name ) ) == 0 || RealBufferLen ) {
    int line = 0;

    while ( GetRespString(line++) ) {

       if ( GetRespToken(0)  && !lstrcmp( RespString2 , ASTERISK ) ) {
          if ( GetRespToken(1) && num == FSF.atoi(RespString2) ) {
             if ( GetRespToken(2) && !FSF.LStricmp( RespString2 , FETCH ) ) {
                if ( RespString[ lstrlen(RespString)-1 ] == '}' ) {
                   char *ptr = RespString + lstrlen(RespString)-1;
                   while ( *ptr != '{' && ptr>=RespString ) ptr--;
                   if ( ptr>RespString ) {
                      ptr++;
                      int addMessageLen = FSF.atoi(ptr);
                      if ( stat ) {
                         addMessageLen = RealBufferLen- (RespStringPtr-ResponseBuffer+RespStringLen);
                         if ( addMessageLen >= FSF.atoi(ptr) ) {
                            addMessageLen = FSF.atoi(ptr);
                            stat = 0;
                         }
                      }
                      ReceivedMessageLen = addMessageLen;

                      int oldMessageLen = MessageLen;
                      MessageLen += addMessageLen;
                      MessageBuffer = (char*)z_realloc( MessageBuffer, MessageLen+1 );
                      if ( MessageBuffer ) {
                         memset( MessageBuffer+oldMessageLen, 0 , addMessageLen+1 );
                         memcpy( MessageBuffer+oldMessageLen, RespStringPtr + RespStringLen , addMessageLen );
                         if ( stat ) {
                            Partial = 1;

                         } else {
                            Partial = 0;

                         }
                      }
                   }
                }
                break;
             }
          }
       }

    }

 } else return NULL;

 return MessageBuffer;
}



char * IMAP::GetMsgTop( int num , int top , const char *_name )
{
 char bbuf[40];

 if ( MessageBuffer ) z_free(MessageBuffer);
 MessageBuffer = NULL;
 MessageLen = 0;

 if ( ( Fetch( num, RFC822HEADER , 0, 0, NULL ) ) == 0 ) {
    int line = 0;

    while ( GetRespString(line++) ) {

       if ( GetRespToken(0)  && !lstrcmp( RespString2 , ASTERISK ) ) {
          if ( GetRespToken(1) && num == FSF.atoi(RespString2) ) {
             if ( GetRespToken(2) && !FSF.LStricmp( RespString2 , FETCH ) ) {
                if ( RespString[ lstrlen(RespString)-1 ] == '}' ) {
                   char *ptr = RespString + lstrlen(RespString)-1;
                   while ( *ptr != '{' && ptr>=RespString ) ptr--;
                   if ( ptr>RespString ) {
                      ptr++;
                      MessageLen = FSF.atoi(ptr);
                      MessageBuffer = (char*)z_calloc( 1, MessageLen+1 );
                      if ( MessageBuffer )memcpy( MessageBuffer, RespStringPtr + RespStringLen , MessageLen );
                   }
                }
                break;
             }
          }
       }

    }

 } else return NULL;

 FSF.sprintf( bbuf, "BODY[TEXT]<0.%d>", top );

 if ( ( Fetch( num, bbuf , top, 0, _name ) ) == 0 ) {
    int line = 0;

    while ( GetRespString(line++) ) {

       if ( GetRespToken(0)  && !lstrcmp( RespString2 , ASTERISK ) ) {
          if ( GetRespToken(1) && num == FSF.atoi(RespString2) ) {
             if ( GetRespToken(2) && !FSF.LStricmp( RespString2 , FETCH ) ) {
                if ( RespString[ lstrlen(RespString)-1 ] == '}' ) {
                   char *ptr = RespString + lstrlen(RespString)-1;
                   while ( *ptr != '{' && ptr>=RespString ) ptr--;
                   if ( ptr>RespString ) {
                      ptr++;
                      int addMessageLen = FSF.atoi(ptr);
                      int oldMessageLen = MessageLen;
                      MessageLen += addMessageLen;
                      MessageBuffer = (char*)z_realloc( MessageBuffer, MessageLen+1 );
                      if ( MessageBuffer ) {
                         memset( MessageBuffer+oldMessageLen, 0 , addMessageLen+1 );
                         memcpy( MessageBuffer+oldMessageLen, RespStringPtr + RespStringLen , addMessageLen );
                      }
                   }
                }
                break;
             }
          }
       }

    }

 } else return NULL;

 return MessageBuffer;
}



char * IMAP::GetMsgTail( int num , long startpos , long endpos , const char * _name)
{
 int stat;
 char bbuf[40];

 if ( MessageBuffer ) z_free(MessageBuffer);
 MessageBuffer = NULL;
 MessageLen = 0;

 FSF.sprintf( bbuf, "BODY[TEXT]<%ld.%ld>", startpos, endpos );

 if ( ( stat = Fetch( num, bbuf , endpos, startpos, _name ) ) == 0 || RealBufferLen ) {
    int line = 0;

    while ( GetRespString(line++) ) {

       if ( GetRespToken(0)  && !lstrcmp( RespString2 , ASTERISK ) ) {
          if ( GetRespToken(1) && num == FSF.atoi(RespString2) ) {
             if ( GetRespToken(2) && !FSF.LStricmp( RespString2 , FETCH ) ) {
                if ( RespString[ lstrlen(RespString)-1 ] == '}' ) {
                   char *ptr = RespString + lstrlen(RespString)-1;
                   while ( *ptr != '{' && ptr>=RespString ) ptr--;
                   if ( ptr>RespString ) {
                      ptr++;
                      MessageLen = FSF.atoi(ptr);
                      if ( stat ) {
                         MessageLen = RealBufferLen- (RespStringPtr + RespStringLen-ResponseBuffer);
                         if ( MessageLen >= FSF.atoi(ptr) )
                            MessageLen = FSF.atoi(ptr);
                      }
                      ReceivedMessageLen = MessageLen;

                      MessageBuffer = (char*)z_calloc( 1, MessageLen+1 );
                      if ( MessageBuffer ) {
                         memcpy( MessageBuffer, RespStringPtr + RespStringLen , MessageLen );
                         if ( stat ) {
                            Partial = 1;

                         } else {
                            Partial = 0;

                         }
                      }
                   }
                }
                break;
             }
          }
       }

    }

 } else return NULL;

 return MessageBuffer;
}





void IMAP::FreeMsgText( void )
{
 if ( MessageBuffer ) z_free( MessageBuffer );
 MessageBuffer = NULL;
 MessageLen = 0;
}



int IMAP::Create(char *dir)
{
 char buf[BUFFER_SIZE];
 const char *command = "CREATE";

 FSF.sprintf( buf, "%s \"%s\"", command, dir );

 int stat = ExecCommand( buf , 0 , 0 , NULL );

 switch ( stat ) {

    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , command );
       lstrcat( ErrMessage , ::GetMsg(MesErrIMAP_NO_BAD) );
       break;

 }
 return stat;
}


int IMAP::DeleteBox( char *dir )
{
 char buf[BUFFER_SIZE];
 const char *command = "DELETE";

 FSF.sprintf( buf, "%s \"%s\"", command, dir );

 int stat = ExecCommand( buf , 0 , 0 , NULL );

 switch ( stat ) {

    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , command );
       lstrcat( ErrMessage , ::GetMsg(MesErrIMAP_NO_BAD) );
       break;

 }
 return stat;
}


int IMAP::Rename( char *olddir , char *newdir )
{
 char buf[BUFFER_SIZE];
 const char *command = "RENAME";

 FSF.sprintf( buf, "%s \"%s\" \"%s\"", command, olddir, newdir );

 int stat = ExecCommand( buf , 0 , 0 , NULL );

 switch ( stat ) {

    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , command );
       lstrcat( ErrMessage , ::GetMsg(MesErrIMAP_NO_BAD) );
       break;

 }
 return stat;
}



int IMAP::Copy( int num , char *dir )
{
 char buf[BUFFER_SIZE];
 const char *command = "COPY";

 FSF.sprintf( buf, "%s %d \"%s\"", command, num, dir );

 int stat = ExecCommand( buf , 0 , 0 , NULL );

 switch ( stat ) {

    case ERR_RESPONSE_NO:
    case ERR_RESPONSE_BAD:
       lstrcpy( ErrMessage , command );
       lstrcat( ErrMessage , ::GetMsg(MesErrIMAP_NO_BAD) );
       break;

 }
 return stat;
}
