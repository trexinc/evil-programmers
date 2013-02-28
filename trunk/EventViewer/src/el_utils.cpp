static void parse_filelist(TCHAR *List,struct MsgPath **File,int *FileCount)
{
  TCHAR *cur_pos=List,*next_pos,expanded[MAX_PATH];
  *FileCount=0;
  while(cur_pos)
  {
    (*FileCount)++;
    cur_pos=_tcschr(cur_pos,';');
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
        next_pos=_tcschr(cur_pos,';');
        if(next_pos)
        {
          _tcsncpy((*File)[i-1].path,cur_pos,next_pos-cur_pos);
          cur_pos=next_pos+1;
        }
        else break;
      }
      _tcscpy((*File)[(*FileCount)-1].path,cur_pos);
      for(int i=0;i<(*FileCount);i++)
      {
        ExpandEnvironmentStrings((*File)[i].path,expanded,ArraySize(expanded));
        t_CharToOem(expanded,(*File)[i].path);
      }
    }
    else *FileCount=0;
  }
}

static void GetMessageFile(TCHAR *Src,struct MsgPath **MsgFile,int *MsgFileCount,struct MsgPath **ParamsFile,int *ParamsFileCount)
{
  HKEY hKey=NULL;
  MsgFile[0]=0; ParamsFile[0]=0;
  if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,EVENTLOG_KEY,0,KEY_READ,&hKey)==ERROR_SUCCESS)
  {
    TCHAR NameBuffer[MAX_PATH]; LONG Result;
    for(int i=0;;i++)
    {
      Result=RegEnumKey(hKey,i,NameBuffer,ArraySize(NameBuffer));
      if(Result==ERROR_NO_MORE_ITEMS)
        break;
      if(Result==ERROR_SUCCESS)
      {
        TCHAR Key[1024];
        TCHAR MsgFileWork[MAX_PATH];
        HKEY hKey2; DWORD Type; DWORD DataSize=0;
        FSF.sprintf(Key,_T("%s\\%s\\%s"),EVENTLOG_KEY,NameBuffer,Src);
        if((RegOpenKeyEx(HKEY_LOCAL_MACHINE,Key,0,KEY_QUERY_VALUE,&hKey2))==ERROR_SUCCESS)
        {
          DataSize=sizeof(MsgFileWork);
          if(RegQueryValueEx(hKey2,_T("EventMessageFile"),0,&Type,(LPBYTE)MsgFileWork,&DataSize)==ERROR_SUCCESS)
          {
            parse_filelist(MsgFileWork,MsgFile,MsgFileCount);
          }
          DataSize=sizeof(MsgFileWork);
          if(RegQueryValueEx(hKey2,_T("ParameterMessageFile"),0,&Type,(LPBYTE)MsgFileWork,&DataSize)==ERROR_SUCCESS)
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

static TCHAR *GetMsgAnsi(int MsgId)
{
  static TCHAR buff[SMALL_BUFFER];
  TCHAR *MsgOem=GetMsg(MsgId);
  if(_tcslen(MsgOem)<SMALL_BUFFER)
    t_OemToChar(MsgOem,buff);
  else
    FSF.sprintf(buff,_T("%s"),_T("invalid message"));
  return buff;
}

#define FILL_COUNT 64
#define FILLER _T("")

static TCHAR *FormatLogMessage(TCHAR *Class,EVENTLOGRECORD *rec)
{
  TCHAR *SourceName=(TCHAR *)(rec+1); HINSTANCE lib; const TCHAR **msgs=NULL;
  TCHAR *fmsg; DWORD fsize=0; TCHAR *params=NULL,*new_params; int params_size=0; TCHAR *res=NULL;
  struct MsgPath *filename=NULL,*params_filename=NULL;
  int filename_count=0,params_filename_count=0;
  if(!SourceName[0]) return NULL;
  GetMessageFile(SourceName,&filename,&filename_count,&params_filename,&params_filename_count);
  if(rec->NumStrings)
  {
    int offset=0; int *param_ptrs=NULL;
    msgs=(const TCHAR **)malloc((rec->NumStrings+FILL_COUNT)*sizeof(TCHAR *));
    if(!msgs) goto clear_exit;
    param_ptrs=(int *)malloc(rec->NumStrings*sizeof(int));
    for(int i=0;i<rec->NumStrings;i++)
    {
      msgs[i]=(TCHAR*)(((char*)rec)+rec->StringOffset+offset);
      offset+=(_tcslen(msgs[i])+1)*sizeof(TCHAR);
      if(param_ptrs&&params_filename_count)
      {
        bool has_params=false;
        const TCHAR *endptr=msgs[i];
        TCHAR* new_endptr;
        param_ptrs[i]=params_size+1;
        int zero_len=1,back_offset=0;
        while(*endptr)
        {
          TCHAR *perc_pos=_tcsstr(endptr,_T("%%"));
          if(!perc_pos) break;
          has_params=true;
          long err=_tcstol(perc_pos+2,&new_endptr,10);
          TCHAR *param_message; DWORD param_message_size=0;
          for(int j=0;j<params_filename_count;j++)
          {
            lib=LoadLibrary(params_filename[j].path);
            if(lib)
            {
              param_message_size=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_MAX_WIDTH_MASK|FORMAT_MESSAGE_IGNORE_INSERTS,lib,err,LANG_NEUTRAL,(TCHAR *)&param_message,512,NULL);
              FreeLibrary(lib);
              if(param_message_size)
                break;
            }
          }
          if(param_message_size)
          {
            param_message_size=_tcslen(param_message);
            new_params=(TCHAR *)realloc(params,(params_size+param_message_size+(perc_pos-endptr)+zero_len)*sizeof(TCHAR));
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
            _tcsncpy(params+params_size+back_offset,endptr,perc_pos-endptr);
            _tcscpy(params+params_size+(perc_pos-endptr)+back_offset,param_message);
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
            int tail_size=_tcslen(endptr)+zero_len;
            new_params=(TCHAR *)realloc(params,(params_size+tail_size)*sizeof(TCHAR));
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
            _tcscpy(params+params_size+back_offset,endptr);
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
        fsize=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_MAX_WIDTH_MASK,lib,rec->EventID,LANG_NEUTRAL,(TCHAR *)&fmsg,512,(char**)msgs);
        FreeLibrary(lib);
        if(fsize)
          break;
      }
    }
  }
  if(fsize)
  {
    res=(TCHAR *)malloc((fsize+1)*sizeof(TCHAR));
    if(res)
      memcpy(res,fmsg,(fsize+1)*sizeof(TCHAR));
    LocalFree(fmsg);
  }
  else
  {
    int mem_size=1;
    for(int i=0;i<rec->NumStrings;i++)
      mem_size+=2+_tcslen(msgs[i]);
    mem_size+=2+_tcslen(GetMsg(mFileErr1))+9+_tcslen((TCHAR *)(rec+1));
    mem_size+=2+_tcslen(GetMsg(mFileErr2));
    res=(TCHAR *)malloc(mem_size*sizeof(TCHAR));
    if(res)
    {
      FSF.sprintf(res,GetMsgAnsi(mFileErr1),rec->EventID&0xffff,(TCHAR *)(rec+1));
      FSF.sprintf(res,_T("%s%s"),res,_T("\r\n"));
      FSF.sprintf(res,_T("%s%s"),res,GetMsgAnsi(mFileErr2));
      FSF.sprintf(res,_T("%s%s"),res,_T("\r\n"));
      for(int i=0;i<rec->NumStrings;i++)
      {
        FSF.sprintf(res,_T("%s%s"),res,msgs[i]);
        FSF.sprintf(res,_T("%s%s"),res,_T("\r\n"));
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

static TCHAR *GetComputerName(EVENTLOGRECORD *rec)
{
  static TCHAR CompName[SMALL_BUFFER];
  TCHAR *SourceName=(TCHAR *)(rec+1);
  FSF.sprintf(CompName,_T("%s"),SourceName+_tcslen(SourceName)+1);
  return CompName;
}

static TCHAR *GetType(DWORD type)
{
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
}

static BOOL CheckLogName(EventViewer *panel,const TCHAR *LogName)
{
  BOOL res=FALSE; TCHAR LogNameAnsi[SMALL_BUFFER];
  t_OemToChar(LogName,LogNameAnsi);
  HANDLE evt=OpenEventLog(panel->computer_ptr,LogNameAnsi); //REMOTE
  if(evt)
  {
    CloseEventLog(evt);
    res=TRUE;
  }
  return res;
}
