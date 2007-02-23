/*
 MailView plugin for FAR Manager
 Copyright (C) 2003-2005 Alex Yaroslavsky

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
#include <windows.h>
#include "crt.hpp"
#include "../MailboxPlugin.h"

#define FLD_FILENAME MAX_PATH
#define FLD_PATH     MAX_PATH+1
#define FLD_FULLPATH FLD_FILENAME+FLD_PATH

const char PlugName[] = "Folder";
const char NULLSTR[] = "";

void AddEndSlash(char *str)
{
  int i=lstrlen(str);
  if (i>0&&str[i-1]!='/'&&str[i-1]!='\\')
  {
    str[i]='\\';
    str[i+1]=0;
  }
}

class FLD
{
private:
  DWORD indexCount;
  DWORD AllocatedCount;
  char path[FLD_PATH];
  char **files;
public:
  FLD() {AllocatedCount=indexCount=0;*path=0;files=NULL;};
  ~FLD()
  {
    if (files)
    {
      for (DWORD i=0; i<indexCount; i++)
        if (files[i])
          free(files[i]);
      free(files);
    }
  };
  DWORD Count() {return indexCount;};
  void SetPath(const char *str) {lstrcpy(path,str);};

  BOOL GetFile( char * str, DWORD i )
  {
    *str = 0;

    if ( i < 0 || i >= indexCount || files[ i ] == NULL )
      return FALSE;

    lstrcpy( str, path );
    lstrcat( str, files[ i ] );

    return TRUE;
  };

  BOOL AddFile(const char *str)
  {
    if (indexCount>=AllocatedCount)
    {
      DWORD NewAllocatedCount=AllocatedCount+256+AllocatedCount/4;
      char **ptr = (char **)realloc(files,(NewAllocatedCount)*sizeof(char *));
      if (!ptr)
        return FALSE;
      AllocatedCount=NewAllocatedCount;
      files = ptr;
    }
    files[indexCount] = (char *)malloc(lstrlen(str)+1);
    if (!files[indexCount])
      return FALSE;
    lstrcpy(files[indexCount++],str);
    return TRUE;
  };
};

class FLDEMAIL
{
  char *email;
  DWORD size;
  public:
  FLDEMAIL() {email=NULL; size=0;};
  ~FLDEMAIL() {if (email) free(email);};
  const char *GetEmail() {return email;};
  DWORD GetSize() {return size;};
  void SetEmail(char *ptr, DWORD dwSize) {if (email) free(email); email=ptr; size=dwSize;};
};

FLD *fld_open( const char *szFileName )
{
  FLD *fld = NULL;
  char mask[FLD_FILENAME];
  char path[FLD_PATH];
  int NewOnly=GetPrivateProfileInt(PlugName,"NewOnly",0,szFileName);
  GetPrivateProfileString(PlugName,"Path",NULLSTR,path,sizeof(path)-1,szFileName);
  GetPrivateProfileString(PlugName,"Mask",NULLSTR,mask,sizeof(mask),szFileName);
  if (*mask&&*path)
  {
    fld = new FLD;
    if (!fld)
    {
      return NULL;
    }
    AddEndSlash(path);
    fld->SetPath(path);
    char find[FLD_FULLPATH];
    lstrcpy(find,path);
    lstrcat(find,mask);
    WIN32_FIND_DATA fd;
    HANDLE fh;
    fh = FindFirstFile(find,&fd);
    if (fh!=INVALID_HANDLE_VALUE)
    {
      do
      {
        if (!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
        {
          if (NewOnly && !(fd.dwFileAttributes&FILE_ATTRIBUTE_READONLY) && (fd.dwFileAttributes&(FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
            continue;
          if (!fld->AddFile(fd.cFileName))
          {
            delete fld;
            fld = NULL;
            break;
          }
        }
      } while (FindNextFile(fh,&fd));
      FindClose(fh);
    }
  }
  return fld;
}

void fld_close( FLD *fld )
{
  if (fld)
    delete fld;
}

FLDEMAIL * fld_get( FLD * fld, int iMsg, bool fHeader )
{
  char file[ FLD_FULLPATH ];

  if ( !fld->GetFile( file, iMsg ) )
    return NULL;

  FLDEMAIL * email = NULL;

  HANDLE f = CreateFile( file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if ( f != INVALID_HANDLE_VALUE )
  {
    DWORD size = GetFileSize( f, NULL );
    if ( size != INVALID_FILE_SIZE )
    {
      char * ptr = (char*)malloc( size );
      if ( ptr || size==0)
      {
        bool ret=false;
        DWORD realsize=0;

        if (size==0)
        {
          ret=true;
        }
        else if (fHeader)
        {
          DWORD rb;
          char *p=ptr;
          DWORD delta=0;
          while (ReadFile( f, p, 1024, &rb, NULL ) && rb)
          {
            ret=true;
            bool fin=false;
            p-=delta;
            if (rb-3+delta<4)
              break;
            for (int i=0; i < rb-3+delta; i++)
            {
              if (p[i]=='\r'&&p[i+1]=='\n'&&p[i+2]=='\r'&&p[i+3]=='\n')
              {
                realsize+=i+1-delta;
                fin = true;
                break;
              }
            }
            if (fin) break;
            p+=rb+delta;
            realsize+=rb;
            delta=2;
          }
        }
        else
        {
          DWORD rb;
          ret = ReadFile( f, ptr, size, &rb, NULL ) && rb;
          realsize = rb;
        }
        if (ret)
        {
          if (realsize<size)
            ptr = (char*)realloc(ptr,realsize);

          if ( (email = new FLDEMAIL) != NULL )
            email->SetEmail( ptr, realsize );
          else if (ptr)
            free( ptr );
        }
        else
        {
          if (ptr) free( ptr );
        }
      }
    }
    CloseHandle( f );
  }

  return email;
}

void fld_free_msg( FLDEMAIL *pMsg )
{
  if (pMsg)
    delete pMsg;
}

IMPLEMENT_INFORMATION( EMT_INET, PlugName, "Folder with messages", "*.fld" )

class CFLDMailbox : public CMailbox
{
private:
  FLD      *hFLD;
  FLDEMAIL *pMsg;
  int       iMsg;
  bool      fHeader;
public:
  CFLDMailbox( FLD *FLD );
  virtual ~CFLDMailbox();
  virtual DWORD GetNextMsg( DWORD dwPrevID );
  BOOL GetMsgExt( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize, bool Header);
//  virtual MV_RESULT GetMsgHead( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize );
  virtual BOOL GetMsg( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize );

  virtual BOOL GetMsgInfo( DWORD dwMsgID, PMsgInfo lpInfo );
  virtual BOOL SetMsgInfo( DWORD dwMsgID, PMsgInfo lpInfo );
  virtual BOOL DelMsg( DWORD dwMsgId );

  virtual MV_RESULT Purge();

};

PMailbox CMailbox::Create( LPCVOID lpMem, DWORD dwSize )
{
  return NULL;
}

PMailbox CMailbox::Create( LPCSTR szFileName )
{
  FLD *fld = fld_open( szFileName );
  if ( fld )
    return new CFLDMailbox( fld );
  return NULL;
}

CFLDMailbox::CFLDMailbox( FLD *fld ) : hFLD(fld), pMsg(NULL), iMsg(-1), fHeader(false)
{
}

CFLDMailbox::~CFLDMailbox()
{
  fld_close( hFLD );
}

DWORD CFLDMailbox::GetNextMsg( DWORD dwPrevID )
{
  if ( dwPrevID == BAD_MSG_ID )
  {
    if ( hFLD->Count() > 0 )
      return 0;
  }
  else if ( (dwPrevID + 1) < hFLD->Count() )
    return dwPrevID + 1;

  return BAD_MSG_ID;
}

BOOL CFLDMailbox::GetMsgExt( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize, bool Header)
{
  if ( dwMsgID == BAD_MSG_ID || lpSize == NULL )
    return FALSE;

  if ( iMsg != (int)dwMsgID || (fHeader != Header))
  {
    fld_free_msg( pMsg );
    iMsg = (int)dwMsgID;
    pMsg = (FLDEMAIL *)fld_get( hFLD, iMsg, Header );
    fHeader = Header;
  }

  if ( pMsg == NULL )
  {
    iMsg = -1;
    return FALSE;
  }

  if ( pMsg->GetEmail() == NULL )
    return FALSE;

  DWORD dwSize = pMsg->GetSize();

  if ( lpMsg == NULL )
  {
    *lpSize = dwSize;
    return TRUE;
  }

  if ( *lpSize > dwSize )
    *lpSize = dwSize;

  memcpy( lpMsg, pMsg->GetEmail(), *lpSize );

  return TRUE;
}

//MV_RESULT CFLDMailbox::GetMsgHead( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize )
//{
  //return (GetMsgExt(dwMsgID, lpMsg, lpSize, true)?MV_OK:MV_FALSE);
//}

BOOL CFLDMailbox::GetMsg( DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize )
{
  return GetMsgExt(dwMsgID, lpMsg, lpSize, false);
}

BOOL CFLDMailbox::GetMsgInfo( DWORD dwMsgId, PMsgInfo lpInfo )
{
  char fileName[ FLD_FULLPATH ];

  if ( !hFLD->GetFile( fileName, (int)dwMsgId ) )
    return FALSE;

  DWORD attributes = GetFileAttributes( fileName );

  if ( attributes & FILE_ATTRIBUTE_HIDDEN )
    lpInfo->Flags |= EMF_DELETED;
  if ( attributes & FILE_ATTRIBUTE_SYSTEM )
    lpInfo->Flags |= EMF_READED;
  if ( attributes & FILE_ATTRIBUTE_READONLY )
    lpInfo->Flags |= EMF_FLAGGED;

  // TODO: добавить lpInfo->Sent, lpInfo->Received и lpInfo->Accessed;

  return TRUE;
}

BOOL CFLDMailbox::SetMsgInfo( DWORD dwMsgId, PMsgInfo lpInfo )
{
  char fileName[ FLD_FULLPATH ];

  if ( !hFLD->GetFile( fileName, (int)dwMsgId ) )
    return FALSE;

  DWORD attributes = FILE_ATTRIBUTE_NORMAL;

  if ( lpInfo->Flags & EMF_DELETED )
    attributes |= FILE_ATTRIBUTE_HIDDEN;
  if ( lpInfo->Flags & EMF_READED )
    attributes |= FILE_ATTRIBUTE_SYSTEM;
  if ( lpInfo->Flags & EMF_FLAGGED )
    attributes |= FILE_ATTRIBUTE_READONLY;

  // TODO: добавить lpInfo->Sent, lpInfo->Received и lpInfo->Accessed;

  return SetFileAttributes( fileName, attributes );
}

BOOL CFLDMailbox::DelMsg( DWORD dwMsgId )
{
  char fileName[ FLD_FULLPATH ];

  if ( !hFLD->GetFile( fileName, (int)dwMsgId ) )
    return FALSE;

  return DeleteFile( fileName );
}

MV_RESULT CFLDMailbox::Purge()
{
  char fileName[ FLD_FULLPATH ];

  for ( DWORD i = 0; i < hFLD->Count(); i ++ )
  {
    if ( !hFLD->GetFile( fileName, i ) )
      continue;

    DWORD attrs = GetFileAttributes( fileName );
    if ( attrs != (DWORD)-1 && attrs & FILE_ATTRIBUTE_HIDDEN )
    {
      DeleteFile( fileName );
    }
  }

  return MV_OK;
}
