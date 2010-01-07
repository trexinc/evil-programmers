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

class ServiceManager
{
  private:
    SC_HANDLE hSCManager;
    DWORD ServiceType;
    ENUM_SERVICE_STATUS_PROCESS *Services;
    DWORD ServiceCount;

  public:
    ServiceManager(const wchar_t *RemoteMachine=NULL) { Services=NULL; ServiceCount=0; ServiceType=0; hSCManager=OpenSCManager(RemoteMachine, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS); }
    ~ServiceManager() { Clear(); if (hSCManager) CloseServiceHandle(hSCManager); }

    bool ManagerStatus() const { return hSCManager!=NULL; }

    void Reset(DWORD NewServiceType=0) { ServiceType=NewServiceType; Clear(); }

    bool ListServices() { ServiceType=SERVICE_WIN32; return RefreshList(); }
    bool ListDrivers()  { ServiceType=SERVICE_DRIVER; return RefreshList(); }

    DWORD GetType() const { return ServiceType; }
    DWORD GetCount() const { return ServiceCount; }
    const ENUM_SERVICE_STATUS_PROCESS &operator [](size_t i) const { return Services[i]; }

    bool RefreshList()
    {
      if (ServiceType == 0)
      {
        Clear();
        return true;
      }

      DWORD cbBytesNeeded=0;
      DWORD dwServicesReturned=0;
      DWORD dwResumeHandle=0;

      BOOL ret = EnumServicesStatusEx(hSCManager,
                                      SC_ENUM_PROCESS_INFO,
                                      ServiceType,
                                      SERVICE_STATE_ALL,
                                      NULL,
                                      0,
                                      &cbBytesNeeded,
                                      &dwServicesReturned,
                                      &dwResumeHandle,
                                      NULL);

      if (!ret && GetLastError() == ERROR_MORE_DATA)
      {
        if (Services) free(Services);

        Services = (ENUM_SERVICE_STATUS_PROCESS *) malloc(cbBytesNeeded);

        if (Services)
        {
          ret = EnumServicesStatusEx(hSCManager,
                                    SC_ENUM_PROCESS_INFO,
                                    ServiceType,
                                    SERVICE_STATE_ALL,
                                    (LPBYTE)Services,
                                    cbBytesNeeded,
                                    &cbBytesNeeded,
                                    &dwServicesReturned,
                                    &dwResumeHandle,
                                    NULL);

          if (ret)
          {
            ServiceCount = dwServicesReturned;
            return true;
          }
        }
      }

      Clear();
      return false;
    }

  private:
    void Clear() { if (Services) { free(Services); Services=NULL; } ServiceCount=0; }

  private:
    ServiceManager(const ServiceManager &copy) { }
};

#define ArraySize(a) (sizeof(a)/sizeof(a[0]))

#endif
