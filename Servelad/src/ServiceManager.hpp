/*
    Servelad plugin for FAR Manager
    Copyright (C) 2009 Alex Yaroslavsky

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

#ifndef __SERVICEMANAGER_HPP__
#define __SERVICEMANAGER_HPP__

#include <vector>
#include <string>

struct SServiceInfo
{
  const wchar_t* iDescription;
  const wchar_t* iPath;
  const wchar_t* iLoadOrderGroup;
  const wchar_t* iDependencies;
  DWORD iStartType;
  DWORD iErrorControl;
  DWORD iTagId;
  const ENUM_SERVICE_STATUS_PROCESS* iStatus;
};

class CServiceManager
{
  private:
    class CServiceInfo
    {
      private:
        std::wstring iDescription;
        std::wstring iPath;
        std::wstring iLoadOrderGroup;
        std::wstring iDependencies;
        DWORD iStartType;
        DWORD iErrorControl;
        DWORD iTagId;
        ENUM_SERVICE_STATUS_PROCESS* iStatus;
      private:
        CServiceInfo();
      public:
        CServiceInfo(const std::wstring& aDescription,const std::wstring& aPath,const std::wstring& aLoadOrderGroup,const std::wstring& aDependencies,DWORD aStartType,DWORD aErrorControl,DWORD aTagId,ENUM_SERVICE_STATUS_PROCESS* aStatus);
        SServiceInfo Data(void)const;
    };
    class CServiceList
    {
      private:
        SC_HANDLE iSCManager;
        ENUM_SERVICE_STATUS_PROCESS* iServices;
        DWORD iServiceCount;
        std::vector<CServiceInfo> iServicesInfo;
      private:
        CServiceList(const CServiceList& copy);
      public:
        CServiceList(const std::wstring& aRemoteMachine=L"");
        ~CServiceList();
      public:
        bool ManagerStatus(void)const {return iSCManager!=NULL;};
        bool Fill(DWORD aServiceType);
        void Clear(void);
        DWORD ServiceCount(void)const {return iServiceCount;};
        SServiceInfo operator[](size_t anIndex)const {return iServicesInfo[anIndex].Data();};
    };
  private:
    CServiceList iServiceList;
    DWORD iServiceType;
  public:
    CServiceManager(const std::wstring& aRemoteMachine=L""): iServiceList(aRemoteMachine),iServiceType(0) {};
    bool ManagerStatus() const {return iServiceList.ManagerStatus();}
    void Reset(DWORD aServiceType=0) {iServiceType=aServiceType;Clear();}

    DWORD GetType()const {return iServiceType;}
    DWORD GetCount()const {return iServiceList.ServiceCount();}
    SServiceInfo operator[](size_t anIndex)const {return iServiceList[anIndex];};

    bool RefreshList(void) {return iServiceList.Fill(iServiceType);}
  private:
    void Clear(void) {iServiceList.Clear();}
  private:
    CServiceManager(const CServiceManager &copy);
};

#define ArraySize(a) (sizeof(a)/sizeof(a[0]))

#endif
