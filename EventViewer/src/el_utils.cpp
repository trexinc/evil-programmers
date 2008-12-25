static void parse_filelist(char *List,struct MsgPath **File,int *FileCount)
{
  char *cur_pos=List,*next_pos,expanded[MAX_PATH];
  *FileCount=0;
  while(cur_pos)
  {
    (*FileCount)++;
    cur_pos=strchr(cur_pos,';');
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
        next_pos=strchr(cur_pos,';');
        if(next_pos)
        {
          strncpy((*File)[i-1].path,cur_pos,next_pos-cur_pos);
          cur_pos=next_pos+1;
        }
        else break;
      }
      strcpy((*File)[(*FileCount)-1].path,cur_pos);
      for(int i=0;i<(*FileCount);i++)
      {
        ExpandEnvironmentStrings((*File)[i].path,expanded,sizeof(expanded));
        CharToOem(expanded,(*File)[i].path);
      }
    }
    else *FileCount=0;
  }
}

static void GetMessageFile(char *Src,struct MsgPath **MsgFile,int *MsgFileCount,struct MsgPath **ParamsFile,int *ParamsFileCount)
{
  HKEY hKey=NULL;
  MsgFile[0]=0; ParamsFile[0]=0;
  if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,EVENTLOG_KEY,0,KEY_READ,&hKey)==ERROR_SUCCESS)
  {
    char NameBuffer[MAX_PATH]; LONG Result;
    for(int i=0;;i++)
    {
      Result=RegEnumKey(hKey,i,NameBuffer,sizeof(NameBuffer));
      if(Result==ERROR_NO_MORE_ITEMS)
        break;
      if(Result==ERROR_SUCCESS)
      {
        char Key[1024];
        char MsgFileWork[MAX_PATH];
        HKEY hKey2; DWORD Type; DWORD DataSize=0;
        sprintf(Key,"%s\\%s\\%s",EVENTLOG_KEY,NameBuffer,Src);
        if((RegOpenKeyEx(HKEY_LOCAL_MACHINE,Key,0,KEY_QUERY_VALUE,&hKey2))==ERROR_SUCCESS)
        {
          DataSize=sizeof(MsgFileWork);
          if(RegQueryValueEx(hKey2,"EventMessageFile",0,&Type,(LPBYTE)MsgFileWork,&DataSize)==ERROR_SUCCESS)
          {
            parse_filelist(MsgFileWork,MsgFile,MsgFileCount);
          }
          DataSize=sizeof(MsgFileWork);
          if(RegQueryValueEx(hKey2,"ParameterMessageFile",0,&Type,(LPBYTE)MsgFileWork,&DataSize)==ERROR_SUCCESS)
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

static char *GetMsgAnsi(int MsgId)
{
  static char buff[SMALL_BUFFER];
  char *MsgOem=GetMsg(MsgId);
  if(strlen(MsgOem)<SMALL_BUFFER)
    OemToChar(MsgOem,buff);
  else
    sprintf(buff,"%s","invalid message");
  return buff;
}

#define FILL_COUNT 64
#define FILLER ""

static char *FormatLogMessage(char *Class,EVENTLOGRECORD *rec)
{
  char *SourceName=(char *)(rec+1); HINSTANCE lib; char **msgs=NULL;
  char *fmsg; DWORD fsize=0; char *params=NULL,*new_params; int params_size=0; char *res=NULL;
  struct MsgPath *filename=NULL,*params_filename=NULL;
  int filename_count=0,params_filename_count=0;
  if(!SourceName[0]) return NULL;
  GetMessageFile(SourceName,&filename,&filename_count,&params_filename,&params_filename_count);
  if(rec->NumStrings)
  {
    int offset=0; int *param_ptrs=NULL;
    msgs=(char **)malloc((rec->NumStrings+FILL_COUNT)*sizeof(char *));
    if(!msgs) goto clear_exit;
    param_ptrs=(int *)malloc(rec->NumStrings*sizeof(int));
    for(int i=0;i<rec->NumStrings;i++)
    {
      msgs[i]=((char *)rec)+rec->StringOffset+offset;
      offset+=strlen(msgs[i])+1;
      if(param_ptrs&&params_filename_count)
      {
        bool has_params=false;
        char *endptr=msgs[i],*new_endptr;
        param_ptrs[i]=params_size+1;
        int zero_len=1,back_offset=0;
        while(*endptr)
        {
          char *perc_pos=strstr(endptr,"%%");
          if(!perc_pos) break;
          has_params=true;
          long err=strtol(perc_pos+2,&new_endptr,10);
          char *param_message; DWORD param_message_size=0;
          for(int j=0;j<params_filename_count;j++)
          {
            lib=LoadLibrary(params_filename[j].path);
            if(lib)
            {
              param_message_size=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_MAX_WIDTH_MASK|FORMAT_MESSAGE_IGNORE_INSERTS,lib,err,LANG_NEUTRAL,(char *)&param_message,512,NULL);
              FreeLibrary(lib);
              if(param_message_size)
                break;
            }
          }
          if(param_message_size)
          {
            param_message_size=strlen(param_message);
            new_params=(char *)realloc(params,params_size+param_message_size+(perc_pos-endptr)+zero_len);
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
            strncpy(params+params_size+back_offset,endptr,perc_pos-endptr);
            strcpy(params+params_size+(perc_pos-endptr)+back_offset,param_message);
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
            int tail_size=strlen(endptr)+zero_len;
            new_params=(char *)realloc(params,params_size+tail_size);
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
            strcpy(params+params_size+back_offset,endptr);
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
        fsize=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_MAX_WIDTH_MASK,lib,rec->EventID,LANG_NEUTRAL,(char *)&fmsg,512,msgs);
        FreeLibrary(lib);
        if(fsize)
          break;
      }
    }
  }
  if(fsize)
  {
    res=(char *)malloc(fsize+1);
    if(res)
      memcpy(res,fmsg,fsize+1);
    LocalFree(fmsg);
  }
  else
  {
    int mem_size=1;
    for(int i=0;i<rec->NumStrings;i++)
      mem_size+=2+strlen(msgs[i]);
    mem_size+=2+strlen(GetMsg(mFileErr1))+9+strlen((char *)(rec+1));
    mem_size+=2+strlen(GetMsg(mFileErr2));
    res=(char *)malloc(mem_size);
    if(res)
    {
      sprintf(res,GetMsgAnsi(mFileErr1),rec->EventID&0xffff,(char *)(rec+1));
      sprintf(res,"%s%s",res,"\r\n");
      sprintf(res,"%s%s",res,GetMsgAnsi(mFileErr2));
      sprintf(res,"%s%s",res,"\r\n");
      for(int i=0;i<rec->NumStrings;i++)
      {
        sprintf(res,"%s%s",res,msgs[i]);
        sprintf(res,"%s%s",res,"\r\n");
      }
    }
  }
  free(params);
  if(msgs) free(msgs);
  CharToOem(res,res);
clear_exit:
  free(filename);
  free(params_filename);
  return res;
}

#undef FILL_COUNT
#undef FILLER

static char *GetComputerName(EVENTLOGRECORD *rec)
{
  static char CompName[SMALL_BUFFER];
  char *SourceName=(char *)(rec+1);
  sprintf(CompName,"%s",SourceName+strlen(SourceName)+1);
  CharToOem(CompName,CompName);
  return CompName;
}

static char *GetType(DWORD type)
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

static BOOL CheckLogName(EventViewer *panel,const char *LogName)
{
  BOOL res=FALSE; char LogNameAnsi[SMALL_BUFFER];
  OemToChar(LogName,LogNameAnsi);
  HANDLE evt=OpenEventLog(panel->computer_ptr,LogNameAnsi); //REMOTE
  if(evt)
  {
    CloseEventLog(evt);
    res=TRUE;
  }
  return res;
}

