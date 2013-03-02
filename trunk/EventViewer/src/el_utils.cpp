static void parse_filelist(wchar_t *List,struct MsgPath **File,int *FileCount)
{
  wchar_t *cur_pos=List,*next_pos,expanded[MAX_PATH];
  *FileCount=0;
  while(cur_pos)
  {
    (*FileCount)++;
    cur_pos=wcschr(cur_pos,';');
    if(cur_pos) cur_pos++;
  }
  if(*FileCount)
  {
    *File=(struct MsgPath *)malloc(sizeof(struct MsgPath)*(*FileCount));
    if(*File)
    {
      cur_pos=List;
      for(int i=1;i<(*FileCount);i++)
      {
        next_pos=wcschr(cur_pos,';');
        if(next_pos)
        {
          wcsncpy((*File)[i-1].path,cur_pos,next_pos-cur_pos);
          cur_pos=next_pos+1;
        }
        else break;
      }
      wcscpy((*File)[(*FileCount)-1].path,cur_pos);
      for(int i=0;i<(*FileCount);i++)
      {
        ExpandEnvironmentStrings((*File)[i].path,expanded,ArraySize(expanded));
        t_CharToOem(expanded,(*File)[i].path);
      }
    }
    else *FileCount=0;
  }
}

static void GetMessageFile(wchar_t *Src,struct MsgPath **MsgFile,int *MsgFileCount,struct MsgPath **ParamsFile,int *ParamsFileCount)
{
  HKEY hKey=NULL;
  MsgFile[0]=0; ParamsFile[0]=0;
  if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,EVENTLOG_KEY,0,KEY_READ,&hKey)==ERROR_SUCCESS)
  {
    wchar_t NameBuffer[MAX_PATH]; LONG Result;
    for(int i=0;;i++)
    {
      Result=RegEnumKey(hKey,i,NameBuffer,ArraySize(NameBuffer));
      if(Result==ERROR_NO_MORE_ITEMS)
        break;
      if(Result==ERROR_SUCCESS)
      {
        wchar_t Key[1024];
        wchar_t MsgFileWork[MAX_PATH];
        HKEY hKey2; DWORD Type; DWORD DataSize=0;
        FSF.sprintf(Key,L"%s\\%s\\%s",EVENTLOG_KEY,NameBuffer,Src);
        if((RegOpenKeyEx(HKEY_LOCAL_MACHINE,Key,0,KEY_QUERY_VALUE,&hKey2))==ERROR_SUCCESS)
        {
          DataSize=sizeof(MsgFileWork);
          if(RegQueryValueEx(hKey2,L"EventMessageFile",0,&Type,(LPBYTE)MsgFileWork,&DataSize)==ERROR_SUCCESS)
          {
            parse_filelist(MsgFileWork,MsgFile,MsgFileCount);
          }
          DataSize=sizeof(MsgFileWork);
          if(RegQueryValueEx(hKey2,L"ParameterMessageFile",0,&Type,(LPBYTE)MsgFileWork,&DataSize)==ERROR_SUCCESS)
          {
            parse_filelist(MsgFileWork,ParamsFile,ParamsFileCount);
          }
          RegCloseKey(hKey2);
          break;
        }
      }
    }
    RegCloseKey(hKey);
  }
}

static wchar_t *GetMsgAnsi(int MsgId)
{
#if 0
  static wchar_t buff[SMALL_BUFFER];
  wchar_t *MsgOem=GetMsg(MsgId);
  if(wcslen(MsgOem)<SMALL_BUFFER)
    t_OemToChar(MsgOem,buff);
  else
    FSF.sprintf(buff,L"%s",L"invalid message");
  return buff;
#endif
  return 0;
}

#define FILL_COUNT 64
#define FILLER L""

static wchar_t *FormatLogMessage(wchar_t *Class,EVENTLOGRECORD *rec)
{
  wchar_t *SourceName=(wchar_t *)(rec+1); HINSTANCE lib; const wchar_t **msgs=NULL;
  wchar_t *fmsg; DWORD fsize=0; wchar_t *params=NULL,*new_params; int params_size=0; wchar_t *res=NULL;
  struct MsgPath *filename=NULL,*params_filename=NULL;
  int filename_count=0,params_filename_count=0;
  if(!SourceName[0]) return NULL;
  GetMessageFile(SourceName,&filename,&filename_count,&params_filename,&params_filename_count);
  if(rec->NumStrings)
  {
    int offset=0; int *param_ptrs=NULL;
    msgs=(const wchar_t **)malloc((rec->NumStrings+FILL_COUNT)*sizeof(wchar_t *));
    if(!msgs) goto clear_exit;
    param_ptrs=(int *)malloc(rec->NumStrings*sizeof(int));
    for(int i=0;i<rec->NumStrings;i++)
    {
      msgs[i]=(wchar_t*)(((char*)rec)+rec->StringOffset+offset);
      offset+=(wcslen(msgs[i])+1)*sizeof(wchar_t);
      if(param_ptrs&&params_filename_count)
      {
        bool has_params=false;
        const wchar_t *endptr=msgs[i];
        wchar_t* new_endptr;
        param_ptrs[i]=params_size+1;
        int zero_len=1,back_offset=0;
        while(*endptr)
        {
          const wchar_t *perc_pos=wcsstr(endptr,L"%%");
          if(!perc_pos) break;
          has_params=true;
          long err=wcstol(perc_pos+2,&new_endptr,10);
          wchar_t *param_message; DWORD param_message_size=0;
          for(int j=0;j<params_filename_count;j++)
          {
            lib=LoadLibrary(params_filename[j].path);
            if(lib)
            {
              param_message_size=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_MAX_WIDTH_MASK|FORMAT_MESSAGE_IGNORE_INSERTS,lib,err,LANG_NEUTRAL,(wchar_t *)&param_message,512,NULL);
              FreeLibrary(lib);
              if(param_message_size)
                break;
            }
          }
          if(param_message_size)
          {
            param_message_size=wcslen(param_message);
            new_params=(wchar_t *)realloc(params,(params_size+param_message_size+(perc_pos-endptr)+zero_len)*sizeof(wchar_t));
            if(new_params)
            {
              params=new_params;
            }
            else
            {
              free(params);
              params=NULL;
              break;
            }
            wcsncpy(params+params_size+back_offset,endptr,perc_pos-endptr);
            wcscpy(params+params_size+(perc_pos-endptr)+back_offset,param_message);
            params_size+=param_message_size+(perc_pos-endptr)+zero_len;

            LocalFree(param_message);
            zero_len=0; back_offset=-1;
          }
          endptr=new_endptr;
        }
        if(has_params)
        {
          if(*endptr)
          {
            int tail_size=wcslen(endptr)+zero_len;
            new_params=(wchar_t *)realloc(params,(params_size+tail_size)*sizeof(wchar_t));
            if(new_params)
            {
              params=new_params;
            }
            else
            {
              free(params);
              params=NULL;
              break;
            }
            wcscpy(params+params_size+back_offset,endptr);
            params_size+=tail_size;
          }
        }
        else
          param_ptrs[i]=0;
      }
    }
    if(param_ptrs)
    {
      if(params)
      {
        for(int i=0;i<rec->NumStrings;i++)
          if(param_ptrs[i])
            msgs[i]=params+param_ptrs[i]-1;
      }
      free(param_ptrs);
    }
    for(int i=rec->NumStrings;i<(rec->NumStrings+FILL_COUNT);i++)
      msgs[i]=FILLER;
  }
  if(filename_count)
  {
    for(int j=0;j<filename_count;j++)
    {
      lib=LoadLibrary(filename[j].path);
      if(lib)
      {
        fsize=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_MAX_WIDTH_MASK,lib,rec->EventID,LANG_NEUTRAL,(wchar_t *)&fmsg,512,(char**)msgs);
        FreeLibrary(lib);
        if(fsize)
          break;
      }
    }
  }
  if(fsize)
  {
    res=(wchar_t *)malloc((fsize+1)*sizeof(wchar_t));
    if(res)
      memcpy(res,fmsg,(fsize+1)*sizeof(wchar_t));
    LocalFree(fmsg);
  }
  else
  {
    int mem_size=1;
    for(int i=0;i<rec->NumStrings;i++)
      mem_size+=2+wcslen(msgs[i]);
    mem_size+=2+wcslen(GetMsg(mFileErr1))+9+wcslen((wchar_t *)(rec+1));
    mem_size+=2+wcslen(GetMsg(mFileErr2));
    res=(wchar_t *)malloc(mem_size*sizeof(wchar_t));
    if(res)
    {
      FSF.sprintf(res,GetMsgAnsi(mFileErr1),rec->EventID&0xffff,(wchar_t *)(rec+1));
      FSF.sprintf(res,L"%s%s",res,L"\r\n");
      FSF.sprintf(res,L"%s%s",res,GetMsgAnsi(mFileErr2));
      FSF.sprintf(res,L"%s%s",res,"\r\n");
      for(int i=0;i<rec->NumStrings;i++)
      {
        FSF.sprintf(res,L"%s%s",res,msgs[i]);
        FSF.sprintf(res,L"%s%s",res,L"\r\n");
      }
    }
  }
  free(params);
  if(msgs) free(msgs);
clear_exit:
  free(filename);
  free(params_filename);
  return res;
}

#undef FILL_COUNT
#undef FILLER

static wchar_t *GetComputerName(EVENTLOGRECORD *rec)
{
  static wchar_t CompName[SMALL_BUFFER];
  wchar_t *SourceName=(wchar_t *)(rec+1);
  FSF.sprintf(CompName,L"%s",SourceName+wcslen(SourceName)+1);
  return CompName;
}

static wchar_t *GetType(DWORD type)
{
#if 0
  switch(type)
  {
    case EVENTLOG_ERROR_TYPE:
      return GetMsg(mTypeError);
    case EVENTLOG_WARNING_TYPE:
      return GetMsg(mTypeWarning);
    case EVENTLOG_INFORMATION_TYPE:
      return GetMsg(mTypeInformation);
    case EVENTLOG_AUDIT_SUCCESS:
      return GetMsg(mTypeSuccessAudit);
    case EVENTLOG_AUDIT_FAILURE:
      return GetMsg(mTypeFailureAudit);
  }
  return GetMsg(mTypeUnknown);
#endif
  return 0;
}

static BOOL CheckLogName(EventViewer *panel,const wchar_t *LogName)
{
  BOOL res=FALSE; wchar_t LogNameAnsi[SMALL_BUFFER];
  t_OemToChar(LogName,LogNameAnsi);
  HANDLE evt=OpenEventLog(panel->computer_ptr,LogNameAnsi); //REMOTE
  if(evt)
  {
    CloseEventLog(evt);
    res=TRUE;
  }
  return res;
}
