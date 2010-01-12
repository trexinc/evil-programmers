/*
    AddressBook sub-plugin for FARMail
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
#include "plugin.hpp"
#include "farkeys.hpp"
#include "farcolor.hpp"
#include "../FARMail/fmp.hpp"
#include "language.hpp"
#include "memory.hpp"
#include "registry.hpp"
#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

#if defined(__GNUC__)

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}

void *memcpy(void * dst, const void * src, size_t count)
{
  void * ret = dst;

  while (count--)
  {
    *(char *)dst = *(char *)src;
    dst = (char *)dst + 1;
    src = (char *)src + 1;
  }
  return(ret);
}
#endif

struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  unsigned int Selected;
  unsigned int Flags;
  unsigned char DefaultButton;
  const char *Data;
};

struct OPTIONS
{
  char ADRBOOK[MAX_PATH];
  char AdrBookFld[4];
  char AdrBookDelim[2];
  char AdrBookDevide[3];
  char AdrBookSort[2];
  char AdrBookDir[MAX_PATH];
} Opt;

char PluginRootKey[80];
static const char ABFLD[] = "AdrBookFld";
static const char ABDLM[] = "AdrBookDelim";
static const char ABSRT[] = "AdrBookSort";
static const char ABDEV[] = "AdrBookDevide";
static const char ABDIR[] = "AdrBookDir";
static const char ADRBOOK[] = "AdrBook";
const char NULLSTR[] = "";

struct PluginStartupInfo FInfo;
FARSTANDARDFUNCTIONS FSF;
struct MailPluginStartupInfo MInfo;

static void Config(void);
static void ReadRegistry(void);
static int EditAddressBook( char *buf );
static int SayError( int s );

typedef struct _ADRREC {
   char *string;
   char Name[80];
   char EMail[80];
   char Comment[80];
   int  InUse;
   int  selected;
   int  num;
} ADRREC;

static char* GetMsg(int MsgNum, char *Str)
{
  MInfo.GetMsg(MInfo.MessageName,MsgNum,Str);
  return Str;
}

static void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item, int ItemsNumber)
{
  for (int I=0;I<ItemsNumber;I++)
  {
    Item[I].Type=Init[I].Type;
    Item[I].X1=Init[I].X1;
    Item[I].Y1=Init[I].Y1;
    Item[I].X2=Init[I].X2;
    Item[I].Y2=Init[I].Y2;
    Item[I].Focus=Init[I].Focus;
    Item[I].Selected=Init[I].Selected;
    Item[I].Flags=Init[I].Flags;
    Item[I].DefaultButton=Init[I].DefaultButton;
    if ((unsigned int)Init[I].Data<2000)
      GetMsg((unsigned int)Init[I].Data,Item[I].Data);
    else
      lstrcpy(Item[I].Data,Init[I].Data);
  }
}

static int SayError( int s )
{
  static const char *err[3];
  char tmp[100], tmp2[100], tmp3[100];
  err[0] = GetMsg(MesError, tmp );
  err[1] = GetMsg(s,tmp2);
  err[2] = GetMsg(MesOk, tmp3 );
  FInfo.Message( FInfo.ModuleNumber, FMSG_WARNING, NULL, err, 3, 1 );
  return 0;
}

static int Confirm( int title )
{
  static const char *mes[2];
  char tmp[100];
  mes[0] = NULLSTR;
  mes[1] = GetMsg(title,tmp);
  return FInfo.Message( FInfo.ModuleNumber, FMSG_MB_YESNO, NULL, mes, 2, 0 );
}

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct MailPluginStartupInfo *FarMailInfo)
{
  FInfo=*FarInfo;
  FSF=*FarInfo->FSF;
  MInfo=*FarMailInfo;
  FSF.sprintf(PluginRootKey,"%s\\AddressBook",MInfo.RootKey);
  ReadRegistry();
  FSF.sprintf(Opt.ADRBOOK,"%saddressbook.adr",Opt.AdrBookDir);
  return 0;
}

void WINAPI _export Exit(void)
{
}

int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData)
{
  (void)InData;
  switch(Msg)
  {
    case FMMSG_GETINFO:
      {
        GetInfoOutData *data=(GetInfoOutData *)OutData;
        GetMsg(MesMenu_Title,data->MenuString);
        lstrcpy(data->HotkeyID,"generic_addressbook");
        data->Flags=FMMSG_MENU|FMMSG_CONFIG|FMMSG_ADDRESSBOOK;
      }
      return TRUE;

    case FMMSG_ADDRESSBOOK:
      {
        EditAddressBook(((AddressOutData *)OutData)->Email);
      }
      return TRUE;

    case FMMSG_CONFIG:
      {
        Config();
      }
      return TRUE;

    case FMMSG_MENU:
      {
        char buf[512];
        memset(buf,0,sizeof(buf));
        EditAddressBook(buf);
        FInfo.EditorControl(ECTL_INSERTTEXT,buf);
      }
      return TRUE;
  }
  return FALSE;
}

static void ReadRegistry(void)
{
  HKEY hRoot = HKEY_CURRENT_USER;
  GetRegKey2( hRoot, PluginRootKey, NULLSTR, ABFLD, Opt.AdrBookFld, "NEC", 4 );
  GetRegKey2( hRoot, PluginRootKey, NULLSTR, ABDLM, Opt.AdrBookDelim, "|", 2 );
  GetRegKey2( hRoot, PluginRootKey, NULLSTR, ABSRT, Opt.AdrBookSort, "N", 2 );
  GetRegKey2( hRoot, PluginRootKey, NULLSTR, ABDEV, Opt.AdrBookDevide, "50", 3 );
  int dev = FSF.atoi(Opt.AdrBookDevide);
  if (dev<1 || dev>99)
    lstrcpy(Opt.AdrBookDevide,"50");
  char path[MAX_PATH];
  lstrcpy(path,MInfo.ModuleName);
  *(FSF.PointToName(path)) = 0;
  GetRegKey2( hRoot, PluginRootKey, NULLSTR, ABDIR, Opt.AdrBookDir, path, MAX_PATH );
  if (!*Opt.AdrBookDir)
  {
    lstrcpy(Opt.AdrBookDir,path);
  }
  else
  {
    FSF.Unquote(Opt.AdrBookDir);
    FSF.AddEndSlash(Opt.AdrBookDir);
  }
}

static void Config(void)
{
  static struct InitDialogItem InitItems[]=
  {
    // type, x1, y1, x2, y2, focus, selected, flags, default, data
    { DI_DOUBLEBOX,3,1,50,13,0,0,0,0,(char*)MesConfig_Title }, // -9

    { DI_TEXT,     5, 2, 0,0, 0,0,0,0, (char*)MesConfig_FieldsOrder },
    { DI_FIXEDIT,  46,2,48,0, 1,0,0,0, NULLSTR },

    { DI_TEXT,     5, 3, 0,0, 0,0,0,0, (char*)MesConfig_FieldsDelimiter },
    { DI_FIXEDIT,  48,3,48,0, 0,0,0,0, NULLSTR },

    { DI_TEXT,     5, 4, 0,0, 0,0,0,0, (char*)MesConfig_FieldsDevide },
    { DI_TEXT,     48,4,48,0, 0,0,0,0, "%" },
    { DI_FIXEDIT,  46,4,47,0, 0,0,0,0, NULLSTR },

    { DI_TEXT,     5, 5, 0,0, 0,0,0,0, (char*)MesConfig_Sort },
    { DI_RADIOBUTTON, 6, 6, 0, 0, 0, 0, DIF_GROUP, 0, (char*)MesConfig_SortByName },
    { DI_RADIOBUTTON, 6, 7, 0, 0, 0, 0, 0, 0,         (char*)MesConfig_SortByEMail },
    { DI_RADIOBUTTON, 6, 8, 0, 0, 0, 0, 0, 0,         (char*)MesConfig_SortByComment },

    { DI_TEXT,     5, 9, 0,0, 0,0,0,0, (char*)MesConfig_AdrBookDir },
    { DI_EDIT,     5, 10,48,0, 1,0,0,0, NULLSTR },

    { DI_TEXT, 3,11,0,0,0,0,DIF_SEPARATOR,0,NULLSTR},

    { DI_BUTTON, 0, 12,0,0,0,0,DIF_CENTERGROUP,1,(char *)MesOk },
    { DI_BUTTON, 0, 12,0,0,0,0,DIF_CENTERGROUP,0,(char *)MesCancel }
  };

  enum {
         C_ORDER = 2,
         C_TXT1,
         C_DELIM,
         C_TXTDEVIDE,
         C_TXTDEVIDE2,
         C_DEVIDE,
         C_TXT2,
         C_O_NAME,
         C_O_MAIL,
         C_O_COMMENT,
         C_TXT3,
         C_ABDIR,
         C_TXT4,
         C_OK,
         C_CANCEL
         };

  struct FarDialogItem DialogItems[sizeofa(InitItems)];
  InitDialogItems(InitItems,DialogItems,sizeofa(InitItems));

  ReadRegistry();

  lstrcpy(DialogItems[C_ORDER].Data, Opt.AdrBookFld);
  lstrcpy(DialogItems[C_DELIM].Data, Opt.AdrBookDelim);
  lstrcpy(DialogItems[C_ABDIR].Data, Opt.AdrBookDir);
  lstrcpy(DialogItems[C_DEVIDE].Data, Opt.AdrBookDevide);

  switch( Opt.AdrBookSort[0] )
  {
    case 'E':
    case 'e':
      DialogItems[C_O_MAIL].Selected = TRUE;
      break;

    case 'C':
    case 'c':
      DialogItems[C_O_COMMENT].Selected = TRUE;
      break;

    case 'N':
    case 'n':
    default:
      DialogItems[C_O_NAME].Selected = TRUE;
      break;
  }

  if (C_OK == FInfo.DialogEx(FInfo.ModuleNumber,-1,-1,54,15,"AddrBookSet",DialogItems,sizeofa(DialogItems),0,0,MInfo.ShowHelpDlgProc,(long)MInfo.ModuleName))
  {
    if (DialogItems[C_O_MAIL].Selected) Opt.AdrBookSort[0] = 'E';
    else if (DialogItems[C_O_COMMENT].Selected) Opt.AdrBookSort[0] = 'C';
    else Opt.AdrBookSort[0] = 'N';
    Opt.AdrBookSort[1] = 0;

    lstrcpyn(Opt.AdrBookFld, DialogItems[C_ORDER].Data ,4);
    lstrcpyn(Opt.AdrBookDelim, DialogItems[C_DELIM].Data ,2);
    lstrcpyn(Opt.AdrBookDir, DialogItems[C_ABDIR].Data, MAX_PATH);
    lstrcpyn(Opt.AdrBookDevide, DialogItems[C_DEVIDE].Data ,3);
    int dev = FSF.atoi(Opt.AdrBookDevide);
    if (dev<1 || dev>99)
      lstrcpy(Opt.AdrBookDevide,"50");
    if (!*Opt.AdrBookDir)
    {
      char path[MAX_PATH];
      lstrcpy(path,MInfo.ModuleName);
      *(FSF.PointToName(path)) = 0;
      lstrcpy(Opt.AdrBookDir,path);
    }
    else
    {
      FSF.Unquote(Opt.AdrBookDir);
      FSF.AddEndSlash(Opt.AdrBookDir);
    }
    FSF.sprintf(Opt.ADRBOOK,"%saddressbook.adr",Opt.AdrBookDir);

    HKEY hRoot = HKEY_CURRENT_USER;
    SetRegKey2(hRoot, PluginRootKey, NULLSTR, ABFLD, Opt.AdrBookFld);
    SetRegKey2(hRoot, PluginRootKey, NULLSTR, ABDLM, Opt.AdrBookDelim);
    SetRegKey2(hRoot, PluginRootKey, NULLSTR, ABSRT, Opt.AdrBookSort);
    SetRegKey2(hRoot, PluginRootKey, NULLSTR, ABDIR, Opt.AdrBookDir);
    SetRegKey2(hRoot, PluginRootKey, NULLSTR, ABDEV, Opt.AdrBookDevide);
  }
}


static int AddLine( char *ptr, int i, ADRREC *aptr )
{
 if ( i<3 ) {

    switch( Opt.AdrBookFld[i] ) {

       case 'N':
       case 'n':
          lstrcpyn( aptr->Name, ptr, 80 );
          aptr->Name[79] = 0;
          break;

       case 'E':
       case 'e':
          lstrcpyn( aptr->EMail, ptr, 80 );
          aptr->EMail[79] = 0;
          break;

       case 'C':
       case 'c':
          lstrcpyn( aptr->Comment, ptr, 80 );
          aptr->Comment[79] = 0;
          break;
    }
 }
 return 0;
}


char *strchr(register const char *s,int c)
{
  do
  {
    if(*s==c)
    {
      return (char*)s;
    }
  } while (*s++);
  return (0);
}

static int MakeAdrRec( ADRREC *aptr , char *text )
{
 char *ptr = text, *ptr2;
 int i = 0;

 while ( ( ptr2 = strchr( ptr, Opt.AdrBookDelim[0] ) ) != NULL ) {

    *ptr2 = 0;
    AddLine( ptr, i , aptr );
    ptr = ptr2+1;

    i++;
 }
 AddLine( ptr, i , aptr );

 return 0;
}



static int MakeString( ADRREC *aptr )
{
 char buf[1000];
 int i;

 *buf = 0;

 for ( i=0 ; i<3 ; i++ ) {

    switch( Opt.AdrBookFld[i] ) {

       case 'N':
       case 'n':
          lstrcat( buf, aptr->Name );
          break;

       case 'E':
       case 'e':
          lstrcat( buf, aptr->EMail );
          break;

       case 'C':
       case 'c':
          lstrcat( buf, aptr->Comment );
          break;
    }
    if ( i < 2 ) lstrcat( buf, Opt.AdrBookDelim );

 }
 aptr->string=z_strdup(buf);
 return 0;
}

static bool ReadLine(HANDLE file,char *buffer,size_t len)
{
  DWORD CurrPos=SetFilePointer(file,0,NULL,FILE_CURRENT),transferred;
  bool res=true;
  if(ReadFile(file,buffer,len-1,&transferred,NULL)&&transferred)
  {
    buffer[len-1]=0;
    DWORD len=0; char *ptr=buffer;
    for(size_t i=0;i<transferred;i++,len++,ptr++)
    {
      if(buffer[i]=='\n'||buffer[i]=='\r')
      {
        if((buffer[i+1]=='\n'||buffer[i+1]=='\r')&&buffer[i]!=buffer[i+1])
          len++;
        break;
      }
    }
    *ptr=0;
    SetFilePointer(file,CurrPos+len+1,NULL,FILE_BEGIN);
  }
  else
  {
    buffer[0]=0;
    res=false;
  }
  return res;
}

static int LoadAdrBook(ADRREC **ptr,int *num)
{
  char buffer[1024];
  char addrBook[MAX_PATH];
  FSF.ExpandEnvironmentStr(Opt.ADRBOOK,addrBook,MAX_PATH);
  HANDLE fp=CreateFile(addrBook,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
  if(fp!=INVALID_HANDLE_VALUE)
  {
    int n=0;
    while(ReadLine(fp,buffer,sizeof(buffer))) (*num)++;
    SetFilePointer(fp,0,NULL,FILE_BEGIN);
    if(*num)
    {
      *ptr=(ADRREC*)z_calloc(*num,sizeof(ADRREC));
      if(!ptr)
      {
        *num=0; return 1;
      }
      while(ReadLine(fp,buffer,sizeof(buffer)))
      {
        FSF.RTrim(buffer);
        (*ptr)[n].string=z_strdup(buffer);
        FSF.Trim(buffer);
        if(*buffer && *buffer!=';')
        {
          (*ptr)[n].InUse=1;
          MakeAdrRec(&(*ptr)[n],buffer);

        }
        else
        {
          (*ptr)[n].InUse=0;
        }
        n++;
      }
    }
    CloseHandle(fp);
  }
  return 0;
}

static int LoadMenu( ADRREC *aptr , int num, ADRREC **mptr, int *mnum )
{
  for (int i=0 ; i<num; i++ )
    if ( aptr[i].InUse )
      (*mnum)++;

  if ( *mnum )
  {
    *mptr = (ADRREC*)z_calloc( *mnum, sizeof( ADRREC ) );
  }
  else
  {
    *mptr = NULL;
    return 0;
  }

  if ( *mptr )
  {
    for (int i=0, j=0 ; i<num; i++ )
    {
      if ( aptr[i].InUse )
      {
        memcpy((*mptr)+j,aptr+i,sizeof(ADRREC));
        (*mptr)[j].num = i;
        (*mptr)[j].string = NULL;
        j++;
      }
    }
  }
  else
  {
    *mnum = 0;
    return 1;
  }
  return 0;
}

void *memset(void *s,int c,size_t n)
{
  BYTE *dst=(BYTE *)s;
  while(n--)
  {
    *dst=(BYTE)(unsigned)c;
    ++dst;
  }
  return s;
}

char *strstr(const char * str1, const char * str2 )
{
  char *cp = (char *) str1;
  char *s1, *s2;

  if ( !*str2 )
    return((char *)str1);

  while (*cp)
  {
    s1 = cp;
    s2 = (char *) str2;

    while ( *s1 && *s2 && !(*s1-*s2) )
      s1++, s2++;

    if (!*s2)
      return(cp);

    cp++;
  }

  return(NULL);
}


static int SaveMenu( ADRREC **aptr , int *num, ADRREC **mptr, int mnum )
{
 int i;

 for ( i=0; i<mnum; i++ )
 {

    int j = (*mptr)[i].num;

    if ( (*mptr)[i].InUse == -1 )
    {
       if ( j>=0 )
       {
          // delete
          (*aptr)[j].InUse = -1;
       }
    }
    else if ( j==-1 ) {
       // new

       (*num)++;
       *aptr = (ADRREC*)z_realloc( *aptr, sizeof(ADRREC)*(*num) );
       if ( *aptr ) {

          memset(  &(*aptr)[(*num)-1] , 0, sizeof( ADRREC) );

          (*aptr)[(*num)-1] = (*mptr)[i];
          (*aptr)[(*num)-1].InUse = 1;

       } else return 1;

    } else {

       lstrcpy( (*aptr)[j].Name,    (*mptr)[i].Name );
       lstrcpy( (*aptr)[j].EMail,   (*mptr)[i].EMail );
       lstrcpy( (*aptr)[j].Comment, (*mptr)[i].Comment );

    }

 }
 if (*mptr)
 {
   z_free(*mptr);
   *mptr=NULL;
 }
 return 0;
}



static int SaveAdrBook( ADRREC **ptr , int num )
{
  if (!*ptr)
    return 0;

  char tail[2]={'\r','\n'}; DWORD transferred;
  char addrBook[MAX_PATH];
  FSF.ExpandEnvironmentStr(Opt.ADRBOOK,addrBook,MAX_PATH);
  HANDLE fp=CreateFile(addrBook,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
  if(fp!=INVALID_HANDLE_VALUE)
  {
    for(int i=0;i<num;i++)
    {
      switch((*ptr)[i].InUse)
      {

        case 0: // comment
          if((*ptr)[i].string)
          {
            WriteFile(fp,(*ptr)[i].string,lstrlen((*ptr)[i].string),&transferred,NULL);
            WriteFile(fp,tail,sizeof(tail),&transferred,NULL);
            z_free((*ptr)[i].string);
          }
          break;
        case -1: // skip!
          break;
        case 1: // record
          if((*ptr)[i].string) z_free((*ptr)[i].string);
          (*ptr)[i].string=NULL;
          // make new string
          MakeString(&(*ptr)[i]);
          if((*ptr)[i].string)
          {
            WriteFile(fp,(*ptr)[i].string,lstrlen((*ptr)[i].string),&transferred,NULL);
            WriteFile(fp,tail,sizeof(tail),&transferred,NULL);
            z_free((*ptr)[i].string);
          }
          break;
      }
    }
    CloseHandle(fp);
    z_free(*ptr);
    ptr=NULL;
  }
  return 0;
}

static void Sort( ADRREC *aptr, int n )
{
  int q=1, qq;

  if (!aptr) return;

  while (q)
  {
    q = 0;
    for (int i=0 ; i < (n-1) ; i++ )
    {
      switch ( Opt.AdrBookSort[0] )
      {
        case 'N':
        case 'n':
          qq = FSF.LStricmp( aptr[i].Name , aptr[i+1].Name );
          break;

        case 'E':
        case 'e':
          qq = FSF.LStricmp( aptr[i].EMail , aptr[i+1].EMail );
          break;

        case 'C':
        case 'c':
          qq = FSF.LStricmp( aptr[i].Comment , aptr[i+1].Comment );
          break;

        default:
          qq=0;
      }
      if (qq > 0 )
      {
        ADRREC z;
        q = 1;
        z = aptr[i];
        aptr[i] = aptr[i+1];
        aptr[i+1] = z;
      }
    }
  }
}

static bool InFilter(const char *str1, const char *str2, const char *filter)
{
  char localfilter[128];
  lstrcpy(localfilter,filter);
  int len=lstrlen(localfilter);
  for (int i=0; i<len; i++)
    if (localfilter[i] == ' ')
      localfilter[i] = 0;

  char *lf=localfilter;
  int i=0;
  do
  {
    while (i<len && !(*lf))
    {
      lf++;
      i++;
    }
    if (!(strstr(str1,lf) || strstr(str2,lf)))
      return false;
    while (*lf)
    {
      lf++;
      i++;
    }
  } while (i<len);

  return true;
}

static int MakeMenu(ADRREC *aptr, struct FarListItem **menu, int n, int selected, const char *filter, int **ri, int x)
{
  int i,j;
  if ( *menu ) z_free(*menu);
  if ( *ri ) z_free(*ri);

  *menu = NULL;
  *ri = NULL;

  if (!aptr || n<=0 ) return 0;

  *menu = (struct FarListItem*)z_calloc(n, sizeof(struct FarListItem));
  *ri = (int*)z_calloc(n, sizeof(int));
  if (!(*menu)||!(*ri)) return 0;

  char deletedformat[50], normalformat[50];
  GetMsg(MesMenu_DeleteFormat,deletedformat);
  GetMsg(MesMenu_NormalFormat,normalformat);

  int left = x*FSF.atoi(Opt.AdrBookDevide)/100;
  int right = x - left;
  for ( i=0, j=0; i<n ; i++ )
  {
    char full[180];
    char fulllwr[180];
    char commentlwr[80];
    if (aptr[i].InUse == -1)
      FSF.sprintf( full, deletedformat, aptr[i].Name, aptr[i].EMail );
    else
      FSF.sprintf( full, normalformat,aptr[i].Name,aptr[i].EMail );
    lstrcpy(commentlwr,aptr[i].Comment);
    lstrcpy(fulllwr,full);
    FSF.LStrlwr(commentlwr);
    FSF.LStrlwr(fulllwr);
    if (InFilter(fulllwr,commentlwr,filter))
    {
      char Text[1000];
      FSF.sprintf(Text, "%-*.*s ³ %-*.*s", left,left,full, right,right,aptr[i].Comment);
      lstrcpyn((*menu)[j].Text,Text,128);
      if (aptr[i].selected)
        (*menu)[j].Flags |= LIF_CHECKED;
      (*ri)[j++]=i;
    }
  }

  if (selected >= 0 && selected < j)
    (*menu)[selected].Flags |= LIF_SELECTED;

  return j;
}

static int PanelAddress( ADRREC *a , const char *title )
{
  static struct InitDialogItem InitItems[]=
  {
    // type, x1, y1, x2, y2, focus, selected, flags, default, data
    { DI_DOUBLEBOX,3,1,66,9,0,0,0,0, NULLSTR },

    { DI_TEXT, 5, 3, 0,0,0,0,0,0, (char *)MesAdrBook_Name },
    { DI_EDIT, 18, 3, 64,3,1,0,0,0, NULLSTR },

    { DI_TEXT, 5, 5, 0,0,0,0,0,0, (char *)MesAdrBook_EMail },
    { DI_EDIT, 18, 5, 64,5,0,0,0,0, NULLSTR },

    { DI_TEXT, 5, 7, 0,0,0,0,0,0, (char *)MesAdrBook_Comment },
    { DI_EDIT, 18, 7, 64,7,0,0,0,0, NULLSTR },

    { DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP,1,(char *)MesOk },
    { DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP,0,(char *)MesCancel }

  };

  enum
  {
    AK_TXT1=1,
    AK_NAME,
    AK_TXT2,
    AK_EMAIL,
    AK_TXT3,
    AK_COMMENT,
    AK_OK,
    AK_CANCEL,
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];

  InitItems[0].Data=title;
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  lstrcpy( DialogItems[AK_NAME].Data , a->Name );
  lstrcpy( DialogItems[AK_EMAIL].Data , a->EMail );
  lstrcpy( DialogItems[AK_COMMENT].Data , a->Comment );

  int key = FInfo.DialogEx(FInfo.ModuleNumber,-1,-1,70,12, ADRBOOK ,DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]),0,0,MInfo.ShowHelpDlgProc,(long)MInfo.ModuleName);

  lstrcpyn(  a->Name    , DialogItems[AK_NAME].Data, 80);
  lstrcpyn(  a->EMail   , DialogItems[AK_EMAIL].Data, 80);
  lstrcpyn(  a->Comment , DialogItems[AK_COMMENT].Data, 80);

  if ( key == AK_OK ) return 0; else return 1;
}


static int EditAddress( ADRREC **aptr, int n )
{
 ADRREC a = (*aptr)[n];
 char tmp[100];

 if ( !PanelAddress( &a , GetMsg( MesAdrBook_EditTitle,tmp ) ) )
 {
    (*aptr)[n] = a;
    return 0;
 }
 return 1;

}


static int AddRec( ADRREC *a, ADRREC **aptr, int *num_recs )
{
 (*num_recs)++;

 *aptr = (ADRREC*)z_realloc( *aptr, (*num_recs) * sizeof( ADRREC ) );
 if ( *aptr ) {
    (*aptr)[(*num_recs)-1] = *a;
    return 0;
 } else {
    *num_recs = 0;
    return 1;
 }

}

static int InsertAddress( ADRREC **aptr, int *num_recs )
{
  ADRREC a;
  char tmp[100];

  memset( &a, 0, sizeof( ADRREC ) );
  a.num = -1;

  if ( !PanelAddress( &a , GetMsg( MesAdrBook_NewTitle,tmp ) ) )
  {
    return AddRec( &a, aptr, num_recs );
  }
  return 1;
}



static int DeleteAddress( ADRREC **aptr, int n )
{
 if ( Confirm( MesAdrBook_Delete ) ) return 1;

 (*aptr)[n].InUse = -1;

 return 0;
}

static int RestoreAddress( ADRREC **aptr, int n )
{
 if ( Confirm( MesAdrBook_Undelete ) ) return 1;

 (*aptr)[n].InUse = 1;

 return 0;
}

enum
{
  DMU_UPDATE = DM_USER+1,
};

long WINAPI EABDProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  static int num_recs, menu_recs, old_menu_recs, i;
  static ADRREC *aptr, *mptr;
  static char *buf;
  static bool changed, sizechanged;
  static char filter[128];
  static int filterlen, sel, shown_menus;
  static FarListItem *menu;
  static int *ri;
  static char status[128];
  static int scrx, scry;

  switch(Msg)
  {
    case DN_INITDIALOG:
      buf = (char *)Param2;
      if(buf) *buf=0;
      aptr = mptr = NULL;
      old_menu_recs = num_recs = menu_recs = 0;
      *filter = 0;
      filterlen = sel = shown_menus = 0;
      menu = NULL;
      ri = NULL;
      {
        CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbiInfo);
        scrx=csbiInfo.dwSize.X;
        if (scrx>128+15)
          scrx=128+15;
        scry=csbiInfo.dwSize.Y;
        sizechanged = true;
      }
      if (LoadAdrBook(&aptr, &num_recs)||LoadMenu(aptr, num_recs, &mptr, &menu_recs))
      {
        SayError(MesNoMem);
        FInfo.SendDlgMessage(hDlg,DM_CLOSE,0,0);
        return FALSE;
      }
      changed = true;
      if (buf)
        GetMsg(MesAdrBook_StatusLine,status);
      else
        GetMsg(MesAdrBook_StatusLine2,status);
      FInfo.SendDlgMessage(hDlg,DMU_UPDATE,0,0);
      return TRUE;

    case DN_RESIZECONSOLE:
      {
        scrx = ((COORD*)Param2)->X;
        if (scrx>128+15)
          scrx=128+15;
        scry = ((COORD*)Param2)->Y;
        sizechanged = true;
        changed = true;
        FInfo.SendDlgMessage(hDlg,DMU_UPDATE,0,0);
      }
      return TRUE;

    case DN_DRAWDIALOG:
      if (old_menu_recs != menu_recs || sizechanged)
      {
        old_menu_recs = menu_recs;
        sizechanged = false;
        FInfo.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
        int s = scry-7;
        s = menu_recs<s ? menu_recs+1 : s;
        COORD coord;
        coord.X = scrx - 3;
        coord.Y = s + 3;
        FInfo.SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(long)&coord);
        coord.X = -1;
        coord.Y = -1;
        FInfo.SendDlgMessage(hDlg,DM_MOVEDIALOG,TRUE,(long)&coord);
        SMALL_RECT rect;
        FInfo.SendDlgMessage(hDlg,DM_GETITEMPOSITION,0,(long)&rect);
        rect.Bottom = s + 1;
        rect.Right = scrx - 6;
        FInfo.SendDlgMessage(hDlg,DM_SETITEMPOSITION,0,(long)&rect);
        struct FarListPos flp;
        flp.SelectPos = FInfo.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
        flp.TopPos = -1;
        FInfo.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,(long)&flp);
        FInfo.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
      }
      return TRUE;

    case DMU_UPDATE:
      if (changed)
      {
        Sort(mptr, menu_recs);
        shown_menus = MakeMenu(mptr, &menu, menu_recs, sel, filter, &ri, scrx-15);
        if (!menu)
        {
          if (InsertAddress(&mptr, &menu_recs))
          {
            FInfo.SendDlgMessage(hDlg,DM_CLOSE,0,0);
            return FALSE;
          }
          Sort(mptr, menu_recs);
          shown_menus = MakeMenu(mptr, &menu, menu_recs, sel, filter, &ri, scrx-15);
        }
        FInfo.SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
        {
          FarList list={shown_menus,menu};
          FInfo.SendDlgMessage(hDlg,DM_LISTSET,0,(long)&list);
        }
        {
          FarListTitles titles;
          char title[70];
          if (filterlen)
          {
            char tmp[100], tmp2[100];
            GetMsg(MesMenu_TitleWithFilter,tmp);
            lstrcpyn(tmp2,filter,filterlen>=(70-lstrlen(tmp2)+2)?70-lstrlen(tmp2)+2+1:filterlen+1);
            FSF.sprintf(title,tmp,tmp2);
          }
          else
            GetMsg(MesMenu_Title,title);
          titles.Title=title;
          titles.TitleLen=lstrlen(titles.Title);
          titles.Bottom=status;
          titles.BottomLen=lstrlen(titles.Bottom);
          FInfo.SendDlgMessage(hDlg,DM_LISTSETTITLES,0,(long)&titles);
        }
        changed = false;
        FInfo.SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
      }
      if (!menu||!ri)
      {
        if (menu_recs>0)
          SayError(MesNoMem);
        FInfo.SendDlgMessage(hDlg,DM_CLOSE,0,0);
        return FALSE;
      }
      return TRUE;

    case DN_KEY:
      sel = FInfo.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
      switch (Param2)
      {
        case KEY_INS:
          if (buf)
          {
            if (menu[sel].Flags&LIF_CHECKED)
            {
              menu[sel].Flags -= LIF_CHECKED;
              mptr[ri[sel]].selected = 0;
            }
            else
            {
              menu[sel].Flags |= LIF_CHECKED;
              mptr[ri[sel]].selected = 1;
            }
            if (shown_menus)
            {
              FarListUpdate listupdate;
              listupdate.Index = sel;
              listupdate.Item = menu[sel];
              FInfo.SendDlgMessage(hDlg,DM_LISTUPDATE,0,(long)&listupdate);
            }
            if (sel<shown_menus)
            {
              FarListInfo listinfo;
              FInfo.SendDlgMessage(hDlg,DM_LISTINFO,0,(long)&listinfo);
              FarListPos listpos = {++sel,(listinfo.SelectPos-listinfo.TopPos+1)>listinfo.MaxHeight?listinfo.TopPos+1:listinfo.TopPos};
              FInfo.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,(long)&listpos);
            }
          }
          return TRUE;

        case KEY_F4:
          EditAddress(&mptr, ri[sel]);
          changed = true;
          break;

        case KEY_F7:
          InsertAddress(&mptr, &menu_recs);
          changed = true;
          break;

        case KEY_F8:
          if (mptr[ri[sel]].InUse != -1)
          {
            if (buf)
            {
              if (menu[sel].Flags&LIF_CHECKED)
              {
                mptr[ri[sel]].selected = 0;
                menu[sel].Flags -= LIF_CHECKED;
                if (shown_menus)
                {
                  FarListUpdate listupdate;
                  listupdate.Index = sel;
                  listupdate.Item = menu[sel];
                  FInfo.SendDlgMessage(hDlg,DM_LISTUPDATE,0,(long)&listupdate);
                }
              }
            }
            DeleteAddress( &mptr, ri[sel] );
          }
          else
          {
            RestoreAddress( &mptr, ri[sel] );
          }
          changed = true;
          break;

        case KEY_ENTER:
          if (!buf && sel >= 0)
          {
            EditAddress( &mptr, ri[sel] );
            changed = true;
          }
          else if (shown_menus)
          {
            if (buf && sel>=0)
            {
              for (i=0; i<menu_recs; i++)
              {
                if (mptr[i].selected)
                {
                  if (lstrlen(buf) + 4 + lstrlen(mptr[i].Name) + lstrlen(mptr[i].EMail) < 510)
                  {
                    if (lstrlen(buf))
                      lstrcat(buf, ",");
                    lstrcat(buf, mptr[i].Name);
                    lstrcat(buf," <");
                    lstrcat(buf, mptr[i].EMail);
                    lstrcat(buf,">");
                  }
                }
              }
            }
            if (buf && !*buf && sel>=0)
            {
              if (lstrlen(buf) + 4 + lstrlen(mptr[ri[sel]].Name) + lstrlen(mptr[ri[sel]].EMail) < 510)
              {
                lstrcat(buf, mptr[ri[sel]].Name);
                lstrcat(buf," <");
                lstrcat(buf, mptr[ri[sel]].EMail);
                lstrcat(buf,">");
              }
            }
            FInfo.SendDlgMessage(hDlg,DM_CLOSE,0,0);
            return TRUE;
          }
          break;

        case KEY_BS:
          if (filterlen)
          {
            filter[--filterlen] = '\0';
            sel = 0;
            changed = true;
          }
          break;

        case KEY_DEL:
          if (filterlen)
          {
            *filter = '\0';
            filterlen = 0;
            sel = 0;
            changed = true;
          }
          break;

        default:
          if (Param2 >= 32 && Param2 <= 255)
          {
            if (filterlen < 127 && shown_menus && !(Param2==32 && filterlen && filter[filterlen-1]==32))
            {
              filter[filterlen] = FSF.LLower(Param2);
              filter[++filterlen] = '\0';
              sel = 0;
              changed = true;
            }
          }
          else
            return FInfo.DefDlgProc(hDlg,Msg,Param1,Param2);
      }
      FInfo.SendDlgMessage(hDlg,DMU_UPDATE,0,0);
      return TRUE;

    case DN_CTLCOLORDIALOG:
      return FInfo.AdvControl(FInfo.ModuleNumber,ACTL_GETCOLOR,(void *)COL_MENUTEXT);
    case DN_CTLCOLORDLGLIST:
      if(Param1==0)
      {
        FarListColors *Colors=(FarListColors *)Param2;
        int ColorIndex[]={COL_MENUTEXT,COL_MENUTEXT,COL_MENUTITLE,COL_MENUTEXT,COL_MENUHIGHLIGHT,COL_MENUTEXT,COL_MENUSELECTEDTEXT,COL_MENUSELECTEDHIGHLIGHT,COL_MENUSCROLLBAR,COL_MENUDISABLEDTEXT};
        int Count=sizeofa(ColorIndex);
        if(Count>Colors->ColorCount) Count=Colors->ColorCount;
        for(int i=0;i<Count;i++)
          Colors->Colors[i]=FInfo.AdvControl(FInfo.ModuleNumber,ACTL_GETCOLOR,(void *)(ColorIndex[i]));
        return TRUE;
      }
      break;

    case DN_HELP:
      FInfo.ShowHelp(MInfo.ModuleName,ADRBOOK,FHELP_SELFHELP);
      return (long)NULL;

    case DN_CLOSE:
      if (menu)
        z_free(menu);
      menu = NULL;
      if (ri)
        z_free(ri);
      ri = NULL;
      if (SaveMenu(&aptr, &num_recs, &mptr, menu_recs)||SaveAdrBook(&aptr, num_recs))
        SayError(MesNoMem);
      return TRUE;
  }
  return FInfo.DefDlgProc(hDlg,Msg,Param1,Param2);
}

static int EditAddressBook( char *buf )
{
  struct FarDialogItem DialogItems[1];
  memset(DialogItems,0,sizeof(DialogItems));
  DialogItems[0].Type=DI_LISTBOX; DialogItems[0].Flags=DIF_LISTWRAPMODE;
  DialogItems[0].X1=2; DialogItems[0].Y1=1; DialogItems[0].X2=77; DialogItems[0].Y2=2;
  FInfo.DialogEx(FInfo.ModuleNumber,-1,-1,80,4,NULL,DialogItems,sizeofa(DialogItems),0,0,EABDProc,(DWORD)buf);
  return 0;
}
