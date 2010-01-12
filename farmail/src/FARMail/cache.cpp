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

MessageCache::MessageCache()
{
  _UseCache=0;
  pPanelItem=NULL;
  pItemsNumber=0;
}


MessageCache::~MessageCache()
{
  ClearCachedData();
}


int MessageCache::LoadCachedData( PluginPanelItem *_pPanelItem, int _pItemsNumber, const char *_MailboxPath )
{
 int i,j;

 InternalClearCachedData();
 _UseCache = 1;

 pPanelItem = (PluginPanelItem *)z_calloc( _pItemsNumber, sizeof(PluginPanelItem) );
 if ( pPanelItem ) {

    pItemsNumber = _pItemsNumber;
    memcpy( pPanelItem, _pPanelItem, pItemsNumber*sizeof(PluginPanelItem) );

    for ( i=0 ; i<pItemsNumber ; i++ ) {

       pPanelItem[i].CustomColumnData = (char**)z_calloc( NUM_OF_CUSTOM_COLS , sizeof(char*) );
       if ( pPanelItem[i].CustomColumnData ) {

          j = 0;
          while ( _pPanelItem[i].CustomColumnData[j] ) {

             pPanelItem[i].CustomColumnData[j] = z_strdup( _pPanelItem[i].CustomColumnData[j] );
             j++;

          }

       } else {
          _UseCache = 0;
          break;
       }
    }
 } else {
    _UseCache = 0;
 }
 if ( !_UseCache ) {
    InternalClearCachedData();
 }
 else if(_MailboxPath)
 {
   lstrcpy(MailboxPath,_MailboxPath);
 }

 return _UseCache;
}



int MessageCache::UseCachedData( PluginPanelItem **_pPanelItem, int *_pItemsNumber )
{
 int stat = 0;

 if ( _UseCache ) {

    int i,j;

    ShortMessage *sm = new ShortMessage( MesCache_Loading );

    *_pPanelItem = (PluginPanelItem*)z_realloc( *_pPanelItem, (pItemsNumber+1)*sizeof(PluginPanelItem) );
    if ( * _pPanelItem ) {

       *_pItemsNumber = pItemsNumber;

       for ( i=0 ; i<pItemsNumber ; i++ ) {

          (*_pPanelItem)[i] = pPanelItem[i];

          (*_pPanelItem)[i].CustomColumnData = (char**)z_calloc( NUM_OF_CUSTOM_COLS , sizeof(char*) );
          if ( (*_pPanelItem)[i].CustomColumnData ) {

             j = 0;
             while ( pPanelItem[i].CustomColumnData[j] ) {

                (*_pPanelItem)[i].CustomColumnData[j] = z_strdup( pPanelItem[i].CustomColumnData[j] );
                j++;

             }

          } else {
             stat = 1;
//             break;
          }
       }

    } else stat = 1;

    delete sm;

 }
 return stat;
}


int MessageCache::ClearCachedData(void)
{
  if(MailboxPath[0])
  {
    WaitForSingleObject(UidlMutex,INFINITE);
    HKEY key_root;
    if(RegOpenKeyEx(HKEY_CURRENT_USER,MailboxPath,0,KEY_ALL_ACCESS,&key_root)==ERROR_SUCCESS)
    {
      HKEY key_new; DWORD Disposition;
      if((RegCreateKeyEx(key_root,UidlKey,0,NULL,0,KEY_READ,NULL,&key_new,&Disposition))==ERROR_SUCCESS)
      {
        RegCloseKey(key_new);
      }
      if(RegDeleteKey(key_root,UidlKey)==ERROR_SUCCESS)
      {
        if((RegCreateKeyEx(key_root,UidlKey,0,NULL,0,KEY_WRITE,NULL,&key_new,&Disposition))==ERROR_SUCCESS)
        {
          for(int i=0;i<pItemsNumber;i++)
          {
            if(pPanelItem[i].CustomColumnNumber>4&&pPanelItem[i].CustomColumnData[4]&&lstrlen(pPanelItem[i].CustomColumnData[4])&&!(pPanelItem[i].UserData&MESSAGE_STATE_DELETED))
            {
              RegSetValueEx(key_new,pPanelItem[i].CustomColumnData[4],0,REG_DWORD,(LPBYTE)&pPanelItem[i].UserData,sizeof(pPanelItem[i].UserData));
            }
          }
          RegCloseKey(key_new);
        }
      }
      RegCloseKey(key_root);
    }
    ReleaseMutex(UidlMutex);
  }
  return InternalClearCachedData();
}

int MessageCache::InternalClearCachedData(void)
{
 int i,j;

 _UseCache = 0;

 if ( pPanelItem ) {

    for ( i=0 ; i<pItemsNumber ; i++ ) {

       if ( pPanelItem[i].CustomColumnData ) {

          j = 0;
          while ( pPanelItem[i].CustomColumnData[j] ) {

             z_free( pPanelItem[i].CustomColumnData[j] );
             j++;

          }
          z_free(pPanelItem[i].CustomColumnData);
       }
    }
    z_free( pPanelItem );
    pPanelItem = NULL;
 }
 pItemsNumber = 0;
 MailboxPath[0]=0;

 return 0;
}

bool MessageCache::MarkMessage(const char *uidl, DWORD state)
{
  bool Res=false;
  for(int i=0;i<pItemsNumber;i++)
  {
    if(pPanelItem[i].CustomColumnNumber>4&&pPanelItem[i].CustomColumnData[4]&&!lstrcmp(pPanelItem[i].CustomColumnData[4],uidl))
    {
      if (!(pPanelItem[i].UserData&MESSAGE_STATE_DELETED))
      {
        pPanelItem[i].UserData=state;
        pPanelItem[i].FindData.dwFileAttributes=MapStateToAttribute(state);
        Res=true;
      }
      else
        Res=false;
      break;
    }
  }
  return Res;
}

bool MessageCache::MarkMessage(int i, DWORD state)
{
  for(int j=0;j<pItemsNumber;j++)
  {
    if(FSF.atoi(pPanelItem[j].FindData.cFileName)==i)
    {
      pPanelItem[j].UserData|=state;
      pPanelItem[j].FindData.dwFileAttributes=MapStateToAttribute(state);
      return true;
    }
  }
  return false;
}

bool MessageCache::ClearState(DWORD state)
{
  for (int i=0; i<pItemsNumber; i++)
  {
    if (pPanelItem[i].UserData&state)
    {
      pPanelItem[i].UserData-=state;
      pPanelItem[i].FindData.dwFileAttributes=MapStateToAttribute(pPanelItem[i].UserData);
    }
  }
  return true;
}

DWORD MapStateToAttribute(DWORD state)
{
  switch(state)
  {
    case MESSAGE_STATE_NEW:
      return Opt.NewAttributes;
    case MESSAGE_STATE_READ:
      return Opt.ReadAttributes;
    case MESSAGE_STATE_MARKED:
      return Opt.MarkedAttributes;
    case MESSAGE_STATE_DELETED:
      return Opt.DeletedAttributes;
  }
  return FILE_ATTRIBUTE_NORMAL;
}
