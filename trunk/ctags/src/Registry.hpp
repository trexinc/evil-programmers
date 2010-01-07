/*
  Copyright (C) 2000 Konstantin Stupnik

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

  Wrapper around registry manipulation functions.
*/

#ifndef __REGISTRY_HPP__
#define __REGISTRY_HPP__

#ifndef __cplusplus
#error This header file is for C++ only!
#endif

#include <windows.h>

#pragma comment(lib,"advapi32")
#pragma comment(lib,"user32")

/*! Wrapper around registry WINAPI functions.
*/
class Registry{
private:
  HKEY _key;
  HKEY _basekey;
  DWORD _index;
  LPCTSTR _base;
public:
  /*! Used by GetInfo function
     \sa GetInfo
  */
  typedef struct tag_KeyInfo{
    //! length of class string
    DWORD classsize;
    //! number of subkeys
    DWORD subkeys;
    //! length of longest subkey name
    DWORD maxsubkey;
    //! length of longest class string
    DWORD maxclass;
    //! number of values accociated with a key
    DWORD values;
    //! length of longest value name
    DWORD maxvaluename;
    //! length in bytes of maximum data size of value
    DWORD maxvalue;
    //! length in bytes of security descriptor
    DWORD maxsec;
    //! time of last modification of the key
    FILETIME lastwrite;
    //! key class string (if length is less than 512 :) )
    char keyclass[512];
  }KeyInfo;
  /*! Default constructor */
  Registry(){_key=NULL;_index=-2;_base=NULL;}
  /*! Automatically try to open subkey of specified key
      \sa Open
  */
  Registry(HKEY basekey,LPCTSTR subkey="",REGSAM access=KEY_ALL_ACCESS)
  {
    _index=-2;_key=NULL;_base=NULL;
    if(!Open(basekey,subkey,access))_key=NULL;
  }
  /*! Open subkey of opened key */
  Registry(const Registry& basekey,LPCTSTR subkey="",REGSAM access=KEY_ALL_ACCESS)
  {
    _index=-2;_key=NULL;_base=NULL;
    if(!Open(basekey,subkey,access))_key=NULL;
  }
  /*! Destructor.
      Will automatically close key if opened.
  */
  ~Registry(){Close();}

  /*! Open subkey of standard key
      \param basekey - one of predefined keys or handle of opened key
      \param subkey - subkey to open, or open the same key if default value used
      \param access - by default full access, or specify KEY_READ for read-only access
      \return 1 on success, 0 otherwise
  */
  int Open(HKEY basekey=HKEY_CURRENT_USER,LPCTSTR subkey="",REGSAM access=KEY_ALL_ACCESS)
  {
    int res;
    char fullname[512];
    if(_key)Close();
    _basekey=basekey;
    wsprintf(fullname,"%s%s%s",_base?_base:"",_base && *subkey?"\\":"",subkey);
    if(access&KEY_WRITE)
    {
      DWORD disposition;
      res=RegCreateKeyEx(basekey,fullname,0,NULL,0,access,NULL,&_key,&disposition);
    }else
    {
      res=RegOpenKeyEx(basekey,fullname,0,access,&_key);
    }
    return res==ERROR_SUCCESS;
  }
  /*! open subkey of opened key
      \return 1 on success, 0 otherwise.
  */
  int Open(const Registry& basekey,LPCTSTR subkey="",REGSAM access=KEY_ALL_ACCESS)
  {
    if(_key)Close();
    return Open(basekey._key,subkey,access);
  }
  /*! open subkey of opened key */
  int Open(LPCTSTR subkey,REGSAM access=KEY_ALL_ACCESS)
  {
    if(_key)Close();
    return Open(_basekey,subkey,access);
  }

  /*! Get DWORD key value
      If failed, return defval
      \param name - name of value
      \param defval - value to return if value doesn't exists or error occured.
      \return DWORD value
  */
  DWORD Get(LPCTSTR name,DWORD defval=-1)
  {
    DWORD value,type,size;
    size=sizeof(value);
    LONG res=RegQueryValueEx(_key,name,NULL,&type,(LPBYTE)&value,&size);
    if(res!=ERROR_SUCCESS || type!=REG_DWORD)return defval;
    return value;
  }

  /* Get string key value
     If failed, return defval
     \param name - name of value
     \param buf - receiving buffer
     \param bufsize - size of receiving buffer
     \param defval - value to return if key doesn't exists or error occured.
  */
  int Get(LPCTSTR name,LPTSTR buf,DWORD bufsize,LPTSTR defval="")
  {
    DWORD type;
    LONG res=RegQueryValueEx(_key,name,NULL,&type,(LPBYTE)buf,&bufsize);
    if(res!=ERROR_SUCCESS || type!=REG_SZ)
    {
      MoveMemory(buf,defval,lstrlen(defval)+1);
      return 0;
    }
    buf[bufsize]=0;
    return 1;
  }
  /*! Get binary key value
      \param name - name of value
      \param buf - receiving buffer
      \para bufsize - in parameter, size of receiving buffer, out - size of data
      \return 1 on success, or 0 otherwise.
  */
  int GetBin(LPCTSTR name,LPBYTE buf,DWORD& bufsize)
  {
    DWORD type;
    LONG res=RegQueryValueEx(_key,name,NULL,&type,(LPBYTE)buf,&bufsize);
    return res==ERROR_SUCCESS && type==REG_BINARY;
  }

  /*! Set DWORD value
      \param name - name of value
      \param value - value to set :)
      \return 1 on success, 0 otherwise.
  */
  int Set(LPCTSTR name,DWORD value)
  {
    return RegSetValueEx(_key,name,0,REG_DWORD,(LPBYTE)&value,sizeof(value))==ERROR_SUCCESS;
  }
  /*! Set string value
      \param name - name of value
      \param value - value to set :)
      \return 1 on success, 0 otherwise.
  */
  int Set(LPCTSTR  name,LPCTSTR  value)
  {
    return RegSetValueEx(_key,name,0,REG_SZ,(LPBYTE)value,lstrlen(value)+1)==ERROR_SUCCESS;
  }

  /*! Set binary value
      \param name - name of value
      \param data - value to set
      \param datasize - size of bnary data in bytes
      \return 1 on success, 0 otherwise.
  */
  int SetBin(LPCTSTR  name,LPBYTE data,DWORD datasize)
  {
    return RegSetValueEx(_key,name,0,REG_BINARY,(LPBYTE)data,datasize);
  }

  /*! Get key info
    \param keyinfo - structure that receive information
    \return 1 on success, 0 otherwise.
    \sa KeyInfo
  */
  int GetInfo(KeyInfo& keyinfo)
  {
    keyinfo.classsize=sizeof(keyinfo.keyclass);
    return RegQueryInfoKey(_key,keyinfo.keyclass,&keyinfo.classsize,NULL,&keyinfo.subkeys,
                     &keyinfo.maxsubkey,&keyinfo.maxclass,&keyinfo.values,
                     &keyinfo.maxvaluename,&keyinfo.maxvalue,&keyinfo.maxsec,
                     &keyinfo.lastwrite)==ERROR_SUCCESS;
  }
  /*! Enumerate key subkeys
      <pre>Used this way:
      while(reg.EnumSubKeys(name,sizeof(name))
      {
        //do something
      }
      If enumeration was aborted, call ResetEnum</pre>
      \param namebuf - buffer that receive name length
      \param bufsize - size of buffer for name
      \return 1 if one more subkey found, 0 if no more subkeys avialable.
      \sa ResetEnum
  */
  int EnumSubKeys(LPTSTR namebuf,DWORD bufsize)
  {
    if(_index==-2)
    {
      KeyInfo ki;
      if(!GetInfo(ki))return 0;
      _index=ki.subkeys-1;
    }
    if(_index==-1)
    {
      _index--;
      return 0;
    }
    if(RegEnumKey(_key,_index,namebuf,bufsize)!=ERROR_SUCCESS)
    {
      _index=-2;
      return 0;
    }
    _index--;
    return 1;
  }
  /*! Enum values of key
      <pre>!!! IMPORTANT !!!
      valnamesize and datasize MUST be set to correct value before EVERY call.
      For example:
      char name[256];
      char value[512];
      DWORD nsize,vsize,type;
      for(nsize=sizeof(name),vsize=sizeof(value);
          reg.EnumValues(name,nsize,type,value,vsize);
          nsize=sizeof(name),vsize=sizeof(value))
      {
        // do something
      }
      If enumeration was aborted call ResetEnum.</pre>
      \param valname - buffer that receive valuename
      \param valnamesize - in/out parameter that receive/return length of name
      \param type - out parameter that receive type of value
      \param data - buffer that receive key data
      \param datasize - in/out parameter that receive/return size of data
      \return 1 if one more subkey found, 0 if no more subkeys avialable.
      \sa ResetEnum
  */
  int EnumValues(LPTSTR valname,DWORD& valnamesize,DWORD& type,
                 LPBYTE data,DWORD& datasize)
  {
    if(_index==-2)
    {
      KeyInfo ki;
      if(!GetInfo(ki))return 0;
      _index=ki.values-1;
    }
    if(_index==-1)
    {
      _index--;
      return 0;
    }
    if(RegEnumValue(_key,_index,valname,&valnamesize,NULL,&type,
                    data,&datasize)!=ERROR_SUCCESS)
    {
      _index=-2;
      return 0;
    }
    _index--;
    return 1;
  }
  /*! Reset enumeration counter.
      Used if enumeration was aborted before Enum* function indicated
      that no more keys or values are avialable.
  */
  void ResetEnum()
  {
    _index=-2;
  }
  /*! Delete value
      \param name - name of value to delete
      \return 1 on success, 0 otherwise.
  */
  int DeleteValue(LPCTSTR name)
  {
    return RegDeleteValue(_key,name)==ERROR_SUCCESS;
  }
  /*! Delete subkey
      \param name - name of subkey to delete, or key itself if name empty
      \return 1 on success or 0 if error occured
  */
  int DeleteKey(LPCTSTR name="")
  {
    if(!*name)
    {
      char buf[512];
      while(EnumSubKeys(buf,sizeof(buf)))
      {
        Registry r(*this,buf,KEY_ALL_ACCESS);
        if(!r.DeleteKey())return 0;
      }
      int res=RegDeleteKey(_key,"");
      if(res!=ERROR_SUCCESS)return 0;
    }else
    {
      char buf[512];
      Registry r(*this,name);
      while(r.EnumSubKeys(buf,sizeof(buf)))
      {
        Registry q(r,buf,KEY_ALL_ACCESS);
        if(!q.DeleteKey())return 0;
      }
      if(!r.DeleteKey())return 0;
    }
    return 1;
  }
  /*! Close key
  */
  void Close()
  {
    if(_key)RegCloseKey(_key);
    _key=NULL;
  }
  /*! Check if key is opened
  */
  int IsOpened(){return _key!=NULL;}
  /*! Set key base
    Used by int Open(LPCTSTR subkey,REGSAM access=KEY_ALL_ACCESS)
  */
  void SetBase(LPCTSTR newbase,HKEY newbasekey=HKEY_CURRENT_USER)
  {
    _basekey=newbasekey;
    _base=newbase;
  }
};

#endif
