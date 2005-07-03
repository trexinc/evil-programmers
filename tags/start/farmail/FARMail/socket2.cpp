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

// 10 ms poll interval
#define STEP_POLL 10000


FMSocket::FMSocket()
{
  WSAData wsaData;
  s = INVALID_SOCKET;

#ifdef FARMAIL_SSL
  ctx = NULL;
  ssl = NULL;
  sbio = NULL;
#endif

  errstate = 0;
  _StopSocket = 0;

#ifdef FARMAIL_SSL
  SSL_library_init();

  ctx = SSL_CTX_new(SSLv23_client_method());
#endif

  if (WSAStartup(MAKEWORD(1, 1), &wsaData) == 0)
  {
    s = socket( AF_INET, SOCK_STREAM, 0 );
    if ( s != INVALID_SOCKET ) {

        u_long arg = 1;
        if ( ioctlsocket ( s, FIONBIO, &arg ) )
        {
          closesocket(s) ;
          s = INVALID_SOCKET;
        }

    }
  }
  else
    errstate = 1;

}


bool FMSocket::ShutdownConnection()
{
  char acReadBuffer[BUFFER_SIZE];

  if ( errstate == 1 ) return false;

#ifdef FARMAIL_SSL
  if (sbio)
  {
    BIO_ssl_shutdown(sbio);
    BIO_free_all(sbio);
    sbio=NULL;
    return true;
  }
#endif

   if ( s == INVALID_SOCKET ) return false;

   if (shutdown(s, SD_SEND ) == SOCKET_ERROR)
      return false;

   while (1) {
      int nNewBytes = Receive( acReadBuffer, BUFFER_SIZE , 1000 );
      if (nNewBytes == SOCKET_ERROR) {
         break;
      }
      else if (nNewBytes != 0) {
         // smth read
      }
      else {
         // Okay, we're done!
         break;
      }
   }

   // Close the socket.
   if (closesocket(s) == SOCKET_ERROR) {
      return false;
   }
   s = INVALID_SOCKET;

   return true;
}




FMSocket::~FMSocket()
{
 ShutdownConnection();
 if ( errstate != 1 ) WSACleanup();
}



int FMSocket::StopSocket()
{
 _StopSocket = 1;
 return 0;
}



int FMSocket::RecreateSocket()
{

#ifdef FARMAIL_SSL
  if (sbio)
  {
    BIO_ssl_shutdown(sbio);
    BIO_free_all(sbio);
    sbio=NULL;
    return 0;
  }
#endif

 shutdown(s, 2 );
 closesocket(s);

 s = INVALID_SOCKET;
 return 0;
}


int FMSocket::Receive( char * buf, int size , long timeout )
{
  if ( errstate == 1 ) return -1;

#ifdef FARMAIL_SSL
  if (sbio)
  {
    int n = BIO_read(sbio, buf, size);
    if (n<=0)
      return SOCKET_ERROR;

    return n;
  }
#endif

  if ( s == INVALID_SOCKET ) return -1;

  if ( _StopSocket )
  {
    RecreateSocket();
    return SOCKET_ERROR;
  }

  {
    int err;
    fd_set fset;
    struct timeval TV = { 0, STEP_POLL };
    DWORD tick = GetTickCount();

    FD_ZERO( &fset );
    FD_SET( s, &fset );

    while ( ( err = select( 0, &fset, NULL, NULL, &TV ) ) == 0 )
    {

      if ( _StopSocket || abs( GetTickCount() - tick )  > timeout )
      {
        RecreateSocket();
        break;
      }

      FD_ZERO( &fset );
      FD_SET( s, &fset );

    }
    if ( err > 0 )
    {

      // workaround of winsock bug (q177346)
      int lenb;
      do
      {
        lenb = recv (s, buf, size, 0 );
      } while ( lenb == SOCKET_ERROR && WSAGetLastError()==WSAEWOULDBLOCK );

      if ( _StopSocket )
      {
        RecreateSocket();
        lenb = SOCKET_ERROR;
      }
      return lenb;
    }
  }

 return SOCKET_ERROR;
}


int FMSocket::Send( const char * buf, int size , long timeout )
{
  if ( errstate == 1 )
    return -1;

#ifdef FARMAIL_SSL
  if (sbio)
  {
    int n = BIO_write(sbio, buf, size);

    if (n<=0)
      return SOCKET_ERROR;

    return n;
  }
#endif

  if ( s == INVALID_SOCKET )
    return -1;

  int n = 0;

  if ( _StopSocket )
  {
    RecreateSocket();
    return SOCKET_ERROR;
  }


  do
  {
    int err;
    fd_set fset;
    struct timeval TV = { 0, STEP_POLL };
    DWORD tick = GetTickCount();

    FD_ZERO( &fset );
    FD_SET( s, &fset );

    while ( ( err = select( 0, NULL, &fset, NULL, &TV ) ) == 0 )
    {

      if ( _StopSocket || abs( GetTickCount() - tick )  > timeout )
      {
        RecreateSocket();
        n = SOCKET_ERROR;
        break;
      }

      FD_ZERO( &fset );
      FD_SET( s, &fset );

    }

    if ( err > 0 )
    {

      // the same workaround
      int nn;
      do
      {
        nn = send (s, buf+n, size-n, 0 );
      } while( nn==SOCKET_ERROR && WSAGetLastError()==WSAEWOULDBLOCK );

      if ( nn == SOCKET_ERROR )
        return nn;

      n += nn;
    }
    else
    {
      n=SOCKET_ERROR;
      break;
    }

  } while ( n<size );

  return n;
}


u_long FMSocket::LookupAddress(char* pcHost)
{
    u_long nRemoteAddr = inet_addr(pcHost);
    if (nRemoteAddr == INADDR_NONE) {
        // pcHost isn't a dotted IP, so resolve it through DNS
        hostent* pHE = gethostbyname(pcHost);
        if (pHE == 0) {
            return INADDR_NONE;
        }
        nRemoteAddr = *((u_long*)pHE->h_addr_list[0]);
    }
    return nRemoteAddr;
}


#ifdef FARMAIL_SSL
int FMSocket::Connect(char *host, int port, long timeout, int type)
#else
int FMSocket::Connect(char *host, int port, long timeout)
#endif
{
  int n;
  if ( errstate == 1 ) return -1;

#ifdef FARMAIL_SSL
  if (type==CON_SSL)
  {
    if (!ctx) return -1;

    SSL_CTX_set_timeout(ctx, timeout/1000); //timeout should be in seconds

    sbio = BIO_new_ssl_connect(ctx);

    if (!sbio)
      return -1;

    BIO_get_ssl(sbio, &ssl);

    if (!ssl)
    {
      BIO_free_all(sbio);
      sbio=NULL;
      return -1;
    }

    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    BIO_set_conn_int_port(sbio,&port);
    if (BIO_set_conn_hostname(sbio, host)<=0)
    {
      BIO_free_all(sbio);
      sbio=NULL;
      return -1;
    }
    if (BIO_do_connect(sbio) <= 0)
    {
      BIO_free_all(sbio);
      sbio=NULL;
      return -1;
    }
    if (BIO_do_handshake(sbio) <= 0)
    {
      BIO_free_all(sbio);
      sbio=NULL;
      return -1;
    }

    return 0;
  }
#endif

  if ( s == INVALID_SOCKET ) return -1;

  if ( _StopSocket )
  {
    RecreateSocket();
    return SOCKET_ERROR;
  }

  u_long nRemoteAddress = LookupAddress(host);
  struct sockaddr_in sa;

  if (nRemoteAddress == INADDR_NONE) return SOCKET_ERROR;

  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = nRemoteAddress;
  sa.sin_port = htons( (u_short) port);

  n = connect( s, (sockaddr*)&sa, sizeof( sockaddr_in ) );

  if ( n == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK )
  {
    int err;
    fd_set fset;
    struct timeval TV = { 0, STEP_POLL };  // 10 ms step
    DWORD tick = GetTickCount();

    FD_ZERO( &fset );
    FD_SET( s, &fset );

    while ( ( err = select( 0, NULL, &fset, NULL, &TV ) ) == 0 )
    {
      if ( _StopSocket || abs( GetTickCount() - tick )  > timeout )
      {
        RecreateSocket();
        break;
      }

      FD_ZERO( &fset );
      FD_SET( s, &fset );
    }

    if ( err > 0 ) return 0;
  }

  return n;
}
