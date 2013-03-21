typedef struct
{
    ULONG          AllocationSize;
    ULONG          ActualSize;
    ULONG          Flags;
    ULONG          Unknown1;
    UNICODE_STRING Unknown2;
    HANDLE         InputHandle;
    HANDLE         OutputHandle;
    HANDLE         ErrorHandle;
    UNICODE_STRING CurrentDirectory;
    HANDLE         CurrentDirectoryHandle;
    UNICODE_STRING SearchPaths;
    UNICODE_STRING ApplicationName;
    UNICODE_STRING CommandLine;
    PVOID          EnvironmentBlock;
    ULONG          Unknown[9];
    UNICODE_STRING Unknown3;
    UNICODE_STRING Unknown4;
    UNICODE_STRING Unknown5;
    UNICODE_STRING Unknown6;
} PROCESS_PARAMETERS, *PPROCESS_PARAMETERS;

typedef struct
{
    ULONG               AllocationSize;
    ULONG               Unknown1;
    HINSTANCE           ProcessHinstance;
    PVOID               ListDlls;
    PPROCESS_PARAMETERS ProcessParameters;
    ULONG               Unknown2;
    HANDLE              Heap;
} Y_PEB, *Y_PPEB;

typedef struct
{
    DWORD ExitStatus;
    PPEB  PebBaseAddress;
    DWORD AffinityMask;
    DWORD BasePriority;
    ULONG UniqueProcessId;
    ULONG InheritedFromUniqueProcessId;
}   Y_PROCESS_BASIC_INFORMATION;

typedef long (WINAPI *PROCNTQSIP)(HANDLE,UINT,PVOID,ULONG,PULONG);

BOOL GetProcessCmdLine(HANDLE hProcess,wchar_t* wBuf,DWORD dwBufLen,DWORD& dwRead)
{
 PROCESS_BASIC_INFORMATION pbi;
 Y_PEB Peb;
 PROCESS_PARAMETERS ProcParam;
 SIZE_T dwDummy;

 static PROCNTQSIP pNtQueryInformationProcess;

	if(!pNtQueryInformationProcess)
	{
		HMODULE hNtDll= GetModuleHandleW(L"ntdll");
		if(hNtDll)
		{
			pNtQueryInformationProcess=(PROCNTQSIP)GetProcAddress(hNtDll,"NtQueryInformationProcess");
		}
	}
	if(!pNtQueryInformationProcess)
		return FALSE;
 // Retrieve information
 if(!pNtQueryInformationProcess(hProcess,ProcessBasicInformation,(PVOID)&pbi,sizeof(PROCESS_BASIC_INFORMATION),NULL) &&
         ReadProcessMemory(hProcess,pbi.PebBaseAddress,&Peb,sizeof(Y_PEB),&dwDummy) &&
         ReadProcessMemory(hProcess,Peb.ProcessParameters,&ProcParam,sizeof(PROCESS_PARAMETERS),&dwDummy))
 {
  dwRead=ProcParam.CommandLine.Length;
  if(dwBufLen>ProcParam.CommandLine.Length && ReadProcessMemory(hProcess,ProcParam.CommandLine.Buffer,wBuf,ProcParam.CommandLine.Length,&dwDummy))
   return TRUE;
 }
 return FALSE;
}