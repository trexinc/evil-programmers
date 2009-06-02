#ifndef __COMMON_HPP__
#define __COMMON_HPP__

typedef struct _PEB *PPEB;

enum PROCESSINFOCLASS
{
  ProcessBasicInformation = 0,
  ProcessQuotaLimits = 1,
  ProcessVmCounters = 3,
  ProcessTimes = 4
};

typedef ULONG KAFFINITY;
typedef LONG  KPRIORITY;
typedef LONG NTSTATUS;

typedef struct _PROCESS_BASIC_INFORMATION
{
  NTSTATUS ExitStatus;
  PPEB PebBaseAddress;
  KAFFINITY AffinityMask;
  KPRIORITY BasePriority;
  ULONG UniqueProcessId;
  ULONG InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;

LPTSTR MyStrChr(LPCTSTR string, TCHAR ch);
INT MyStrChrPos(LPCTSTR string, TCHAR ch);
BOOL Val(LPCTSTR lpSrc, PINT Result);
VOID MyWriteConsoleA(LPCSTR ConBuf, BOOL fUnicode, BOOL fOutOrErr);
VOID MyWriteConsole(LPCTSTR ConBuf, BOOL fUnicode, BOOL fOutOrErr);
LPTSTR GetErrorMessage(DWORD MessageId);
DWORD GetParentProcessID(DWORD dwPID);

#endif /* __COMMON_HPP__ */