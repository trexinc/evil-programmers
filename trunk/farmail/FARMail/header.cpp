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
#include "crt_file.hpp"

static char X_START[101]; //= "%start%";
static char X_END  [101]; //= "%end%";
static const char X_FROM     [] = "%from%";
static const char X_DATE     [] = "%date%";
static const char X_CURDATE  [] = "%curdate%";
static const char X_TO       [] = "%to%";
static const char X_CC       [] = "%cc%";
static const char X_BCC      [] = "%bcc%";
static const char X_SUBJ     [] = "%subj%";
static const char X_SIGN     [] = "%sign%";
static const char X_HOW      [] = "%how%";
static const char X_CHARSET  [] = "%charset%";
static const char X_ENCODETO [] = "%encodeto%";
static const char X_REPLY    [] = "%reply%";
static const char X_ORG      [] = "%org%";
const char DEFSTARTBLOCK [] = "%start%";
const char DEFENDBLOCK   [] = "%end%";
const char STARTBLOCK    [] = "StartBlock";
const char ENDBLOCK      [] = "EndBlock";

static const char ILLEGAL_SYMB[] = "/<>:\\|?*\"";
static const char ILLEGAL_REPL[] = "!()_!!__'";


void FreeMailSend( MAILSEND *parm )
{
  parm->Header.ClearAll();
  parm->infopos = 0;
  parm->infofound = false;
}

void FARMail::FinalMacroProcessing(MAILSEND *parm)
{
  DecodeStr8(parm->Mailer, parm->charset);
  DecodeStr8(parm->ReplyTo, parm->charset);
  DecodeStr8(parm->Organization, parm->charset );
  ProcessHeaderDirectives(parm);
  DecodeAttachList(parm);
}

void FARMail::DecodeAttachList(MAILSEND *parm)
{
  for (size_t i=0; i < parm->Header.CountAttach(); i++)
    parm->Header.DecodeAttach(this, parm->charset, i);
}

int FARMail::ProcessHeaderDirectives(MAILSEND *parm)
{
  const char *maxptr;
  const char *ptr;
  const char *org;
  size_t len;
  char *processed;
  char *prc_ptr;
  size_t offset;
  for (size_t i=0; i < parm->Header.CountHeader(); i++)
  {
      parm->Header.DecodeName(this, parm->charset, i );
      parm->Header.DecodeValues(this, parm->charset, i );
      for (size_t j=0; j < parm->Header.CountValues(i); j++)
      {
        org = parm->Header.GetValue(i,j);
        len = lstrlen(org);
        maxptr = org+len;
        for (int pass=0; pass<2; pass++)
        {
          ptr = org;
          if (pass==1)
          {
            prc_ptr = processed = (char *) z_calloc(len+1, sizeof(char));
            if (!processed)
              return 0;
          }
          while ( ptr < maxptr )
          {

              if ( *ptr != '%' )
              {
                if (pass == 1)
                  *(prc_ptr++) = *(ptr++);
                else
                  ptr++;
              }
              else
              {

                if ( !FSF.LStrnicmp( ptr, X_TO , lstrlen(X_TO) ) )
                {
                    if (pass == 1)
                    {
                      lstrcpy( prc_ptr, parm->Recipient);
                      ptr+=lstrlen(X_TO);
                      prc_ptr+=lstrlen(parm->Recipient);
                    }
                    else
                    {
                      offset=lstrlen(X_TO);
                      len+=lstrlen(parm->Recipient)-offset;
                    }
                }
                else if ( !FSF.LStrnicmp( ptr, X_CC , lstrlen(X_CC) ) )
                {
                    if (pass == 1)
                    {
                      lstrcpy( prc_ptr, parm->CC);
                      ptr+=lstrlen(X_CC);
                      prc_ptr+=lstrlen(parm->CC);
                    }
                    else
                    {
                      offset=lstrlen(X_CC);
                      len+=lstrlen(parm->CC)-offset;
                    }
                }
                else if ( !FSF.LStrnicmp( ptr, X_BCC , lstrlen(X_BCC) ) )
                {
                    if (pass == 1)
                    {
                      lstrcpy( prc_ptr, parm->BCC);
                      ptr+=lstrlen(X_BCC);
                      prc_ptr+=lstrlen(parm->BCC);
                    }
                    else
                    {
                      offset=lstrlen(X_BCC);
                      len+=lstrlen(parm->BCC)-offset;
                    }
                }
                else if ( !FSF.LStrnicmp( ptr, X_SUBJ , lstrlen(X_SUBJ) ) )
                {
                    if (pass == 1)
                    {
                      lstrcpy( prc_ptr, parm->Subject);
                      ptr+=lstrlen(X_SUBJ);
                      prc_ptr+=lstrlen(parm->Subject);
                    }
                    else
                    {
                      offset=lstrlen(X_SUBJ);
                      len+=lstrlen(parm->Subject)-offset;
                    }
                }
                else if ( !FSF.LStrnicmp( ptr, X_FROM , lstrlen(X_FROM) ) )
                {
                    if (pass == 1)
                    {
                      lstrcpy( prc_ptr, parm->Sender);
                      ptr+=lstrlen(X_FROM);
                      prc_ptr+=lstrlen(parm->Sender);
                    }
                    else
                    {
                      offset=lstrlen(X_FROM);
                      len+=lstrlen(parm->Sender)-offset;
                    }
                }
                else if ( !FSF.LStrnicmp( ptr, X_REPLY , lstrlen(X_REPLY) ) )
                {
                    if (pass == 1)
                    {
                      lstrcpy( prc_ptr, parm->ReplyTo);
                      ptr+=lstrlen(X_REPLY);
                      prc_ptr+=lstrlen(parm->ReplyTo);
                    }
                    else
                    {
                      offset=lstrlen(X_REPLY);
                      len+=lstrlen(parm->ReplyTo)-offset;
                    }
                }
                else
                {
                  if (pass==0)
                    offset = 1;
                  else
                    *(prc_ptr++)= *(ptr++);
                }
                if (pass==0)
                  ptr += offset;
              }
          }

        }
        parm->Header.SetValue(i,j,processed);
        z_free(processed);
      }
  }
  return 0;
}


int FARMail::ProcessMacro( char *buf, MAILSEND *parm )
{

  if ( !FSF.LStrnicmp( buf, "set", 3 ) ) {

     buf+=3;
     FSF.Trim(buf);
     if ( !FSF.LStrnicmp( buf, X_TO , lstrlen(X_TO) ) ) {
        buf+=lstrlen(X_TO);
        FSF.Trim(buf);
        if ( *buf == '=' ) {
           buf ++;
           FSF.Trim(buf);
        }
        lstrcpyn( parm->Recipient, buf, 512 );
     } else if ( !FSF.LStrnicmp( buf, X_CC , lstrlen(X_CC)) ) {
        buf+=lstrlen(X_CC);
        FSF.Trim(buf);
        if ( *buf == '=' ) {
           buf ++;
           FSF.Trim(buf);
        }
        lstrcpyn( parm->CC, buf, 512 );
     } else if ( !FSF.LStrnicmp( buf, X_BCC , lstrlen(X_BCC)) ) {
        buf+=lstrlen(X_BCC);
        FSF.Trim(buf);
        if ( *buf == '=' ) {
           buf ++;
           FSF.Trim(buf);
        }
        lstrcpyn( parm->BCC, buf, 512 );
     } else if ( !FSF.LStrnicmp( buf, X_SUBJ , lstrlen(X_SUBJ)) ) {
        buf+=lstrlen(X_SUBJ);
        FSF.Trim(buf);
        if ( *buf == '=' ) {
           buf ++;
           FSF.Trim(buf);
        }
        lstrcpyn( parm->Subject, buf, 512 );
     } else if ( !FSF.LStrnicmp( buf, X_FROM , lstrlen(X_FROM)) ) {
        buf+=lstrlen(X_FROM);
        FSF.Trim(buf);
        if ( *buf == '=' ) {
           buf ++;
           FSF.Trim(buf);
        }
        lstrcpyn( parm->Sender, buf, 512 );
        parm->UserSender = true;
     } else if ( !FSF.LStrnicmp( buf, X_REPLY , lstrlen(X_REPLY)) ) {
        buf+=lstrlen(X_REPLY);
        FSF.Trim(buf);
        if ( *buf == '=' ) {
           buf ++;
           FSF.Trim(buf);
        }
        lstrcpyn( parm->ReplyTo, buf, 512 );
     } else if ( !FSF.LStrnicmp( buf, X_SIGN , lstrlen(X_SIGN)) ) {
        buf+=lstrlen(X_SIGN);
        FSF.Trim(buf);
        if ( *buf == '=' ) {
           buf ++;
           FSF.Trim(buf);
        }
        lstrcpyn( parm->Mailer, buf, 512 );
     } else if ( !FSF.LStrnicmp( buf, X_ORG , lstrlen(X_ORG)) ) {
        buf+=lstrlen(X_ORG);
        FSF.Trim(buf);
        if ( *buf == '=' ) {
           buf ++;
           FSF.Trim(buf);
        }
        lstrcpyn( parm->Organization, buf, 512 );
     } else if ( !FSF.LStrnicmp( buf, X_HOW , lstrlen(X_HOW)) ) {
        buf+=lstrlen(X_HOW);
        FSF.Trim(buf);
        if ( *buf == '=' ) {
           buf ++;
           FSF.Trim(buf);
        }
        int how = FSF.atoi(buf);
        if (how>0 && how<7)
          parm->how = how;
     } else if ( !FSF.LStrnicmp( buf, X_ENCODETO , lstrlen(X_ENCODETO)) ) {
        buf+=lstrlen(X_ENCODETO);
        FSF.Trim(buf);
        if ( *buf == '=' ) {
           buf ++;
           FSF.Trim(buf);
        }
        int encodeto = FindCharset(buf, &CharsetTable);
        if (encodeto>=0)
          lstrcpy(parm->encode, CharsetTable[encodeto].charset );
     } else if ( !FSF.LStrnicmp( buf, X_CHARSET , lstrlen(X_CHARSET)) ) {
        buf+=lstrlen(X_CHARSET);
        FSF.Trim(buf);
        if ( *buf == '=' ) {
           buf ++;
           FSF.Trim(buf);
        }
        int charset = FindCharset(buf, &CharsetTable);
        if (charset>=0)
          lstrcpy(parm->charset, CharsetTable[charset].charset );
     }
     return 0;
  }

  if ( !FSF.LStrnicmp( buf, "header", 6 ) )
  {

     char *delim;
     buf+=6;
     FSF.Trim(buf);

     if ( ( delim = strchr( buf, '=' ) ) != NULL )
     {

        *delim = '\0';

        char * field = buf;
        char * value = delim+1;

        FSF.Trim( field );
        FSF.Trim( value );

        if ( !strchr( field,':' ) && FSF.LStricmp( field, "from" ) &&
             FSF.LStricmp( field, "reply-to" ) &&
             FSF.LStricmp( field, "to" ) &&
             FSF.LStricmp( field, "cc" ) &&
             FSF.LStricmp( field, "bcc" ) &&
             FSF.LStricmp( field, "subject" ) &&
             FSF.LStricmp( field, "received" ) &&
             FSF.LStricmp( field, "mime-version" ) &&
             FSF.LStricmp( field, "content-type" ) &&
             FSF.LStricmp( field, "content-transfer-encoding" ) &&
             FSF.LStricmp( field, "x-mailer" ) &&
             FSF.LStricmp( field, "organization" ) )
        {
          parm->Header.AppendHeader(field,value);
        }
     }

  }
  if ( !FSF.LStrnicmp( buf, "appendheader", 12 ) )
  {
     buf+=12;
     FSF.Trim(buf);
     if (parm->Header.CountHeader() && *buf)
       parm->Header.AppendValue(parm->Header.CountHeader()-1,buf);
  }
  if ( !FSF.LStrnicmp( buf, "attach", 6 ) )
  {
     buf+=6;
     FSF.Trim(buf);
     FSF.Unquote(buf);
     parm->Header.AppendAttach(buf);
  }
  if(!FSF.LStricmp(buf,"related"))
    parm->Header.SetRelated();
  return 0;
}


int FARMail::GetHeaderInfo(HANDLE fp,MAILSEND *parm)
{
  HKEY hRoot=HKEY_CURRENT_USER;

  if(!Opt.UseHeader) return 0;

  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, STARTBLOCK, X_START , DEFSTARTBLOCK, 100 );
  GetRegKey2(  hRoot, PluginCommonKey, NULLSTR, ENDBLOCK  , X_END   , DEFENDBLOCK  , 100 );

  if(fp!=INVALID_HANDLE_VALUE)
  {
    char buf[1000];
    if(ReadLine(fp,buf,sizeof(buf)))
    {
      FSF.Trim(buf);
      if(!FSF.LStrnicmp(buf,X_START,lstrlen(X_START)))
      {
        while(true)
        {
          if(ReadLine(fp,buf,sizeof(buf)))
          {
             FSF.Trim(buf);
             if(!FSF.LStrnicmp(buf,X_END,lstrlen(X_END))) break;
             ProcessMacro(buf,parm);
          }
          else break;
        }
        parm->infopos=SetFilePointer(fp,0,NULL,FILE_CURRENT);
        parm->infofound=true;
      }
      else SetFilePointer(fp,0,NULL,FILE_BEGIN);
    }
    else SetFilePointer(fp,0,NULL,FILE_BEGIN);

  }
  else
  {
    struct EditorGetString egs;
    int lline = 0, k;
    char buf[1000];

    parm->infopos = 0;

    egs.StringNumber = lline;
    while ( _Info.EditorControl( ECTL_GETSTRING, &egs )  )
    {

      for ( k=0 ; ( k<egs.StringLength && k<999 ); k++ )
          buf[k] = egs.StringText[k];

      buf[k] = 0;

      FSF.Trim(buf);

      if ( lline == 0 && FSF.LStrnicmp( buf, X_START, lstrlen(X_START) ) )
      {
        break;
      }
      else
      {
          if ( !FSF.LStrnicmp( buf, X_END, lstrlen(X_END) ) ) { parm->infopos = lline+1; break; }
          ProcessMacro( buf, parm );
          parm->infofound=true;
      }

      egs.StringNumber = ++lline;
    }
  }
  DecodeStr8(parm->Recipient, parm->charset );
  DecodeStr8(parm->CC, parm->charset );
  DecodeStr8(parm->BCC, parm->charset );
  DecodeStr8(parm->Subject, parm->charset );
  if (parm->UserSender)
    DecodeStr8(parm->Sender, parm->charset );
  DecodeStr8(parm->ReplyTo, parm->charset );
  return 0;
}




int FARMail::MakeDescription( char *buf, char *format, char *from, char *subj, char *date, int buflen )
{
 char * ptr = buf;
 char slashes[MAX_PATH];

 memset(slashes,32,MAX_PATH);

 while ( *format ) {

    if ( *format == '%' ) {

       if ( !FSF.LStrnicmp( format, X_FROM, lstrlen( X_FROM )-1 ) ) {

          char *ptr = from;
          int  fl   = -1;
          format += lstrlen( X_FROM )-1;

          if ( *format == '%' ) format++;
          else if ( *format == ':' ) {
             fl = FSF.atoi( format+1);
             while ( *format && *format != '%' ) format++;
             if ( *format ) format++;
          }

          while ( *ptr && fl ) {

             *buf = *ptr;
             ptr++;
             buf++;
             buflen--;
             fl--;

             if ( buflen <= 0  ) break;

          }
          if ( buflen <= 0  ) break;

       } else if ( !FSF.LStrnicmp( format, X_SUBJ, lstrlen( X_SUBJ )-1 ) ) {

          char *ptr = subj;
          int  fl   = -1;
          format += lstrlen( X_SUBJ )-1;

          if ( *format == '%' ) format++;
          else if ( *format == ':' ) {
             fl = FSF.atoi( format+1);
             while ( *format && *format != '%' ) format++;
             if ( *format ) format++;
          }

          while ( *ptr && fl ) {

             *buf = *ptr;
             ptr++;
             buf++;
             buflen--;
             fl--;

             if ( buflen <= 0  ) break;

          }
          if ( buflen <= 0  ) break;

       } else if ( !FSF.LStrnicmp( format, X_DATE, lstrlen( X_DATE )-1 ) ) {

          char *ptr = date;
          int  fl   = -1;
          format += lstrlen( X_DATE )-1;

          if ( *format == '%' ) format++;
          else if ( *format == ':' ) {
             fl = FSF.atoi( format+1);
             while ( *format && *format != '%' ) format++;
             if ( *format ) format++;
          }

          while ( *ptr && fl ) {

             *buf = *ptr;
             ptr++;
             buf++;
             buflen--;
             fl--;

             if ( buflen <= 0  ) break;

          }
          if ( buflen <= 0  ) break;

       } else if ( !FSF.LStrnicmp( format, X_CURDATE, lstrlen( X_CURDATE )-1 ) ) {

          char curdate[51];
          *curdate = 0;
          char *ptr=curdate;
          SYSTEMTIME st;
          GetLocalTime(&st);
          char year[5], syear[3], month[3], day[3];
          FSF.sprintf(year,"%04d",st.wYear);
          FSF.sprintf(syear,"%02d",st.wYear%1000);
          FSF.sprintf(month,"%02d",st.wMonth);
          FSF.sprintf(day,"%02d",st.wDay);
          format += lstrlen( X_CURDATE )-1;

          if ( *format == '%' )
          {
             lstrcpy(curdate,year);
             lstrcat(curdate,month);
             lstrcat(curdate,day);
             format++;
          }
          else if ( *format == ':' ) {
             format++;
             while ( *format && *format != '%' )
             {
               switch (*format)
               {
                 case 'd':
                 case 'D':
                   lstrcat(curdate,day);
                   break;
                 case 'm':
                 case 'M':
                   lstrcat(curdate,month);
                   break;
                 case 'y':
                 case 'Y':
                   lstrcat(curdate,year);
                   break;
                 case 's':
                 case 'S':
                   lstrcat(curdate,syear);
                   break;
                 default:
                   strcatchr(curdate,*format);
               }
               format++;
             }
             if ( *format ) format++;
          }

          while ( *ptr) {

             *buf = *ptr;
             ptr++;
             buf++;
             buflen--;

             if ( buflen <= 0  ) break;

          }
          if ( buflen <= 0  ) break;

       } else {

          *buf = *format;
          if (*format == '\\' || *format == '/')
            *buf = slashes[buf-ptr] = '\\';
          format++;
          buf++;
          buflen--;

          if ( buflen <= 0  ) break;

       }
    } else {

       *buf = *format;
       if (*format == '\\' || *format == '/')
         *buf = slashes[buf-ptr] = '\\';
       format++;
       buf++;
       buflen--;

       if ( buflen <= 0  ) break;
    }

 }
 *buf = 0;

 char *org = ptr;
 while ( *ptr )
 {

    const char *zz;
    if ( (slashes[ptr-org] != '\\') && (( zz = strchr( ILLEGAL_SYMB, *ptr ) ) != NULL) )
       *ptr = ILLEGAL_REPL[zz-ILLEGAL_SYMB];

    if ( *ptr < 32 || *ptr == 127 ) *ptr = 32;

    ptr++;
 }
 return 0;
}



char * GetShortFileName( const char *fname )
{
  static char retval[MAX_PATH];
  lstrcpy( retval, FSF.PointToName(fname) );

  return retval;
}
