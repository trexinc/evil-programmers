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
#include <stdio.h> //FIXME
#include "farmail.hpp"
#include "version.hpp"
#include "crt_file.hpp"

const char ASTERISK[] = "*";
const char QUESTIONMARK[] = "?";
const char D1[] = "%d";
const char CRLF[] = "\r\n";
const char BACKSLASH[] = "\\";

const char XRESUMEDATA[] = "X-FARMail-ResumeData: %013ld:%013ld\r\n";

char *GenerateName(int i, char *str)
{
  if ( Opt.LeadingZeros )
  {
    if ( *Opt.EXT )
    {
      char _buf[20];
      FSF.sprintf( _buf, "%%0%dd.%%s", Opt.LeadingZeros);
      FSF.sprintf( str, _buf, i , Opt.EXT );
      return str;
    }
    else
    {
      char _buf[20];
      FSF.sprintf( _buf, "%%0%dd", Opt.LeadingZeros);
      FSF.sprintf( str, _buf, i );
      return str;
    }
  }
  else
  {
    if ( *Opt.EXT )
    {
      FSF.sprintf( str, "%d.%s", i , Opt.EXT );
      return str;
    }
    else
    {
      FSF.sprintf( str, D1, i );
      return str;
    }
  }
}

const char *get_token( const char *str, int n , int imap )
{
  static char token[512];
  int j=0,i;
  const char *delims[] = { " ,:\t", ";" };

  while ( strchr( delims[imap], *str ) ) str++;

  for ( i=0 ; i<n; i++ )
  {
    while ( *str != 0 && !strchr( delims[imap], *str ) )
      str++;
    if ( *str == 0 )
    {
      *(token) = 0;
      return token;
    }
    while ( strchr( delims[imap], *str ) ) str++;
  }

  while ( !strchr( delims[imap], *str ) && *str != 0 && j<511 )
    token[j++] = *(str++);
  token[j] = 0;

  return token;
}


static int GetMonth( const char *str )
{
 int i;
 for ( i=0 ; i<12; i++ )
    if ( !FSF.LStricmp( str, Mon[i] ) ) return i+1;
 return 0;
}



static void ConvertDate( char *str, FAR_FIND_DATA *fd )
{
 int tok = 0;
 SYSTEMTIME st;
 FILETIME ft1, ft2;

 if ( !FSF.atoi(get_token(str,tok,0)) ) tok++;

 st.wDay    = (WORD)FSF.atoi( get_token(str,tok,0) );
 st.wMonth  = (WORD)GetMonth( get_token(str,++tok,0) );
 st.wYear   = (WORD)FSF.atoi( get_token(str,++tok,0) );
 st.wHour   = (WORD)FSF.atoi( get_token(str,++tok,0) );
 st.wMinute = (WORD)FSF.atoi( get_token(str,++tok,0) );
 st.wSecond = (WORD)FSF.atoi( get_token(str,++tok,0) );

 SystemTimeToFileTime( &st, &ft1 ); // local here
 LocalFileTimeToFileTime( &ft1, &ft2 ); // local->UTC
 fd->ftCreationTime = ft2; // must be UTC!
 fd->ftLastAccessTime = fd->ftCreationTime;
 fd->ftLastWriteTime =  fd->ftCreationTime;
}



const char * GetColumnMes( char *buf )
{
 if ( strstr( buf, "N" ) ) return GetMsg(MesMessageName_Panel);
 if ( strstr( buf, "S" ) ) return GetMsg(MesSizeName_Panel);
 if ( strstr( buf, "C0" ) ) return GetMsg(MesFromName_Panel);
 if ( strstr( buf, "C1" ) ) return GetMsg(MesDateName_Panel);
 if ( strstr( buf, "C2" ) ) return GetMsg(MesSubjName_Panel);
 return NULL; // STR;
}

static void FillColumns( const char *titles[], char *type )
{
 char *begin = type;
 int nump = 0;
 char buf[512];

 while ( 1 ) {
    char *ptr = strchr( begin, ',' );
    if ( ptr ) {
       lstrcpyn( buf, begin, (int)(ptr-begin)+1 );
       FSF.LStrupr( buf );
       titles[nump] = GetColumnMes( buf );
       begin = ptr+1;
       nump++;
    } else {
       if ( *begin ) {
          lstrcpy( buf, begin );
          FSF.LStrupr( buf );
          titles[nump] = GetColumnMes( buf );
       }
       break;
    }
    if ( nump>=5 ) break;
 }
}

static BOOL MakeDirs(const char *root, const char *path)
{
  char dest[MAX_PATH];

  lstrcpy(dest, root);
  if (dest[lstrlen(dest)-1] != '\\')
    lstrcat(dest,"\\");
  const char *ptr = (path + lstrlen(dest));
  const char *i;
  while ((i = strstr(ptr,"\\")) != NULL)
  {
    lstrcpyn(dest+lstrlen(dest),ptr,i-ptr+1);
    ptr = i + 1;
    DWORD a = GetFileAttributes(dest);
    if (a == 0xFFFFFFFF)
    {
      if (!CreateDirectory(dest,NULL))
      {
        return FALSE;
      }
    }
    else
    {
      if (!(a&FILE_ATTRIBUTE_DIRECTORY))
      {
        return FALSE;
      }
    }
    lstrcat(dest,"\\");
  }
  return TRUE;
}

int ExtDecodeChar8(char *c,const char *charset,CHARSET_TABLE **CharsetTable)
{
  if (!charset) return 0;
  if (!FSF.LStricmp(charset,NULLSTR)) return 0;
  if (!FSF.LStricmp(charset,USASCII)) return 0;

  // charset->OEM
  int t=FindCharset(charset,CharsetTable);
  if(t>=0)
  {
    *c=(*CharsetTable)[t].DecodeTable[*c];
  }
  return 0;
}

int ExtDecodeStr8(char *str,const char *charset,CHARSET_TABLE **CharsetTable)
{
  if(!charset) return 0;
  if(!FSF.LStricmp(charset,NULLSTR)) return 0;
  if(!FSF.LStricmp(charset,USASCII)) return 0;

  // charset->OEM
  int t=FindCharset(charset,CharsetTable );
  if(t>=0)
  {
    for(int i=0;i<lstrlen(str);i++)
      str[i]=(*CharsetTable)[t].DecodeTable[str[i]];
  }
  return 0;
}


int DecodeField(char *str,char *charset_h,CHARSET_TABLE **CharsetTable)
{
  unsigned long str_size=lstrlen(str)+1;
  char *_tempbuf=(char *)z_malloc(str_size*3*sizeof(char));
  if(_tempbuf)
  {
    char *tempbuf=_tempbuf;
    char *text=_tempbuf+str_size;
    char *decodedtext=_tempbuf+2*str_size;

    lstrcpy(tempbuf,str);
    *str=0;

    if(charset_h)
    {
      char *tt=charset_h;
      while(*tt!=';'&&*tt>32&&*tt!='\"') tt++;
      *tt='\0';
    }

    while(TRUE)
    {
      char *ptr=strstr(tempbuf,"=?");
      if(ptr)
      {
        while(tempbuf<ptr)
        {
          ExtDecodeChar8(tempbuf,charset_h,CharsetTable);
          strcatchr(str,*(tempbuf++));
        }
        char charset[1000]; //FIXME
        char encoding[1000]; //FIXME

        *decodedtext=0;
        tempbuf=SplitHeaderLine(tempbuf,charset,encoding,text);

        if (!FSF.LStricmp(encoding,"b"))
        {
          // subj in base64;
          DecodeBase64(decodedtext,text,lstrlen(text));
        }
        else if(!FSF.LStricmp(encoding,"q"))
        {
          // subj in quoted-printable
          DecodeQuotedPrintable(text,lstrlen(text),decodedtext);
        }
        ExtDecodeStr8(decodedtext,charset,CharsetTable);
        lstrcat(str,decodedtext);
        continue;
      }
      else
      {
        while(*tempbuf)
        {
          ExtDecodeChar8(tempbuf,charset_h,CharsetTable);
          strcatchr(str,*(tempbuf++));
        }
        break;
      }
    }
    z_free(_tempbuf);
  }
  return 0;
}


void FARMail::InitMAILSEND(MAILSEND *parm)
{
  *(parm->Sender)=0;
  *(parm->ReplyTo)=0;
  *(parm->Recipient)=0;
  *(parm->Subject)=0;
  *(parm->CC)=0;
  *(parm->BCC)=0;
  *(parm->Mailer)=0;
  *(parm->Organization)=0;
  parm->nbit = parm->how = parm->all = parm->analize_all = parm->bitcontrol = 0;
  *(parm->charset) = 0;
  *(parm->encode) = 0;

  parm->multipart = 0;
  *(parm->boundary) = 0;
  parm->infopos = 0;
  parm->infofound = false;

  parm->UseCC = false;
  parm->UserSender = false;
}


void FARMail::DecodeSubj( char *str , char *charset_h )
{
  ::DecodeField(str,charset_h,&CharsetTable);
}



int FARMail::SelectMailbox( void )
{
 int i, flag = 0;
 if ( ServerCount == 0 ) return 1;
 if ( ServerCount == 1 ) {
    current = &server[0];
    return 0;
 }
 struct FarMenuItem *menu = (struct FarMenuItem *)z_calloc( ServerCount, sizeof( struct FarMenuItem ) );
 if ( !menu ) return 1;

 for ( i=0; i<ServerCount ; i++ ) {
    lstrcpy( menu[i].Text, server[i].Name );
    if ( server[i].Default ) { menu[i].Selected = 1; flag=1; }
 }
 if ( !flag ) menu[0].Selected = 1;

 int sel = _Info.Menu( _Info.ModuleNumber,-1, -1, 0,
                       FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,
                       ::GetMsg(MesSelectMailbox_Title), NULL, NULL, NULL,
                       NULL, menu, ServerCount );
 z_free( menu );

 if ( sel == -1 ) return 1;
 current = &server[sel];
 return 0;
}

void LoadLastViewMode ()
{
  HKEY hRoot = HKEY_CURRENT_USER;
  GetRegKey2 (hRoot, PluginCommonKey, NULLSTR, "MailboxesLastViewMode", &Opt.LastViewMode[PLUGIN_PANEL_MAILBOXES], 0);
  GetRegKey2 (hRoot, PluginCommonKey, NULLSTR, "IMAP4FoldersLastViewMode", &Opt.LastViewMode[PLUGIN_PANEL_IMAP4_FOLDERS], 0);
  GetRegKey2 (hRoot, PluginCommonKey, NULLSTR, "POP3LastViewMode", &Opt.LastViewMode[PLUGIN_PANEL_POP3], 0);
  GetRegKey2 (hRoot, PluginCommonKey, NULLSTR, "IMAP4LastViewMode", &Opt.LastViewMode[PLUGIN_PANEL_IMAP4], 0);
}

void SaveLastViewMode ()
{
  HKEY hRoot = HKEY_CURRENT_USER;
  SetRegKey2 (hRoot, PluginCommonKey, NULLSTR, "MailboxesLastViewMode", Opt.LastViewMode[PLUGIN_PANEL_MAILBOXES]);
  SetRegKey2 (hRoot, PluginCommonKey, NULLSTR, "IMAP4FoldersLastViewMode", Opt.LastViewMode[PLUGIN_PANEL_IMAP4_FOLDERS]);
  SetRegKey2 (hRoot, PluginCommonKey, NULLSTR, "POP3LastViewMode", Opt.LastViewMode[PLUGIN_PANEL_POP3]);
  SetRegKey2 (hRoot, PluginCommonKey, NULLSTR, "IMAP4LastViewMode", Opt.LastViewMode[PLUGIN_PANEL_IMAP4]);
}

FARMail::FARMail()
{
  m_pColumnTitles = NULL;
 Level = 0;
 server = NULL;
 ServerCount = 0;

 clnt = NULL;
 imap = NULL;
 FMsmtp = NULL;

 * SelectedMailbox = 0;
 * SelectedServer = 0;
 CharsetTable = NULL;
 savefp=INVALID_HANDLE_VALUE;
 tempfp=INVALID_HANDLE_VALUE;
 *savefp_name = 0;
 *tempfp_name = 0;
 tempfp = NULL;

 if ( !ConstructCharset(&CharsetTable) )
 {
    ReadCharsetTable(CharsetTable);
    InitCharset(&CharsetTable);
 }
 Init();

 LoadLastViewMode ();
}


FARMail::~FARMail()
{
 if ( clnt ) { clnt->Disconnect(); delete (MailClient*)clnt; clnt = NULL; }
 if ( imap ) { imap->Disconnect(); delete (IMAP*)imap; imap = NULL; }
 DestructCharset(&CharsetTable);
 Close();

  if ( m_pColumnTitles )
    z_free (m_pColumnTitles);
}


int SayError( const char *s )
{
 const char *err[3]; //  = { "Error", "", " Ok " };
 err[0] = _Info.GetMsg( _Info.ModuleNumber, MesError );
 err[1] = s;
 err[2] = _Info.GetMsg( _Info.ModuleNumber, MesOk );
 _Info.Message( _Info.ModuleNumber, FMSG_WARNING, NULL, err, 3, 1 );
 return 0;
}



int FARMail::GetUser( char *User, char *Pass )
{
 struct FarDialogItem *ptr;
 int answer;

 ptr = (struct FarDialogItem*)z_calloc( 6, sizeof( struct FarDialogItem ) );
 if ( ptr ) {
    ptr[0].Type = DI_EDIT; ptr[1].Type = DI_PSWEDIT; ptr[2].Type = ptr[3].Type = DI_TEXT;
    ptr[4].Type = DI_BUTTON; ptr[5].Type = DI_SINGLEBOX;

    ptr[0].X1 = ptr[1].X1 = 12; ptr[2].X1 = ptr[3].X1 = 2;
    ptr[0].Y1 = ptr[2].Y1 = 2; ptr[1].Y1 = ptr[3].Y1 = 4;
    ptr[0].X2 = ptr[1].X2 = 42;

    ptr[4].X1 = 18; ptr[4].Y1 = 6;
    ptr[5].X1 = ptr[5].Y1 = 0;
    ptr[5].X2 = 44; ptr[5].Y2 = 7;

    ptr[0].Focus = TRUE;
    ptr[4].DefaultButton = 1;

    lstrcpy(ptr[0].Data , User );
    lstrcpy(ptr[1].Data , Pass );
    lstrcpy(ptr[2].Data , GetMsg(MesGetLogin_User) );
    lstrcpy(ptr[3].Data , GetMsg(MesGetLogin_Pass) );
    lstrcpy(ptr[4].Data , GetMsg(MesOk) );
    lstrcpy(ptr[5].Data , GetMsg(MesGetLogin_Title) );

    if ( !*Pass &&  *User ) {
       ptr[0].Focus = FALSE;
       ptr[1].Focus = TRUE;
    }
    answer = _Info.Dialog( _Info.ModuleNumber, -1, -1, 45, 8 , NULL, ptr, 6 );
    if ( answer > -1 ) {
       lstrcpyn( User, ptr[0].Data , 80 );
       lstrcpyn( Pass, ptr[1].Data , 80 );
       answer = 0;
    }
    z_free( ptr );
    return answer;

 } else return -1;
}



int FARMail::Init()
{
 return ReadServers();
}


int FARMail::Close()
{
 if ( server ) z_free( server );
 return 0;
}



int FARMail::GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
#ifdef TDEBUG
try {
#endif

 PanelInfo pis;
 int mode = 1;

 if ( !Level ) {
    int i;
    PluginPanelItem *NewPanelItem=(PluginPanelItem *)z_realloc(*pPanelItem,(ServerCount+1)*sizeof(PluginPanelItem));

    Cache.ClearCachedData();

    if (NewPanelItem==NULL) return FALSE;
    *pPanelItem=NewPanelItem;
    for ( i=0; i<ServerCount; i++ ) {

       memset(&NewPanelItem[i],0,sizeof(PluginPanelItem));

       NewPanelItem[i].CustomColumnData= (char**)z_calloc( 1, sizeof(char*) ); //new LPSTR[1];

       NewPanelItem[i].CustomColumnData[0]=(char*)z_calloc(1, 80); //new char[80];
       lstrcpy(NewPanelItem[i].CustomColumnData[0], server[i].Url );
       if ( server[i].Default ) lstrcat( NewPanelItem[i].CustomColumnData[0] , " (default)" );

       NewPanelItem[i].CustomColumnNumber=1;

       CharToOem( server[i].Name , NewPanelItem[i].FindData.cFileName);
       NewPanelItem[i].FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;

    }

    *pItemsNumber  = ServerCount;
 }
 else
 {
    if ( current->Type == TYPE_POP3 )
    {

       BOOL res  = FALSE;

       if ( Cache.UseCache() )
       {
          Cache.UseCachedData( pPanelItem, pItemsNumber );
          res = TRUE;
       }

       if ( clnt && clnt->connected && !res )
       {
          ShortMessage *sm = new ShortMessage( MsgListPOP );
          res = clnt->Statistics();
          if ( res )
          {

             if ( !Opt.DisableTOP && clnt->NumberMail )
             {

                res = clnt->List();
                if (res)
                  if (!Opt.DisplayZeroSizeMess)
                    res = clnt->CorrectList();
                if(current->uidl) clnt->Uidl();

             }
             else
             {
                res = TRUE;
             }

             delete sm;

             if ( res ) {

                int i;

                Bar *bar = new Bar(clnt->NumberMail, ::GetMsg(MesConnect_RetrMsgHeaders), PROGRESS_LEN );

                PluginPanelItem *NewPanelItem=(PluginPanelItem *)z_realloc(*pPanelItem,(clnt->NumberMail+2)*sizeof(PluginPanelItem));
                if (NewPanelItem==NULL) { return FALSE; }
                *pPanelItem=NewPanelItem;
                for ( i=0; i<clnt->NumberMail; i++ ) //FIXME: check result of memory allocations
                {
                   memset(&NewPanelItem[i],0,sizeof(PluginPanelItem));

                   NewPanelItem[i].CustomColumnData=(char**)z_calloc( NUM_OF_CUSTOM_COLS, sizeof(char*) );

                   NewPanelItem[i].CustomColumnData[0]=(char*)z_calloc(1,1001);//new char[81]; // frm
                   NewPanelItem[i].CustomColumnData[1]=(char*)z_calloc(1,81);//new char[81]; // date
                   NewPanelItem[i].CustomColumnData[2]=(char*)z_calloc(1,1001);//new char[1001]; // subj

                   lstrcpy(NewPanelItem[i].CustomColumnData[0], QUESTIONMARK );
                   lstrcpy(NewPanelItem[i].CustomColumnData[1], QUESTIONMARK );
                   lstrcpy(NewPanelItem[i].CustomColumnData[2], QUESTIONMARK );


                   if ( !Opt.DisableTOP && clnt->Top( clnt->MessageNums[i] , current->TopValue ) )
                   {
                      char chbf[100], *charsetptr;

                      NewPanelItem[i].CustomColumnData[3]=z_strdup(clnt->GetMsg());
                      {
                        char *ptr=strstr(NewPanelItem[i].CustomColumnData[3],"\r\n\r\n");
                        if (ptr!=NULL)
                        {
                          *(ptr+2)=0;
                        }

                      }

                      GetGeaderField( clnt->GetMsg(), NewPanelItem[i].CustomColumnData[0], FROM, 1000 );
                      GetGeaderField( clnt->GetMsg(), NewPanelItem[i].CustomColumnData[1], _DATE, 80 );
                      GetGeaderField( clnt->GetMsg(), NewPanelItem[i].CustomColumnData[2], SUBJECT, 1000 );
                      GetGeaderField( clnt->GetMsg(), chbf,  CONTENTTYPE, 100 );

                      ConvertDate( NewPanelItem[i].CustomColumnData[1], &NewPanelItem[i].FindData );

                      FSF.LStrupr( chbf );
                      charsetptr = strstr( chbf, "CHARSET" );
                      if ( charsetptr ) {
                         charsetptr += 7;
                         while ( *charsetptr == 32 || *charsetptr == '=' ||
                                 *charsetptr == 9  || *charsetptr == '\"' ) charsetptr++;
                      } else {
                         char xsun[100];
                         *xsun = 0;
                         GetGeaderField( clnt->GetMsg(), xsun, "X-Sun-Text-Type:", 100 );
                         if ( *xsun ) {
                            charsetptr = xsun;
                            while ( *charsetptr == 32 ||
                                    *charsetptr == 9  || *charsetptr == '\"' ) charsetptr++;
                         }
                      }
                      if ( !charsetptr && *Opt.DefCharset ) charsetptr = Opt.DefCharset;

                      DecodeSubj(NewPanelItem[i].CustomColumnData[2], charsetptr );
                      DecodeSubj(NewPanelItem[i].CustomColumnData[1], charsetptr );
                      DecodeSubj(NewPanelItem[i].CustomColumnData[0], charsetptr );
                   } else  {
                      NewPanelItem[i].CustomColumnData[3]=z_strdup(NULLSTR);
                   }

                   if(clnt->MessageUidls[i])
                   {
                     NewPanelItem[i].CustomColumnData[4]=z_strdup(clnt->MessageUidls[i]);
                     if(lstrlen(NewPanelItem[i].CustomColumnData[4]))
                     {
                       NewPanelItem[i].UserData=GetUidlState(NewPanelItem[i].CustomColumnData[4]); //FIXME
                     }
                   }
                   else
                     NewPanelItem[i].CustomColumnData[4]=z_strdup(NULLSTR);

                   NewPanelItem[i].CustomColumnNumber=5;

                   if(current->uidl) NewPanelItem[i].FindData.dwFileAttributes=MapStateToAttribute(NewPanelItem[i].UserData);
                   NewPanelItem[i].FindData.nFileSizeLow = (Opt.DisableTOP ? 0 : clnt->MessageLens[i]);

                   GenerateName((Opt.DisableTOP ? i+1 : clnt->MessageNums[i]), NewPanelItem[i].FindData.cFileName);

                   if ( bar ) bar->UseBar(i+1);
                }
                *pItemsNumber=clnt->NumberMail+(Opt.UseAttrHighlighting?1:0);

                if(Opt.UseAttrHighlighting)
                {
                   memset(&NewPanelItem[clnt->NumberMail],0,sizeof(PluginPanelItem));
                   lstrcpy(NewPanelItem[clnt->NumberMail].FindData.cFileName,"..");
                   NewPanelItem[clnt->NumberMail].CustomColumnData=(char**)z_calloc( NUM_OF_CUSTOM_COLS, sizeof(char*) );
                }

                Cache.LoadCachedData(*pPanelItem,*pItemsNumber,current->MailboxPath);

                if ( bar ) delete bar;

             } else { if ( !(OpMode & OPM_FIND) ) SayError( clnt->GetErrorMessage() ); }
          } else { delete sm; if ( !(OpMode & OPM_FIND) ) SayError( clnt->GetErrorMessage() ); }
       } else if ( !clnt || !clnt->connected ) res = FALSE;

       if ( !res ) {
          PluginPanelItem *NewPanelItem=(PluginPanelItem *)z_realloc(*pPanelItem,sizeof(PluginPanelItem));
          if (NewPanelItem==NULL) return FALSE;
          *pPanelItem=NewPanelItem;
          *pItemsNumber  = 0;

       }

       _Info.Control (this, FCTL_SETVIEWMODE, &Opt.LastViewMode[PLUGIN_PANEL_POP3]);
    } else {

       int stat = 1, i;

       if ( Level == 1 ) {

          Cache.ClearCachedData();

          if ( imap && imap->connected ) {
             ShortMessage *sm = new ShortMessage( MsgIMAPList );

             int folders = 0, old_folders, select_result;
             int tok = 0;
             const char *mb ;

             while ( ( mb = get_token( Opt.IMAP_Inbox, tok++ , 1 ) ) != NULL && *mb ) {
                stat = imap->List( "\"\"", mb );
                if ( !stat ) {

                   int line = 0;

                   old_folders = folders;
                   select_result = 0;

                   while ( imap->GetRespString( line++ ) ) {

                      if ( imap->GetRespToken(0) && !lstrcmp( imap->RespString2, ASTERISK ) ) {
                         if ( imap->GetRespToken(1) && !FSF.LStricmp( imap->RespString2, LIST ) ) {
                            if ( imap->GetRespToken(4) ) {
                               folders++;
                               select_result++;
                            }
                         }
                      }
                   }


                   PluginPanelItem *NewPanelItem=(PluginPanelItem *)z_realloc(*pPanelItem,(folders+1)*sizeof(PluginPanelItem));
                   if (NewPanelItem==NULL) { delete sm; return FALSE; }
                   *pPanelItem=NewPanelItem;

                   line = 0;
                   i = old_folders;

                   while ( imap->GetRespString( line++ ) ) {

                      if ( imap->GetRespToken(0) && !lstrcmp( imap->RespString2, ASTERISK ) ) {
                         if ( imap->GetRespToken(1) && !FSF.LStricmp( imap->RespString2, LIST ) ) {
                            if ( imap->GetRespToken(4) ) {
                               imap->UnquotString();

                               memset(&NewPanelItem[i],0,sizeof(PluginPanelItem));

                               NewPanelItem[i].CustomColumnData=(char**)z_calloc( NUM_OF_CUSTOM_COLS, sizeof(char*) );

                               NewPanelItem[i].CustomColumnData[0]=(char*)z_calloc(1,10);//new char[10];
                               NewPanelItem[i].CustomColumnData[1]=(char*)z_calloc(1,10);//new char[10];
                               NewPanelItem[i].CustomColumnData[2]=(char*)z_calloc(1,10);//new char[10];
                               NewPanelItem[i].CustomColumnData[3]=(char*)z_calloc(1,10);//new char[10];

                               *(NewPanelItem[i].CustomColumnData[0]) = 0;
                               lstrcpy(NewPanelItem[i].CustomColumnData[1], QUESTIONMARK );
                               lstrcpy(NewPanelItem[i].CustomColumnData[2], QUESTIONMARK );
                               lstrcpy(NewPanelItem[i].CustomColumnData[3], QUESTIONMARK );

                               NewPanelItem[i].CustomColumnNumber=4;

                               lstrcpy( NewPanelItem[i].FindData.cFileName, imap->RespString2 );
                               NewPanelItem[i].FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;

                               if ( imap->GetRespToken(2) ) {
                                  FSF.LStrupr( imap->RespString2 );

                                  if ( strstr( imap->RespString2 , "\\MARKED" ) )
                                     lstrcat( NewPanelItem[i].CustomColumnData[0], "!" );
                                  else
                                     lstrcat( NewPanelItem[i].CustomColumnData[0], "ú" );

                                  if ( strstr( imap->RespString2 , "\\NOSELECT" ) )
                                     lstrcat( NewPanelItem[i].CustomColumnData[0], "N" );
                                  else
                                     lstrcat( NewPanelItem[i].CustomColumnData[0], "ú" );
                               }

                               i++;
                            }
                         }
                      }
                   }
                   *pItemsNumber  = i;

                   for ( i=0 ; i<select_result ; i++ ) {
                      if ( !imap->Status( NewPanelItem[i+old_folders].FindData.cFileName, "(MESSAGES RECENT UNSEEN)" ) ) {
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
                                                 lstrcpy( NewPanelItem[i+old_folders].CustomColumnData[1] , imap->RespString2 );
                                              else
                                                 break;
                                           }

                                           if ( !lstrcmp( imap->RespString2, RECENT ) ) {
                                              if ( imap->GetRespToken( token++ ) )
                                                 lstrcpy( NewPanelItem[i+old_folders].CustomColumnData[2] , imap->RespString2 );
                                              else
                                                 break;
                                           }

                                           if ( !lstrcmp( imap->RespString2, UNSEEN ) ) {
                                              if ( imap->GetRespToken( token++ ) )
                                                 lstrcpy( NewPanelItem[i+old_folders].CustomColumnData[3] , imap->RespString2 );
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

                } else { if ( !(OpMode & OPM_FIND) ) SayError( imap->GetErrorMessage() ); }
             }

             delete sm;
          } else stat = 1;

          _Info.Control( this, FCTL_SETVIEWMODE, (void*)&Opt.LastViewMode[PLUGIN_PANEL_IMAP4_FOLDERS]);
       } else {

          if ( Cache.UseCache() ) {
             Cache.UseCachedData( pPanelItem, pItemsNumber );
             stat = 0;
          } else {

             ShortMessage *sm = new ShortMessage( MsgListPOP );

             imap->Noop();

             if ( !imap->Status( IMAP_Mailbox, BRACED_MESSAGES ) ) {
                int line = 0;
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

             stat = 0;

             *pItemsNumber  = imap->MessageNumber;
             PluginPanelItem *NewPanelItem=(PluginPanelItem *)z_realloc(*pPanelItem,(imap->MessageNumber+1)*sizeof(PluginPanelItem));
             if (NewPanelItem==NULL) { return FALSE; }
             *pPanelItem=NewPanelItem;

             Bar *bar = new Bar ( imap->MessageNumber, ::GetMsg(MesConnect_RetrMsgHeaders), PROGRESS_LEN );

             for ( i=0 ; i<imap->MessageNumber; i++ ) {

                memset(&NewPanelItem[i],0,sizeof(PluginPanelItem));

                NewPanelItem[i].CustomColumnData=(char**)z_calloc( NUM_OF_CUSTOM_COLS , sizeof(char*) );

                NewPanelItem[i].CustomColumnData[0]=(char*)z_calloc(1,1001);//new char[81]; // frm
                NewPanelItem[i].CustomColumnData[1]=(char*)z_calloc(1,81);//new char[81]; // date
                NewPanelItem[i].CustomColumnData[2]=(char*)z_calloc(1,1001);//new char[1001]; // subj
                NewPanelItem[i].CustomColumnData[3]=(char*)z_calloc(1,41);//new char[41]; // uid

                lstrcpy(NewPanelItem[i].CustomColumnData[0], QUESTIONMARK );
                lstrcpy(NewPanelItem[i].CustomColumnData[1], QUESTIONMARK );
                lstrcpy(NewPanelItem[i].CustomColumnData[2], QUESTIONMARK );
                lstrcpy(NewPanelItem[i].CustomColumnData[3], "0" );

                NewPanelItem[i].CustomColumnNumber=5;
                NewPanelItem[i].FindData.dwFileAttributes=FILE_ATTRIBUTE_NORMAL;
                NewPanelItem[i].FindData.nFileSizeLow = 0;

                if ( !imap->Fetch( i+1, RFC822SIZE ,0,0,NULL ) ) {
                   int line = 0;
                   while ( imap->GetRespString( line++) ) {
                      if ( imap->GetRespToken(0) && !lstrcmp( imap->RespString2, ASTERISK ) ) {
                         if ( imap->GetRespToken(1) && i+1==FSF.atoi(imap->RespString2) ) {
                            if ( imap->GetRespToken(2) && !lstrcmp( imap->RespString2, FETCH ) ) {
                               imap->GetRespToken(3);
                               lstrcpy( imap->RespString, imap->RespString2 );
                               FSF.LStrupr( imap->RespString );
                               if ( imap->GetRespToken(0) && !lstrcmp( imap->RespString2, RFC822SIZE ) ) {
                                  if ( imap->GetRespToken(1) ) {
                                      NewPanelItem[i].FindData.nFileSizeLow = FSF.atoi(imap->RespString2);
                                  }
                               }
                            }
                         }
                      }
                   }
                }
                if ( !imap->Fetch( i+1, RFC822HEADER ,0,0,NULL) ) {
                   char chbf[100], *charsetptr;

                   NewPanelItem[i].CustomColumnData[4] = z_strdup(imap->GetMsg());

                   GetGeaderField( imap->GetMsg(), NewPanelItem[i].CustomColumnData[0], FROM, 1000 );
                   GetGeaderField( imap->GetMsg(), NewPanelItem[i].CustomColumnData[1], _DATE, 80 );
                   GetGeaderField( imap->GetMsg(), NewPanelItem[i].CustomColumnData[2], SUBJECT, 1000 );
                   GetGeaderField( imap->GetMsg(), chbf,  CONTENTTYPE, 100 );

                   ConvertDate( NewPanelItem[i].CustomColumnData[1], &NewPanelItem[i].FindData );

                   FSF.LStrupr( chbf );
                   charsetptr = strstr( chbf, "CHARSET" );
                   if ( charsetptr ) {
                      charsetptr += 7;
                      while ( *charsetptr == 32 || *charsetptr == '=' ||
                           *charsetptr == 9  || *charsetptr == '\"' ) charsetptr++;
                   } else {
                      char xsun[100];
                      *xsun = 0;
                      GetGeaderField( imap->GetMsg(), xsun, "X-Sun-Text-Type:", 100 );
                      if ( *xsun ) {
                         charsetptr = xsun;
                         while ( *charsetptr == 32 ||
                                 *charsetptr == 9  || *charsetptr == '\"' ) charsetptr++;
                      }
                   }
                   if ( !charsetptr && *Opt.DefCharset ) charsetptr = Opt.DefCharset;

                   DecodeSubj(NewPanelItem[i].CustomColumnData[2], charsetptr );
                   DecodeSubj(NewPanelItem[i].CustomColumnData[1], charsetptr );
                   DecodeSubj(NewPanelItem[i].CustomColumnData[0], charsetptr );

                } else {
                   NewPanelItem[i].CustomColumnData[4] = z_strdup(NULLSTR);
                }
                if ( !imap->Fetch( i+1, UID ,0,0,NULL) ) {
                   int line = 0;
                   while ( imap->GetRespString( line++) ) {
                      if ( imap->GetRespToken(0) && !lstrcmp( imap->RespString2, ASTERISK ) ) {
                         if ( imap->GetRespToken(1) && i+1==FSF.atoi(imap->RespString2) ) {
                            if ( imap->GetRespToken(2) && !lstrcmp( imap->RespString2, FETCH ) ) {
                               imap->GetRespToken(3);
                               lstrcpy( imap->RespString, imap->RespString2 );
                               FSF.LStrupr( imap->RespString );
                               if ( imap->GetRespToken(0) && !lstrcmp( imap->RespString2, UID ) ) {
                                  if ( imap->GetRespToken(1) ) {
                                     lstrcpy(NewPanelItem[i].CustomColumnData[3], imap->RespString2 );
                                  }
                               }
                            }
                         }
                      }
                   }

                }

                GenerateName(i+1,NewPanelItem[i].FindData.cFileName);

                if ( bar ) bar->UseBar(i+1);
             }
             Cache.LoadCachedData(*pPanelItem,*pItemsNumber,NULLSTR);
             // delete sm;
             delete bar;
          }

          _Info.Control( this, FCTL_SETVIEWMODE, (void*)&Opt.LastViewMode[PLUGIN_PANEL_IMAP4]);

       }
       if ( stat ) {
          PluginPanelItem *NewPanelItem=(PluginPanelItem *)z_realloc(*pPanelItem,sizeof(PluginPanelItem));
          if (NewPanelItem==NULL) return FALSE;
          *pPanelItem=NewPanelItem;
          *pItemsNumber  = 0;
       }
    }
 }
 return TRUE;
#ifdef TDEBUG
}
#ifdef __BORLANDC__
__except(1)
#else
catch(...)
#endif
{
 SayException( "GetFindData" );
 return FALSE;
}
#endif
}




void FARMail::FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber)
{
#ifdef TDEBUG
try {
#endif
  if ( !Level ) {
     if ( PanelItem ) {
        for (int I=0;I<ItemsNumber;I++)
        {
          if ( PanelItem[I].CustomColumnData ) {
             if ( PanelItem[I].CustomColumnData[0] ) z_free( PanelItem[I].CustomColumnData[0] );
             z_free( PanelItem[I].CustomColumnData );
          }
        }
        z_free( PanelItem );
     }
  }
  else {
     if ( PanelItem ) {
        for (int I=0;I<ItemsNumber;I++)
        {
          if (PanelItem[I].CustomColumnData ) {
             if ( PanelItem[I].CustomColumnData[0] ) z_free( PanelItem[I].CustomColumnData[0]);
             if ( current->Type == TYPE_POP3 ) {
                if ( PanelItem[I].CustomColumnData[1] ) z_free( PanelItem[I].CustomColumnData[1]);
                if ( PanelItem[I].CustomColumnData[2] ) z_free( PanelItem[I].CustomColumnData[2]);
                if ( PanelItem[I].CustomColumnData[3] ) z_free( PanelItem[I].CustomColumnData[3]);
                if ( PanelItem[I].CustomColumnData[4] ) z_free( PanelItem[I].CustomColumnData[4]);

             } else if ( Level == 1 || Level == 2 ) {
                if ( PanelItem[I].CustomColumnData[1] ) z_free( PanelItem[I].CustomColumnData[1]);
                if ( PanelItem[I].CustomColumnData[2] ) z_free( PanelItem[I].CustomColumnData[2]);
                if ( PanelItem[I].CustomColumnData[3] ) z_free( PanelItem[I].CustomColumnData[3]);
                if ( Level == 2 ) {
                   if ( PanelItem[I].CustomColumnData[4] ) z_free( PanelItem[I].CustomColumnData[4] );
                }
             }
             z_free( PanelItem[I].CustomColumnData );
          }
        }
        z_free( PanelItem );
     }
  }
#ifdef TDEBUG
}
#ifdef __BORLANDC__
__except(1)
#else
catch(...)
#endif
{
  SayException( "FreeFindData" );
}
#endif
}




int FARMail::DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
#ifdef TDEBUG
try {
#endif

  if ( !Level )
  {
    Cache.ClearCachedData();
    const char *mes[4];
    int i;
    mes[0] = GetMsg(MesDeleteMailbox);
    mes[1] = GetMsg(MesDeleteMailboxConfirm);
    mes[2] = GetMsg(MesOk);
    mes[3] = GetMsg(MesCancel);

    int answer = _Info.Message( _Info.ModuleNumber, 0, NULL, mes, 4, 2 );

    if ( answer == 0 )
    {
      for (i=0;i<ItemsNumber;i++)
        DeleteMailbox( PanelItem[i].FindData.cFileName );

      ServerCount = 0;
      if ( server ) z_free(server);
      ReadServers();
      _Info.Control( this, FCTL_UPDATEPANEL, NULL );
      _Info.Control( this, FCTL_REDRAWPANEL, NULL );

    }
    else return FALSE;

    return TRUE;
  }

  if ( current->Type == TYPE_POP3 || ( current->Type == TYPE_IMAP4 && Level > 1 ) )
  {
    if (current->Type != TYPE_POP3)
      Cache.ClearCachedData();
    const char *err[5] ;
    int i,num,answer, all=0;
    char buf[100];

    err[0] = GetMsg(MesDelete_Title);
    err[2] = NULLSTR;
    err[3] = GetMsg(MesDelete_DeleteBtn);
    err[4] = GetMsg(MesDelete_CancelBtn);

    FSF.sprintf( buf, GetMsg(MesDelete_Question) , ItemsNumber );
    err[1] = buf;

    for (i=0;i<ItemsNumber;i++)
    {
      if (PanelItem[i].UserData&MESSAGE_STATE_DELETED)
        continue;
      num = FSF.atoi( PanelItem[i].FindData.cFileName );
      if ( num && ( current->Type == TYPE_POP3 && clnt && clnt->connected ) || ( imap && imap->connected ) )
      {
        if ( !all && !( OpMode & OPM_SILENT ))
          answer = _Info.Message( _Info.ModuleNumber, 0, NULL, err, 5, 2 );
        if ( answer == -1 || answer == 1 )
          goto EXPUNGE;
        all = 1;

        BOOL res = ( current->Type == TYPE_POP3? clnt->Delete( num ) : (!imap->DeleteUID(PanelItem[i].CustomColumnData[3])) );
        if ( res == FALSE )
        {
          if (!(OpMode && OPM_SILENT))
            SayError( current->Type == TYPE_POP3? clnt->GetErrorMessage() : imap->GetErrorMessage() );
          goto EXPUNGE;
        }
        else if (current->Type==TYPE_POP3) Cache.MarkMessage(num,MESSAGE_STATE_DELETED);
      }
    }
    if ( current->Type == TYPE_IMAP4 && imap && imap->connected )
    {
      if ( imap->Expunge() && !(OpMode && OPM_SILENT ) )
        SayError( imap->GetErrorMessage() );
    }
    if (current->Type == TYPE_POP3)
    {
      _Info.Control(this,FCTL_UPDATEPANEL,NULL);
      _Info.Control(this,FCTL_REDRAWPANEL,NULL);
    }
    return TRUE;
EXPUNGE:
    if ( current->Type == TYPE_IMAP4 && imap && imap->connected )
    {
      if ( imap->Expunge() && !(OpMode && OPM_SILENT ) )
        SayError( imap->GetErrorMessage() );
    }
    if (current->Type == TYPE_POP3)
    {
      _Info.Control(this,FCTL_UPDATEPANEL,NULL);
      _Info.Control(this,FCTL_REDRAWPANEL,NULL);
    }
    return FALSE;
  }
  else
  {
    Cache.ClearCachedData();
    if ( imap && imap->connected )
    {
      const char *mes[4];
      int i;
      mes[0] = GetMsg(MesDeleteDir1);
      mes[1] = GetMsg(MesDeleteDir2);
      mes[2] = GetMsg(MesOk);
      mes[3] = GetMsg(MesCancel);

      int answer = _Info.Message( _Info.ModuleNumber, 0, NULL, mes, 4, 2 );

      if ( answer == 0 )
      {
        for (i=0;i<ItemsNumber;i++)
        {
          if ( imap->DeleteBox( PanelItem[i].FindData.cFileName ) )
          {
            if ( !(OpMode && OPM_SILENT ) ) SayError( imap->GetErrorMessage() );
            return FALSE;
          }
        }
      }
      else return FALSE;
    }
    return TRUE;
  }
#ifdef TDEBUG
}
#ifdef __BORLANDC__
__except(1)
#else
catch(...)
#endif
{
 SayException( "DeleteFiles" );
 return FALSE;
}
#endif
}

const char *GetMailboxesTitle (const char *lpType)
{
  if ( strstr (lpType, "N") )
    return GetMsg(MesMailboxName_Panel);

  if ( strstr (lpType, "C0") )
    return GetMsg(MesServerName_Panel);

  return NULL;
}

const char *GetIMAP4FoldersTitle (const char *lpType)
{
  if ( strstr (lpType, "N") )
    return GetMsg(MesIMAPPanelMailbox);

  if ( strstr (lpType, "C0") )
    return GetMsg(MesIMAPPanelFlags);

  if ( strstr (lpType, "C1") )
    return GetMsg(MesIMAPPanelMessages);

  if ( strstr (lpType, "C2") )
    return GetMsg(MesIMAPPanelRecent);

  if ( strstr (lpType, "C3") )
    return GetMsg(MesIMAPPanelUnseen);

  return NULL;
}

const char *GetIMAP4Title (const char *lpType)
{
  if ( strstr (lpType, "N") )
    return GetMsg(MesMessageName_Panel);

  if ( strstr (lpType, "S") )
    return GetMsg(MesSizeName_Panel);

  if ( strstr (lpType, "C0") )
    return GetMsg(MesFromName_Panel);

  if ( strstr (lpType, "C1") )
    return GetMsg(MesDateName_Panel);

  if ( strstr (lpType, "C2") )
    return GetMsg(MesSubjName_Panel);

  return NULL;
}

const char *GetPOP3Title (const char *lpType)
{
  if ( strstr (lpType, "N") )
    return GetMsg(MesMessageName_Panel);

  if ( strstr (lpType, "S") )
    return GetMsg(MesSizeName_Panel);

  if ( strstr (lpType, "C0") )
    return GetMsg(MesFromName_Panel);

  if ( strstr (lpType, "C1") )
    return GetMsg(MesDateName_Panel);

  if ( strstr (lpType, "C2") )
    return GetMsg(MesSubjName_Panel);

  return NULL;
}

char **GetPanelTitles (
    const char *lpUserColumnTypes,
    int nPanel
    )
{
  int nTokens = 0;

  char *lpTokens = z_strdup (lpUserColumnTypes);
  char *lpToken = strtok (lpTokens, ",");

  while ( lpToken && *lpToken )
  {
    nTokens++;
    lpToken = strtok (NULL, ",");
  }

  z_free (lpTokens);

  char **pColumnTitles = NULL;

  if ( nTokens )
  {
    pColumnTitles = (char**)z_malloc (nTokens*4);

      lpTokens = z_strdup (lpUserColumnTypes);
    lpToken = strtok (lpTokens, ",");

    nTokens = 0;

    while ( lpToken && *lpToken )
    {
        FSF.Trim (lpToken);

      if ( nPanel == PLUGIN_PANEL_MAILBOXES )
        pColumnTitles[nTokens] = (char*)GetMailboxesTitle (lpToken);

      if ( nPanel == PLUGIN_PANEL_IMAP4_FOLDERS )
        pColumnTitles[nTokens] = (char*)GetIMAP4FoldersTitle (lpToken);

      if ( nPanel == PLUGIN_PANEL_IMAP4 )
        pColumnTitles[nTokens] = (char*)GetIMAP4Title (lpToken);

      if ( nPanel == PLUGIN_PANEL_POP3 )
        pColumnTitles[nTokens] = (char*)GetPOP3Title (lpToken);

      nTokens++;

      lpToken = strtok (NULL, ",");
    }

    z_free (lpTokens);
  }

  return pColumnTitles;
}



void FARMail::GetOpenPluginInfo(struct OpenPluginInfo *Info)
{
  static char Title[100];
  static struct KeyBarTitles kb =
  {
    { NULL, NULL, NULL, NULL, NULL, NULL, NULLSTR,   NULL, NULL, NULL, NULL, NULL} ,
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL} , // ctrl
    { NULL, NULL, NULL, NULL, NULL, NULLSTR,   NULL, NULL, NULL, NULL, NULL, NULL} , // alt
    { NULLSTR  , NULLSTR  , NULLSTR  , NULL, NULL, NULL, NULLSTR,   NULL, NULL, NULL, NULL, NULL}   // shift
  };

  static struct KeyBarTitles mb =
  {
    { NULL, NULL, NULLSTR  , NULL, NULLSTR  , NULLSTR  , NULLSTR,   NULL, NULL, NULL, NULL, NULL} ,
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL} , // ctrl
    { NULL, NULL, NULLSTR  , NULLSTR  , NULLSTR  , NULLSTR,   NULL, NULL, NULL, NULL, NULL, NULL} , // alt
    { NULLSTR  , NULLSTR  , NULLSTR  , NULL, NULLSTR  , NULLSTR  , NULLSTR,   NULLSTR,   NULL, NULL, NULL, NULL}   // shift
  };

  static struct KeyBarTitles ikb =
  {
    { NULL, NULL, NULLSTR  , NULLSTR  , NULLSTR  , NULL, NULL, NULL, NULL, NULL, NULL, NULL} ,
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL} , // ctrl
    { NULL, NULL, NULLSTR  , NULLSTR  , NULLSTR  , NULLSTR,   NULL, NULL, NULL, NULL, NULL, NULL} , // alt
    { NULLSTR  , NULLSTR  , NULLSTR  , NULLSTR  , NULLSTR  , NULLSTR  , NULLSTR,   NULLSTR  , NULL, NULL, NULL, NULL}   // shift
  };

  kb.Titles[6] = GetMsg(MesKeybar_Undel);
  kb.ShiftTitles[6] = GetMsg(MesKeybar_Reconnect);

  mb.ShiftTitles[3] = GetMsg(MesKeybar_NewMailbox );
  mb.ShiftTitles[7] = GetMsg(MesKeybar_FastDelete );
  mb.ShiftTitles[0] = kb.ShiftTitles[0] = ikb.ShiftTitles[0] = GetMsg(MesKeybar_AdrBook);
  mb.Titles[2] = GetMsg(MesKeybar_Status);
  mb.Titles[4] = GetMsg(MesKeybar_FastCopy);
  mb.Titles[5] = GetMsg(MesKeybar_FastMove);
  mb.Titles[6] = GetMsg(MesKeybar_Version);

  kb.ShiftTitles[2] = GetMsg(MesKeybar_QuickView);
  kb.ShiftTitles[1] = GetMsg(MesKeybar_QuickEdit);
  kb.AltTitles[5] = GetMsg(MesKeybar_Filter);

  Info->StructSize=sizeof(*Info);
  //FIXME
  Info->Flags=((Level&&current->Type==TYPE_POP3&&Opt.UseAttrHighlighting)?OPIF_USEATTRHIGHLIGHTING:OPIF_USEHIGHLIGHTING)|
              OPIF_ADDDOTS|OPIF_RAWSELECTION|
              OPIF_SHOWNAMESONLY|OPIF_SHOWPRESERVECASE;

  Info->HostFile=NULL;

  if ( *SelectedMailbox )
    Info->CurDir = SelectedMailbox;
  else
    Info->CurDir = SelectedServer;

  Info->Format=GetMsg(MesRootPanelTitle); // POP3;

  if ( * Info->CurDir )
    FSF.sprintf(Title," %s ", Info->CurDir);
  else
    FSF.sprintf(Title," %s ", GetMsg(MesRootPanelTitle) );

  Info->PanelTitle=Title;

  Info->InfoLines=NULL;
  Info->InfoLinesNumber=0;

  Info->DescrFiles=NULL;
  Info->DescrFilesNumber=0;

  static struct PanelMode PanelModesArray[10];

  memset (&PanelModesArray, 0, sizeof (PanelModesArray));

  static const char *ColumnTitles[2];
  ColumnTitles[0]= GetMsg(MesMailboxName_Panel); // "Mailbox name";
  ColumnTitles[1]= GetMsg(MesServerName_Panel); // "Server";

  static const char *ColumnTitles2[5];
  FillColumns( ColumnTitles2, "N,S,C0,C1,C2" );

  static const char *ColumnTitles3[5];
  ColumnTitles3[0]= GetMsg(MesIMAPPanelMailbox); // "Mailbox name";
  ColumnTitles3[1]= GetMsg(MesIMAPPanelFlags);   // "flags";
  ColumnTitles3[2]= GetMsg(MesIMAPPanelMessages);
  ColumnTitles3[3]= GetMsg(MesIMAPPanelRecent);
  ColumnTitles3[4]= GetMsg(MesIMAPPanelUnseen);

  int nPanel;

  if ( !Level )
    nPanel = PLUGIN_PANEL_MAILBOXES;
  else
  {
    if ( current && current->Type == TYPE_IMAP4 )
    {
      if ( Level == 1 )
        nPanel = PLUGIN_PANEL_IMAP4_FOLDERS;
      else
        nPanel = PLUGIN_PANEL_IMAP4;
    }
    else
      nPanel = PLUGIN_PANEL_POP3;
  };

  PanelInfo pnInfo;

  _Info.Control (this, FCTL_GETPANELSHORTINFO, &pnInfo);

  if ( m_pColumnTitles )
    z_free (m_pColumnTitles);

  m_pColumnTitles = GetPanelTitles (Opt.Modes[nPanel][pnInfo.ViewMode].lpColumnTypes, nPanel);

  for (int i = 0; i < 10; i++)
  {
     FSF.Trim (Opt.Modes[nPanel][i].lpColumnTypes);
     FSF.Trim (Opt.Modes[nPanel][i].lpColumnWidths);
     FSF.Trim (Opt.Modes[nPanel][i].lpStatusColumnTypes);
     FSF.Trim (Opt.Modes[nPanel][i].lpStatusColumnWidths);

     PanelModesArray[i].ColumnTypes = lstrlen(Opt.Modes[nPanel][i].lpColumnTypes)?Opt.Modes[nPanel][i].lpColumnTypes:NULL;
     PanelModesArray[i].ColumnWidths = lstrlen(Opt.Modes[nPanel][i].lpColumnWidths)?Opt.Modes[nPanel][i].lpColumnWidths:NULLSTR;
     PanelModesArray[i].StatusColumnTypes = lstrlen(Opt.Modes[nPanel][i].lpStatusColumnTypes)?Opt.Modes[nPanel][i].lpStatusColumnTypes:NULL;
     PanelModesArray[i].StatusColumnWidths = lstrlen(Opt.Modes[nPanel][i].lpStatusColumnWidths)?Opt.Modes[nPanel][i].lpStatusColumnWidths:NULLSTR;
     PanelModesArray[i].FullScreen = Opt.Modes[nPanel][i].bFullScreen;
     PanelModesArray[i].ColumnTitles = (const char**)m_pColumnTitles;
  }

  Info->PanelModesArray=PanelModesArray;
  Info->PanelModesNumber=sizeof(PanelModesArray)/sizeof(PanelModesArray[0]);
  Info->StartPanelMode='0'+Opt.LastViewMode[PLUGIN_PANEL_MAILBOXES];
  Info->StartSortMode=SM_NAME;
  Info->StartSortOrder=0;

  if ( !Level )
    Info->KeyBar = &mb;
  else if (*Info->CurDir)
  {
    if (current && current->Type == TYPE_IMAP4 && Level==1)
      Info->KeyBar = &ikb;
    else
      Info->KeyBar = &kb;
  }
}


static const char UPDIR[] = "..";

int FARMail::SetDirectory(const char *Dir,int OpMode)
{
#ifdef TDEBUG
try {
#endif
  BOOL res;
  char savb[100];

  if ( (OpMode & OPM_FIND)  || !lstrcmp(Dir,BACKSLASH) )   return(FALSE);

  Cache.ClearCachedData();

  lstrcpy( savb, SelectedServer );

  if ( lstrcmp( Dir, UPDIR) ) {
     if ( !Level )
        lstrcpy( SelectedServer, Dir );
     else
        lstrcpy( SelectedMailbox, Dir );
  }
  else {
     if ( Level <= 1 )
        *SelectedServer = 0;
     else
        *SelectedMailbox = 0;
  }

  if (!lstrcmp(Dir,UPDIR)) {
    if (Level==1)  {
      int mode = 0;
      Level = 0;
      if ( clnt ) { clnt->Disconnect(); delete (MailClient*)clnt; clnt = NULL; }
      if ( imap ) { imap->Disconnect(); delete (IMAP*)imap; imap = NULL; }
      current = NULL;
      _Info.Control( this, FCTL_SETVIEWMODE, (void*)&Opt.LastViewMode[PLUGIN_PANEL_MAILBOXES] );
      return(TRUE);
    } else if ( Level > 1 && current->Type == TYPE_IMAP4 && imap ) {
      if ( !imap->Close() ) {
         Level--;
         return TRUE;

      } else if ( !(OpMode & OPM_FIND) ) SayError( imap->GetErrorMessage() );

      Level = 0;
      imap->Disconnect();
      delete (IMAP*)imap;
      imap = NULL;
      *SelectedServer = 0;
      *SelectedMailbox = 0;
      current = NULL;
      int mode = 0;
      _Info.Control( this, FCTL_SETVIEWMODE, (void*)&Opt.LastViewMode[PLUGIN_PANEL_MAILBOXES] );

      return FALSE;
    } else {
      _Info.Control(this,FCTL_CLOSEPLUGIN,(void *)Dir);
      return(TRUE);
    }
  }
  else if ( !Level ) {
    int i; // mode = 1,i;

    Level = 1;
    current = NULL;

    for ( i=0 ; i<ServerCount ; i++ )
    {
       if ( !lstrcmp( Dir, server[i].Name ) )
       {
          if (*server[i].Url)
            current = &server[i];
          break;
       }
    }
    if ( current ) {
       if ( (*current->User && *current->Pass) || !GetUser(current->_User,current->_Pass) ) {

         if ( current->Type == TYPE_POP3 ) {
            clnt = new MailClient( (Opt.DebugSession?TRUE:FALSE) , Opt.LOGFILE , current->timeout*60 );
            if ( clnt && current )
            {
#ifdef FARMAIL_SSL
               res = clnt->Connect( current->Url, current->_User, current->_Pass, current->Port, (current->UsePOP3S?CON_SSL:CON_NORMAL) );
#else
               res = clnt->Connect( current->Url, current->_User, current->_Pass, current->Port);
#endif
               if ( !res && !(OpMode & OPM_FIND) ) {
                  SayError( clnt->GetErrorMessage() );
               }
            }
            if ( !res ) {
               // stop changing directory!
               if ( clnt ) { clnt->Disconnect(); delete (MailClient*)clnt; clnt = NULL; }
               *SelectedServer = 0;
               current = NULL;
               Level = 0;
               return FALSE;
            }
         } else {

            imap = new IMAP( (Opt.DebugSession?TRUE:FALSE) , Opt.LOGFILE3 , current->timeout*60 );
            if ( imap && current )
            {
#ifdef FARMAIL_SSL
               res = imap->Connect( current->Url, current->Port, (current->UsePOP3S?CON_SSL:CON_NORMAL) );
#else
               res = imap->Connect( current->Url, current->Port);
#endif
               if ( res && !(OpMode & OPM_FIND) ) {
                  SayError( imap->GetErrorMessage() );
               } else if ( !res ) {
                  res = imap->Login( current->_User, current->_Pass  );
                  if ( !res ) {
                     res = imap->Capability();
                     if ( !res ) {

                     } else if ( !(OpMode & OPM_FIND) ) {
                        SayError( imap->GetErrorMessage() );
                     }
                  } else if ( !(OpMode & OPM_FIND) ) {
                     SayError( imap->GetErrorMessage() );
                  }
               }
            }
            if ( res ) {
               if ( imap ) { imap->Disconnect(); delete (IMAP*)imap; imap = NULL; }
               *SelectedServer = 0;
               current = NULL;
               Level = 0;
               return FALSE;
            }

         }

       } else {
         *SelectedServer = 0;
         current = NULL;
         Level = 0;
         return FALSE;
       }
    } else {
       *SelectedServer = 0;
       Level = 0;
       return FALSE;
    }
  } else {
       if ( current->Type == TYPE_IMAP4 && imap ) {
          int stat = imap->Select( Dir );
          if ( !stat ) {
             Level++;
             lstrcpy( IMAP_Mailbox, Dir );
             return TRUE;
          } else if ( !(OpMode & OPM_FIND) ) SayError( imap->GetErrorMessage() );
       }
       *SelectedMailbox = 0;
       // Not for IMAP
       lstrcpy( SelectedServer , savb );
       return FALSE;
  }
  return(TRUE);
#ifdef TDEBUG
}
#ifdef __BORLANDC__
__except(1)
#else
catch(...)
#endif
{
 SayException( "SetDirectory" );
 return FALSE;
}
#endif
}

static long WINAPI MessageAttributeDialogProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  int *DlgParams=(int *)_Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_HOTKEY:
      if(Param1>=DlgParams[0]&&Param1<=DlgParams[1])
      {
        Dialog_Focus(Param1+DlgParams[2]);
        Dialog_SetState(Param1+DlgParams[2],true);
        return FALSE;
      }
      break;
  }
  return _Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}


int FARMail::ProcessKey(int Key,unsigned int ControlState)
{
#ifdef TDEBUG
try {
#endif
  int i;
  if ( Key == VK_F1 && ControlState==PKF_SHIFT ) {
     pm->ShowAddressBookMenu( NULL );
     return TRUE;
  }

  if ( Key == VK_F7 && !ControlState && !Level ) {
     About();
     return TRUE;
  }

  if ( Key == VK_F7 && !ControlState && Level && current->Type == TYPE_POP3 ) {
     if ( clnt && clnt->connected ) {
        if ( !clnt->Reset() ) SayError( clnt->GetErrorMessage() );
        else
        {
          Cache.ClearState(MESSAGE_STATE_DELETED);
          _Info.Control( this, FCTL_UPDATEPANEL, (void *)1);
          _Info.Control( this, FCTL_REDRAWPANEL, NULL );
        }
     }
     return TRUE;
  }

  if ( ( (Key == VK_F7 && ControlState==PKF_SHIFT ) || (Key == 'R' && ControlState==PKF_CONTROL ) )&& Level ) {
     Cache.ClearCachedData();
     if ( current->Type == TYPE_POP3 ) {
        if ( clnt ) {
           clnt->Disconnect();
           delete (MailClient*)clnt;
           clnt = NULL;
        }
        clnt = new MailClient( (Opt.DebugSession?TRUE:FALSE) , Opt.LOGFILE , current?current->timeout*60:0 );
        if ( clnt && current )
        {
#ifdef FARMAIL_SSL
           if ( !clnt->Connect( current->Url, current->_User, current->_Pass, current->Port, (current->UsePOP3S?CON_SSL:CON_NORMAL) ) )
#else
           if ( !clnt->Connect( current->Url, current->_User, current->_Pass, current->Port) )
#endif
              SayError( clnt->GetErrorMessage() );
           else {
              _Info.Control( this, FCTL_UPDATEPANEL, NULL );
              _Info.Control( this, FCTL_REDRAWPANEL, NULL );
           }
        }
     } else if ( Level && Key == 'R') {
        _Info.Control( this, FCTL_UPDATEPANEL, NULL );
        _Info.Control( this, FCTL_REDRAWPANEL, NULL );

     }
     return TRUE;
  }
  if ( Key == VK_F4 && ControlState==PKF_SHIFT && Level == 0 ) {
     POPSERVER s;
     memset( &s, 0, sizeof( POPSERVER ) );

     if ( !EditMailbox( &s , 1 ) ) {

          ServerCount = 0;
          if ( server ) z_free(server);
          ReadServers();

          _Info.Control( this, FCTL_UPDATEPANEL, NULL );
          _Info.Control( this, FCTL_REDRAWPANEL, NULL );

     }
     return TRUE;
  }
  if ( Key == VK_F4 && !ControlState && Level == 0 ) {
     struct PanelInfo PInfo;
     _Info.Control(this,FCTL_GETPANELINFO,&PInfo);
     if ( PInfo.ItemsNumber ) {
        if ( lstrcmp( PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName , UPDIR ) ) {
           current = NULL;
           for ( i=0 ; i<ServerCount ; i++ ) {
                if ( !lstrcmp( PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName, server[i].Name ) ) {
                   current = &server[i];
                   break;
                }
           }
           if ( current )
              EditMailbox( current , 0 );

              ServerCount = 0;
              if ( server ) z_free(server);
              ReadServers();

              _Info.Control( this, FCTL_UPDATEPANEL, NULL );
              _Info.Control( this, FCTL_REDRAWPANEL, NULL );
           current = NULL;
        }
     }
     return TRUE;
  }

  if ( ( Key == VK_F2 || Key == VK_F3 )&& ControlState==PKF_SHIFT && Level ) {

     struct PanelInfo PInfo;
     _Info.Control(this,FCTL_GETPANELINFO,&PInfo);
     int num = ( PInfo.ItemsNumber?FSF.atoi( PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName ):0);

     if ( current->Type == TYPE_POP3 && num ) {

        if ( clnt && clnt->connected ) {

           if ( clnt->Top( num, Opt.QuickLines+current->TopValue ) ) {

              char temp[MAX_PATH], tempfile[MAX_PATH];

              *temp = *tempfile = '\0';

              GetTempPath( MAX_PATH, temp );
              GetTempFileName( temp, "fml", 0, tempfile );

              HANDLE fp=CreateFile(tempfile,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
              if (fp!=INVALID_HANDLE_VALUE) {
                 SaveToFile( fp, clnt->GetMsg() );
                 CloseHandle(fp);
                 if ( Key == VK_F2 )
                    _Info.Editor( tempfile, NULL, 0,0,-1,-1, 0, 0,0 );
                 else
                    _Info.Viewer( tempfile, NULL, 0,0,-1,-1, 0 );

                 DeleteFile( tempfile );
              }

           } else {
              SayError( clnt->GetErrorMessage() );
              return TRUE;
           }

        }

     } else if ( Level>1 && num ) {

        if ( imap && imap->connected ) {

           char * msgtxt;
           if ( ( msgtxt = imap->GetMsgTop( num , Opt.QuickBytes , NULL ) ) != NULL ) {

              char temp[MAX_PATH], tempfile[MAX_PATH];

              *temp = *tempfile = '\0';

              GetTempPath( MAX_PATH, temp );
              GetTempFileName( temp, "fml", 0, tempfile );

              HANDLE fp=CreateFile(tempfile,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
              if(fp!=INVALID_HANDLE_VALUE)
              {
                 DWORD written;
                 WriteFile(fp,msgtxt,lstrlen(msgtxt),&written,NULL);
                 CloseHandle(fp);
                 if ( Key == VK_F2 )
                    _Info.Editor( tempfile, NULL, 0,0,-1,-1, 0, 0,0 );
                 else
                    _Info.Viewer( tempfile, NULL, 0,0,-1,-1, 0 );
                 DeleteFile( tempfile );
              }

           } else {
              SayError( imap->GetErrorMessage() );
              return TRUE;
           }
        }
     }
     return TRUE;
  }
  if ( Key == VK_F6 && ControlState==PKF_ALT && Level && current )
  {
     if ( current->Type == TYPE_POP3 || Level > 1 )
     {
        pm->ShowFilterMenu((HANDLE)this,(current->Type==TYPE_POP3)?3:4);
        return TRUE;
     }
  }

  if ( Key == VK_F7 && ControlState==0 && Level==1 && current && current->Type == TYPE_IMAP4 ) {

     char newdir[512];

     if ( imap && imap->connected ) {
        if ( !InvWin( ::GetMsg(MesCreateDir1), ::GetMsg(MesCreateDir2), NULLSTR, newdir ) ) {

           if ( *newdir ) {
              if ( imap->Create( newdir ) )
                 SayError( imap->GetErrorMessage() );
              else {
                 _Info.Control( this, FCTL_UPDATEPANEL, NULL );
                 _Info.Control( this, FCTL_REDRAWPANEL, NULL );
              }
           }
        }
     }
     return TRUE;
  }

  if (((((Key==VK_F5 || Key==VK_F6) && ControlState==0) || (Key==VK_F8 && ControlState==PKF_SHIFT)) && !Level) || (Key == VK_F3 && !ControlState && !Level))
  {
    char path[MAX_PATH];
    struct PanelInfo PInfo;
    _Info.Control(INVALID_HANDLE_VALUE, FCTL_GETANOTHERPANELINFO, &PInfo);
    lstrcpy(path,PInfo.CurDir);
    _Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &PInfo);
    if (PInfo.ItemsNumber > 0)
    {
      if (Key==VK_F8)
        FastExpunge(PInfo.SelectedItems, PInfo.SelectedItemsNumber, 0);
      else if (Key==VK_F3)
        FastStatus(PInfo.SelectedItems, PInfo.SelectedItemsNumber, 0);
      else
        FastDownload(PInfo.SelectedItems, PInfo.SelectedItemsNumber, (Key==VK_F5?FALSE:TRUE), path, 0);
      if ( clnt ) { clnt->Disconnect(); delete (MailClient*)clnt; clnt = NULL; }
      if ( imap ) { imap->Disconnect(); delete (IMAP*)imap; imap = NULL; }
      current = NULL;
      for (int i=0, j=0; i<PInfo.ItemsNumber&&j<PInfo.SelectedItemsNumber; i++) //FIXME
      {
        if (!lstrcmp(PInfo.PanelItems[i].FindData.cFileName,PInfo.SelectedItems[j].FindData.cFileName))
        {
          PInfo.PanelItems[i].Flags = PInfo.SelectedItems[j].Flags;
          j++;
        }
      }
      _Info.Control(INVALID_HANDLE_VALUE, FCTL_SETSELECTION, &PInfo);
      _Info.Control(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, NULL);
    }
    return TRUE;
  }

  if ( Key == VK_F6 && ControlState==0 && Level==1 && current && current->Type == TYPE_IMAP4 ) {

     char newdir[512];
     struct PanelInfo PInfo;

     _Info.Control(this,FCTL_GETPANELINFO,&PInfo);
     if ( PInfo.ItemsNumber && lstrcmp( PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName , UPDIR ) && PInfo.SelectedItemsNumber < 2 ) {

        if ( imap && imap->connected ) {
           if ( !InvWin( ::GetMsg(MesRenameDir1), ::GetMsg(MesRenameDir2), NULLSTR, newdir ) ) {

              if ( *newdir ) {

                 if ( imap->Rename( PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName , newdir ) )
                    SayError( imap->GetErrorMessage() );
                 else {
                    _Info.Control( this, FCTL_UPDATEPANEL, NULL );
                    _Info.Control( this, FCTL_REDRAWPANEL, NULL );
                 }
              }
           }
        }
     }
     return TRUE;
  }
  if ( (Key==VK_F6 || Key==VK_F5) && ControlState==PKF_SHIFT && Level>1 && current && current->Type == TYPE_IMAP4 ) {

     if ( Key==VK_F6 )
        CopyMoveIMAP( 1 );
     else
        CopyMoveIMAP( 0 );

     return TRUE;
  }
  if(Key=='A'&&(ControlState==PKF_CONTROL||ControlState==(PKF_CONTROL|PKF_SHIFT))&&Level&&current->Type==TYPE_POP3&&current->uidl)
  {
    enum
    {
      C_NEW_H=1,
      C_READ_H,
      C_MARKED_H,
      C_NEW,
      C_READ,
      C_MARKED,
      C_SET,
      C_CANCEL
    };
    InitDialogItem InitItems[]=
    {
      {DI_DOUBLEBOX,3,1,34,7,0,0,0,0,NULLSTR},
      {DI_TEXT,5,2,0,0,0,0,0,0,(char*)MesHotkey1},
      {DI_TEXT,5,3,0,0,0,0,0,0,(char*)MesHotkey2},
      {DI_TEXT,5,4,0,0,0,0,0,0,(char*)MesHotkey3},
      {DI_RADIOBUTTON,6,2,0,0,1,1,DIF_GROUP,0,(char*)MesConfig_NewMessage},
      {DI_RADIOBUTTON,6,3,0,0,0,0,0,0,(char*)MesConfig_ReadMessage},
      {DI_RADIOBUTTON,6,4,0,0,0,0,0,0,(char*)MesConfig_MarkedMessage},
      {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP,1,(char *)MesOk},
      {DI_BUTTON,0,6,0,0,0,0,DIF_CENTERGROUP,0,(char *)MesCancel},
      {DI_TEXT,3,5,0,0,0,0,DIF_SEPARATOR,0,NULLSTR},
    };
    FarDialogItem DialogItems[sizeofa(InitItems)];
    InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));
    int params[3]={C_NEW_H,C_MARKED_H,C_NEW-C_NEW_H};
    if(_Info.DialogEx(_Info.ModuleNumber,-1,-1,38,9,NULL,DialogItems,sizeofa(DialogItems),0,0,MessageAttributeDialogProc,(DWORD)params)==C_SET)
    {
      PanelInfo PInfo;
      _Info.Control(this,FCTL_GETPANELINFO,&PInfo);
      if(PInfo.SelectedItemsNumber)
      {
        int state=MESSAGE_STATE_MARKED;
        if(DialogItems[C_NEW].Selected) state=MESSAGE_STATE_NEW;
        else if(DialogItems[C_READ].Selected) state=MESSAGE_STATE_READ;
        if(ControlState&PKF_SHIFT)
        {
          if(PInfo.PanelItems[PInfo.CurrentItem].CustomColumnNumber>4&&PInfo.PanelItems[PInfo.CurrentItem].CustomColumnData&&PInfo.PanelItems[PInfo.CurrentItem].CustomColumnData[4]&&lstrlen(PInfo.PanelItems[PInfo.CurrentItem].CustomColumnData[4]))
            Cache.MarkMessage(PInfo.PanelItems[PInfo.CurrentItem].CustomColumnData[4],state);
        }
        else
          for(int i=0;i<PInfo.SelectedItemsNumber;i++)
            if(PInfo.SelectedItems[i].CustomColumnNumber>4&&PInfo.SelectedItems[i].CustomColumnData&&PInfo.SelectedItems[i].CustomColumnData[4]&&lstrlen(PInfo.SelectedItems[i].CustomColumnData[4]))
              Cache.MarkMessage(PInfo.SelectedItems[i].CustomColumnData[4],state);
        _Info.Control(this,FCTL_UPDATEPANEL,(ControlState&PKF_SHIFT)?(void *)1:NULL);
        _Info.Control(this,FCTL_REDRAWPANEL,NULL);
      }
    }
  }
  return(FALSE);
#ifdef TDEBUG
}
#ifdef __BORLANDC__
__except(1)
#else
catch(...)
#endif
{
 SayException( "ProcessKey" );
 return FALSE;
}
#endif
}

int FARMail::ProcessCommandLine(const char *Cmd)
{
#ifdef TDEBUG
try {
#endif
  char Prefix[50];
  if (!strstr(Cmd, ":")) return FALSE;
  const char *Command = strstr(Cmd, ":") + 1;
  lstrcpyn(Prefix,Cmd,Command-Cmd);
  if (!FSF.LStricmp(Prefix, FASTDOWNLOADCOPY_PREFIX) || !FSF.LStricmp(Prefix, FASTDOWNLOADMOVE_PREFIX))
  {
    char mbox[80];
    if (*Command == '"')
    {
      int i = 0;
      *mbox = 0;
      Command++;
      while (*Command && *Command != '"' && i < 79)
      {
        if (*Command == '\\' && *(Command+1) == '"')
          Command++;
        mbox[i++] = *Command;
        Command++;
      }
      if (*Command != '"' || i == 0) return FALSE;
      if (*(Command+1) && (*(Command+1) != 0x20) && (*(Command+1) != 0x09)) return FALSE;
      Command++;
      mbox[i] = 0;
    }
    else
    {
      const char *ptr = Command;
      while (*Command && (*Command != 0x20) && (*Command != 0x09)) Command++;
      if ((Command-ptr) > 79 || (Command-ptr) == 0) return FALSE;
      lstrcpyn(mbox,ptr,Command-ptr+1);
    }
    while (*Command == 0x20 || *Command == 0x09) Command++;
    if (lstrlen(Command) > (MAX_PATH-1)) return FALSE;
    char path[MAX_PATH];
    lstrcpy(path,Command);
    FSF.Unquote(FSF.Trim(path));
    struct PluginPanelItem PanelItem;
    memset(&PanelItem,0,sizeof(PanelItem));
    FSF.sprintf(PanelItem.FindData.cFileName,mbox);
    int UseInbox = Opt.UseInbox;
    if (*path) Opt.UseInbox = 0;
    FastDownload(&PanelItem,1,!FSF.LStricmp(Prefix, FASTDOWNLOADMOVE_PREFIX),path,OPM_SILENT);
    Opt.UseInbox = UseInbox;
    return TRUE;
  }
  else if (!FSF.LStricmp(Prefix, FASTEXPUNGE_PREFIX))
  {
    char mbox[80];
    if (*Command == '"')
    {
      if (Command[lstrlen(Command)-1] != '"' || (lstrlen(Command)-2) > 79) return FALSE;
      lstrcpyn(mbox,Command+1,lstrlen(Command+1));
    }
    else
    {
      if (lstrlen(Command) > 79) return FALSE;
      lstrcpy(mbox,Command);
    }
    struct PluginPanelItem PanelItem;
    memset(&PanelItem,0,sizeof(PanelItem));
    FSF.sprintf(PanelItem.FindData.cFileName,mbox);
    FastExpunge(&PanelItem,1,OPM_SILENT);
    return TRUE;
  }
  return FALSE;
#ifdef TDEBUG
}
#ifdef __BORLANDC__
__except(1)
#else
catch(...)
#endif
{
 SayException( "ProcessCommandLine" );
 return FALSE;
}
#endif
}


int FARMail::GetDialog( int Move , const char *dir , char *newdir , int * uniq )
{
  int key;

  static struct InitDialogItem InitItems[]={
    // type, x1, y1, x2, y2, focus, selected, flags, default, data
    { DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(char*)MesCopy_Title },
    { DI_TEXT, 5, 2, 0, 0, 0,0,0, 0, (char*)MesCopy_Message },
    { DI_TEXT, 3, 4, 0, 0, 0,0,DIF_SEPARATOR, 0, NULLSTR },
    { DI_EDIT, 5, 3, 70,3,1,0,0,0, NULLSTR },
    { DI_TEXT, 3, 6, 0, 0, 0,0,DIF_SEPARATOR, 0, NULLSTR },
    { DI_CHECKBOX, 5, 5, 0, 0, 0,0,0,0, (char*)MesCopy_AssignUniqueNamesChkBx },
    { DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(char *)MesCopy_CopyBtn },
    { DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(char *)MesCancel }
  };
  enum {
    FD_EDIT = 3,
    FD_TXT1,
    FD_CHK,
    FD_OK,
    FD_CANCEL
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  if ( Move ) {
     lstrcpy( DialogItems[0].Data, GetMsg(MesMove_Title) );
     lstrcpy( DialogItems[1].Data, GetMsg(MesMove_Message) );
     lstrcpy( DialogItems[FD_OK].Data, GetMsg(MesMove_MoveBtn) );
  }
  lstrcpy( DialogItems[FD_EDIT].Data, dir );
  DialogItems[FD_CHK].Selected = *uniq;

  if ( *DialogItems[FD_EDIT].Data && DialogItems[FD_EDIT].Data[lstrlen(DialogItems[FD_EDIT].Data)-1] != '\\' && DialogItems[FD_EDIT].Data[lstrlen(DialogItems[FD_EDIT].Data)-1] != ':' )
     lstrcat( DialogItems[FD_EDIT].Data, BACKSLASH );

  key = _Info.Dialog(_Info.ModuleNumber,-1,-1,76,10, NULL, DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));
  if ( key == FD_OK ) {
     lstrcpy( newdir , DialogItems[FD_EDIT].Data );
     if ( DialogItems[FD_CHK].Selected ) *uniq = 1; else *uniq = 0;
  }

  return !( key == FD_OK );
}



static int SetDate( char *path, FILETIME *tm , int need )
{
 if ( need ) {

    Sleep(50);
    HANDLE hFile = CreateFile( path, GENERIC_WRITE|GENERIC_READ,
                     0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

    if ( hFile ) {
       SetFileTime( hFile, tm, tm, tm );
       CloseHandle(hFile);
    }

 }
 return 0;
}


int FARMail::GetFiles(struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, char *DestPath, int OpMode)
{
#ifdef TDEBUG
try {
#endif

  if ( !Level ) return FALSE;

  char buf[MAX_PATH*2+100];
  const char *err[8], *err2[6];
  char NewDestPath[512];
  int resume_flag = 0;
  long resume_length = 0;
  int newnum = 0;
  bool silent=(OpMode&OPM_SILENT);
  bool UseNameF=(Opt.UseNameF&&(!Opt.DisableTOP||(current->Type!=TYPE_POP3)));

  lstrcpy( NewDestPath, DestPath );

  err[0] = GetMsg(MesOverwrite_Title);
  err[1] = NULLSTR;
  err[2] = GetMsg(MesOverwrite_Question);
  err[3] = GetMsg(MesOverwrite_YesBtn);
  err[4] = GetMsg(MesOverwrite_AllBtn);
  err[5] = GetMsg(MesOverwrite_SkipBtn);
  err[6] = GetMsg(MesOverwrite_SkipAllBtn);
  err[7] = GetMsg(MesCancel);

  err2[0] = GetMsg(MesOverwrite_Title);
  err2[1] = NULLSTR;
  err2[2] = GetMsg(MesOverwrite_Question);
  err2[3] = GetMsg(MesOverwrite_YesBtn);
  err2[4] = GetMsg(MesOverwrite_ResumeBtn);
  err2[5] = GetMsg(MesCancel);

  if ( !Opt.Resume ) err2[4] = GetMsg(MesCancel);

  int i, all = 0, skip_all = 0;

  if ( current->Type == TYPE_POP3 )
  {
    bool refresh=false;
    if ( clnt && clnt->connected )
    {
      if ( silent || !GetDialog( Move, DestPath , NewDestPath , &Opt.Unique ) )
      {

        for (i=0;i<ItemsNumber;i++)
        {
           if (PanelItem[i].UserData&MESSAGE_STATE_DELETED)
             continue;
           //mark read messages
           if((OpMode&OPM_EDIT)||(OpMode&OPM_VIEW))
           {
             if(PanelItem[i].CustomColumnNumber>4&&PanelItem[i].CustomColumnData&&PanelItem[i].CustomColumnData[4]&&lstrlen(PanelItem[i].CustomColumnData[4])&&PanelItem[i].UserData==MESSAGE_STATE_NEW)
             {
               Cache.MarkMessage(PanelItem[i].CustomColumnData[4],MESSAGE_STATE_READ);
               refresh=true;
             }
           }

           int num = FSF.atoi( PanelItem[i].FindData.cFileName );

           char dest[MAX_PATH*2];

           lstrcpy( dest, NewDestPath );
           if ( * ( dest + lstrlen(dest) -1 ) != '\\' ) {
              // a filename?
              DWORD aa = GetFileAttributes(dest);
              if ( aa!=0xFFFFFFFF && (aa&FILE_ATTRIBUTE_DIRECTORY) ) {
                 // it's a directory!
                 if ( Opt.Unique && !UseNameF && !silent) {
                    newnum = GetFreeNumber( dest );
                 }
                 lstrcat ( dest, BACKSLASH );
                 if ( UseNameF && !silent) {

                    char newname[MAX_PATH];
                    char nbuf[MAX_PATH*2];

                    newnum = 0;

                    do
                    {

                       MakeDescription( newname, Opt.Format,
                                        PanelItem[i].CustomColumnData[0],
                                        PanelItem[i].CustomColumnData[2],
                                        PanelItem[i].CustomColumnData[1],
                                        MAX_PATH-1 );
                       if ( newnum )
                         FSF.sprintf( nbuf, "%s%s%d.%s", dest, newname, newnum, Opt.EXT );
                       else
                       {
                         lstrcpy( nbuf, dest );
                         lstrcat( nbuf, newname );
                         lstrcat( nbuf, "." );
                         lstrcat( nbuf, Opt.EXT );
                       }
                       newnum++;

                    } while ( UseNameF && Opt.Unique && GetFileAttributes(nbuf)!=0xFFFFFFFF );

                    lstrcpy( dest, nbuf );

                 }
                 else
                 {
                    if ( newnum > 0 )
                    {
                       char newname[MAX_PATH];
                       FSF.sprintf( newname, "%08ld.%s", newnum, Opt.EXT );
                       lstrcat( dest, newname );
                    }
                    else
                       lstrcat( dest, PanelItem[i].FindData.cFileName );
                 }
              } // else - a filename

           }
           else
           { // it's a directory name
              if ( Opt.Unique && !UseNameF && !silent)
              {
                 newnum = GetFreeNumber( dest );
              }
              if ( UseNameF && !silent)
              {

                 char newname[MAX_PATH];
                 char nbuf[MAX_PATH*2];

                 newnum = 0;

                 do
                 {

                    MakeDescription( newname, Opt.Format,
                                     PanelItem[i].CustomColumnData[0],
                                     PanelItem[i].CustomColumnData[2],
                                     PanelItem[i].CustomColumnData[1],
                                     MAX_PATH-1 );
                    if ( newnum )
                      FSF.sprintf( nbuf, "%s%s%d.%s", dest, newname, newnum, Opt.EXT );
                    else
                    {
                       lstrcpy( nbuf, dest );
                       lstrcat( nbuf, newname );
                       lstrcat( nbuf, "." );
                       lstrcat( nbuf, Opt.EXT );
                    }
                    newnum++;

                 } while ( UseNameF && Opt.Unique && GetFileAttributes(nbuf)!=0xFFFFFFFF );

                 lstrcpy( dest, nbuf );

              }
              else
              {
                 if ( newnum > 0 )
                 {
                    char newname[MAX_PATH];
                    FSF.sprintf( newname, "%08ld.%s", newnum, Opt.EXT );
                    lstrcat( dest, newname );
                 } else lstrcat( dest, PanelItem[i].FindData.cFileName );
              }
           }

           if ( GetFileAttributes( dest ) != 0xFFFFFFFF && !all && !silent)
           {
              if ( skip_all ) continue;
              FSF.sprintf( buf, GetMsg(MesOverwrite_Filename), /* PanelItem[i].FindData.cFileName */ dest );
              err[1] = buf; //GetMsg(MesOverwrite_Filename);
              int answer = _Info.Message( _Info.ModuleNumber, FMSG_WARNING, NULL, err, 8, 5 );
              if ( answer == -1 || answer == 4 ) return -1;
              if ( answer == 2 ) continue; // skip
              if ( answer == 3 ) { skip_all = 1; continue; }
              if ( answer == 1 ) all = 1;
           }

           if ( clnt->Retrieve(num, OpMode) )
           {
              BOOL cd=MakeDirs(NewDestPath,dest);
              HANDLE fp=CreateFile(dest,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_ARCHIVE|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
              if(fp!=INVALID_HANDLE_VALUE && cd)
              {
                 int wrote = SaveToFile(fp,clnt->GetMsg());
                 CloseHandle(fp);
                 SetDate( dest , &PanelItem[i].FindData.ftCreationTime, Opt.FileDate );
                 if (wrote==1)
                 {
                   SayError(::GetMsg(MesErrWriteFile));
                   return 0;
                 }
              } else {
                 SayError(::GetMsg(MesErrOpenFile));
                 return 0;
              }
              if (PanelItem[i].Flags&PPIF_SELECTED)
                PanelItem[i].Flags-=PPIF_SELECTED;
              if ( Move)
              {
                 if (clnt->Delete( num ))
                 {
                   Cache.MarkMessage(num,MESSAGE_STATE_DELETED);
                   refresh=true;
                 }
                 else if (!silent)
                 {
                   SayError( clnt->GetErrorMessage() );
                   return 0;
                 }
              }

           } else {
              SayError( clnt->GetErrorMessage() );
              return 0;
           }
        }
      } else return 0;
    } else return 0;
    if(refresh)
    {
      _Info.Control(this,FCTL_UPDATEPANEL,(void *)1);
      _Info.Control(this,FCTL_REDRAWPANEL,NULL);
    }
  }
  else
  {
     if ( imap && imap->connected && Level>1 )
     {
      if ( silent || !GetDialog( Move, DestPath , NewDestPath , &Opt.Unique ) )
      {
        if ( Move ) Cache.ClearCachedData();
        int retval = 0;
        for (i=0;i<ItemsNumber;i++)
        {
           int num = FSF.atoi( PanelItem[i].FindData.cFileName );

           char dest[MAX_PATH*2];
           lstrcpy( dest, NewDestPath );
           if ( * ( dest + lstrlen(dest) -1 ) != '\\' )
           {
              DWORD aa = GetFileAttributes(dest);
              if ( aa!=0xFFFFFFFF && (aa&FILE_ATTRIBUTE_DIRECTORY) )
              {
                 // it's a directory!
                 if ( Opt.Unique && !UseNameF && !silent)
                 {
                    newnum = GetFreeNumber( dest );
                 }
                 lstrcat ( dest, BACKSLASH );
                 if ( UseNameF && !silent)
                 {

                    char newname[MAX_PATH];
                    char nbuf[MAX_PATH*2];

                    newnum = 0;

                    do
                    {

                       MakeDescription( newname, Opt.Format,
                                        PanelItem[i].CustomColumnData[0],
                                        PanelItem[i].CustomColumnData[2],
                                        PanelItem[i].CustomColumnData[1],
                                        MAX_PATH-1 );
                       if ( newnum )
                         FSF.sprintf( nbuf, "%s%s%d.%s", dest, newname, newnum, Opt.EXT );
                       else
                       {
                          lstrcpy( nbuf, dest );
                          lstrcat( nbuf, newname );
                          lstrcat( nbuf, "." );
                          lstrcat( nbuf, Opt.EXT );
                       }
                       newnum++;

                    } while ( UseNameF && Opt.Unique && GetFileAttributes(nbuf)!=0xFFFFFFFF );

                    lstrcpy( dest, nbuf );

                 }
                 else
                 {
                    if ( newnum > 0 )
                    {
                       char newname[MAX_PATH];
                       FSF.sprintf( newname, "%08ld.%s", newnum, Opt.EXT );
                       lstrcat( dest, newname );
                    }
                    else
                       lstrcat( dest, PanelItem[i].FindData.cFileName );
                 }
              } // else - a filename
           }
           else
           {
              if ( Opt.Unique && !UseNameF && !silent)
              {
                 newnum = GetFreeNumber( dest );
              }
              if ( UseNameF && !silent) {

                 char newname[MAX_PATH];
                 char nbuf[MAX_PATH*2];

                 newnum = 0;

                 do
                 {

                    MakeDescription( newname, Opt.Format,
                                     PanelItem[i].CustomColumnData[0],
                                     PanelItem[i].CustomColumnData[2],
                                     PanelItem[i].CustomColumnData[1],
                                     MAX_PATH-1 );
                    if ( newnum )
                      FSF.sprintf( nbuf, "%s%s%d.%s", dest, newname, newnum, Opt.EXT );
                    else
                    {
                      lstrcpy( nbuf, dest );
                      lstrcat( nbuf, newname );
                      lstrcat( nbuf, "." );
                      lstrcat( nbuf, Opt.EXT );
                    }
                    newnum++;

                 } while ( UseNameF && Opt.Unique && GetFileAttributes(nbuf)!=0xFFFFFFFF );

                 lstrcpy( dest, nbuf );

              }
              else
              {
                 if ( newnum > 0 )
                 {
                    char newname[MAX_PATH];
                    FSF.sprintf( newname, "%08ld.%s", newnum, Opt.EXT );
                    lstrcat( dest, newname );
                 }
                 else
                   lstrcat( dest, PanelItem[i].FindData.cFileName );
              }
           }


           if ( GetFileAttributes( dest ) != 0xFFFFFFFF && !all && !silent)
           {

              if ( skip_all ) continue;

              if ( ItemsNumber > 1 )
              {
                 FSF.sprintf( buf, GetMsg(MesOverwrite_Filename), PanelItem[i].FindData.cFileName );
                 err[1] = buf;
                 int answer = _Info.Message( _Info.ModuleNumber, FMSG_WARNING, NULL, err,
                                             8, 5 );
                 if ( answer == -1 || answer == 4 ) { retval = -1; goto RET; } //eturn -1;
                 if ( answer == 2 ) continue; // skip
                 if ( answer == 3 ) { skip_all = 1; continue; }
                 if ( answer == 1 ) all = 1;
              }
              else
              {
                 FSF.sprintf( buf, GetMsg(MesOverwrite_Filename), PanelItem[i].FindData.cFileName );
                 err2[1] = buf;
                 int delta = 0;

                 if ( !Opt.Resume ) delta=1;

                 int answer = _Info.Message( _Info.ModuleNumber, FMSG_WARNING, NULL, err2,
                                             6-delta, 3-delta );
                 if ( answer == -1 || answer == 2 || (answer == 1 && !Opt.Resume ))
                 {
                   retval = -1;
                   goto RET;
                 }
                 if ( answer == 1 )
                 { // resume
                    HANDLE tfp=CreateFile(dest,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
                    if (tfp!=INVALID_HANDLE_VALUE)
                    {
                       char qw[100];

                       if (ReadLine(tfp,qw,sizeof(qw)))
                       {
                          if ( !strncmp( qw, XRESUMEDATA, 22 ) )
                          {
                            char *qw_ptr = qw+22;
                            char *qw_ptr2 = strchr( qw_ptr, ':' );

                            if ( qw_ptr2 )
                            {
                              *qw_ptr2 = 0;
                              qw_ptr2++;

                              resume_length = atol( qw_ptr2 );
                              if ( atol( qw_ptr ) == atol(PanelItem[i].CustomColumnData[3] ) && resume_length )
                              {
                                resume_flag = 1;
                              }
                            }

                          }
                       }
                       CloseHandle(tfp);
                    }
                    else
                    {
                       SayError( ::GetMsg(MesErrOpenFile) );
                       retval = 0;
                       goto RET;
                    }
                    if ( !resume_flag )
                    {
                       SayError( ::GetMsg(MesErrNoResume) );
                       retval = -1; goto RET;
                    }
                 }

              }
           }

           char *msgtxt;

           if ( !resume_flag )
              msgtxt = imap->GetMsgText( num , PanelItem[i].FindData.nFileSizeLow, PanelItem[i].FindData.cFileName );
           else
              msgtxt = imap->GetMsgTail( num , resume_length, PanelItem[i].FindData.nFileSizeLow , PanelItem[i].FindData.cFileName );

           if ( msgtxt != NULL && ( !imap->Partial || Opt.Resume ))
           {
              HANDLE fp;
              BOOL cd=TRUE;

              if(resume_flag)
              {
                fp=CreateFile(dest,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
                if(fp!=INVALID_HANDLE_VALUE)
                {
                  if(SetFilePointer(fp,0,NULL,FILE_END)==INVALID_SET_FILE_POINTER)
                  {
                    CloseHandle(fp);
                    fp=INVALID_HANDLE_VALUE;
                  }
                }
              }
              else
              {
                cd=MakeDirs(NewDestPath,dest);
                fp=CreateFile(dest,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
              }

              if(fp!=INVALID_HANDLE_VALUE && cd)
              {
                char write_buf[128];
                DWORD written;
                bool wrote = true;
                if(imap->Partial)
                {
                  if(resume_flag) SetFilePointer(fp,0,NULL,FILE_BEGIN);
                  FSF.sprintf(write_buf,XRESUMEDATA,atol(PanelItem[i].CustomColumnData[3]),imap->ReceivedMessageLen+resume_length);
                  wrote = wrote && (WriteFile(fp,write_buf,lstrlen(write_buf),&written,NULL)&&written);
                  if(resume_flag) SetFilePointer(fp,0,NULL,FILE_END);
                }
                else if(resume_flag)
                {
                  SetFilePointer(fp,0,NULL,FILE_BEGIN);
                  FSF.sprintf(write_buf,XRESUMEDATA,0,0);
                  wrote = wrote && (WriteFile(fp,write_buf,lstrlen(write_buf),&written,NULL)&&written);
                  SetFilePointer(fp,0,NULL,FILE_END);
                }
                {
                  int len = lstrlen(msgtxt);
                  wrote = wrote && (WriteFile(fp,msgtxt,len,&written,NULL)&&(len?written:1));
                }
                CloseHandle(fp);
                if ( imap->Partial )
                {
                   SetFileAttributes(dest, FILE_ATTRIBUTE_HIDDEN );
                }
                else
                {
                   SetFileAttributes(dest, FILE_ATTRIBUTE_NORMAL );
                   SetDate( dest , &PanelItem[i].FindData.ftCreationTime, Opt.FileDate );
                }
                if (!wrote)
                {
                  SayError( ::GetMsg(MesErrWriteFile) );
                  retval = 0;
                  goto RET;
                }
              }
              else
              {
                 SayError( ::GetMsg(MesErrOpenFile) );
                 retval = 0;
                 goto RET;
              }
              if ( Move && !imap->Partial && !silent && imap->DeleteUID( PanelItem[i].CustomColumnData[3] )  ) {
                 SayError( imap->GetErrorMessage() );
                 goto RET;
              }

           }
           else
           {
              SayError( imap->GetErrorMessage() );
              goto RET;
           }
        }
        retval = 1;

RET:
        if ( Move && imap->Expunge() )
            SayError( imap->GetErrorMessage() );
        return retval;

      } else return 0;
     } else return 0;

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
 SayException( "GetFiles" );
 return -1;
}
#endif
}

int FARMail::ProcessEvent( int Event, void *Param )
{
 PanelInfo pnInfo;

 (void) Param;
 if ( Event == FE_BREAK ) {
    Cache.ClearState(MESSAGE_STATE_DELETED);
    Cache.ClearCachedData();
    if ( clnt ) clnt->PopServer.StopSocket();
    if ( imap ) imap->Socket.StopSocket();
    if ( FMsmtp ) FMsmtp->wsocket.StopSocket();
 }

 if ( Event == FE_CLOSE )
   SaveLastViewMode ();
 
 if ( Event == FE_CHANGEVIEWMODE )
 {
   _Info.Control (
       this,
       FCTL_GETPANELSHORTINFO,
       &pnInfo
       );

   int nPanel;

   if ( !Level )
     nPanel = PLUGIN_PANEL_MAILBOXES;
   else
   {
     if ( current && current->Type == TYPE_IMAP4 )
     {
       if ( Level == 1 )
         nPanel = PLUGIN_PANEL_IMAP4_FOLDERS;
       else
         nPanel = PLUGIN_PANEL_IMAP4;
     }
     else
      nPanel = PLUGIN_PANEL_POP3;
   };

   Opt.LastViewMode[nPanel] = pnInfo.ViewMode;
 }

 return FALSE;
}




int FARMail::PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode)
{
(void) Move;
(void) OpMode;
#ifdef TDEBUG
try {
#endif
 /*static*/ MAILSEND parm;
 int i, retval = 1,aio=1;

 if ( !Level ) {
    if ( SelectMailbox() ) return -1;
 }

 InitMAILSEND(&parm);
 if (*current->UserName)
   FSF.sprintf(parm.Sender,"%s <%s>",current->UserName,current->EMail);
 else
   FSF.sprintf(parm.Sender,"<%s>",current->EMail);
 parm.UseCC=Opt.UseCC;
 //parm.all = 0; //it's already 0 isn't it?

 if ( ItemsNumber>1 )
 {
    aio = DoAllInOne();
    if ( aio == -1 ) { retval = -1; goto DISCONNECT; }
 }

 if ( ItemsNumber && aio )
 {

    for (i=0;i<ItemsNumber;i++)
    {

       int err = 0;

       if ( PanelItem[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
       {
         continue;
       }

       if ( !parm.all || parm.analize_all )
       {
         err = AnalizeFile( PanelItem[i].FindData.cFileName , &parm );
       }

       if ( !err )
       {
          if ( !parm.all ) {
             if ( ItemsNumber > 1 )
               err=MailSendDialog(&parm,PanelItem[i].FindData.cFileName,false,true,(Opt.UseHeader?true:false));
             else
               err=MailSendDialog(&parm,PanelItem[i].FindData.cFileName,false,false,false);
             if (err==-1)
             {
               FreeMailSend( &parm );
               continue;
             }
          }
          if ( err ) { retval = -1; goto DISCONNECT; }//return -1;

          // process file...
          err = SendFileWA( &parm, PanelItem[i].FindData.cFileName , PanelItem[i].FindData.nFileSizeLow );
          SaveOutgoingMessage( SAVE_KILL, NULL );
          if ( !err )
             PanelItem[i].Flags -= PPIF_SELECTED;
          else
             { retval = -1; goto DISCONNECT; }// return -1;

       } else { retval = -1; goto DISCONNECT; }// return -1;

       /* if ( !parm.all || parm.analize_all ) */ FreeMailSend( &parm );
    }
 } else if ( ItemsNumber>1 && !aio ) {

    int err;
    AnalizeFileMulti( &parm );
//    err = MailSendParmsMulti( &parm );
    err=MailSendDialog(&parm,NULL,true,false,false);

    if ( !err ) err = BeginSendFileMulti( &parm );

    if ( err ) { retval = -1; goto DISCONNECT; }//return -1;

    for (i=0;i<ItemsNumber;i++)   {

       if ( PanelItem[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
          continue;
       }

       if ( !err ) {
          err = SendFileMulti( &parm, PanelItem[i].FindData.cFileName , PanelItem[i].FindData.nFileSizeLow );
          if ( !err )
             PanelItem[i].Flags -= PPIF_SELECTED;
          else
             { retval = -1; goto DISCONNECT; }// return -1;
       } else {
          retval = -1;
          goto DISCONNECT;
       }
    }
    EndSendFileMulti( &parm );
    SaveOutgoingMessage( SAVE_KILL, NULL );

 }

DISCONNECT:
 FreeMailSend( &parm );
 if ( !Level ) {
    // disconnect here!
    current = NULL;
 }

 return retval;
#ifdef TDEBUG
}
#ifdef __BORLANDC__
__except(1)
#else
catch(...)
#endif
{
 SayException( "PutFiles" );
 return -1;
}
#endif
}

int FARMail::SaveToFile(HANDLE fp,const char *msg,int UseInbox)
{
  char line[1001],*data;
  DWORD written;
  bool wrote = true;
  long i=0;
  if (UseInbox && !strncmp(msg,"From ",5))
    wrote = wrote && (WriteFile(fp,">",1,&written,NULL)&&written);
  while(TRUE&&wrote)
  {
    int j=0;
    char c=msg[i];
    while(c!=0&&(c!='\n')&&j<1000)
    {
      line[j++]=c;
      i++;
      c=msg[i];
    }
    line[j]=0;
    if(j-1>=0&&line[j-1]=='\r') line[j-1]=0;
    if(c=='\n') i++;

    data=NULL;
    if(line[0]=='.'&&line[1]=='.')
      data=line+1;
    else if(lstrcmp(line,"."))
      data=line;
    if(data)
    {
      if (UseInbox && !strncmp(data,"From ",5))
        wrote = wrote && (WriteFile(fp,">",1,&written,NULL)&&written);
      {
        int len = lstrlen(data);
        wrote = wrote && (WriteFile(fp,data,len,&written,NULL)&&(len?written:1));
        wrote = wrote && (WriteFile(fp,CRLF,2,&written,NULL)&&written);
      }
    }
    if(!c) break;
  }
  if (!wrote)
    return 1;
  return 0;
}

int FARMail::CopyMoveIMAP( int move )
{
 struct PanelInfo PInfo;
 char mailbox[512];
 const char *msg1, *msg2;

 if ( move ) {
    msg1 = ::GetMsg( MesMoveMsg1 );
    msg2 = ::GetMsg( MesMoveMsg2 );
 } else {
    msg1 = ::GetMsg( MesCopyMsg1 );
    msg2 = ::GetMsg( MesCopyMsg2 );
 }

 _Info.Control(this,FCTL_GETPANELINFO,&PInfo);

 if ( imap && imap->connected ) {
    if ( PInfo.SelectedItemsNumber || lstrcmp( PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName , UPDIR ) ) {
       if ( !InvWin( msg1, msg2, NULLSTR, mailbox ) && *mailbox ) {

          int stat /*= 0*/;
          int new_selection = 0;

          if ( move ) Cache.ClearCachedData();

          if ( PInfo.SelectedItemsNumber > 1 ) {
             int i;
             for ( i=0 ; i<PInfo.ItemsNumber; i++ ) {

                if ( !(PInfo.PanelItems[i].Flags & PPIF_SELECTED) ) continue;

                stat = CopyMoveMsg( move , &PInfo.PanelItems[i] , mailbox );
                if ( stat ) {
                   break;
                } else {
                   PInfo.PanelItems[i].Flags -= PPIF_SELECTED;
                   new_selection = 1;
                }
             }
             if ( new_selection )
                _Info.Control( this, FCTL_SETSELECTION, (void*)&PInfo );

             if ( move ) {
                if ( new_selection ) imap->Expunge();
                _Info.Control( this, FCTL_UPDATEPANEL, NULL );
             }

             _Info.Control( this, FCTL_REDRAWPANEL, NULL );


          } else {
             /*stat = */CopyMoveMsg( move , &PInfo.PanelItems[PInfo.CurrentItem]  , mailbox );
             if ( move ) {
                imap->Expunge();
                _Info.Control( this, FCTL_UPDATEPANEL, NULL );
                _Info.Control( this, FCTL_REDRAWPANEL, NULL );
             }
          }

       }
    }
 }
 return 0;
}


int FARMail::CopyMoveMsg( int move , struct PluginPanelItem *item , char *mb )
{
 int stat = 0;
 int num = FSF.atoi( item->FindData.cFileName );

 if ( num ) {

    if ( ( stat = imap->Copy( num , mb ) ) == 0 ) {
       if ( move ) stat = imap->DeleteUID( item->CustomColumnData[3] );
       if ( !stat ) {

          // Ok.

       } else SayError(imap->GetErrorMessage() );

    } else {
       SayError(imap->GetErrorMessage() );
    }
 }
 return stat;
}


int FARMail::About( void )
{
  const char *Messages[7];
  char Version[80];
  FSF.sprintf(Version,COPYRIGHT1,VER_HI,VER_LO,BUILD,BUILD_DAY,BUILD_MONTH,BUILD_YEAR);

  Messages[0]=::GetMsg(MesAboutBox);
  Messages[1]=NULLSTR;
  Messages[2]=Version;
  Messages[3]=COPYRIGHT2;
  Messages[4]=COPYRIGHT3;
  Messages[5]=NULLSTR;
  Messages[6]=::GetMsg(MesOk);
  _Info.Message(_Info.ModuleNumber,0,NULL,Messages,sizeofa(Messages),1);
  return 0;
}
