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
#include "farkeys.hpp"
#include "version.hpp"
#include "crt_file.hpp"
#include "progress.hpp"

#define MAXLINE 1000
#define DETECTSIZE 2048

static const char CHARSETSTR[] = "charset=%s";
const char SUBJECT          [] = "Subject:";
const char FROM             [] = "From:";
const char _DATE            [] = "Date:";
const char CONTENTTYPE      [] = "Content-Type:";

static char *GetRecipientEMail( char * adr );

const char *Mon[] = {"Jan" , "Feb" , "Mar" , "Apr" , "May" , "Jun" ,
                     "Jul" , "Aug" , "Sep" , "Oct" , "Nov" , "Dec"  };
const char *Day[] = {"Sun", "Mon", "Tue", "Wed" , "Thu", "Fri", "Sat" };

char FARMail::TranscodeChar8( char ch, const char * charset, const char * encode )
{
 if ( !FSF.LStricmp( charset, encode ) ) return ch;

 if ( !FSF.LStricmp( charset, USASCII ) )
    charset = NULLSTR;
 if ( !FSF.LStricmp( encode, USASCII ) )
    encode = NULLSTR;

 // 1. charset->OEM
 if ( lstrcmp( charset, NULLSTR ) ) {

    int i = FindCharset( charset , &CharsetTable );
    if ( i>=0 ) {
       ch = CharsetTable[i].DecodeTable[ ch ];
    } else return ch;
 }

 // 2. OEM -> encode
 if ( lstrcmp( encode, NULLSTR ) ) {

    int i = FindCharset( encode , &CharsetTable );
    if ( i>=0 ) {
       ch = CharsetTable[i].EncodeTable[ ch ];
    }
 }
 return ch;
}


int FARMail::DecodeStr8(char *str,const char *charset)
{
  return ExtDecodeStr8(str,charset,&CharsetTable);
}


int FARMail::DecodeChar8( char *c , const char *charset )
{
  return ExtDecodeChar8(c,charset,&CharsetTable);
}

int FARMail::EncodeStr8( char *str, const char *encode )
{

 if (!encode) return 0;

 if ( !FSF.LStricmp( encode, NULLSTR ) ) return 0;

 if ( !FSF.LStricmp( encode, USASCII ) ) return 0;

 // OEM -> encode
 int t = FindCharset( encode , &CharsetTable );
 if ( t>=0 )
 {
   for (int i=0; i<lstrlen( str ); i++ )
     str[i] = CharsetTable[t].EncodeTable[ str[i] ];
 }

 return 0;

}

int FARMail::TranscodeStr8Ext( char *str, int size, const char * charset, const char * encode )
{

 if ( !charset ) return 0;
 if ( !encode ) return 0;

 if ( !FSF.LStricmp( charset, encode ) ) return 0;

 if ( !FSF.LStricmp( charset, USASCII ) )
    charset = NULLSTR;
 if ( !FSF.LStricmp( encode, USASCII ) )
    encode = NULLSTR;

 int i;
 int t;
 // 1. charset->OEM
 if ( lstrcmp( charset, NULLSTR ) ) {

    t = FindCharset( charset , &CharsetTable );
    if ( t>= 0)
    {
      for (i =0; i<size; i++)
        str[i] = CharsetTable[t].DecodeTable[ str[i] ];
    }
    else return 0;
 }

 // 2. OEM -> encode
 if ( lstrcmp( encode, NULLSTR ) ) {

    t = FindCharset( encode , &CharsetTable );
    if ( t>= 0)
    {
      for (i=0; i<size; i++)
        str[i] = CharsetTable[t].EncodeTable[ str[i] ];
    }
 }
 return 0;
}


char *GetDateInSMTP( void )
{
  static char _date[80];
  char tz[40];
  int _bias;
  int stat;
  SYSTEMTIME st;
  TIME_ZONE_INFORMATION tzi;

  GetLocalTime( &st );
  stat = GetTimeZoneInformation( &tzi );

  if ( stat == TIME_ZONE_ID_STANDARD )
    tzi.Bias += tzi.StandardBias;
  else if ( stat == TIME_ZONE_ID_DAYLIGHT )
    tzi.Bias += tzi.DaylightBias;

  if ( tzi.Bias )
    _bias = -tzi.Bias/abs(tzi.Bias);
  else
    _bias = 1;

  tzi.Bias = abs( tzi.Bias);
  FSF.sprintf( tz, "%+03d%02d", _bias*(tzi.Bias/60) , tzi.Bias%60 );

  FSF.sprintf( _date , "%s, %d %s %d %02d:%02d:%02d %s",
                    Day[st.wDayOfWeek],
                    st.wDay, Mon[st.wMonth-1], st.wYear,
                    st.wHour, st.wMinute, st.wSecond, tz );

  return _date;
}




long FARMail::GetEditorFileSize( void )
{
  int line = 0;
  long tsize = 0;
  struct EditorGetString egs;
  egs.StringNumber = line;

  while ( _Info.EditorControl( ECTL_GETSTRING , &egs ) )
  {
    tsize += egs.StringLength;
    egs.StringNumber = ++line;
  }
  return tsize;
}


void FARMail::ReadReset( long pos )
{
  rd_curline = pos;
  rd_curpos = 0;
  rd_eof = 0;

  rd_egs.StringNumber = rd_curline;

  if (!_Info.EditorControl( ECTL_GETSTRING , &rd_egs ) )
    rd_eof = 1;
}


int FARMail::ReadChar( void )
{
  if ( rd_eof ) return -1;

  if ( rd_curpos < rd_egs.StringLength )
  {
    return rd_egs.StringText[rd_curpos++];
  }
  else if ( rd_curpos == rd_egs.StringLength )
  {
    rd_curpos++;
    return '\r';
  }
  else if ( rd_curpos == rd_egs.StringLength + 1 )
  {
    rd_curpos++;
    return '\n';
  }
  else
  {
    rd_curpos = 0;
    rd_egs.StringNumber = ++rd_curline;
    if ( _Info.EditorControl( ECTL_GETSTRING , &rd_egs ) )
    {
      if ( rd_egs.StringLength )
      {
        return rd_egs.StringText[rd_curpos++];
      }
      else
      {
        rd_curpos++;
        return '\r';
      }
    }
    else
    {
      rd_eof = 1;
      return -1;
    }
  }
}


int FARMail::ReadBlock( char *ptr, int size )
{
  int i;

  for (i=0; i<size; i++)
  {
    int c = ReadChar();
    if ( c>=0 )
    {
      ptr[i] = (char)c;
    }
    else
    {
      return i;
    }
  }
  return i;
}


void FARMail::SendBody(HANDLE fp,MAILSEND *parm,SMTP *smtp,const char *filename,long tsize)
{
  char buf[MAXLINE+10], buf2[MAXLINE+10];
  long curr = 0;

  if ( !tsize ) tsize = GetEditorFileSize();

  if ( !filename ) filename = GetEditorFileName();
  Progress *p = new Progress( tsize, filename );

  if(parm->infopos)
    if(fp!=INVALID_HANDLE_VALUE)
      SetFilePointer(fp,parm->infopos,NULL,FILE_BEGIN);

  // text or html "as is"
  if ( parm->how == 1 || parm->how == 4 || parm->how == 6)
  {
     // send as is;

     if ( parm->multipart == MULTI_SENDTEXT)
     {

       char bbuf[1000], bbuf2[1000]; char bbuf3[20];

       FSF.sprintf( bbuf, "\r\n--%s\r\n", parm->boundary );
       smtp->DataLine( bbuf , 0 );
       SaveOutgoingMessage( SAVE_STR, bbuf );

       FSF.sprintf( bbuf2, CHARSETSTR, parm->encode );
       FSF.sprintf( bbuf3, "%dbit", parm->nbit );

       switch ( parm->how )
       {
         case 1:
           SendHeaderLine("Content-Type: text/plain;", bbuf2, NULL );
           SendHeaderLine("Content-Transfer-Encoding:", bbuf3, NULL );
           break;
         case 4: // html
           SendHeaderLine("Content-Type: text/html;", bbuf2, NULL );
           SendHeaderLine("Content-Transfer-Encoding:", bbuf3, NULL );
           break;
       }
       lstrcpy( bbuf, CRLF );
       FMsmtp->DataLine( bbuf , 0 );
       SaveOutgoingMessage( SAVE_STR, bbuf );
     }

     if(fp!=INVALID_HANDLE_VALUE)
     {
       while(!FileEOF(fp))
       {
         if(ReadLine(fp,buf,MAXLINE))
         {
           int l = lstrlen(buf);
           TranscodeStr8Ext( buf, l, parm->charset, parm->encode );

           curr += l;
           p->UseProgress(curr);

           l--;
           while ( buf[l] == '\r' || buf[l] == '\n' ) l--;
           buf[++l] = '\r';
           buf[++l] = '\n';
           buf[++l] = '\0';

           if ( buf[0] != '.' ) {
              if ( !smtp->DataLine( buf , 0 ) ) break;
              SaveOutgoingMessage( SAVE_STR, buf );
           }
           else {
              lstrcpy( buf2, "." );
              lstrcat( buf2, buf );
              if ( !smtp->DataLine( buf2 , 0 ) ) break;
              SaveOutgoingMessage( SAVE_STR, buf2 );
           }
         } else break;
       }
     } else {
        int line = parm->infopos;
        struct EditorGetString egs;
        egs.StringNumber = line;

        while ( _Info.EditorControl( ECTL_GETSTRING , &egs ) ) {

           lstrcpyn( buf, egs.StringText, MAXLINE+1 );

           int l = lstrlen(buf);

           TranscodeStr8Ext( buf, l, parm->charset, parm->encode );

           curr += l;
           p->UseProgress(curr);

           l--;
           while ( buf[l] == '\r' || buf[l] == '\n' ) l--;
           buf[++l] = '\r';
           buf[++l] = '\n';
           buf[++l] = '\0';

           if ( buf[0] != '.' ) {
              if ( !smtp->DataLine( buf , 0 ) ) break;
              SaveOutgoingMessage( SAVE_STR, buf );
           }
           else {
              lstrcpy( buf2, "." );
              lstrcat( buf2, buf );
              if (!smtp->DataLine( buf2 , 0 ) ) break;
              SaveOutgoingMessage( SAVE_STR, buf2 );
           }
           egs.StringNumber = ++line;
        }
     }
  } else { // text/base64, html/base64, attached file/base64
     int z;

     if(fp==INVALID_HANDLE_VALUE) ReadReset(parm->infopos);

     if ( parm->multipart == MULTI_SENDFILE || parm->multipart == MULTI_SENDTEXT) {

        char bbuf[1000];

        FSF.sprintf( bbuf, "\r\n--%s\r\n", parm->boundary );
        smtp->DataLine( bbuf , 0 );
        SaveOutgoingMessage( SAVE_STR, bbuf );

        FSF.sprintf( bbuf, "name=\"%s\"", GetShortFileName(filename) );
        SendHeaderLine("Content-Type: application/octet-stream;", bbuf, parm->encode );

        SendHeaderLine("Content-Transfer-Encoding:", "Base64", NULL );

        if(parm->Header.GetRelated())
        {
          FSF.sprintf(bbuf,"<%s>\r\n",GetShortFileName(filename));
          SendHeaderLine("Content-ID:", bbuf, parm->encode);
        }
        else
        {
          FSF.sprintf( bbuf, "filename=\"%s\"\r\n", GetShortFileName(filename) );
          SendHeaderLine("Content-Disposition: attachment;", bbuf, parm->encode );
        }
     }
     do {

        if(fp!=INVALID_HANDLE_VALUE)
        {
          DWORD transferred;
          if(!ReadFile(fp,buf,19*3,&transferred,NULL)) z=0;
          else z=transferred;
        }
        else
          z = ReadBlock( buf, 19*3 );

        if ( parm->how == 2 || parm->how == 5 ) {
          TranscodeStr8Ext( buf, 19*3, parm->charset, parm->encode );
        }
        curr += z;
        p->UseProgress(curr);
        EncodeBase64 ( buf2, buf , z );
        lstrcat( buf2, CRLF );
        if (!smtp->DataLine( buf2 , 0 )) break;
        SaveOutgoingMessage( SAVE_STR, buf2 );

     } while ( z == 19*3 );

  }
  delete p;
}


static char * GetRecipientEMail( char * adr )
{
 static char email[80];

 char *s1 = strchr( adr, '<' );
 char *s2 = strchr( adr, '>' );
 char *s3 = strchr( adr, '@' );
 char *s4 = strchr( adr, '\"' );

 if ( !s3 ) return NULL;
 if ( s4 ) {
    if ( ( s4 = strchr( s4+1, '\"' ) ) != NULL ) {

       adr  = s4+1;
       s1 = strchr( adr, '<' );
       s2 = strchr( adr, '>' );
       s3 = strchr( adr, '@' );

       if ( !s3 ) return NULL;
    }
 }
 if ( s1 || s2 )
 {
   if ( s2 < s1 ) return NULL;
   if ( s3 < s1 || s3 > s2 ) return NULL;
   int len = s2-s1-1;
   if ( len <=0 ) return NULL;
   lstrcpyn( email , s1+1 , len+1 );
 }
 else
   lstrcpyn( email , adr, 80 );
 return email;
}

int FARMail::AnalizeFile(char *name,MAILSEND *parm)
{
  int bit8=0, bitcontrol=0;
  char buf[DETECTSIZE];
  int j=0,k;
  HANDLE fp=INVALID_HANDLE_VALUE;

  memset(buf,0,DETECTSIZE);

  parm->nbit=8;
  parm->how=3;
  parm->bitcontrol=1;

  lstrcpy(parm->charset,NULLSTR);
  lstrcpy(parm->encode,Opt.DefOutCharset);

  if(name) fp=CreateFile(name,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);

  if(fp!=INVALID_HANDLE_VALUE||!name)
  {
    if(fp!=INVALID_HANDLE_VALUE)
    {
      while(!(bit8&&bitcontrol&&j>=DETECTSIZE-1))
      {
        DWORD transferred; unsigned char c;
        if(ReadFile(fp,&c,sizeof(c),&transferred,NULL)&&transferred)
        {
          if(j<DETECTSIZE-1) buf[j++]=c;
          if(c<32&&c!=10&&c!=13&&c!=9) bitcontrol=1;
          if(c>127) bit8=1;
        }
        else break;
      }
    }
    else
    {
      struct EditorInfo ei;
      struct EditorGetString egs;
      struct EditorSetPosition esp;

      _Info.EditorControl(ECTL_GETINFO,&ei);
      esp.CurLine=esp.CurPos=0;
      esp.TopScreenLine=esp.LeftPos=esp.Overtype=esp.CurTabPos=-1;
      _Info.EditorControl(ECTL_SETPOSITION,&esp);
      egs.StringNumber=-1;
      while(_Info.EditorControl(ECTL_GETSTRING,&egs)&&esp.CurLine<ei.TotalLines)
      {
        int c;
        for(k=0;k<egs.StringLength;k++)
        {
          c=egs.StringText[k];
          if(j<DETECTSIZE-1)
            buf[j++]=(char)c;
          if(c>=0)
          {
            if(c<32&&c!=10&&c!=13&&c!=9) bitcontrol=1;
            if(c>127) bit8=1;
          }
        }
        esp.CurLine++;
       _Info.EditorControl(ECTL_SETPOSITION,&esp);
      }
      esp.CurLine=ei.CurLine;
      esp.CurPos=ei.CurPos;
      esp.TopScreenLine=ei.TopScreenLine;
      esp.LeftPos=ei.LeftPos;
      esp.CurTabPos=-1;
      esp.Overtype=-1;
      _Info.EditorControl(ECTL_SETPOSITION,&esp);
    }
    if(bit8)
    {
      if(name) //call from panels
      {
        char copybuf[DETECTSIZE];
        lstrcpy( copybuf, buf );
        for(int i=0;i<lstrlen(copybuf);i++)
          if(copybuf[i]<128) copybuf[i]=32;

        parm->nbit=8;
        //lstrcpy( parm->charset,NULLSTR ); // unknown -->

        int num=_Info.CharTable(FCT_DETECT,copybuf,lstrlen(copybuf));
        if(num>-1)
          for(int k=2;k<7;k++)
          {
            if(num==CharsetTable[k].num)
            {
              lstrcpy(parm->charset,CharsetTable[k].charset);
            }
          }
      }
      else //call from editor
      {
        EditorInfo ei;
        if(_Info.EditorControl(ECTL_GETINFO,&ei))
        {
          if(ei.TableNum==-1)
          {
            if(ei.AnsiMode) lstrcpy(parm->charset,CharsetTable[3].charset); //CP-1251
          }
          else
            for(int k=2;k<7;k++)
            {
              if(ei.TableNum==CharsetTable[k].num)
              {
                lstrcpy(parm->charset,CharsetTable[k].charset);
              }
            }
        }
      }
    }
    else
    {
      parm->nbit=7;
      lstrcpy(parm->charset,USASCII);
    }

    FSF.LStrupr(buf);
    parm->bitcontrol=bitcontrol;

    if(fp!=INVALID_HANDLE_VALUE) SetFilePointer(fp,0,NULL,FILE_BEGIN);
    GetHeaderInfo(fp,parm);
    if(fp!=INVALID_HANDLE_VALUE) CloseHandle(fp);

    if(bitcontrol)
    {
      if (name&&(!parm->infofound))
        parm->how=3;
      else if (strstr(buf,"<HTML>"))
        parm->how=5;
      else
        parm->how=2;
    }
    else
    {
      if(strstr(buf,"<HTML>")) parm->how=4;
      else parm->how=1;
    }
  }
  return 0;
}

int FARMail::SendFileWA( MAILSEND * parm, const char *FileName , long tsize )
{
 int stat;
 int savehow = parm->how;
 int savenbit = parm->nbit;
 int saveall = parm->all;

 if(parm->Header.CountAttach()==0) return SendFile( parm, FileName, tsize );

 InitBoundary( parm->boundary );

 parm->nbit = 8;
 parm->how  = 3;
 parm->all  = 1;

 parm->multipart = MULTI_START;
 stat = SendFile( parm, NULL , 0 );
 if ( !stat ) {

    parm->how = savehow;
    parm->nbit = savenbit;
    parm->all  = saveall;

    parm->multipart = MULTI_SENDTEXT;
    stat = SendFile( parm, FileName , tsize );

    if ( !stat ) {
       int savehow = parm->how;

       parm->nbit = 8;
       parm->how  = 3;
       parm->all  = 1;
       parm->infopos = 0;

       parm->multipart = MULTI_SENDFILE;

       for (size_t i=0; i<parm->Header.CountAttach(); i++ )
       {
          stat = SendFile( parm, parm->Header.GetAttach(i) , 0 );
          if ( stat ) break;
       }
       parm->how = savehow;
       parm->nbit = savenbit;
       parm->all  = saveall;

       if ( !stat ) {
          parm->multipart = MULTI_END;
          stat = SendFile( parm, NULL , 0 );
          parm->multipart = 0;
       }
    }
 }

 return stat;
}

static char *FindComma(const char *str)
{
  bool InQuote=false;
  while(*str)
  {
    if(*str=='"') InQuote=!InQuote;
    else if(*str=='\\'&&*(str+1)) str++;
    else if(!InQuote&&*str==',') break;
    str++;
  }
  if(*str==',') return (char *)str;
  return NULL;
}

int FARMail::SendRecipients(char *emptr)
{
  char embuf[512];
  int err_em = 0;

  do
  {
    int len;
    char *qw = FindComma(emptr);
    if ( qw )
    {
      len = qw - emptr;
      lstrcpyn( embuf, emptr, len+1 );
      char *rcpt = GetRecipientEMail( embuf );
      if ( rcpt )
      {
        if ( FMsmtp->Receipt( rcpt ) )
        {
            emptr = qw+1;
        } else { SayError( FMsmtp->GetErrorMessage() ); err_em=1; break; }
      } else { SayError( ::GetMsg( MesSMTP_RecipientError ) ); err_em=1; break; }
    } else {
      if ( lstrlen(emptr) )
      {
        lstrcpy( embuf, emptr );
        char *rcpt = GetRecipientEMail( embuf );
        if ( rcpt )
        {
          if ( FMsmtp->Receipt( rcpt ) )
          {
            break;
          } else { SayError( FMsmtp->GetErrorMessage() ); err_em=1; break; }
        } else { SayError( ::GetMsg( MesSMTP_RecipientError ) ); err_em=1; break; }
      }
      break;
    }

  } while(1);
  return err_em;
}

void FARMail::SendRecipientsHeader(const char *emptr, const char *headername, const char *encode)
{
  char buf[BUFFER_SIZE];

  if(FindComma(emptr))
    FSF.sprintf( buf,"%s %s", headername, CRLF );
  else
    lstrcpy( buf, headername );

  FMsmtp->DataLine( buf , 0 );
  SaveOutgoingMessage( SAVE_STR, buf );
  int loop=0;
  do
  {
    char *qw = FindComma(emptr);
    while (*emptr=='\t'||*emptr=='\x20')
      emptr++;
    if ( qw )
    {
      int len = qw - emptr + 1; // "," included
      char embuf[512];
      lstrcpyn( embuf, emptr, len+1 );
      FSF.sprintf( buf, " %s\r\n" , embuf );
      EncodeStr8( buf, encode );
      FMsmtp->DataLine( buf , 0 );
      SaveOutgoingMessage( SAVE_STR, buf );
      emptr = qw+1;
      loop = 1;
    }
    else
    {
      if (lstrlen(emptr) || loop==0)
      {
        FSF.sprintf( buf, " %s\r\n" , emptr );
        EncodeStr8( buf, encode );
        FMsmtp->DataLine( buf , 0 );
        SaveOutgoingMessage( SAVE_STR, buf );
      }
      break;
    }
  }
  while(1);
}

void FARMail::SendHeaderLine(const char *headername, const char *content, const char *encode)
{
  char buf[BUFFER_SIZE];

  FSF.sprintf( buf, "%s %s\r\n", headername, content );
  EncodeStr8( buf, encode );
  FMsmtp->DataLine( buf , 0 );
  SaveOutgoingMessage( SAVE_STR, buf );
}

int FARMail::SendFile( MAILSEND * parm, const char *FileName , long tsize )
{
  int gg ;
  HANDLE fp=INVALID_HANDLE_VALUE;

  if ( !current ) return -1;
  if ( parm->multipart < MULTI_SENDFILE )
  {
     FMsmtp = new SMTP( (Opt.DebugSession?TRUE:FALSE) , Opt.LOGFILE2 );
     if (current->UseAuth && (!*current->SMTP_Pass || !*current->SMTP_User))
       if (GetUser(current->SMTP_User,current->SMTP_Pass))
         return -1;
  }

  if ( !FMsmtp ) return -1;

  if ( FileName )
  {
    fp=CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if(fp==INVALID_HANDLE_VALUE)
    {
      SayError( ::GetMsg( MesSMTP_OpeningFileError ) );
      return -1;
    }
    else
    {
      if(!tsize) tsize=GetFileSize(fp,NULL);
    }
  }

  if ( parm->multipart > MULTI_START || (
      (
#ifdef FARMAIL_SSL
        (!current->UseAuth && FMsmtp->Connect( current->SMTPUrl, current->SMTPPort, (current->UseSMTPS?CON_SSL:CON_NORMAL) ) )||
        ( current->UseAuth && FMsmtp->Connect( current->SMTPUrl, current->SMTPPort ,current->SMTP_User, current->SMTP_Pass, (current->UseSMTPS?CON_SSL:CON_NORMAL) ) )
#else
        (!current->UseAuth && FMsmtp->Connect( current->SMTPUrl, current->SMTPPort ) )||
        ( current->UseAuth && FMsmtp->Connect( current->SMTPUrl, current->SMTPPort ,current->SMTP_User, current->SMTP_Pass ) )
#endif
      )
      ) ) {
     char *sender_adr = GetRecipientEMail(parm->Sender);
     sender_adr = ( sender_adr ? sender_adr : parm->Sender );
     if ( parm->multipart > MULTI_START || FMsmtp->Mail( sender_adr ) )
     {
        int err_em = 0;

        if ( parm->multipart < MULTI_SENDFILE )
        {
          err_em = SendRecipients(parm->Recipient);
          if (parm->UseCC)
          {
            if (!err_em && *parm->CC)
              err_em = SendRecipients(parm->CC);
            if (!err_em && *parm->BCC)
              err_em = SendRecipients(parm->BCC);
          }
        }

        if ( !err_em ) {

              if ( parm->multipart > MULTI_START || FMsmtp->Data() ) {

                 char buf[BUFFER_SIZE], buf2[100];

                 if ( parm->multipart < MULTI_SENDFILE ) {

                    ShortMessage *sm = new ShortMessage( MsgDataSMTP );

                    SaveOutgoingMessage( SAVE_OPEN, NULL );

                    SendHeaderLine(_DATE, GetDateInSMTP(), NULL );

                    if (*parm->Sender)
                      SendHeaderLine(FROM, parm->Sender, parm->encode );

                    if (*parm->ReplyTo)
                      SendHeaderLine("Reply-To:", parm->ReplyTo, parm->encode );
                    else if (*parm->Sender)
                      SendHeaderLine("Reply-To:", parm->Sender, parm->encode );

                    if (*parm->Recipient)
                      SendRecipientsHeader(parm->Recipient,"To:",parm->encode);

                    if (parm->UseCC && *parm->CC)
                      SendRecipientsHeader(parm->CC,"CC:",parm->encode);

                    if (*parm->Organization)
                      SendHeaderLine("Organization:", parm->Organization, parm->encode );
                    else if (*current->Organization)
                      SendHeaderLine("Organization:", current->Organization, parm->encode );

                    if (*parm->Subject)
                      SendHeaderLine(SUBJECT, parm->Subject, parm->encode );

                    FSF.sprintf(buf2,"FARMail %s.%s.%s-%s%s%s-%s%s",VER_HI,VER_LO,BUILD,BUILD_YEAR,BUILD_MONTH,BUILD_DAY,
#if defined(FARMAIL_SSL)
                    "ssl-",
#else
                    NULLSTR,
#endif
#if defined(__GNUC__)
                    "gcc"
#else
#if defined(__BORLANDC__)
                    "bcc"
#else
                    "unknown_c"
#endif
#endif
                    );

                    if(*(parm->Mailer))
                      SendHeaderLine("X-Mailer:", parm->Mailer, parm->encode );
                    else
                      SendHeaderLine("X-Mailer:", buf2, NULL );
                    SendHeaderLine("MIME-Version:", "1.0", NULL );

                    FSF.sprintf( buf2, CHARSETSTR, parm->encode );
                    char buf3[20];
                    FSF.sprintf( buf3, "%dbit", parm->nbit);
                    switch ( parm->how )
                    {
                      case 1:
                        SendHeaderLine("Content-Type: text/plain;", buf2, NULL );
                        SendHeaderLine("Content-Transfer-Encoding:", buf3, NULL );
                        break;
                      case 2:
                        SendHeaderLine("Content-Type: text/plain;", buf2, NULL );
                        SendHeaderLine("Content-Transfer-Encoding:", "Base64", NULL );
                        break;
                      case 3:
                        if ( !parm->multipart )
                        {
                          gg = 0;
                          if ( !FileName ) { gg=1; FileName = GetEditorFileName(); }
                          FSF.sprintf( buf, "name=\"%s\"", GetShortFileName(FileName) );
                          SendHeaderLine("Content-Type: application/octet-stream;", buf, parm->encode );
                          SendHeaderLine("Content-Transfer-Encoding:", "Base64", NULL );
                          FSF.sprintf( buf, "filename=\"%s\"", GetShortFileName(FileName) );
                          SendHeaderLine("Content-Disposition: attachment;", buf, parm->encode );
                          if ( gg ) FileName = NULL;
                        }
                        else
                        {
                          FSF.sprintf(buf,"%s; boundary=\"%s\"",(parm->Header.GetRelated())?"multipart/related":"multipart/mixed",parm->boundary);
                          SendHeaderLine(CONTENTTYPE, buf, NULL);
                        }
                        break;
                      case 4: // html
                        SendHeaderLine("Content-Type: text/html;", buf2, NULL );
                        SendHeaderLine("Content-Transfer-Encoding:", buf3, NULL );
                        break;
                      case 5: // html / base64
                        SendHeaderLine("Content-Type: text/html;", buf2, NULL );
                        SendHeaderLine("Content-Transfer-Encoding:", "Base64", NULL );
                        break;
                      case 6:
                        break;

                    }
                    for (size_t g=0; g<parm->Header.CountHeader(); g++ )
                    {
                       FSF.sprintf( buf, "%s:", parm->Header.GetName(g) );
                       SendHeaderLine(buf, parm->Header.GetValue(g,0), parm->encode );
                       for (size_t gg=1; gg < parm->Header.CountValues(g); gg++)
                       {
                         SendHeaderLine(NULLSTR, parm->Header.GetValue(g,gg), parm->encode );
                       }
                    }
                    if (parm->how != 6)
                    {
                      FMsmtp->DataLine( CRLF , 0 );
                      SaveOutgoingMessage( SAVE_STR, CRLF );
                    }

                    delete sm;
                 }

                 if ( parm->multipart == MULTI_START )
                 {
                    // let's say it's a multipart message
                    lstrcpy( buf, "This is a MIME multipart message generated by FARMail.\r\n" );
                    FMsmtp->DataLine( buf, 0 );
                    SaveOutgoingMessage( SAVE_STR, buf );
                 }
                 // body
                 if ( !parm->multipart || parm->multipart == MULTI_SENDFILE || parm->multipart == MULTI_SENDTEXT )
                    SendBody( fp, parm, FMsmtp , FileName, tsize );

                 if ( parm->multipart == MULTI_END )
                 {
                    // let's close the multipart
                    FSF.sprintf( buf, "--%s--\r\n", parm->boundary );
                    FMsmtp->DataLine( buf , 0 );
                    SaveOutgoingMessage( SAVE_STR, buf );
                 }

                 if ( !parm->multipart || parm->multipart == MULTI_END ) {
                    FMsmtp->DataLine( CRLF , 0 );
                    SaveOutgoingMessage( SAVE_STR, CRLF );
                    if ( !FMsmtp->DataLine( ".\r\n" , 1 ) )  SayError( FMsmtp->GetErrorMessage() );
                    else
                    {
                       SaveOutgoingMessage( SAVE_STR, ".\r\n" );
                       SaveOutgoingMessage( SAVE_CLOSE, FMsmtp->message_id );
                    }
                 }

              } else { SayError( FMsmtp->GetErrorMessage() ); }
        }
     } else SayError( FMsmtp->GetErrorMessage() );

     if ( !parm->multipart || parm->multipart == MULTI_END )
        if( !FMsmtp->Disconnect() ) SayError( FMsmtp->GetErrorMessage() );

  } else SayError( FMsmtp->GetErrorMessage() );

  if ( !parm->multipart || parm->multipart == MULTI_END ) {
     SaveOutgoingMessage( SAVE_KILL, NULL );
     delete FMsmtp;
     FMsmtp = NULL;
  }
  if(fp!=INVALID_HANDLE_VALUE) CloseHandle(fp);

  return 0;
}

#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif
#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif

char * FARMail::GetEditorFileName( void )
{
  static char ZZZ[_MAX_FNAME+_MAX_EXT+2];
  struct EditorInfo ei;

  lstrcpy( ZZZ, QUESTIONMARK );

  if ( _Info.EditorControl( ECTL_GETINFO, &ei ) )
    lstrcpyn( ZZZ, ei.FileName, _MAX_FNAME+_MAX_EXT+2 );
  return ZZZ;
}
