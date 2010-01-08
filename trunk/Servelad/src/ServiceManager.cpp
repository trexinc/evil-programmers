/*
    ServiceManager.cpp
    Copyright (C) 2010 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "plugin.hpp"
#include "ServiceManager.hpp"
#include <exception>

CServiceManager::CServiceInfo::CServiceInfo(const std::wstring& aDescription,const std::wstring& aPath,const std::wstring& aLoadOrderGroup,const std::wstring& aDependencies,DWORD aStartType,DWORD aErrorControl,DWORD aTagId,ENUM_SERVICE_STATUS_PROCESS* aStatus):
iDescription(aDescription),iPath(aPath),iLoadOrderGroup(aLoadOrderGroup),iDependencies(aDependencies),iStartType(aStartType),iErrorControl(aErrorControl),iTagId(aTagId),iStatus(aStatus)
{
}

SServiceInfo CServiceManager::CServiceInfo::Data(void)const
{
  SServiceInfo info={iDescription.c_str(),iPath.c_str(),iLoadOrderGroup.c_str(),iDependencies.c_str(),iStartType,iErrorControl,iTagId,iStatus};
  return info;
}

bool CServiceManager::CServiceList::Fill(DWORD aServiceType)
{
  bool result=false;
  Clear();
  if(aServiceType==0)
  {
    return true;
  }

  DWORD cbBytesNeeded=0;
  DWORD dwServicesReturned=0;
  DWORD dwResumeHandle=0;
  SC_HANDLE handle=NULL;
  char* serviceConfigData=NULL;
  BOOL ret=EnumServicesStatusEx(iSCManager,SC_ENUM_PROCESS_INFO,aServiceType,SERVICE_STATE_ALL,NULL,0,&cbBytesNeeded,&dwServicesReturned,&dwResumeHandle,NULL);
  if(!ret&&GetLastError()==ERROR_MORE_DATA)
  {
    try
    {
      iServices=(ENUM_SERVICE_STATUS_PROCESS*)new char[cbBytesNeeded];
      ret=EnumServicesStatusEx(iSCManager,SC_ENUM_PROCESS_INFO,aServiceType,SERVICE_STATE_ALL,(LPBYTE)iServices,cbBytesNeeded,&cbBytesNeeded,&dwServicesReturned,&dwResumeHandle,NULL);
      if(ret)
      {
        for(size_t ii=0;ii<dwServicesReturned;++ii)
        {
          handle=OpenService(iSCManager,iServices[ii].lpServiceName,SERVICE_QUERY_CONFIG);
          if(!handle) throw std::exception();
          DWORD needed;
          ret=QueryServiceConfig(handle,NULL,0,&needed);
          if(ret||GetLastError()!=ERROR_INSUFFICIENT_BUFFER) throw std::exception();
          serviceConfigData=new char[needed];
          QUERY_SERVICE_CONFIG* serviceConfig=(QUERY_SERVICE_CONFIG*)serviceConfigData;
          if(!QueryServiceConfig(handle,serviceConfig,needed,&needed)) throw std::exception();
          CloseServiceHandle(handle);
          handle=NULL;

          CServiceInfo si(L"",serviceConfig->lpBinaryPathName,serviceConfig->lpLoadOrderGroup,serviceConfig->lpDependencies,serviceConfig->dwStartType,serviceConfig->dwErrorControl,serviceConfig->dwTagId,iServices+ii);
          iServicesInfo.push_back(si);
          delete[]serviceConfigData;
        }
        iServiceCount=dwServicesReturned;
        result=true;
      }
    }
    catch(...)
    {
      Clear();
    }
  }
  if(handle) CloseServiceHandle(handle);
  if(serviceConfigData) delete[]serviceConfigData;
  return result;
}

void CServiceManager::CServiceList::Clear(void)
{
  iServicesInfo.clear();
  delete[]iServices;
  iServices=NULL;
  iServiceCount=0;
}

CServiceManager::CServiceList::CServiceList(const std::wstring& aRemoteMachine): iSCManager(NULL),iServices(NULL),iServiceCount(0)
{
  iSCManager=OpenSCManager(aRemoteMachine.c_str(),SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);
}

CServiceManager::CServiceList::~CServiceList()
{
  Clear();
  if(iSCManager) CloseServiceHandle(iSCManager);
}
