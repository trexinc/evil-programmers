#define WRAPPER_PROLOG(ERROR_VALUE,ERROR_TYPE) DWORD Error,Result; ERROR_TYPE Ret=ERROR_VALUE; while(Ret==ERROR_VALUE) { Ret=
#define WRAPPER_EPILOG(ERROR_VALUE,SKIP_ERROR) if(Ret==ERROR_VALUE&&(*flags&COPYFLAG_FAIL_ASK)) { Error=GetLastError(); if(Error==SKIP_ERROR) break; Result=AskUserInfo(ASKGROUP_RETRY,Error); Result=Result&ASKTYPE_MASK; if(Result!=ASKTYPE_RETRY) { if(Result==ASKTYPE_CANCEL) { (*flags)&=~COPYFLAG_FAIL_ASK; (*flags)|=COPYFLAG_FAIL; } else if(Result==ASKTYPE_SKIPALL) { (*flags)&=~(COPYFLAG_FAIL_ASK|COPYFLAG_FAIL); } SetLastError(Error); break; } } else if(Ret==ERROR_VALUE) break; } return Ret;

static BOOL MyCopyFileEx(LPCWSTR lpExistingFileName,LPCWSTR lpNewFileName,LPPROGRESS_ROUTINE lpProgressRoutine,LPVOID lpData,LPBOOL pbCancel,DWORD dwCopyFlags,DWORD *flags)
{
  WRAPPER_PROLOG(0,BOOL)
  CopyFileExW(lpExistingFileName,lpNewFileName,lpProgressRoutine,lpData,pbCancel,dwCopyFlags);
  WRAPPER_EPILOG(0,ERROR_REQUEST_ABORTED)
}

static BOOL MySetFileAttributesEx(wchar_t *file,DWORD *flags,wchar_t **errname)
{
  WRAPPER_PROLOG(0,BOOL)
  SetFileAttributesExW(file,flags,errname);
  WRAPPER_EPILOG(0,ERROR_SUCCESS)
}

static HANDLE MyCreateFile(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile,DWORD *flags)
{
  WRAPPER_PROLOG(INVALID_HANDLE_VALUE,HANDLE)
  CreateFileW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
  WRAPPER_EPILOG(INVALID_HANDLE_VALUE,ERROR_SUCCESS)
}

static BOOL MySetEndOfFile(HANDLE hFile,DWORD *flags)
{
  WRAPPER_PROLOG(0,BOOL)
  SetEndOfFile(hFile);
  WRAPPER_EPILOG(0,ERROR_SUCCESS)
}

static BOOL MyReadFile(HANDLE hFile,PVOID lpBuffer,DWORD nNumberOfBytesToRead,PDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped,DWORD *flags)
{
  WRAPPER_PROLOG(0,BOOL)
  ReadFile(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped);
  WRAPPER_EPILOG(0,ERROR_SUCCESS)
}

static BOOL MyWriteFile(HANDLE hFile,PCVOID lpBuffer,DWORD nNumberOfBytesToWrite,PDWORD lpNumberOfBytesWritten,LPOVERLAPPED lpOverlapped,DWORD *flags)
{
  WRAPPER_PROLOG(0,BOOL)
  WriteFile(hFile,lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWritten,lpOverlapped);
  WRAPPER_EPILOG(0,ERROR_SUCCESS)
}

static BOOL MyDeleteFile(LPCWSTR lpFileName,DWORD *flags)
{
  WRAPPER_PROLOG(0,BOOL)
  DeleteFileW(lpFileName);
  WRAPPER_EPILOG(0,ERROR_FILE_NOT_FOUND)
}

static BOOL MyMoveFileEx(LPCWSTR lpExistingFileName,LPCWSTR lpNewFileName,DWORD dwFlags,DWORD *flags)
{
  WRAPPER_PROLOG(0,BOOL)
  MoveFileExW(lpExistingFileName,lpNewFileName,dwFlags);
  WRAPPER_EPILOG(0,ERROR_NOT_SAME_DEVICE)
}

static HANDLE MyFindFirstFile(LPCWSTR lpFileName,LPWIN32_FIND_DATAW lpFindFileData,DWORD *flags)
{
  WRAPPER_PROLOG(INVALID_HANDLE_VALUE,HANDLE)
  FindFirstFileW(lpFileName,lpFindFileData);
  WRAPPER_EPILOG(INVALID_HANDLE_VALUE,ERROR_SUCCESS)
}

static BOOL MyRemoveDirectoryEx(wchar_t *src,DWORD *flags)
{
  WRAPPER_PROLOG(0,BOOL)
  RemoveDirectoryExW(src,flags);
  WRAPPER_EPILOG(0,ERROR_FILE_NOT_FOUND)
}

static BOOL MySetFilePointer(HANDLE hFile,LONG lDistanceToMove,LONG lDistanceToMoveHigh,DWORD dwMoveMethod,DWORD *flags)
{
  WRAPPER_PROLOG(0,BOOL)
  SetFilePointerNTEx(hFile,lDistanceToMove,lDistanceToMoveHigh,dwMoveMethod);
  WRAPPER_EPILOG(0,ERROR_SUCCESS)
}

static BOOL MyWipeFile(wchar_t *filename,DWORD *flags)
{
  WRAPPER_PROLOG(0,BOOL)
  WipeFileW(filename);
  WRAPPER_EPILOG(0,ERROR_SUCCESS)
}

static BOOL MyCreateDirectoryEx(LPCWSTR lpPathName,DWORD *flags)
{
  WRAPPER_PROLOG(0,BOOL)
  CreateDirectoryNTExW(lpPathName);
  WRAPPER_EPILOG(0,ERROR_ALREADY_EXISTS)
}

static BOOL MyCopyLink(LPCWSTR lpSourcrFileName,LPCWSTR lpDestinationFileName,DWORD *flags)
{
  WRAPPER_PROLOG(0,BOOL)
  CopyLinkW(lpSourcrFileName,lpDestinationFileName);
  WRAPPER_EPILOG(0,ERROR_SUCCESS)
}

/*static BOOL MyCopyFileEx(LPCWSTR lpExistingFileName,LPCWSTR lpNewFileName,LPPROGRESS_ROUTINE lpProgressRoutine,LPVOID lpData,LPBOOL pbCancel,DWORD dwCopyFlags,DWORD *flags)
{
  DWORD Error,Result;
  BOOL Ret=0;
  (void)flags;
  while(Ret==0)
  {
    Ret=CopyFileExW(lpExistingFileName,lpNewFileName,lpProgressRoutine,lpData,pbCancel,dwCopyFlags);
    if(Ret==0&&(*flags&COPYFLAG_FAIL_ASK))
    {
      Error=GetLastError();
      if(Error==??) break;
      Result=AskUserInfo(ASKGROUP_RETRY,Error);
      Result=Result&ASKTYPE_MASK;
      if(Result!=ASKTYPE_RETRY)
      {
        if(Result==ASKTYPE_CANCEL)
        {
          (*flags)&=~COPYFLAG_FAIL_ASK;
          (*flags)|=COPYFLAG_FAIL;
        }
        else if(Result==ASKTYPE_SKIPALL)
        {
          (*flags)&=~(COPYFLAG_FAIL_ASK|COPYFLAG_FAIL);
        }
        SetLastError(Error);
        break;
      }
    } else if(Ret==0) break;
  }
  return Ret;
}
*/

#undef WRAPPER_PROLOG
#undef WRAPPER_EPILOG
