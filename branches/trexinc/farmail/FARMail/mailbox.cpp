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

#define DEF_SMTPPORT 25
#define DEF_POP3PORT 110
#define DEF_IMAP4PORT 143

#ifdef FARMAIL_SSL
#define DEF_SMTPSPORT 465
#define DEF_POP3SPORT 995
#define DEF_IMAP4SPORT 993
#endif

static const char USERNAME[] = "UserName";
static const char EMAIL   [] = "EMail";
static const char SMTPURL [] = "SMTPserver";
static const char URL     [] = "POPserver";
static const char USER    [] = "Login";
static const char PASS    [] = "Password";
static const char SMTPPORT[] = "SMTPport";
static const char PORT    [] = "POPport";
static const char _TIMEOUT[] = "Timeout";
static const char ORG     [] = "Organisation";
static const char DEFAULT [] = "Default";
static const char TYPE    [] = "Type";
static const char SMTPUSER[] = "SMTPLogin";
static const char SMTPPASS[] = "SMTPPassword";
static const char USEAUTH [] = "UseAuth";
#ifdef FARMAIL_SSL
static const char USESMTPS[] = "UseSMTPS";
static const char USEPOP3S[] = "UsePOP3S";
#endif
static const char UIDL    [] = "Uidl";
static const char TOPVALUE[] = "TopValue";

int FARMail::InsertMailbox( POPSERVER *server )
{
  HKEY hRoot = HKEY_CURRENT_USER;
  HKEY hKey;
  DWORD Disposition;
  char FullKeyName[512];
  char psw[80];

  char *Key = server->Name;

  FSF.sprintf(FullKeyName,S3,PluginMailBoxKey,*Key ? BACKSLASH:NULLSTR,Key);
  RegCreateKeyEx(hRoot,FullKeyName,0,NULL,0,KEY_WRITE,NULL,
                 &hKey,&Disposition);

  if ( Disposition == REG_OPENED_EXISTING_KEY ) {
     RegCloseKey( hKey );
     SayError( GetMsg(MesMailbox_AlreadyExist) );
     return -1;
  }

  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, USERNAME, server->UserName );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, EMAIL ,   server->EMail );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, SMTPURL , server->SMTPUrl );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, URL ,     server->Url );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, USER ,    server->User );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, SMTPUSER ,server->SMTP_User );

  lstrcpy( psw, server->Pass );
  EncodeString( (uchar*)psw );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, PASS ,    psw, REG_BINARY);
  lstrcpy( psw, server->SMTP_Pass );
  EncodeString( (uchar*)psw );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, SMTPPASS, psw, REG_BINARY);


  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, SMTPPORT, (DWORD)server->SMTPPort );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, PORT,     (DWORD)server->Port );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, _TIMEOUT, (DWORD)server->timeout );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, UIDL,     (DWORD)server->uidl );

  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, DEFAULT,  (DWORD)server->Default );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, USEAUTH,  (DWORD)server->UseAuth );
#ifdef FARMAIL_SSL
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, USESMTPS, (DWORD)server->UseSMTPS );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, USEPOP3S, (DWORD)server->UsePOP3S );
#endif
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, ORG ,     server->Organization );

  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, TYPE    , (DWORD)server->Type );

  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, TOPVALUE , (DWORD)server->TopValue );

  RegCloseKey( hKey );
  return 0;
}


int FARMail::FillMailbox( POPSERVER *server, char * name )
{
  HKEY hRoot = HKEY_CURRENT_USER;

  lstrcpy(  server->Name , name );

  GetRegKey2( hRoot, PluginMailBoxKey, name, USERNAME, server->UserName , NULLSTR, 80 );
  GetRegKey2( hRoot, PluginMailBoxKey, name, EMAIL ,   server->EMail , NULLSTR, 80 );
  GetRegKey2( hRoot, PluginMailBoxKey, name, SMTPURL , server->SMTPUrl , NULLSTR, 80 );
  GetRegKey2( hRoot, PluginMailBoxKey, name, URL ,     server->Url , NULLSTR, 80 );
  GetRegKey2( hRoot, PluginMailBoxKey, name, USER ,    server->User , NULLSTR, 80 );
  GetRegKey2( hRoot, PluginMailBoxKey, name, SMTPUSER, server->SMTP_User , NULLSTR, 80 );

  GetRegKey2( hRoot, PluginMailBoxKey, name, PASS ,    server->Pass , NULLSTR, 80 );
  DecodeString( (uchar*)server->Pass );
  GetRegKey2( hRoot, PluginMailBoxKey, name, SMTPPASS, server->SMTP_Pass , NULLSTR, 80 );
  DecodeString( (uchar*)server->SMTP_Pass );

  GetRegKey2( hRoot, PluginMailBoxKey, name, SMTPPORT, &server->SMTPPort , (DWORD)DEF_SMTPPORT );
  GetRegKey2( hRoot, PluginMailBoxKey, name, PORT,     &server->Port , (DWORD)DEF_POP3PORT );
  GetRegKey2( hRoot, PluginMailBoxKey, name, _TIMEOUT,  &server->timeout , (DWORD)0 );
  GetRegKey2( hRoot, PluginMailBoxKey, name, UIDL, &server->uidl , (DWORD)0 );

  GetRegKey2( hRoot, PluginMailBoxKey, name, DEFAULT,  &server->Default , (DWORD)0 );
  GetRegKey2( hRoot, PluginMailBoxKey, name, USEAUTH,  &server->UseAuth , (DWORD)0 );
#ifdef FARMAIL_SSL
  GetRegKey2( hRoot, PluginMailBoxKey, name, USESMTPS, &server->UseSMTPS , (DWORD)0 );
  GetRegKey2( hRoot, PluginMailBoxKey, name, USEPOP3S, &server->UsePOP3S , (DWORD)0 );
#endif
  GetRegKey2( hRoot, PluginMailBoxKey, name, ORG ,     server->Organization , NULLSTR, 80 );

  GetRegKey2( hRoot, PluginMailBoxKey, name, TYPE,     &server->Type , (DWORD)0 );

  GetRegKey2( hRoot, PluginMailBoxKey, name, TOPVALUE , &server->TopValue, (DWORD)0 );

  lstrcpy(server->MailboxPath,PluginMailBoxKey);
  lstrcat(server->MailboxPath,"\\");
  lstrcat(server->MailboxPath,name);

  return 0;

}


int FARMail::UpdateMailbox( POPSERVER *server )
{
  HKEY hRoot = HKEY_CURRENT_USER;
  char psw[80];

  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, USERNAME, server->UserName );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, EMAIL ,   server->EMail );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, SMTPURL , server->SMTPUrl );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, URL ,     server->Url );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, USER ,    server->User );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, SMTPUSER, server->SMTP_User );

  lstrcpy( psw, server->Pass );
  EncodeString( (uchar*)psw );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, PASS ,    psw, REG_BINARY);
  lstrcpy( psw, server->SMTP_Pass );
  EncodeString( (uchar*)psw );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, SMTPPASS ,psw, REG_BINARY);

  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, SMTPPORT, (DWORD)server->SMTPPort );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, PORT,     (DWORD)server->Port );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, _TIMEOUT,  (DWORD)server->timeout );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, UIDL,  (DWORD)server->uidl );

  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, DEFAULT,  (DWORD)server->Default );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, USEAUTH,  (DWORD)server->UseAuth );
#ifdef FARMAIL_SSL
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, USESMTPS, (DWORD)server->UseSMTPS );
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, USEPOP3S, (DWORD)server->UsePOP3S );
#endif
  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, ORG ,     server->Organization );

  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, TYPE,     (DWORD)server->Type );

  SetRegKey2( hRoot, PluginMailBoxKey, server->Name, TOPVALUE , (DWORD)server->TopValue );

  return 0;
}

static void DeleteRegSubKeys(HKEY hRoot,char *name)
{
  char key[1024];
  unsigned long i=0;
  HKEY hKey=OpenRegKey2(hRoot,name,NULLSTR);
  if(hKey)
  {
    char key_name[512]; unsigned long key_name_size=sizeof(key_name); FILETIME ft;
    while((RegEnumKeyEx(hKey,i,key_name,&key_name_size,NULL,NULL,NULL,&ft))==ERROR_SUCCESS)
    {
      key_name_size=sizeof(key_name);
      lstrcpy(key,name);
      lstrcat(key,"\\");
      lstrcat(key,key_name);
      DeleteRegSubKeys(hRoot,key);
      DeleteRegKey2(hRoot,key,NULLSTR);
    }
    RegCloseKey(hKey);
  }
}

int FARMail::DeleteMailbox(char *name)
{
  HKEY hRoot = HKEY_CURRENT_USER;
  char key[512];
  FSF.sprintf(key,"%s\\%s",PluginMailBoxKey,name);
  DeleteRegSubKeys(hRoot,key);
  DeleteRegKey2(hRoot,PluginMailBoxKey,name);
  return 0;
}




int FARMail::UpdateAndRenameMailbox( POPSERVER *server , char * oldname )
{
 if ( lstrcmp( oldname , server->Name ) ) {
    int stat = InsertMailbox( server );
    if ( !stat ) stat = DeleteMailbox( oldname );
    return stat;
 }
 return 0;
}


static int InvalidChars( char *s )
{
 for ( int i=0 ; i<lstrlen(s); i++ )
   if ( s[i]<32 || s[i]>126 || s[i]==':') return 1;
 return 0;
}


static int check( POPSERVER *s )
{
 if ( InvalidChars( s->Name ) ||
      // InvalidChars( s->UserName ) ||
      InvalidChars( s->EMail ) ||
      // InvalidChars( s->Organization ) ||
      InvalidChars( s->SMTPUrl ) ||
      InvalidChars( s->Url ) ||
      InvalidChars( s->User ) ||
      InvalidChars( s->SMTP_User ) //||
      //InvalidChars( s->SMTP_Pass ) ||
      //InvalidChars( s->Pass )
    )
   return 1;
 return 0;
}

long WINAPI EditMailboxDialogFunc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  static int *IDs;
  char Port[4];
  switch(Msg)
  {
    case DN_INITDIALOG:
      IDs=(int *)Param2;
      return FALSE;
    case DN_BTNCLICK:
      if(IDs[0]==Param1) //POP3
      {
#ifdef FARMAIL_SSL
        if (_Info.SendDlgMessage(hDlg,DM_GETCHECK,IDs[8],0))
          FSF.sprintf(Port,D1,DEF_POP3SPORT);
        else
          FSF.sprintf(Port,D1,DEF_POP3PORT);
#else
        FSF.sprintf(Port,D1,DEF_POP3PORT);
#endif
        _Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,IDs[2],(long)Port);
        for (int i=3; i<8; i++)
          _Info.SendDlgMessage(hDlg,DM_ENABLE,IDs[i],TRUE);
        return TRUE;
      }
      else if(IDs[1]==Param1) //IMAP4
      {
#ifdef FARMAIL_SSL
        if (_Info.SendDlgMessage(hDlg,DM_GETCHECK,IDs[8],0))
          FSF.sprintf(Port,D1,DEF_IMAP4SPORT);
        else
          FSF.sprintf(Port,D1,DEF_IMAP4PORT);
#else
        FSF.sprintf(Port,D1,DEF_IMAP4PORT);
#endif
        _Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,IDs[2],(long)Port);
        for (int i=3; i<8; i++)
          _Info.SendDlgMessage(hDlg,DM_ENABLE,IDs[i],FALSE);
        return TRUE;
      }
#ifdef FARMAIL_SSL
      else if(IDs[8]==Param1) //IMAP4||POP3 Use SSL
      {
        if (Param2)
        {
          if (_Info.SendDlgMessage(hDlg,DM_GETCHECK,IDs[0],0))
            FSF.sprintf(Port,D1,DEF_POP3SPORT);
          else
            FSF.sprintf(Port,D1,DEF_IMAP4SPORT);
        }
        else
        {
          if (_Info.SendDlgMessage(hDlg,DM_GETCHECK,IDs[0],0))
            FSF.sprintf(Port,D1,DEF_POP3PORT);
          else
            FSF.sprintf(Port,D1,DEF_IMAP4PORT);
        }
        _Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,IDs[2],(long)Port);
        return TRUE;
      }
      else if(IDs[9]==Param1) //SMTP Use SSL
      {
        if (Param2)
          FSF.sprintf(Port,D1,DEF_SMTPSPORT);
        else
          FSF.sprintf(Port,D1,DEF_SMTPPORT);

        _Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,IDs[10],(long)Port);
        return TRUE;
      }
#endif
      break;

  }
  return _Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}


int FARMail::EditMailbox( POPSERVER *server , int isnew )
{
  int key;
  char oldname[80];
  POPSERVER olds;
  static char HistoryMailboxName[] = "FARMailMailboxName";
  static char HistoryUserName[]    = "FARMailUserName";
  static char HistoryEMail[]       = "FARMailEMail";
  static char HistoryOrg[]         = "FARMailOrg";
  static char HistoryUrl[]         = "FARMailUrl";

  static struct InitDialogItem InitItems[]={
    // type, x1, y1, x2, y2, focus, selected, flags, default, data
    { DI_DOUBLEBOX,3,1,67,21,0,0,0,0,(char*)MesMailbox_Title },
    { DI_TEXT, 6, 2, 0,0,0,0,0,0, (char *)MesMailbox_Name },
    { DI_EDIT, 22, 2, 63,3,1,(DWORD)HistoryMailboxName,DIF_HISTORY,0, NULLSTR },
    { DI_TEXT, 6, 3, 0,0,0,0,0,0, (char *)MesMailbox_UserName },
    { DI_EDIT, 22, 3, 63,4,0,(DWORD)HistoryUserName,DIF_HISTORY,0, NULLSTR },
    { DI_TEXT, 6, 4, 0,0,0,0,0,0, (char *)MesMailbox_EMail },
    { DI_EDIT, 22, 4, 63,5,0,(DWORD)HistoryEMail,DIF_HISTORY,0, NULLSTR },
    { DI_TEXT, 6, 5, 0,0,0,0,0,0, (char *)MesMailbox_Organization },
    { DI_EDIT, 22, 5, 63,5,0,(DWORD)HistoryOrg,DIF_HISTORY,0, NULLSTR },
    { DI_CHECKBOX, 6, 6, 0, 0, 0,0,0,0, (char*)MesMailbox_DefaultChkBx },

    // auth
    { DI_CHECKBOX, 20, 8, 0, 0, 0,0,0,0, (char*)MesMailbox_UseAuthChkBx },

    { DI_TEXT, 7, 9, 0,0,0,0,0,0, (char *)MesMailbox_SMTPServer },
    { DI_EDIT, 20, 9, 52,9,0,(DWORD)HistoryUrl,DIF_HISTORY,0, NULLSTR },
    { DI_TEXT, 55, 9, 0,0,0,0,0,0, (char *)MesMailbox_SMTPPort },
    { DI_FIXEDIT, 60, 9, 64,9,0,0,0,0, NULLSTR },
#ifdef FARMAIL_SSL
    { DI_CHECKBOX, 55, 10, 0, 0, 0,0,0,0, (char*)MesMailbox_UseSSLChkBx},
#endif

    // Auth
    { DI_TEXT, 7, 10, 0,0,0,0,0,0, (char *)MesMailbox_SMTPUser },
    { DI_EDIT, 20, 10, 29,10,0,0,0,0, NULLSTR },
    { DI_TEXT, 32, 10, 0,0,0,0,0,0, (char *)MesMailbox_SMTPPass },
    { DI_PSWEDIT, 42, 10, 52,10,0,0,0,0, NULLSTR },

    { DI_RADIOBUTTON, 6, 12, 0, 0, 0, 0, DIF_GROUP, 0, (char*)MesMailbox_POP3 },
    { DI_RADIOBUTTON, 17, 12, 0, 0, 0, 0, 0, 0, (char*)MesMailbox_IMAP4 },

    { DI_TEXT, 7, 13, 0,0,0,0,0,0, (char *)MesMailbox_POP3IMAP4Server },
    { DI_EDIT, 20, 13, 52,13,0,(DWORD)HistoryUrl,DIF_HISTORY,0, NULLSTR },
    { DI_TEXT, 55, 13, 0,0,0,0,0,0, (char *)MesMailbox_POP3IMAP4Port },
    { DI_FIXEDIT, 60, 13, 64,13,0,0,0,0, NULLSTR },
#ifdef FARMAIL_SSL
    { DI_CHECKBOX, 55, 14, 0, 0, 0,0,0,0, (char*)MesMailbox_UseSSLChkBx},
#endif
    { DI_TEXT, 7, 14, 0,0,0,0,0,0, (char *)MesMailbox_POP3IMAP4Login },
    { DI_EDIT, 20, 14, 52,14,0,0,0,0, NULLSTR },
    { DI_TEXT, 7, 15, 0,0,0,0,0,0, (char *)MesMailbox_POP3IMAP4Password },
    { DI_PSWEDIT, 20, 15, 52,15,0,0,0,0, NULLSTR },
    { DI_TEXT, 7, 16, 0,0,0,0,0,0, (char *)MesMailbox_AM_1 },
    { DI_FIXEDIT, 32, 16, 32, 16, 0,0,0,0, NULLSTR },
    { DI_TEXT, 9, 17, 0,0,0,0,0,0, (char *)MesMailbox_UIDL_2 },
    { DI_CHECKBOX, 0, 17, 0, 0, 0,0,0,0, (char *)MesMailbox_UIDL_3 },
    { DI_TEXT, 9, 18, 0,0,0,0,0,0, (char *)MesMailbox_MinTOP_2 },
    { DI_FIXEDIT, 0, 18, 0, 0, 0,0,0,0, NULLSTR },
    { DI_TEXT, 0, 18, 0,0,0,0,0,0, (char *)MesMailbox_MinTOP_3 },

    { DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,1,(char *)MesOk },
    { DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,0,(char *)MesCancel },

    { DI_TEXT,3,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULLSTR },
    { DI_TEXT,3,11,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULLSTR },
    { DI_TEXT,3,19,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULLSTR },
    { DI_TEXT, 6, 8, 0,0,0,0,0,0, (char *)MesMailbox_SMTP },
    { DI_TEXT, 34, 16, 0,0,0,0,0,0, (char *)MesMailbox_AM_2 },
    { DI_TEXT, 7, 17, 0,0,0,0,0,0, (char *)MesMailbox_UIDL_1 },
    { DI_TEXT, 7, 18, 0,0,0,0,0,0, (char *)MesMailbox_MinTOP_1},


  };

  enum { FLD_TXT1 = 1,
         FLD_NAME,
         FLD_TXT2,
         FLD_USER,
         FLD_TXT3,
         FLD_EMAIL,
         FLD_TXT4,
         FLD_ORG,
         FLD_DEFAULT,

         FLD_USEAUTH,

         FLD_TXT5,
         FLD_SMTPURL,
         FLD_TXT6,
         FLD_SMTPPORT,
#ifdef FARMAIL_SSL
         FLD_USESMTPS,
#endif

         FLD_TXT_1,
         FLD_SMTP_USER,
         FLD_TXT_2,
         FLD_SMTP_PASS,

         FLD_CHKBX1,
         FLD_CHKBX2,
         FLD_TXT7,
         FLD_POPURL,
         FLD_TXT8,
         FLD_POPPORT,
#ifdef FARMAIL_SSL
         FLD_USEPOP3S,
#endif
         FLD_TXT9,
         FLD_LOGIN,
         FLD_TXT10,
         FLD_PASS,
         FLD_TXT11,
         FLD_TIMEOUT,
         FLT_TXT_UIDL,
         FLT_UIDL,
         FLT_TXT_MINTOP1,
         FLT_MINTOP,
         FLT_TXT_MINTOP2,

         FLD_OK,
         FLD_CANCEL,
       };

#ifdef FARMAIL_SSL
  int IDs[] = { FLD_CHKBX1, FLD_CHKBX2, FLD_POPPORT, FLT_TXT_UIDL, FLT_UIDL, FLT_TXT_MINTOP1, FLT_MINTOP, FLT_TXT_MINTOP2, FLD_USEPOP3S, FLD_USESMTPS, FLD_SMTPPORT };
#else
  int IDs[] = { FLD_CHKBX1, FLD_CHKBX2, FLD_POPPORT, FLT_TXT_UIDL, FLT_UIDL, FLT_TXT_MINTOP1, FLT_MINTOP, FLT_TXT_MINTOP2};
#endif
  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  //calculate positions
  DialogItems[FLT_UIDL].X1=DialogItems[FLT_TXT_UIDL].X1+lstrlen(DialogItems[FLT_TXT_UIDL].Data)+(strchr(DialogItems[FLT_TXT_UIDL].Data,'&')?0:1);
  DialogItems[FLT_MINTOP].X1=DialogItems[FLT_TXT_MINTOP1].X1+lstrlen(DialogItems[FLT_TXT_MINTOP1].Data)+(strchr(DialogItems[FLT_TXT_MINTOP1].Data,'&')?0:1);
  DialogItems[FLT_MINTOP].X2=DialogItems[FLT_MINTOP].X1+3;
  DialogItems[FLT_TXT_MINTOP2].X1=DialogItems[FLT_MINTOP].X2+2;

  if ( isnew ) {
     server->SMTPPort = DEF_SMTPPORT;
     server->Port = DEF_POP3PORT;
  }
  olds = *server;

  lstrcpy( DialogItems[FLD_NAME].Data , server->Name );
  lstrcpy( oldname , server->Name );
  lstrcpy( DialogItems[FLD_USER].Data , server->UserName );
  lstrcpy( DialogItems[FLD_EMAIL].Data , server->EMail );
  lstrcpy( DialogItems[FLD_SMTPURL].Data , server->SMTPUrl );
  FSF.sprintf( DialogItems[FLD_SMTPPORT].Data , D1, server->SMTPPort );
  lstrcpy( DialogItems[FLD_POPURL].Data , server->Url );
  FSF.sprintf( DialogItems[FLD_POPPORT].Data , D1, server->Port );
  lstrcpy( DialogItems[FLD_LOGIN].Data , server->User );
  lstrcpy( DialogItems[FLD_PASS].Data , server->Pass );
  FSF.sprintf( DialogItems[FLD_TIMEOUT].Data , D1, server->timeout );
  lstrcpy( DialogItems[FLD_ORG].Data , server->Organization );
  if ( server->Default ) DialogItems[FLD_DEFAULT].Selected = 1;

  lstrcpy( DialogItems[FLD_SMTP_USER].Data , server->SMTP_User );
  lstrcpy( DialogItems[FLD_SMTP_PASS].Data , server->SMTP_Pass );
  if ( server->UseAuth ) DialogItems[FLD_USEAUTH].Selected = 1;
#ifdef FARMAIL_SSL
  if ( server->UseSMTPS ) DialogItems[FLD_USESMTPS].Selected = 1;
  if ( server->UsePOP3S ) DialogItems[FLD_USEPOP3S].Selected = 1;
#endif

  DialogItems[FLD_CHKBX1+server->Type].Selected = 1;
  if(server->Type==TYPE_IMAP4)
  {
    DialogItems[FLT_TXT_UIDL].Flags|=DIF_DISABLE;
    DialogItems[FLT_UIDL].Flags|=DIF_DISABLE;
    DialogItems[FLT_TXT_MINTOP1].Flags|=DIF_DISABLE;
    DialogItems[FLT_MINTOP].Flags|=DIF_DISABLE;
    DialogItems[FLT_TXT_MINTOP2].Flags|=DIF_DISABLE;
  }
  DialogItems[FLT_UIDL].Selected=server->uidl;
  FSF.sprintf(DialogItems[FLT_MINTOP].Data,D1,server->TopValue);

  do {

     key = _Info.DialogEx(_Info.ModuleNumber,-1,-1,71,23,"MailboxSettings",DialogItems, sizeof(DialogItems)/sizeof(DialogItems[0]), 0,0,EditMailboxDialogFunc,(long)IDs);
     if ( key == FLD_OK ) {

        // common check
        if (
             !lstrcmp( DialogItems[FLD_NAME].Data, NULLSTR )
             //|| !lstrcmp( DialogItems[FLD_USER].Data, NULLSTR )
             || !lstrcmp( DialogItems[FLD_EMAIL].Data, NULLSTR )
             || !lstrcmp( DialogItems[FLD_SMTPURL].Data, NULLSTR )
             //|| !lstrcmp( DialogItems[FLD_POPURL].Data, NULLSTR )
             //|| !FSF.atoi(DialogItems[FLD_SMTPPORT].Data)
             //|| !FSF.atoi(DialogItems[FLD_POPPORT].Data)
           )
           SayError( GetMsg( MesMailbox_MissingFields ) );
        else {

           lstrcpyn( server->Name, DialogItems[FLD_NAME].Data , 80 );
           lstrcpyn( server->UserName, DialogItems[FLD_USER].Data , 80 );
           lstrcpyn( server->EMail, DialogItems[FLD_EMAIL].Data , 80 );
           lstrcpyn( server->SMTPUrl, DialogItems[FLD_SMTPURL].Data , 80 );
           lstrcpyn( server->Url, DialogItems[FLD_POPURL].Data , 80 );
           lstrcpyn( server->User, DialogItems[FLD_LOGIN].Data , 80 );
           lstrcpyn( server->Organization, DialogItems[FLD_ORG].Data , 80 );
           lstrcpyn( server->Pass, DialogItems[FLD_PASS].Data , 70 );
           lstrcpyn( server->SMTP_User, DialogItems[FLD_SMTP_USER].Data , 80 );
           lstrcpyn( server->SMTP_Pass, DialogItems[FLD_SMTP_PASS].Data , 70 );

#ifdef FARMAIL_SSL
           if ( DialogItems[FLD_USESMTPS].Selected )
              server->UseSMTPS = 1;
           else
              server->UseSMTPS = 0;

           if ( DialogItems[FLD_USEPOP3S].Selected )
              server->UsePOP3S = 1;
           else
              server->UsePOP3S = 0;
#endif

           if (*DialogItems[FLD_SMTPPORT].Data)
             server->SMTPPort = FSF.atoi( DialogItems[FLD_SMTPPORT].Data );
           else
           {
#ifdef FARMAIL_SSL
             server->SMTPPort = server->UseSMTPS?DEF_SMTPSPORT:DEF_SMTPPORT;
#else
             server->SMTPPort = DEF_SMTPPORT;
#endif
           }

           if ( DialogItems[FLD_CHKBX1].Selected )
              server->Type = TYPE_POP3;
           else
              server->Type = TYPE_IMAP4;

           if (*DialogItems[FLD_POPPORT].Data)
             server->Port = FSF.atoi( DialogItems[FLD_POPPORT].Data );
           else
           {
#ifdef FARMAIL_SSL
             server->Port=(server->Type==TYPE_POP3?(server->UsePOP3S?DEF_POP3SPORT:DEF_POP3PORT):(server->UsePOP3S?DEF_IMAP4SPORT:DEF_IMAP4PORT));
#else
             server->Port = (server->Type == TYPE_POP3 ? DEF_POP3PORT : DEF_IMAP4PORT);
#endif
           }

           server->timeout = FSF.atoi(DialogItems[FLD_TIMEOUT].Data);
           server->uidl=DialogItems[FLT_UIDL].Selected;
           server->TopValue=FSF.atoi(DialogItems[FLT_MINTOP].Data);

           if ( DialogItems[FLD_DEFAULT].Selected )
              server->Default = 1;
           else
              server->Default = 0;

           if ( DialogItems[FLD_USEAUTH].Selected )
              server->UseAuth = 1;
           else
              server->UseAuth = 0;

           if ( check( server ) ) {
              SayError( GetMsg( MesMailbox_InvalidChars ) );
              *server = olds;
              key = 1;
           } else {
              if ( server->Default ) ClearDefault();
              if ( isnew ) key = InsertMailbox( server );
              else         {
                           if ( !lstrcmp( oldname, server->Name ) )
                              key = UpdateMailbox( server );
                           else
                              key = UpdateAndRenameMailbox( server , oldname );
              }
           }
        }
     } else key = -1;
  } while ( key > 0 );
  return key;
}




int FARMail::ReadServers( void )
{
 DWORD SubKeys = 0;
 HKEY hRoot = HKEY_CURRENT_USER;
 ServerCount = 0;
 server = NULL;
 char clss[512];
 DWORD s_clss = 512;
 FILETIME ft;
 DWORD maxsubkey, maxclass, nvalues, maxvaluename, maxvaluedata,
       secdesc;

 HKEY hKey = OpenRegKey2( hRoot, PluginMailBoxKey, NULLSTR );

 RegQueryInfoKey( hKey,
                  clss,
                  &s_clss,
                  NULL,
                  &SubKeys,
                  &maxsubkey,
                  &maxclass,
                  &nvalues,
                  &maxvaluename,
                  &maxvaluedata,
                  &secdesc,
                  &ft );


 if ( SubKeys ) {
    char name[80];

    server = (POPSERVER*)z_calloc( SubKeys, sizeof( POPSERVER ) );
    if ( server ) {
       DWORD i = 0;

       if ( hKey ) {
          DWORD namesize = 80;
          while ( ( RegEnumKeyEx( hKey, i, name, &namesize, NULL, NULL, NULL, &ft ) ) == ERROR_SUCCESS ) {
             namesize = 80;
             FillMailbox( &server[i++], name );
             lstrcpy( server[i-1]._User, server[i-1].User );
             lstrcpy( server[i-1]._Pass, server[i-1].Pass );
          }
          ServerCount = i;
       }
    } else SayError( GetMsg( MesNoMem ) );
 }
 RegCloseKey( hKey );
 return 0;
}




int FARMail::ClearDefault( void )
{
 DWORD SubKeys = 0;
 HKEY hRoot = HKEY_CURRENT_USER;
 POPSERVER _server;
 char clss[512];
 DWORD s_clss = 512;
 FILETIME ft;
 DWORD maxsubkey, maxclass, nvalues, maxvaluename, maxvaluedata,
       secdesc;

 HKEY hKey = OpenRegKey2( hRoot, PluginMailBoxKey, NULLSTR );

 RegQueryInfoKey( hKey,
                  clss,
                  &s_clss,
                  NULL,
                  &SubKeys,
                  &maxsubkey,
                  &maxclass,
                  &nvalues,
                  &maxvaluename,
                  &maxvaluedata,
                  &secdesc,
                  &ft );

 if ( SubKeys ) {
    char name[80];
    DWORD i = 0;

    if ( hKey ) {
       DWORD namesize = 80;
       while ( ( RegEnumKeyEx( hKey, i, name, &namesize, NULL, NULL, NULL, &ft ) ) == ERROR_SUCCESS ) {
          namesize = 80;
          FillMailbox( &_server, name );
          _server.Default = 0;
          UpdateMailbox(&_server );
          i++;
       }
    }
 }
 RegCloseKey( hKey );
 return 0;
}

const char UidlKey[]="uidls";

DWORD FARMail::GetUidlState(const char *uidl)
{
  HKEY hRoot=HKEY_CURRENT_USER;
  DWORD Res=0;
  GetRegKey2(hRoot,current->MailboxPath,UidlKey,uidl,(int *)&Res,MESSAGE_STATE_NEW);
  return Res;
}
