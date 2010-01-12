#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  if(argc>1)
  {
    HANDLE file=CreateFile(argv[1],GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(file!=INVALID_HANDLE_VALUE)
    {
      if (argc>2)
      {
        char ibuff[1024], obuff[1024];
        DWORD transferred;
        DWORD build;
        ReadFile(file,ibuff,1024,&transferred,NULL);
        ibuff[transferred]=0;
        sscanf(ibuff,"m4_define(BLD,%u",&build);
        SetFilePointer(file,0,0,FILE_BEGIN);
        SetEndOfFile(file);
        sprintf(obuff,"m4_define(BLD,%u",++build);
        strcat(obuff,strstr(ibuff+strlen("m4_define(BLD,"),")"));
        WriteFile(file,obuff,strlen(obuff),&transferred,NULL);
      }
      else
      {
        SYSTEMTIME currt, sfilet;
        FILETIME filet, lfilet;
        GetLocalTime(&currt);
        GetFileTime(file,NULL,NULL,&filet);
        FileTimeToLocalFileTime(&filet,&lfilet);
        FileTimeToSystemTime(&lfilet,&sfilet);
        if (currt.wDay!=sfilet.wDay||currt.wMonth!=sfilet.wMonth||currt.wYear!=sfilet.wYear)
        {
          SystemTimeToFileTime(&currt,&lfilet);
          LocalFileTimeToFileTime(&lfilet,&filet);
          SetFileTime(file,NULL,NULL,&filet);
        }
      }
      CloseHandle(file);
    }
  }
  else
  {
    SYSTEMTIME currt;
    GetLocalTime(&currt);
    printf("%2.2d/%2.2d/%4.4d\n",currt.wDay,currt.wMonth,currt.wYear);
  }
  return 0;
}
