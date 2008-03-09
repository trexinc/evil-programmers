{
  FileRec *files;
  int i;

  files=(FileRec *)(send+3);
  for(i=0;i<PInfo.SelectedItemsNumber;i++)
  {
    if(PInfo.Plugin)
    {
      MultiByteToWideChar(CP_OEMCP,0,PInfo.SelectedItems[i].FindData.cFileName,-1,files[i].name,MAX_PATH);
      if(PInfo.SelectedItems[i].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
      {
        files[i].type=FILETYPE_FOLDER;
        if(PInfo.SelectedItems[i].FindData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
          files[i].type|=FILETYPE_JUNCTION;
        wchar_t TestSrcW[MAX_PATH];
        wcscpy(TestSrcW,files[i].name);
        wcscat(TestSrcW,L"\\");
        CHECK_DEST
      }
      else
        files[i].type=FILETYPE_FILE;
      continue;
    }
    const char *RealFileName=GetRealName(&(PInfo.SelectedItems[i].FindData));
    char FileNameA[MAX_PATH]; wchar_t FileNameW[MAX_PATH]; WIN32_FIND_DATAW find;
    files[i].type=FILETYPE_UNKNOWN;
    if(RealFileName)
    {
      strcpy(FileNameA,SrcA);
      strcat(FileNameA,RealFileName);
      MultiByteToWideChar(CP_OEMCP,0,FileNameA,-1,FileNameW,MAX_PATH);
      HANDLE hFind=FindFirstFileW(FileNameW,&find);
      if(hFind!=INVALID_HANDLE_VALUE)
      {
        FindClose(hFind);
        if(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
        {
          files[i].type=FILETYPE_FOLDER;
          if(find.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
            files[i].type|=FILETYPE_JUNCTION;
          wchar_t TestSrcW[MAX_PATH];
          wcscpy(TestSrcW,SrcW);
          wcscat(TestSrcW,find.cFileName);
          wcscat(TestSrcW,L"\\");
          CHECK_DEST
        }
        else
          files[i].type=FILETYPE_FILE;
        //wcscpy(files[i].name,strcmp(RealFileName,"..")?find.cFileName:L"..");
        wcscpy(files[i].name,find.cFileName);
      }
    }
  }
/*  if(PInfo.SelectedItemsNumber==1&&!wcscmp(files[0].name,L".."))
  {
    wchar_t FileNameW[MAX_PATH],*FilePtrW;
    wcscpy(FileNameW,SrcW);
    wcscat(FileNameW,files[0].name);
    DWORD full_res=GetFullPathNameW(FileNameW,sizeofa(SrcW),SrcW,&FilePtrW);
    if(!full_res||full_res>=sizeofa(SrcW))
    {
      ON_ERROR
    }
    else if(FilePtrW)
    {
      wcscpy(files[0].name,FilePtrW);
      FilePtrW[0]=0;
    }
  }*/
  if(bcopy_flags[BCOPY_REN_COPY]&&bcopy_flags[BCOPY_NOREAL_COPY]&&bcopy_flags[BCOPY_MOVE])
  {
    if(files[0].type==FILETYPE_FOLDER)
    {
      wchar_t move_source[MAX_PATH];
      wcscpy(move_source,SrcW);
      wcscat(move_source,files[0].name);
      if(!MoveFileExW(move_source,DEST_W,0))
      {
        const char *MsgItems[]={GetMsg(mError),GetMsg(mOk)};
        Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeofa(MsgItems),1);
        ON_ERROR
      }
      else
      {
        Info.Control(INVALID_HANDLE_VALUE,FCTL_UPDATEPANEL,NULL);
        Info.Control(INVALID_HANDLE_VALUE,FCTL_REDRAWPANEL,NULL);
      }
    }
  }
  else
  {
    if(bcopy_flags[BCOPY_REN_COPY])
    {
      if(files[0].type==FILETYPE_FOLDER)
      {
        wchar_t Mask[MAX_PATH]; FileRec *new_files=NULL; long new_files_count=0;
        HANDLE hFind; WIN32_FIND_DATAW find;
        wcscat(SrcW,files[0].name);
        wcscat(SrcW,L"\\");
        wcscpy(Mask,SrcW);
        wcscat(Mask,L"*");
        hFind=FindFirstFileW(Mask,&find);
        if(hFind!=INVALID_HANDLE_VALUE)
        {
          do
          {
            FileRec *renew_files;
            if(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY&&!(wcscmp(find.cFileName,L".")&&wcscmp(find.cFileName,L".."))) continue;

            renew_files=(FileRec *)realloc(new_files,sizeof(FileRec)*(new_files_count+1));
            if(renew_files)
            {
              new_files=renew_files;
              if(find.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
              {
                new_files[new_files_count].type=FILETYPE_FOLDER;
                if(find.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
                  new_files[new_files_count].type|=FILETYPE_JUNCTION;
              }
              else
                new_files[new_files_count].type=FILETYPE_FILE;
              wcscpy(new_files[new_files_count].name,find.cFileName);
              new_files_count++;
            }
            else
            {
              free(new_files);
              new_files=NULL;
              new_files_count=0;
              ShowError(mErrorMemory,false);
              break;
            }
          } while(FindNextFileW(hFind,&find));
          FindClose(hFind);
        }
        else ON_ERROR
        if(!new_files_count) ON_ERROR
        DWORD *new_send=NULL;
        DWORD new_sendsize=sizeof(DWORD)*3+sizeof(FileRec)*(new_files_count+2);
        new_send=(DWORD *)malloc(new_sendsize);
        if(!new_send) { free(new_files); ON_ERROR }
        new_send[0]=send[0]; new_send[1]=send[1]; new_send[2]=new_files_count+2;
        memcpy(new_send+3,new_files,sizeof(FileRec)*new_files_count);
        free(new_files);
        free(send);
        send=new_send; sendsize=new_sendsize;
        files=(FileRec *)(send+3);
        i=new_files_count;
      }
    }

    files[i].type=FILETYPE_FOLDER;
    if(bcopy_flags[BCOPY_REN_COPY]&&bcopy_flags[BCOPY_MOVE])
      files[i].type|=FILETYPE_DELETE;
    wcscpy(files[i].name,SrcW); i++;

    files[i].type=FILETYPE_FOLDER;
    wcscpy(files[i].name,DEST_W);

    SendToPipe(send,sendsize,&RetData);
    //clear selection
    for(int j=0;j<PInfo.ItemsNumber;j++)
      PInfo.PanelItems[j].Flags&=~PPIF_SELECTED;
    Info.Control(INVALID_HANDLE_VALUE,FCTL_SETSELECTION,&PInfo);
    Info.Control(INVALID_HANDLE_VALUE,FCTL_REDRAWPANEL,NULL);
    //show info dialog
    if(PlgOpt.AutoShowInfo)
      ShowInfoDialog(&RetData);
  }
}

#undef CHECK_DEST
#undef DEST_W
#undef ON_ERROR
