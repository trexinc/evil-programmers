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

#include "plugin.hpp"
#include "ServiceManager.hpp"

struct PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
const wchar_t PluginName[] = L"Servelad";
const wchar_t DriversDirName[] = L"Drivers";
const wchar_t ServicesDirName[] = L"Services";

const wchar_t *GetMsg(int MsgId)
{
  return Info.GetMsg(Info.ModuleNumber,MsgId);
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *psi)
{
  Info=*psi;
  FSF=*psi->FSF;
  Info.FSF=&FSF;
}

HANDLE WINAPI OpenPluginW(int OpenFrom,INT_PTR Item)
{
  CServiceManager* sm=new CServiceManager();
  return (HANDLE)sm;
}

void WINAPI ClosePluginW(HANDLE hPlugin)
{
  CServiceManager* sm=(CServiceManager*)hPlugin;
  delete sm;
}

void WINAPI GetPluginInfoW(PluginInfo* pi)
{
  static const wchar_t* MenuStrings[1];

  pi->StructSize=sizeof(PluginInfo);
  pi->Flags=0;
  MenuStrings[0]=PluginName;
  pi->PluginMenuStrings=MenuStrings;
  pi->PluginMenuStringsNumber=ArraySize(MenuStrings);
}

void WINAPI GetOpenPluginInfoW(HANDLE hPlugin, struct OpenPluginInfo *Info)
{
  static wchar_t PanelTitle[50];
  CServiceManager* sm=(CServiceManager*)hPlugin;
  Info->StructSize=sizeof(*Info);
  Info->Flags=OPIF_ADDDOTS|OPIF_SHOWPRESERVECASE|OPIF_USEHIGHLIGHTING|OPIF_USEFILTER;
  Info->CurDir=NULL;
  if(sm->GetType()==SERVICE_WIN32)
    Info->CurDir=ServicesDirName;
  else if(sm->GetType()==SERVICE_DRIVER)
    Info->CurDir=DriversDirName;

  FSF.snprintf(PanelTitle,ArraySize(PanelTitle)-1,L"%s%s%s",PluginName,Info->CurDir?L": ":L"",Info->CurDir?Info->CurDir:L"");
  Info->PanelTitle=PanelTitle;

  static PanelMode PanelModesArray[10];
  memset(&PanelModesArray, 0, sizeof (PanelModesArray));
  Info->PanelModesArray=PanelModesArray;
  Info->PanelModesNumber=ArraySize(PanelModesArray);
  if(sm->GetType()==0)
  {
    PanelModesArray[0].ColumnTypes=L"N";
    PanelModesArray[0].ColumnWidths=L"0";
    PanelModesArray[0].StatusColumnTypes=L"N";
    PanelModesArray[0].StatusColumnWidths=L"0";
  }
  else
  {
    static const wchar_t* ColumnTitles[]={NULL,L"Status",L"Startup"};
    PanelModesArray[0].ColumnTypes=L"N,C0,C1";
    PanelModesArray[0].ColumnWidths=L"0,8,8";
    PanelModesArray[0].StatusColumnTypes=L"N";
    PanelModesArray[0].StatusColumnWidths=L"0";
    PanelModesArray[0].ColumnTitles=ColumnTitles;
  }
  Info->StartPanelMode=L'0';
}

int WINAPI GetFindDataW(HANDLE hPlugin, struct PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode)
{
  CServiceManager* sm=(CServiceManager*)hPlugin;

  *pPanelItem=NULL;
  *pItemsNumber=0;

  if (sm->GetType()==0)
  {
    *pPanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*2);
    memset(*pPanelItem, 0, sizeof(PluginPanelItem)*2);
    *pItemsNumber=2;

    (*pPanelItem)[0].FindData.lpwszFileName=DriversDirName;
    (*pPanelItem)[0].FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
    (*pPanelItem)[1].FindData.lpwszFileName=ServicesDirName;
    (*pPanelItem)[1].FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
  }
  else
  {
    sm->RefreshList();

    *pPanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*sm->GetCount());
    memset(*pPanelItem, 0, sizeof(PluginPanelItem)*sm->GetCount());
    *pItemsNumber=sm->GetCount();

    for (int i=0; i < *pItemsNumber; i++)
    {
      SServiceInfo info=(*sm)[i];
      (*pPanelItem)[i].FindData.lpwszFileName=info.iStatus->lpDisplayName;
      (*pPanelItem)[i].FindData.lpwszAlternateFileName=info.iStatus->lpServiceName;
      if (info.iStartType == SERVICE_DISABLED)
        (*pPanelItem)[i].FindData.dwFileAttributes=FILE_ATTRIBUTE_HIDDEN;
      const wchar_t** CustomColumnData=(const wchar_t**)calloc(2,sizeof(const wchar_t*));
      if(CustomColumnData)
      {
        switch(info.iStatus->ServiceStatusProcess.dwCurrentState)
        {
          case SERVICE_CONTINUE_PENDING:
            CustomColumnData[0]=L"Continue";
            break;
          case SERVICE_PAUSE_PENDING:
            CustomColumnData[0]=L"Pausing";
            break;
          case SERVICE_PAUSED:
            CustomColumnData[0]=L"Paused";
            break;
          case SERVICE_RUNNING:
            CustomColumnData[0]=L"Running";
            break;
          case SERVICE_START_PENDING:
            CustomColumnData[0]=L"Starting";
            break;
          case SERVICE_STOP_PENDING:
            CustomColumnData[0]=L"Stopping";
            break;
          case SERVICE_STOPPED:
            CustomColumnData[0]=L"";
            break;
        }
        switch(info.iStartType)
        {
          case SERVICE_AUTO_START:
            CustomColumnData[1]=L"Auto";
            break;
          case SERVICE_BOOT_START:
            CustomColumnData[1]=L"Boot";
            break;
          case SERVICE_DEMAND_START:
            CustomColumnData[1]=L"Manual";
            break;
          case SERVICE_DISABLED:
            CustomColumnData[1]=L"Disabled";
            break;
          case SERVICE_SYSTEM_START:
            CustomColumnData[1]=L"System";
            break;
        }
        (*pPanelItem)[i].CustomColumnData=CustomColumnData;
        (*pPanelItem)[i].CustomColumnNumber=2;
      }
      (*pPanelItem)[i].UserData=i;
    }
  }

  return TRUE;
}

void WINAPI FreeFindDataW(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber)
{
  for(int i=0;i<ItemsNumber;i++)
  {
    free((void*)PanelItem[i].CustomColumnData);
  }
  free(PanelItem);
}

int WINAPI SetDirectoryW(HANDLE hPlugin, const wchar_t *Dir, int OpMode)
{
  if (OpMode&OPM_FIND)
    return FALSE;

  CServiceManager* sm=(CServiceManager*)hPlugin;

  if (!wcscmp(Dir,L"\\") || !wcscmp(Dir,L".."))
  {
    sm->Reset();
  }
  else if (!wcscmp(Dir,ServicesDirName))
  {
    sm->Reset(SERVICE_WIN32);
  }
  else if (!wcscmp(Dir,DriversDirName))
  {
    sm->Reset(SERVICE_DRIVER);
  }
  else
  {
    return FALSE;
  }

  return TRUE;
}
