#include <winioctl.h>

static BOOL DiskEject(char *ADisk,BOOL type)
{
  HANDLE DiskHandle;
  BOOL Retry=TRUE,Ignore=FALSE,Result=FALSE;
  DWORD temp;
  DWORD AccessFlags=GENERIC_READ|GENERIC_WRITE;
  char DiskPath[MAX_PATH],DrivePath[MAX_PATH];
  sprintf(DiskPath,"\\\\.\\%s",ADisk);
  sprintf(DrivePath,"\\\\.\\%s\\",ADisk);
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
        const char *MsgItems[]={GetMsg(mError),GetMsg(mEjectRetry),GetMsg(mEjectIgnore),GetMsg(mEjectCancel)};
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
    char DrivePath[4]=" :\\";
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
    char DiskName[MAX_PATH];
    DiskMenuItems=(struct FarMenuItem *)malloc(DiskCount*sizeof(FarMenuItem));
    if(DiskMenuItems)
    {
      for(int i=0,j=2;i<27;i++)
        if(Disks&(1<<i))
        {
          DiskMenuItems[j].Text[0]='&';
          DiskMenuItems[j].Text[1]='A'+i;
          DiskMenuItems[j].Text[2]=':';
          j++;
        }
      DiskMenuItems[0].Selected=TRUE;
      strcpy(DiskMenuItems[0].Text,GetMsg(mEjectCurrent));
      DiskMenuItems[1].Separator=TRUE;
    }
    int i=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_WRAPMODE,type?GetMsg(mEjectLoad):GetMsg(mEjectEject),NULL,"Contents",NULL,NULL,DiskMenuItems,DiskCount);
    if(i>1)
    {
      DiskName[0]=DiskMenuItems[i].Text[1];
      DiskName[1]=':';
      DiskName[2]=0;
      DiskEject(DiskName,type);
    }
    else if(i==0)
    {
      const char VolumeID[]="\\\\?\\Volume{";
      PanelInfo PInfo;
      if(Info.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELSHORTINFO,&PInfo))
      {
        if(!PInfo.Plugin)
        {
          bool restore=false;
          { //set directory to far folder
            char buffer1[MAX_PATH],buffer2[MAX_PATH],*filename;
            if(GetModuleFileName(NULL,buffer1,sizeof(buffer1)))
            {
              DWORD res=GetFullPathName(buffer1,sizeof(buffer2),buffer2,&filename);
              if(res&&(res<sizeof(buffer2))&&filename)
              {
                if(filename>buffer2) *(filename-1)=0;
                if(Info.Control(INVALID_HANDLE_VALUE,FCTL_SETPANELDIR,buffer2)) restore=true;
              }
            }
          }
          char root_buffer[MAX_PATH];
          FSF.GetPathRoot(PInfo.CurDir,root_buffer);
          if(!strncmp(root_buffer,VolumeID,strlen(VolumeID)))
          {
            strcpy(DiskName,root_buffer+4);
            if(DiskName[strlen(DiskName)-1]=='\\') DiskName[strlen(DiskName)-1]=0;
            restore=restore&&!DiskEject(DiskName,type);
          }
          else if(FSF.LUpper(root_buffer[0])>='A'&&FSF.LUpper(root_buffer[0])<='Z')
          {
            DiskName[0]=root_buffer[0];
            DiskName[1]=':';
            DiskName[2]=0;
            restore=restore&&!DiskEject(DiskName,type);
          }
          if(restore) Info.Control(INVALID_HANDLE_VALUE,FCTL_SETPANELDIR,PInfo.CurDir);
        }
      }
    }
    free(DiskMenuItems);
  }
}

static void RefreshSCSI(void)
{
  char buff[20],Msg[256]; HANDLE hSCSI; DWORD ret; BOOL WaitMessage=FALSE;
  HANDLE hSScr=Info.SaveScreen(0,0,-1,-1);
  for(int i=0;i<10;i++)
  {
    sprintf(buff,"\\\\.\\Scsi%d:",i);
    sprintf(Msg,GetMsg(mSCSIRefresh),buff);
    const char *MsgItems[]={"",Msg};
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
