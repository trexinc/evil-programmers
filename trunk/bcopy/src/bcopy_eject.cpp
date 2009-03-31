/*
    bcopy_eject.cpp
    Copyright (C) 2000-2009 zg

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

#include <winioctl.h>

static BOOL DiskEject(TCHAR *ADisk,BOOL type)
{
  HANDLE DiskHandle;
  BOOL Retry=TRUE,Ignore=FALSE,Result=FALSE;
  DWORD temp;
  DWORD AccessFlags=GENERIC_READ|GENERIC_WRITE;
  TCHAR DiskPath[MAX_PATH],DrivePath[MAX_PATH];
  _stprintf(DiskPath,_T("\\\\.\\%s"),ADisk);
  _stprintf(DrivePath,_T("\\\\.\\%s\\"),ADisk);
  if(GetDriveType(DrivePath)==DRIVE_CDROM) AccessFlags=GENERIC_READ;
  DiskHandle=CreateFile(DiskPath,AccessFlags,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,0);
  if(DiskHandle!=INVALID_HANDLE_VALUE)
  {
    while(Retry)
      if(PlgOpt.ForceEject||Ignore||(DeviceIoControl(DiskHandle,FSCTL_LOCK_VOLUME,NULL,0,NULL,0,&temp,NULL)&&DeviceIoControl(DiskHandle,FSCTL_DISMOUNT_VOLUME,NULL,0,NULL,0,&temp,NULL)))
      {
        PREVENT_MEDIA_REMOVAL PreventMediaRemoval;
        PreventMediaRemoval.PreventMediaRemoval=FALSE;
        if(DeviceIoControl(DiskHandle,IOCTL_STORAGE_MEDIA_REMOVAL,&PreventMediaRemoval,sizeof(PreventMediaRemoval),NULL,0,&temp,NULL))
        {
          if(!type)
            Result=DeviceIoControl(DiskHandle,IOCTL_STORAGE_EJECT_MEDIA,NULL,0,NULL,0,&temp,NULL);
          else
            Result=DeviceIoControl(DiskHandle,IOCTL_STORAGE_LOAD_MEDIA,NULL,0,NULL,0,&temp,NULL);
        }
        Retry=FALSE;
      }
      else
      {
        Ignore=FALSE;
        const TCHAR *MsgItems[]={GetMsg(mError),GetMsg(mEjectRetry),GetMsg(mEjectIgnore),GetMsg(mEjectCancel)};
        int MsgCode=Info.Message(Info.ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,NULL,MsgItems,sizeofa(MsgItems),3);
        switch(MsgCode)
        {
          case 1:
            Ignore=TRUE;
            break;
          case -1:
          case 2:
            Retry=FALSE;
            break;
        }
      }
    DeviceIoControl(DiskHandle,FSCTL_UNLOCK_VOLUME,NULL,0,NULL,0,&temp,NULL);
    CloseHandle(DiskHandle);
  }
  return Result;
}

static void ShowDiskMenu(BOOL type)
{
  DWORD Disks;
  int DiskCount=2;
  struct FarMenuItem *DiskMenuItems=NULL;
  Disks=GetLogicalDrives();
  {
    TCHAR DrivePath[4]=_T(" :\\");
    for(int i=0;i<27;i++)
      if(Disks&(1<<i))
      {
        DrivePath[0]=i+'A';
        unsigned int disk_type=GetDriveType(DrivePath);
        if((disk_type==DRIVE_REMOVABLE)||(disk_type==DRIVE_CDROM))
          DiskCount++;
        else
          Disks&=~(1<<i);
      }
  }
  if(DiskCount)
  {
    TCHAR DiskName[MAX_PATH];
    size_t size=sizeof(FarMenuItem)*DiskCount
#ifdef UNICODE
    +DiskCount*128*sizeof(TCHAR)
#endif
    ;
    DiskMenuItems=(struct FarMenuItem *)malloc(size);
    if(DiskMenuItems)
    {
#ifdef UNICODE
      TCHAR* data=(TCHAR*)(((char*)DiskMenuItems)+sizeof(FarMenuItem)*DiskCount);
#endif
      TCHAR* text;
      for(int i=0,j=2;i<27;i++)
      {
        text=
#ifdef UNICODE
        data+j*128
#else
        DiskMenuItems[j].Text
#endif
        ;
#ifdef UNICODE
        DiskMenuItems[j].Text=text;
#endif
        if(Disks&(1<<i))
        {
          text[0]='&';
          text[1]='A'+i;
          text[2]=':';
          j++;
        }
      }
      DiskMenuItems[0].Selected=TRUE;
      text=
#ifdef UNICODE
      data
#else
      DiskMenuItems[0].Text
#endif
      ;
#ifdef UNICODE
      DiskMenuItems[0].Text=text;
#endif
      _tcscpy(text,GetMsg(mEjectCurrent));
      DiskMenuItems[1].Separator=TRUE;
    }
    int i=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_WRAPMODE,type?GetMsg(mEjectLoad):GetMsg(mEjectEject),NULL,_T("Contents"),NULL,NULL,DiskMenuItems,DiskCount);
    if(i>1)
    {
      DiskName[0]=DiskMenuItems[i].Text[1];
      DiskName[1]=':';
      DiskName[2]=0;
      DiskEject(DiskName,type);
    }
    else if(i==0)
    {
      const TCHAR VolumeID[]=_T("\\\\?\\Volume{");
      CFarPanel pInfo(INVALID_HANDLE_VALUE,FCTL_GETPANELSHORTINFO);
      if(pInfo.IsOk())
      {
        if(!pInfo.Plugin())
        {
          bool restore=false;
          { //set directory to far folder
            TCHAR buffer1[MAX_PATH],buffer2[MAX_PATH],*filename;
            if(GetModuleFileName(NULL,buffer1,sizeof(buffer1)))
            {
              DWORD res=GetFullPathName(buffer1,sizeof(buffer2),buffer2,&filename);
              if(res&&(res<sizeof(buffer2))&&filename)
              {
                if(filename>buffer2) *(filename-1)=0;
                if(Info.ControlShort(INVALID_HANDLE_VALUE,FCTL_SETPANELDIR,(SECOND_PARAM)buffer2)) restore=true;
              }
            }
          }
          TCHAR root_buffer[MAX_PATH];
          FSF.GetPathRoot(pInfo.CurDir(),root_buffer
#ifdef UNICODE
          ,MAX_PATH
#endif
          );
          if(!_tcsncmp(root_buffer,VolumeID,_tcslen(VolumeID)))
          {
            _tcscpy(DiskName,root_buffer+4);
            if(DiskName[_tcslen(DiskName)-1]=='\\') DiskName[_tcslen(DiskName)-1]=0;
            restore=restore&&!DiskEject(DiskName,type);
          }
          else if(FSF.LUpper(root_buffer[0])>='A'&&FSF.LUpper(root_buffer[0])<='Z')
          {
            DiskName[0]=root_buffer[0];
            DiskName[1]=':';
            DiskName[2]=0;
            restore=restore&&!DiskEject(DiskName,type);
          }
          if(restore) Info.ControlShort(INVALID_HANDLE_VALUE,FCTL_SETPANELDIR,(SECOND_PARAM)pInfo.CurDir());
        }
      }
    }
    free(DiskMenuItems);
  }
}

static void RefreshSCSI(void)
{
  TCHAR buff[20],Msg[256]; HANDLE hSCSI; DWORD ret; BOOL WaitMessage=FALSE;
  HANDLE hSScr=Info.SaveScreen(0,0,-1,-1);
  for(int i=0;i<10;i++)
  {
    _stprintf(buff,_T("\\\\.\\Scsi%d:"),i);
    _stprintf(Msg,GetMsg(mSCSIRefresh),buff);
    const TCHAR *MsgItems[]={_T(""),Msg};
    Info.Message(Info.ModuleNumber,WaitMessage?FMSG_KEEPBACKGROUND:0,NULL,MsgItems,sizeofa(MsgItems),0);
    WaitMessage=TRUE;
    hSCSI=CreateFile(buff,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    if(hSCSI!=INVALID_HANDLE_VALUE)
    {
      DeviceIoControl(hSCSI,0x4101c,NULL,0,NULL,0,&ret,NULL);
      CloseHandle(hSCSI);
    }
    else
      break;
  }
  Info.RestoreScreen(hSScr);
}
