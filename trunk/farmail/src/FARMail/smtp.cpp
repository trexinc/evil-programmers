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

static const char AUTH_LOGIN[] = "LOGIN";
static const char AUTH_PLAIN[] = "PLAIN";

BOOL SMTP::AuthLogin( char *user, char *pwd )
{
  char  buf   [BUFFER_SIZE];

  FSF.sprintf ( buf, "AUTH %s\r\n", AUTH_LOGIN );

  AddLog( buf );
  if ( wsocket.Send( buf, lstrlen (buf) , Opt.Timeout*1000 ) < 0 /*!= lstrlen (buf)*/ ) {
      return FALSE;
  }

  if(CheckResponse(SMTP_AUTH_CHECK)==FALSE) {
      return FALSE;
  }

  EncodeBase64 ( buf, user , lstrlen(user) );
  lstrcat( buf, CRLF );
  AddLog( buf );
  if ( wsocket.Send( buf, lstrlen (buf) , Opt.Timeout*1000 ) < 0 /*!= lstrlen (buf)*/ ) {
      return FALSE;
  }

  if(CheckResponse(SMTP_LOGIN_CHECK1)==FALSE) {
      return FALSE;
  }

  EncodeBase64 ( buf, pwd , lstrlen(pwd) );
  lstrcat( buf, CRLF );
  AddLog( ASTERISK );
  if ( wsocket.Send( buf, lstrlen (buf) , Opt.Timeout*1000 ) < 0 /*!= lstrlen (buf)*/ ) {
      return FALSE;
  }

  if(CheckResponse(SMTP_LOGIN_CHECK2)==FALSE) {
      return FALSE;
  }
  return TRUE;
}

BOOL SMTP::AuthPlain( char *user, char *pwd )
{
 char  buf   [BUFFER_SIZE];
 char  buf2  [BUFFER_SIZE];
 char  pln   [BUFFER_SIZE];
 const char *form = "AUTH %s %s\r\n";

 memset( pln, 0, BUFFER_SIZE );
 lstrcpy( pln, user );
 lstrcpy( pln+lstrlen(user)+1, user );
 lstrcpy( pln+lstrlen(user)*2+2, pwd );

 EncodeBase64 ( buf2, pln , lstrlen(user)*2+lstrlen(pwd)+2 );

 FSF.sprintf ( buf, form, AUTH_PLAIN , ASTERISK );
 AddLog( buf );

 FSF.sprintf ( buf, form, AUTH_PLAIN , buf2 );

 if ( wsocket.Send( buf, lstrlen (buf) , Opt.Timeout*1000 ) < 0 /*!= lstrlen (buf)*/ ) {
    return FALSE;
 }

 if(CheckResponse(SMTP_LOGIN_CHECK2)==FALSE) {
    return FALSE;
 }

 return TRUE;
}



int SMTP::SetAuthAlgorithms( char *buf)
{
 char *ptr = buf;

 FSF.LStrupr(buf);
 do {
    ptr = strstr( ptr, "AUTH " );
    if ( !ptr ) break;
    if ( *(ptr-2) == '0' && *(ptr-3) == '5' && *(ptr-4) == '2' &&
         ( ptr-4 == buf || *(ptr-5) == '\n' ) ) {
       break;
    }

 } while(1);

 if ( ptr ) {

    lstrcpyn( Auth, ptr+5, 1001 );

    ptr = Auth;

    while ( *ptr ) {
       if ( *ptr == '\r' || *ptr == '\n' ) { *ptr=0 ; break; }
       ptr++;
    }

 }
 return 0;
}



SMTP::SMTP( BOOL _log , char *file )
{
 log = _log;
 if ( log && file && *file ) fplog = CreateFile( file, GENERIC_WRITE,  FILE_SHARE_READ , NULL, OPEN_ALWAYS , 0, NULL );
 else       fplog = INVALID_HANDLE_VALUE;
 if ( fplog != INVALID_HANDLE_VALUE ) {
    SetFilePointer( fplog, 0, 0, FILE_END );
 }
 connected = 0;
 *Auth=0;
 AddLog("Starting SMTP session..\n");
 lstrcpy( ErrMessage , ::GetMsg(MesErrorBreak) );
 *message_id = 0;
}


SMTP::~SMTP()
{
 AddLog("Closing SMTP session..\n");
 if ( fplog != INVALID_HANDLE_VALUE ) CloseHandle(fplog);
 fplog = INVALID_HANDLE_VALUE;
}


const char * SMTP::GetSMTPError( int code )
{
 switch ( code ) {

    case 421:
       return ::GetMsg(MesSMTP_421);
    case 450:
       return ::GetMsg(MesSMTP_450);
    case 451:
       return ::GetMsg(MesSMTP_451);
    case 452:
       return ::GetMsg(MesSMTP_452);
    case 500:
       return ::GetMsg(MesSMTP_500);
    case 501:
       return ::GetMsg(MesSMTP_501);
    case 503:
       return ::GetMsg(MesSMTP_503);
    case 504:
       return ::GetMsg(MesSMTP_504);
    case 550:
       return ::GetMsg(MesSMTP_550);
    case 551:
       return ::GetMsg(MesSMTP_551);
    case 552:
       return ::GetMsg(MesSMTP_552);
    case 553:
       return ::GetMsg(MesSMTP_553);
    case 554:
       return ::GetMsg(MesSMTP_554);
    default:
       return ::GetMsg(MesSMTP_DefaultError);
 }
}


BOOL SMTP::IsError( char *buf , int response )
{
 int err = FSF.atoi(buf);

 switch ( response ) {

    case SMTP_CONNECTION_CHECK:
       //220 - service ready.. 421 - err
       if ( err == 220 ) return FALSE;
       lstrcpy( ErrMessage , GetSMTPError( err ) );
       return TRUE;

    case SMTP_HELLO_CHECK:
       // 250 - ok 500, 501, 504, 421 - err
       if ( err == 250 ) return FALSE;
       lstrcpy( ErrMessage , GetSMTPError( err ) );
       return TRUE;

    case SMTP_MAIL_CHECK:
       // 250 - ok 500, 501, 421 - err 552,451,452-fail
       if ( err == 250 ) return FALSE;
       lstrcpy( ErrMessage , GetSMTPError( err ) );
       return TRUE;
    case SMTP_RCPT_CHECK:
       // 250,251 - ok
       // 500, 501, 503, 421 - err
       // 550, 551, 552, 553, 450, 451,452-fail
       if ( err == 250 || err == 251 ) return FALSE;
       lstrcpy( ErrMessage , GetSMTPError( err ) );
       return TRUE;
    case SMTP_DATA_CHECK:
       // 354 - intermed.
       // 451, 554 - fail
       // 500, 501, 503, 421 - err
       if ( err == 354 ) return FALSE;
       lstrcpy( ErrMessage , GetSMTPError( err ) );
       return TRUE;
    case SMTP_DATA2_CHECK:
       // 250 - ok
       // 552, 554, 451, 452 - fail
       if ( err == 250 ) return FALSE;
       lstrcpy( ErrMessage , GetSMTPError( err ) );
       return TRUE;
    case SMTP_QUIT_CHECK:
       // 221 - ok
       // 500 - err
       if ( err == 221 || err==0 ) return FALSE;
       lstrcpy( ErrMessage , GetSMTPError( err ) );
       return TRUE;

    case SMTP_AUTH_CHECK:
       if ( err == 334 ) return FALSE;
       lstrcpy( ErrMessage , ::GetMsg(MesSMTP_AUTHError) );
       return TRUE;

    case SMTP_LOGIN_CHECK1:
       return FALSE;

    case SMTP_LOGIN_CHECK2:
       if ( err == 235 ) return FALSE;
       lstrcpy( ErrMessage , ::GetMsg(MesSMTP_AUTHError) );
       return TRUE;

 }
 return FALSE; // no error
}



int SMTP::AddLog( const char *s )
{
 if ( log && fplog != INVALID_HANDLE_VALUE ) {
    SYSTEMTIME tm;
    GetLocalTime( &tm );

    char *z = (char*)z_calloc( 1, lstrlen(s) + 30 );
    if ( z ) {
       DWORD res;

       FSF.sprintf( z, "%02d:%02d:%02d %s", (int)tm.wHour, (int)tm.wMinute, (int)tm.wSecond , s );
       if ( !lstrlen(s) || s[ lstrlen(s)-1 ] != '\n' )
          lstrcat( z, CRLF );

       WriteFile( fplog, z, lstrlen(z), &res, NULL );

       z_free(z);
    }
 }
 return 0;
}



BOOL SMTP::CheckResponse(int ResponseType)
{
   char  *buf = (char*)z_calloc( 1, 32000 );
   char buf2[ BUFFER_SIZE ];
   int n;
   BOOL stat;

   if ( !buf ) return FALSE;

   do {

      memset( buf2, 0, sizeof(buf2) );

      n = wsocket.Receive(buf2, sizeof(buf2)-1, Opt.Timeout*1000 );

      if ( n>0 ) {

         char *ptr = buf, *ptr_old = buf;

         lstrcat ( buf , buf2 );

         do {
            if ( strstr( ptr, CRLF ) ) {
               ptr_old = ptr;
               ptr = strstr( ptr, CRLF )+2;
            } else break;
         } while(1);

         if ( ptr > buf ) {

            if ( * ( ptr_old+3 )  ==  ' ' ) break;

         }

      }  else break;
   } while(1);

   if ( n == SOCKET_ERROR ) {
       z_free(buf);
       if (ResponseType==SMTP_QUIT_CHECK)
         return TRUE;
       lstrcpy( ErrMessage, ::GetMsg(MesErrWinsock) );
       return FALSE;
   }

   switch (ResponseType) {

      case SMTP_CONNECTION_CHECK:
      case SMTP_HELLO_CHECK:
      case SMTP_MAIL_CHECK:
      case SMTP_RCPT_CHECK:
      case SMTP_DATA_CHECK:
      case SMTP_QUIT_CHECK:

      case SMTP_AUTH_CHECK:
      case SMTP_LOGIN_CHECK1:
      case SMTP_LOGIN_CHECK2:

         AddLog( buf );
         stat = ! ( IsError( buf , ResponseType ) );
         z_free(buf);
         return stat;

      case SMTP_EHLO_CHECK:
         AddLog( buf );
         if ( FSF.atoi(buf) != 250 ) {
            lstrcpy( ErrMessage , GetSMTPError( FSF.atoi(buf) ) );
            return FALSE;
         }
         SetAuthAlgorithms(buf);
         return TRUE;

     case SMTP_DATA2_CHECK:
         AddLog( buf );
         stat = ! ( IsError( buf , ResponseType ) );
         if (stat)
         {
           FSF.sscanf(buf,Opt.MessageIDTemplate,message_id);
         }
         z_free(buf);
         return stat;
   }
   return TRUE;
}



#ifdef FARMAIL_SSL
BOOL SMTP::Connect( char * Host, int port, int type )
#else
BOOL SMTP::Connect( char * Host, int port)
#endif
{
   char  buf   [BUFFER_SIZE];
   char  local [80];

   ShortMessage *sm = new ShortMessage( MsgConnectSMTP );

   gethostname( local, 80 );

#ifdef FARMAIL_SSL
   if (wsocket.Connect( Host, port , Opt.Timeout*1000, type ) )
#else
   if (wsocket.Connect( Host, port , Opt.Timeout*1000) )
#endif
   {
      lstrcpy( ErrMessage , ::GetMsg(MesSMTP_ConnectionError) );
      delete sm;
      return FALSE;
   }
   else
   {
      if( CheckResponse(SMTP_CONNECTION_CHECK)==FALSE) {
         delete sm;
         return FALSE;
      }

      FSF.sprintf (buf, "HELO %s\r\n", local );
      AddLog( buf );
      if ( wsocket.Send( buf, lstrlen (buf) , Opt.Timeout*1000 ) < 0 /*!= lstrlen (buf)*/ ) {
         delete sm;
         return FALSE;
      }

      if(CheckResponse(SMTP_HELLO_CHECK)==FALSE) {
         delete sm;
         return FALSE;
      }

      connected = 1;
      delete sm;
      return TRUE;
   }

}


#ifdef FARMAIL_SSL
BOOL SMTP::Connect( char * Host, int port , char *user , char *pwd, int type )
#else
BOOL SMTP::Connect( char * Host, int port , char *user , char *pwd)
#endif
{
   char  buf   [BUFFER_SIZE];
   char  local [80];

   ShortMessage *sm = new ShortMessage( MsgConnectSMTP );

   gethostname( local, 80 );

#ifdef FARMAIL_SSL
   if (wsocket.Connect( Host, port , Opt.Timeout*1000, type ) )
#else
   if (wsocket.Connect( Host, port , Opt.Timeout*1000) )
#endif
   {
      lstrcpy( ErrMessage , ::GetMsg(MesSMTP_ConnectionError) );
      delete sm;
      return FALSE;
   }
   else
   {
      if( CheckResponse(SMTP_CONNECTION_CHECK)==FALSE) {
         delete sm;
         return FALSE;
      }

      FSF.sprintf (buf, "EHLO %s\r\n", local );
      AddLog( buf );
      if ( wsocket.Send( buf, lstrlen (buf) , Opt.Timeout*1000 ) < 0 /*!= lstrlen (buf)*/ ) {
         delete sm;
         return FALSE;
      }

      if(CheckResponse(SMTP_EHLO_CHECK)==FALSE) {
         delete sm;
         return FALSE;
      }

      if ( strstr( Auth, AUTH_LOGIN ) ) {
         if ( AuthLogin( user, pwd ) == FALSE )
            if ( AuthPlain( user, pwd ) == FALSE ) {
               delete sm;
               return FALSE;
            }
      }
      else {
         if ( AuthPlain( user, pwd ) == FALSE )
            if ( AuthLogin( user, pwd ) == FALSE ) {
               delete sm;
               return FALSE;
            }
      }

      connected = 1;
      delete sm;
      return TRUE;
   }

}


BOOL SMTP::Disconnect()
{
   char  buf [BUFFER_SIZE];

   if ( !connected ) return TRUE;

   ShortMessage *sm = new ShortMessage( MsgQuitSMTP );

   FSF.sprintf (buf, "QUIT\r\n");
   AddLog( buf );
   if ( wsocket.Send(buf, lstrlen (buf), Opt.Timeout*1000 ) < 0 /*!= lstrlen (buf)*/ ) {
      delete sm;
      return FALSE;
   }
   if (CheckResponse(SMTP_QUIT_CHECK)==FALSE) {
      delete sm;
      return FALSE;
   }
   delete sm;
   return TRUE;
}



BOOL SMTP::Mail( char *from )
{
   char  buf [BUFFER_SIZE];

   if ( !connected ) return TRUE;

   ShortMessage *sm = new ShortMessage( MsgMailSMTP );

   FSF.sprintf (buf, "MAIL FROM:<%s>\r\n", from );
   AddLog( buf );
   if ( wsocket.Send(buf, lstrlen (buf), Opt.Timeout*1000 ) < 0 /*!= lstrlen (buf)*/ ) {
      delete sm;
      return FALSE;
   }
   if (CheckResponse(SMTP_MAIL_CHECK)==FALSE) {
      delete sm;
      return FALSE;
   }
   delete sm;
   return TRUE;
}



BOOL SMTP::Receipt( char *rcp )
{
   char  buf [BUFFER_SIZE];

   if ( !connected ) return TRUE;

   ShortMessage *sm = new ShortMessage( MsgRcptSMTP );

   FSF.sprintf (buf, "RCPT TO:<%s>\r\n", rcp );
   AddLog( buf );
   if ( wsocket.Send(buf, lstrlen (buf), Opt.Timeout*1000 ) < 0 /*!= lstrlen (buf)*/ ) {
      delete sm;
      return FALSE;
   }
   if (CheckResponse(SMTP_RCPT_CHECK)==FALSE) {
      delete sm;
      return FALSE;
   }

   delete sm;
   return TRUE;
}


BOOL SMTP::Data()
{
   char  buf [BUFFER_SIZE];

   if ( !connected ) return TRUE;

   FSF.sprintf (buf, "DATA\r\n" );
   AddLog( buf );
   if ( wsocket.Send(buf, lstrlen (buf), Opt.Timeout*1000 ) < 0 /*!= lstrlen (buf)*/ ) {
      return FALSE;
   }
   if (CheckResponse(SMTP_DATA_CHECK)==FALSE)
      return FALSE;
   else
      return TRUE;
}



BOOL SMTP::DataLine( const char *line , int check )
{
   if ( !connected ) return TRUE;

   AddLog( line );
   if ( ( wsocket.Send( line, lstrlen (line), Opt.Timeout*1000 ) ) < 0 /*!= lstrlen (line)*/ ) {
      return FALSE;
   }

   if (check && CheckResponse(SMTP_DATA2_CHECK)==FALSE )
      return FALSE;
   else
      return TRUE;
}


char * SMTP::GetErrorMessage()
{
   char * i = ErrMessage;
   while ( *i != '\0' ) {
     if ( *i == '\r' || *i == '\n' ) *i = ' ';
     i++;
   }
   return ErrMessage;
}
