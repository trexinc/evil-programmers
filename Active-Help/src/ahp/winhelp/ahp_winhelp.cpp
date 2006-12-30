/*
    WinHelp sub-plugin for Active-Help plugin for FAR Manager
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

static struct AHPluginStartupInfo AHInfo;

int WINAPI _export Start(const struct PluginStartupInfo *FarInfo,const struct AHPluginStartupInfo *AHInfo)
{
  (void)FarInfo;
  ::AHInfo=*AHInfo;
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
        lstrcpy(TypesInfo[0].TypeString,"WinHelp");
        lstrcpy(TypesInfo[0].Mask,"*.hlp");
        lstrcpy(TypesInfo[0].Encoding,"WIN");
        data->TypesInfo=TypesInfo;
      }
      return TRUE;

    case AHMSG_SHOWHELP:
      {
        HelpInData *data = (HelpInData *)InData;
        UINT uiCommand;
        DWORD dwData;
        if (*data->Keyword)
        {
          uiCommand = HELP_KEY;
          dwData = (DWORD)data->Keyword;
        }
        else
        {
          uiCommand = HELP_FINDER;
          dwData = 0;
        }
        char FileName[512];
        lstrcpy(FileName,data->FileName);
        AHInfo.Encode(FileName,lstrlen(FileName),"WIN");
        int ret = WinHelp(GetDesktopWindow(),FileName,uiCommand,dwData);
        if (!ret)
        {
          HelpOutData *odata = (HelpOutData *)OutData;
          LPSTR MessageBuffer;
          DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_SYSTEM;
          DWORD dwBufferLength = FormatMessageA(dwFormatFlags,NULL,GetLastError(),MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPSTR)&MessageBuffer,0,NULL);
          lstrcpyn(odata->Error,MessageBuffer,dwBufferLength>128?128:dwBufferLength);
          CharToOem(odata->Error,odata->Error);
        }
        return ret;
      }
  }
  return FALSE;
}
