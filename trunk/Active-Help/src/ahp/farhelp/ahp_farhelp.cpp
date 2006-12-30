/*
    FarHelp sub-plugin for Active-Help plugin for FAR Manager
    Copyright (C) 2003-2004 Alex Yaroslavsky

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
#include "../../ahp.hpp"
#include "crt.hpp"

#if defined(__GNUC__)
extern "C"
{
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
};

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}
#endif

static struct PluginStartupInfo Info;

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct AHPluginStartupInfo *AHInfo)
{
  (void)AHInfo;
  Info=*FarInfo;
  return 0;
}

void WINAPI _export Exit(void)
{
}

int WINAPI _export Message(unsigned long Msg,void *InData,void *OutData)
{
  switch(Msg)
  {
    case AHMSG_GETINFO:
      {
        GetInfoOutData *data=(GetInfoOutData *)OutData;
        data->TypesNumber=1;
        static struct TypeInfo TypesInfo[1];
        memset(TypesInfo,0,sizeof(TypesInfo));
        TypesInfo[0].StructSize=sizeof(TypesInfo[0]);
        lstrcpy(TypesInfo[0].TypeString,"FarHelp");
        lstrcpy(TypesInfo[0].Mask,"*.hlf");
        TypesInfo[0].HFlags=F_NOKEYWORDSEARCH;
        data->TypesInfo=TypesInfo;
      }
      return TRUE;

    case AHMSG_SHOWHELP:
      {
        HelpInData *data = (HelpInData *)InData;
        Info.ShowHelp(data->FileName,data->Keyword,FHELP_CUSTOMFILE|FHELP_USECONTENTS);
        return TRUE; //FAR shows the error by itself.
      }
  }
  return FALSE;
}
