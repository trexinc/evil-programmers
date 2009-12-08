#include "headers.hpp"

#include "Imports.hpp"

#include "update.hpp"
#include "lng.hpp"

#include "Underscore.Archives.h"

#include "Console.hpp"
#include "CursorPos.hpp"
#include "headers.hpp"
#include "HideCursor.hpp"
#include "TextColor.hpp"

enum STATUS
{
	S_CANTCONNECT,
	S_UPTODATE,
	S_REQUIRED,
};

enum EVENT
{
	E_ASKLOAD,
	E_CONNECTFAIL,
	E_DOWNLOADED,
};

struct EventStruct
{
	EVENT Event;
	LPVOID Data;
	bool *Result;
};

struct IPC
{
	TCHAR FarModule[MAX_PATH];
	TCHAR FarDirectory[MAX_PATH];
	TCHAR PluginModule[MAX_PATH];
	TCHAR PluginDirectory[MAX_PATH];
	TCHAR FarParams[MAX_PATH*4];
	TCHAR TempDirectory[MAX_PATH];
	TCHAR Config[MAX_PATH];
	TCHAR UpdateList[MAX_PATH];
	TCHAR SevenZip[MAX_PATH];
	bool UseMsi;
} ipc;

LPCTSTR strVer=TEXT("1.0");
LPCTSTR Directory=TEXT("FUTMP");
LPCTSTR RemoteSrv=TEXT("www.farmanager.com");
LPCTSTR RemotePath=TEXT("/nightly");

LPCTSTR phpFile=

#ifndef UNICODE
                TEXT("update.php");
#else
                TEXT("update2.php");
#endif

LPCTSTR phpRequest=

#ifdef _WIN64
                   TEXT("?p=64");
#else
                   TEXT("?p=32");
#endif

bool bUseProxy;
TCHAR strProxyName[512];
TCHAR strProxyUser[512];
TCHAR strProxyPass[512];

bool lock=false;
bool exitfar=false;
#ifndef UNICODE
bool wrapper=false;
#endif
bool NeedRestart=false;

SYSTEMTIME SavedTime;

HANDLE hThread=NULL;

HANDLE hRunDll=NULL;

CRITICAL_SECTION cs;

PluginStartupInfo Info;
FarStandardFunctions FSF;

DWORD Mode=0;

INT mprintf(LPCTSTR format,...)
{
	va_list argptr;
	va_start(argptr,format);
	TCHAR buff[1024];
	DWORD n=wvsprintf(buff,format,argptr);
	va_end(argptr);
	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE),buff,n,&n,NULL);
	return n;
}

template<class T>T StringToNumber(LPCTSTR String, T &Number)
{
	Number=0;
	for(LPCTSTR p=String;p&&*p;p++)
		if(*p>=TEXT('0')&&*p<=TEXT('9'))
			Number=Number*10+(*p-TEXT('0'));
	return Number;
}

#ifndef UNICODE
LPSTR * CommandLineToArgvA(LPCSTR lpCmdLineA, __out int* pNumArgs)
{
	INT sz=MultiByteToWideChar(CP_ACP,0,lpCmdLineA,-1,NULL,0);
	LPWSTR CmdLineW=new WCHAR[sz];
	MultiByteToWideChar(CP_ACP,0,lpCmdLineA,-1,CmdLineW,sz);
	LPWSTR * argvW=CommandLineToArgvW(CmdLineW,pNumArgs);
	delete[] CmdLineW;
	
	sz=sizeof(LPSTR)**pNumArgs;
	for(size_t i=0;i<static_cast<size_t>(*pNumArgs);i++)
		sz+=lstrlenW(argvW[i])+1;

	LPSTR *argvA=static_cast<LPSTR*>(LocalAlloc(LMEM_FIXED,sz));
	LPSTR ptr=reinterpret_cast<LPSTR>(argvA)+sizeof(LPSTR)**pNumArgs;
	for(size_t i=0;i<static_cast<size_t>(*pNumArgs);i++)
	{
		int s=lstrlenW(argvW[i])+1;
		WideCharToMultiByte(CP_ACP,0,argvW[i],-1,ptr,s,0,0);
		argvA[i]=ptr;
		ptr+=s;
	}
	LocalFree(argvW);
	return argvA;
}
#define CommandLineToArgv CommandLineToArgvA
#else
#define CommandLineToArgv CommandLineToArgvW
#endif

VOID GetNewModuleVersion(DWORD &NewMajor,DWORD &NewMinor,DWORD &NewBuild)
{
	NewMajor=GetPrivateProfileInt(TEXT("far"),TEXT("major"),-1,ipc.UpdateList);
	NewMinor=GetPrivateProfileInt(TEXT("far"),TEXT("minor"),-1,ipc.UpdateList);
	NewBuild=GetPrivateProfileInt(TEXT("far"),TEXT("build"),-1,ipc.UpdateList);
}

bool NeedUpdate()
{
	DWORD CurVer=static_cast<DWORD>(Info.AdvControl(Info.ModuleNumber,ACTL_GETFARVERSION,NULL));
	DWORD CurMajor=HIBYTE(LOWORD(CurVer)),NewMajor;
	DWORD CurMinor=LOBYTE(LOWORD(CurVer)),NewMinor;
	DWORD CurBuild=HIWORD(CurVer),NewBuild;
	
	GetNewModuleVersion(NewMajor,NewMinor,NewBuild);

	return (NewMajor>CurMajor) || 
	((NewMajor==CurMajor)&&(NewMinor>CurMinor))||
	((NewMajor==CurMajor)&&(NewMinor==CurMinor)&&(NewBuild>CurBuild));
}

typedef BOOL (CALLBACK* DOWNLOADPROC)(DWORD);

BOOL CALLBACK DownloadProc(DWORD Percent)
{
	CursorPos cp;
	if(Percent)
	{
		mprintf(TEXT("%d%%"),Percent);
	}
	else
	{
		mprintf(TEXT("    "));
	}
	return TRUE;
}

DWORD WINAPI WinInetDownload(LPCTSTR strSrv, LPCTSTR strURL, LPCTSTR strFile,DOWNLOADPROC Proc=NULL)
{
	DWORD err=0;

	DWORD ProxyType=INTERNET_OPEN_TYPE_DIRECT;
	if(bUseProxy)
		ProxyType=*strProxyName?INTERNET_OPEN_TYPE_PROXY:INTERNET_OPEN_TYPE_PRECONFIG;

	HINTERNET hInternet=InternetOpen(TEXT("Mozilla/5.0 (compatible; FAR Update)"),ProxyType,strProxyName,NULL,0);
	if(hInternet) 
	{
		BYTE Data[2048];
		DWORD dwBytesRead;
		BOOL bRead=FALSE;

		HINTERNET hConnect=InternetConnect(hInternet,strSrv,INTERNET_DEFAULT_HTTP_PORT,NULL,NULL,INTERNET_SERVICE_HTTP,0,1);
		if(hConnect) 
		{
			if(bUseProxy && *strProxyName)
			{
				if(*strProxyUser)
				{
					if (!InternetSetOption(hConnect,INTERNET_OPTION_PROXY_USERNAME,(LPVOID)strProxyUser,lstrlen(strProxyUser)))
					{
						err=GetLastError();
					}
				}
				if (*strProxyPass)
				{
					if(!InternetSetOption(hConnect,INTERNET_OPTION_PROXY_PASSWORD,(LPVOID)strProxyPass,lstrlen(strProxyPass)))
					{
						err=GetLastError();
					}
				}
			}
			HTTP_VERSION_INFO httpver={1,1};
			if (!InternetSetOption(hConnect, INTERNET_OPTION_HTTP_VERSION, &httpver, sizeof(httpver)))
			{
				err=GetLastError();
			}

			HINTERNET hRequest=HttpOpenRequest(hConnect,TEXT("GET"),strURL,TEXT("HTTP/1.1"),NULL,0,INTERNET_FLAG_KEEP_CONNECTION|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_PRAGMA_NOCACHE|INTERNET_FLAG_RELOAD,1);

			if (hRequest) 
			{
				if (HttpSendRequest(hRequest,NULL,0,NULL,0)) 
				{
					HANDLE hFile=CreateFile(strFile,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
					if(hFile!=INVALID_HANDLE_VALUE)
					{
						DWORD Size=0;
						DWORD sz=sizeof(Size);
						if (!HttpQueryInfo(hRequest,HTTP_QUERY_CONTENT_LENGTH|HTTP_QUERY_FLAG_NUMBER,&Size,&sz,NULL))
						{
							err=GetLastError();
						}
						if(Size!=GetFileSize(hFile,NULL))
						{
							SetEndOfFile(hFile);
							UINT BytesDone=0;
							HideCursor hc;
							while((bRead=InternetReadFile(hRequest,Data,sizeof(Data),&dwBytesRead))!=0&&dwBytesRead)
							{
								BytesDone+=dwBytesRead;
								if(Proc && Size)
								{
									Proc(BytesDone*100/Size);
								}
								DWORD dwWritten=0;
								if (!WriteFile(hFile,Data,dwBytesRead,&dwWritten,NULL))
								{
									err=GetLastError();
									break;
								}
							}
						}
						if(Proc)
						{
							Proc(0);
						}
						CloseHandle(hFile);
					}
					else
					{
						err=GetLastError();
					}
				}
				else
				{
					err=GetLastError();
				}
				InternetCloseHandle(hRequest);
			}
			InternetCloseHandle(hConnect);
		}
		InternetCloseHandle(hInternet);
	}
	return err;
}

bool DownloadFile(LPCTSTR RemoteFile,LPCTSTR LocalName=NULL,bool UseCallBack=false)
{
	TCHAR LocalFile[MAX_PATH];
	lstrcpy(LocalFile,ipc.TempDirectory);
	lstrcat(LocalFile,LocalName?LocalName:FSF.PointToName(RemoteFile));
	return WinInetDownload(RemoteSrv,RemoteFile,LocalFile,UseCallBack?DownloadProc:NULL)==0;
}

DWORD CheckUpdates()
{
	DWORD Ret=S_UPTODATE;
	CreateDirectory(ipc.TempDirectory,NULL);
	TCHAR URL[1024];
	FSF.sprintf(URL,TEXT("%s/%s%s"),RemotePath,phpFile,phpRequest);
	if(DownloadFile(URL,phpFile))
	{
		TCHAR sVer[MAX_PATH];
		GetPrivateProfileString(TEXT("info"),TEXT("version"),TEXT(""),sVer,ARRAYSIZE(sVer),ipc.UpdateList);
		if(lstrcmp(sVer, strVer))
		{
			return S_CANTCONNECT;
		}
		if(NeedUpdate())
		{
			Ret=S_REQUIRED;
		}
	}
	else
	{
		Ret=S_CANTCONNECT;
	}
	return Ret;
}

bool DownloadUpdates(bool Silent=false)
{
	TCHAR URL[1024],arc[MAX_PATH];
	GetPrivateProfileString(TEXT("far"),ipc.UseMsi?TEXT("msi"):TEXT("arc"),TEXT(""),arc,ARRAYSIZE(arc),ipc.UpdateList);
	FSF.sprintf(URL,TEXT("%s/%s"),RemotePath,arc);
	if(!Silent)
	{
		mprintf(TEXT("%s %-50s"),MSG(MLoad),TEXT("Far"));
	}
	if(DownloadFile(URL,NULL,!Silent))
	{
		if(!Silent)
		{
			TextColor color(FOREGROUND_GREEN|FOREGROUND_INTENSITY);
			mprintf(TEXT("OK"));
		}
		NeedRestart=true;
	}
	else
	{
		if(!Silent)
		{
			TextColor color(FOREGROUND_RED|FOREGROUND_INTENSITY);
			mprintf(TEXT("download error %d"),GetLastError());
		}
	}
	if(!Silent)
	{
		mprintf(TEXT("\n"));
	}
	return true;
}

bool Clean()
{
	DeleteFile(ipc.UpdateList);
	RemoveDirectory(ipc.TempDirectory);
	return true;
}

bool IsTime()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	return st.wYear!=SavedTime.wYear||st.wMonth!=SavedTime.wMonth||st.wDay!=SavedTime.wDay;
}

VOID SaveTime()
{
	GetLocalTime(&SavedTime);
}

VOID StartUpdate(bool Silent=false)
{
	DWORD RunDllExitCode=0;
	GetExitCodeProcess(hRunDll,&RunDllExitCode);
	if(RunDllExitCode==STILL_ACTIVE)
	{
		if(!Silent)
		{
			TextColor color(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY);
			mprintf(TEXT("\n%s\n"),MSG(MExitFAR));
		}
	}
	else if(NeedRestart)
	{
		HANDLE ProcDup;
		DuplicateHandle(GetCurrentProcess(),GetCurrentProcess(),GetCurrentProcess(),&ProcDup,NULL,TRUE,DUPLICATE_SAME_ACCESS);

		TCHAR cmdline[MAX_PATH];

		size_t NumArgs=0;
		LPTSTR *Argv=CommandLineToArgv(GetCommandLine(),reinterpret_cast<PINT>(&NumArgs));
		*ipc.FarParams=0;
		for(size_t i=1;i<NumArgs;i++)
		{
			lstrcat(ipc.FarParams,Argv[i]);
			if(i<NumArgs-1)
				lstrcat(ipc.FarParams,TEXT(" "));
		}
		LocalFree(Argv);

		TCHAR WinDir[MAX_PATH];
		GetWindowsDirectory(WinDir,ARRAYSIZE(WinDir));
		BOOL IsWow64=FALSE;
		FSF.sprintf(cmdline,TEXT("%s\\%s\\rundll32.exe \"%s\", RestartFAR %I64d %I64d"),WinDir,ifn.IsWow64Process(GetCurrentProcess(),&IsWow64)&&IsWow64?TEXT("SysWOW64"):TEXT("System32"),ipc.PluginModule,reinterpret_cast<INT64>(ProcDup),reinterpret_cast<INT64>(&ipc));

		STARTUPINFO si={sizeof(si)};
		PROCESS_INFORMATION pi;

		BOOL Created=CreateProcess(NULL,cmdline,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);

		if(Created)
		{
			hRunDll=pi.hProcess;
			CloseHandle(pi.hThread);
			if(!Silent)
			{
				TextColor color(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY);
				mprintf(TEXT("\n%s\n"),MSG(MExitFAR));
			}
			else
			{
#ifndef UNICODE
				char Title[1024],Msg[1024];
				lstrcpy(Title,MSG(MName));
				OemToCharBuff(Title,Title,ARRAYSIZE(Title));
				lstrcpy(Msg,MSG(MUpdatesDownloaded));
				lstrcat(Msg,"\n");
				lstrcat(Msg,MSG(MExitFAR));
				OemToCharBuff(Msg,Msg,ARRAYSIZE(Msg));
				MessageBox(NULL,Msg,Title,MB_ICONINFORMATION);
#else
				HANDLE hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
				EventStruct es={E_DOWNLOADED,hEvent};
				Info.AdvControl(Info.ModuleNumber,ACTL_SYNCHRO,&es);
				WaitForSingleObject(hEvent,INFINITE);
				CloseHandle(hEvent);
#endif
			}
			EnterCriticalSection(&cs);
			SaveTime();
			exitfar=true;
			LeaveCriticalSection(&cs);
		}
		else
		{
			if(!Silent)
			{
				TextColor color(FOREGROUND_RED|FOREGROUND_INTENSITY);
				mprintf(TEXT("%s - error %d"),MSG(MCantCreateProcess),GetLastError());
			}
		}

	}
	else
	{
		if(!Silent)
		{
			TextColor color(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
			mprintf(MSG(MDone));
		}
	}
}

DWORD WINAPI ThreadProc(LPVOID /*lpParameter*/)
{
	bool End=false;

	while(!End)
	{
		bool Time=false;

		EnterCriticalSection(&cs);
		End=exitfar;
		Time=(!lock && IsTime());
		LeaveCriticalSection(&cs);

		if(Time)
		{
			switch(CheckUpdates())
			{
			case S_REQUIRED:
				{
#ifndef UNICODE
					TCHAR Msg[2048];
					lstrcpy(Msg,MSG(MAvailableUpdates));
					OemToCharBuff(Msg,Msg,lstrlen(Msg));
					lstrcat(Msg,TEXT("\n\n"));
					DWORD NewMajor,NewMinor,NewBuild;
					GetNewModuleVersion(NewMajor,NewMinor,NewBuild);
					{
						TCHAR Version[MAX_PATH];
						wsprintf(Version,TEXT("Far Manager (%d.%d build %d)"),NewMajor,NewMinor,NewBuild);
						lstrcat(Msg,Version);
					}
					lstrcat(Msg,TEXT("\n"));
					TCHAR Tmp[1024];
					lstrcpy(Tmp,MSG(MAsk));
					OemToCharBuff(Tmp,Tmp,lstrlen(Tmp));
					lstrcat(Msg,Tmp);
#endif
					EnterCriticalSection(&cs);
					SaveTime();
					lock=true;
					LeaveCriticalSection(&cs);
					bool Load=(Mode==2);
					if(!Load)
					{
#ifndef UNICODE
						Load=(MessageBox(NULL,Msg,TEXT("FAR Update"),MB_ICONINFORMATION|MB_YESNO)==IDYES);
#else
						HANDLE hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
						EventStruct es={E_ASKLOAD,hEvent,&Load};
						Info.AdvControl(Info.ModuleNumber,ACTL_SYNCHRO,&es);
						WaitForSingleObject(hEvent,INFINITE);
						CloseHandle(hEvent);
#endif
					}
					if(Load)
					{
						if(DownloadUpdates(true))
						{
							StartUpdate(true);
						}
					}
					else
					{
						Clean();
					}
					EnterCriticalSection(&cs);
					lock=false;
					LeaveCriticalSection(&cs);
					
				}
				break;
			case S_CANTCONNECT:
				{
#ifndef UNICODE
					CHAR Msg[1024];
					lstrcpy(Msg,MSG(MCantConnect));
					OemToCharBuff(Msg,Msg,lstrlen(Msg));
					if(MessageBox(NULL,Msg,TEXT("FAR Update"),MB_ICONERROR|MB_RETRYCANCEL)==IDCANCEL)
#else
					HANDLE hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
					bool Cancel=false;
					EventStruct es={E_CONNECTFAIL,hEvent,&Cancel};
					Info.AdvControl(Info.ModuleNumber,ACTL_SYNCHRO,&es);
					WaitForSingleObject(hEvent,INFINITE);
					CloseHandle(hEvent);
					if(Cancel)
#endif
					{
						EnterCriticalSection(&cs);
						SaveTime();
						LeaveCriticalSection(&cs);
					}
					Clean();
				}
				break;
			case S_UPTODATE:
				{
					EnterCriticalSection(&cs);
					SaveTime();
					LeaveCriticalSection(&cs);
					Clean();
				}
				break;
			}
		}
		Sleep(1000);
	}
	return 0;
}

VOID InitPaths()
{
	GetModuleFileName(NULL,ipc.FarModule,ARRAYSIZE(ipc.FarModule));

	lstrcpy(ipc.FarDirectory,ipc.FarModule);
	*(StrRChr(ipc.FarDirectory,NULL,TEXT('\\'))+1)=0;

	lstrcpy(ipc.TempDirectory,ipc.FarDirectory);
	lstrcat(ipc.TempDirectory,TEXT("FUTMP\\"));

	lstrcpy(ipc.UpdateList,ipc.TempDirectory);
	lstrcat(ipc.UpdateList,phpFile);

	lstrcpy(ipc.PluginModule,Info.ModuleName);

	lstrcpy(ipc.PluginDirectory,ipc.PluginModule);
	*(StrRChr(ipc.PluginDirectory,NULL,'\\')+1)=0;

	lstrcpy(ipc.Config,ipc.PluginModule);
	lstrcat(ipc.Config,TEXT(".config"));

	lstrcpy(ipc.SevenZip,ipc.PluginDirectory);
	lstrcat(ipc.SevenZip,TEXT("7zxr.dll"));

}

VOID ReadSettings()
{
	Mode=GetPrivateProfileInt(TEXT("update"),TEXT("Mode"),2,ipc.Config);
	ipc.UseMsi=(GetPrivateProfileInt(TEXT("update"),TEXT("Msi"),0,ipc.Config)!=0);
	bUseProxy=GetPrivateProfileInt(TEXT("connect"),TEXT("proxy"),0,ipc.Config) == 1;
	GetPrivateProfileString(TEXT("connect"),TEXT("srv"),TEXT(""),strProxyName,ARRAYSIZE(strProxyName),ipc.Config);
	GetPrivateProfileString(TEXT("connect"),TEXT("user"), TEXT(""),strProxyUser,ARRAYSIZE(strProxyUser),ipc.Config);
	GetPrivateProfileString(TEXT("connect"),TEXT("pass"), TEXT(""),strProxyPass,ARRAYSIZE(strProxyPass),ipc.Config);
}


INT WINAPI EXP_NAME(GetMinFarVersion)()
{
	return MAKEFARVERSION(MIN_FAR_MAJOR_VER,MIN_FAR_MINOR_VER,MIN_FAR_BUILD);
}

VOID WINAPI EXP_NAME(SetStartupInfo)(const PluginStartupInfo* psInfo)
{
	ifn.Load();
	Info=*psInfo;
	FSF=*psInfo->FSF;
	Info.FSF=&FSF;

	bUseProxy=false;
	*strProxyName=0;
	*strProxyUser=0;
	*strProxyPass=0;

	InitPaths();
#ifndef UNICODE
	if(HIBYTE(LOWORD(Info.AdvControl(Info.ModuleNumber,ACTL_GETFARVERSION,NULL)))>1)
	{
		wrapper=true;
		return;
	}
#endif
	ReadSettings();
	InitializeCriticalSection(&cs);

	if(Mode)
	{
		hThread=CreateThread(NULL,0,ThreadProc,NULL,0,NULL);
	}
}

VOID WINAPI EXP_NAME(GetPluginInfo)(PluginInfo* pInfo)
{
#ifndef UNICODE
	if(wrapper)
		return;
#endif
	pInfo->StructSize=sizeof(PluginInfo);
	static LPCTSTR PluginMenuStrings[1],PluginConfigStrings[1];
	PluginMenuStrings[0]=MSG(MName);
	PluginConfigStrings[0]=MSG(MCfgName);
	pInfo->PluginMenuStrings=PluginMenuStrings;
	pInfo->PluginConfigStrings=PluginConfigStrings;
	pInfo->PluginMenuStringsNumber=ARRAYSIZE(PluginMenuStrings);
	pInfo->PluginConfigStringsNumber=ARRAYSIZE(PluginConfigStrings);
	pInfo->Flags=PF_EDITOR|PF_VIEWER|PF_DIALOG|PF_PRELOAD;
	static LPCTSTR CommandPrefix=TEXT("update");
	pInfo->CommandPrefix=CommandPrefix;
}

VOID WINAPI EXP_NAME(ExitFAR)()
{
#ifndef UNICODE
	if(wrapper)
	{
		return;
	}
#endif
	EnterCriticalSection(&cs);
	exitfar=true;
	LeaveCriticalSection(&cs);
	WaitForSingleObject(hThread,INFINITE);
	DeleteCriticalSection(&cs);
	if(hRunDll)
		CloseHandle(hRunDll);
}

HANDLE WINAPI EXP_NAME(OpenPlugin)(INT /*OpenFrom*/,INT_PTR /*item*/)
{
#ifndef UNICODE
	if(wrapper)
	{
		return INVALID_HANDLE_VALUE;
	}
#endif
	EnterCriticalSection(&cs);
	if(lock)
	{
		LeaveCriticalSection(&cs);
		return INVALID_HANDLE_VALUE;
	}
	else
	{
		lock=true;
		LeaveCriticalSection(&cs);
	}
	Console console;
	NeedRestart=false;
	switch(CheckUpdates())
	{
	case S_REQUIRED:
		{
			LPCTSTR Items[6];
			TCHAR Str[128];
			Items[0]=MSG(MName);
			Items[1]=MSG(MAvailableUpdates);
			Items[2]=TEXT("\x1");
			DWORD NewMajor,NewMinor,NewBuild;
			GetNewModuleVersion(NewMajor,NewMinor,NewBuild);
			FSF.sprintf(Str,TEXT("Far Manager (%d.%d build %d)"),NewMajor,NewMinor,NewBuild);
			Items[3]=Str;
			Items[4]=TEXT("\x1");
			Items[5]=MSG(MAsk);

			if(!Info.Message(Info.ModuleNumber,FMSG_MB_YESNO|FMSG_LEFTALIGN,NULL,Items,ARRAYSIZE(Items),2))
			{
				DownloadUpdates();
				StartUpdate();
			}
			else
			{
				TextColor color(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY);
				mprintf(MSG(MCancelled));
				Clean();
			}
			break;
		}
		break;

	case S_UPTODATE:
		{
			TextColor color(FOREGROUND_GREEN|FOREGROUND_INTENSITY);
			mprintf(TEXT("%s %s"),MSG(MSystem),MSG(MUpToDate));
			Clean();
		}
		break;

	default:
		{
			TextColor color(FOREGROUND_RED|FOREGROUND_INTENSITY);
			mprintf(MSG(MCantConnect));
		}
		break;
	}

	EnterCriticalSection(&cs);
	lock=false;
	SaveTime();
	LeaveCriticalSection(&cs);
	mprintf(TEXT("\n\n"));

	return INVALID_HANDLE_VALUE;
}

bool Extract(LPCTSTR lpArc,LPCTSTR lpPath,LPCTSTR lpDestDir)
{
	bool Result=false;
	if(!ipc.UseMsi)
	{
		SevenZipModuleManager ArchiveManager(lpArc);
		Result=ArchiveManager.Extract(lpPath,lpDestDir);
	}
	else
	{
		STARTUPINFO si={sizeof(si)};
		PROCESS_INFORMATION pi;
		TCHAR cmdline[MAX_PATH];
		wsprintf(cmdline,TEXT("msiexec.exe /promptrestart /qb /i \"%s\""),lpPath);
		if(CreateProcess(NULL,cmdline,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
		{
			WaitForSingleObject(pi.hProcess,INFINITE);
			DWORD ExitCode=0;
			if(GetExitCodeProcess(pi.hProcess,&ExitCode) && ExitCode==ERROR_SUCCESS)
			{
				Result=true;
			}
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}
	}
	return Result;
}

#ifdef UNICODE
INT WINAPI EXP_NAME(ProcessSynchroEvent)(INT Event,LPVOID Param)
{
	switch(Event)
	{
	case SE_COMMONSYNCHRO:
		{
			EventStruct* es=reinterpret_cast<EventStruct*>(Param);
			switch(es->Event)
			{
			case E_ASKLOAD:
				{
					TCHAR Str[128];
					DWORD NewMajor,NewMinor,NewBuild;
					GetNewModuleVersion(NewMajor,NewMinor,NewBuild);
					FSF.sprintf(Str,TEXT("Far Manager (%d.%d build %d)"),NewMajor,NewMinor,NewBuild);
					LPCTSTR Items[]={MSG(MName),MSG(MAvailableUpdates),TEXT("\x1"),Str,TEXT("\x1"),MSG(MAsk)};
					if(!Info.Message(Info.ModuleNumber,FMSG_MB_YESNO|FMSG_LEFTALIGN,NULL,Items,ARRAYSIZE(Items),2))
					{
						*es->Result=true;
					}
					SetEvent(reinterpret_cast<HANDLE>(es->Data));
				}
				break;
			case E_CONNECTFAIL:
				{
					LPCTSTR Items[]={MSG(MName),MSG(MCantConnect)};
					if(Info.Message(Info.ModuleNumber,FMSG_MB_RETRYCANCEL|FMSG_LEFTALIGN|FMSG_WARNING,NULL,Items,ARRAYSIZE(Items),2))
					{
						*es->Result=true;
					}
					SetEvent(reinterpret_cast<HANDLE>(es->Data));
				}
				break;
			case E_DOWNLOADED:
				{
					LPCWSTR Items[]={MSG(MName),MSG(MUpdatesDownloaded),MSG(MExitFAR)};
					Info.Message(Info.ModuleNumber,FMSG_MB_OK,NULL,Items,ARRAYSIZE(Items),0);
					SetEvent(reinterpret_cast<HANDLE>(es->Data));
				}
				break;
			}
		}
		break;
	}
	return 0;
}
#endif

EXTERN_C VOID WINAPI EXP_NAME(RestartFAR)(HWND,HINSTANCE,LPCTSTR lpCmd,DWORD)
{
	ifn.Load();
	INT argc=0;
	LPTSTR *argv=CommandLineToArgv(lpCmd,&argc);
	INT n=0;
	if(argc==2+n)
	{
		if(!ifn.AttachConsole(ATTACH_PARENT_PROCESS))
		{
			AllocConsole();
		}
		INT_PTR ptr=StringToNumber(argv[n+0],ptr);
		HANDLE hFar=static_cast<HANDLE>(reinterpret_cast<LPVOID>(ptr));
		if(hFar && hFar!=INVALID_HANDLE_VALUE)
		{
			HMENU hMenu=GetSystemMenu(GetConsoleWindow(),FALSE);
			INT Count=GetMenuItemCount(hMenu);
			INT Pos=-1;
			for(int i=0;i<Count;i++)
			{
				if(GetMenuItemID(hMenu,i)==SC_CLOSE)
				{
					Pos=i;
					break;
				}
			}
			MENUITEMINFO mi={sizeof(mi),MIIM_ID,0,0,0};
			if(Pos!=-1)
			{
				SetMenuItemInfo(hMenu,Pos,MF_BYPOSITION,&mi);
			}
			INT_PTR IPCPtr=StringToNumber(argv[n+1],IPCPtr);
			if(ReadProcessMemory(hFar,reinterpret_cast<LPCVOID>(IPCPtr),&ipc,sizeof(IPC),NULL))
			{
				WaitForSingleObject(hFar,INFINITE);

				CONSOLE_SCREEN_BUFFER_INFO csbi;
				GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&csbi);
				while(csbi.dwSize.Y--)
				{
					mprintf(TEXT("\n"));
				}
				CloseHandle(hFar);

				mprintf(TEXT("\n\n\n"));

				TCHAR destpath[MAX_PATH];
				lstrcpy(destpath,ipc.FarDirectory);

				TCHAR arc[MAX_PATH];
				GetPrivateProfileString(TEXT("far"),ipc.UseMsi?TEXT("msi"):TEXT("arc"),TEXT(""),arc,ARRAYSIZE(arc),ipc.UpdateList);
				if(*arc)
				{
					TCHAR local_arc[MAX_PATH];
					lstrcpy(local_arc,ipc.TempDirectory);
					lstrcat(local_arc,arc);
					if(GetFileAttributes(local_arc)!=INVALID_FILE_ATTRIBUTES)
					{
						bool Result=false;
						while(!Result)
						{
							mprintf(TEXT("Unpacking %-50s"),arc);
							if(!Extract(ipc.SevenZip,local_arc,destpath))
							{
								{
									TextColor color(FOREGROUND_RED|FOREGROUND_INTENSITY);
									mprintf(TEXT("\nUnpack error. Retry? (Y/N) "));
								}
								INPUT_RECORD ir={0};
								DWORD n;
								while(!(ir.EventType==KEY_EVENT && !ir.Event.KeyEvent.bKeyDown && (ir.Event.KeyEvent.wVirtualKeyCode==L'Y'||ir.Event.KeyEvent.wVirtualKeyCode==L'N')))
								{
									ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE),&ir,1,&n);
									Sleep(1);
								}
								if(ir.Event.KeyEvent.wVirtualKeyCode==L'N')
								{
									mprintf(TEXT("\n"));
									break;
								}
								mprintf(TEXT("\n"));
							}
							else
							{
								Result=true;
							}
						}
						if(Result)
						{
							TextColor color(FOREGROUND_GREEN|FOREGROUND_INTENSITY);
							mprintf(TEXT("OK\n"));
							if(GetPrivateProfileInt(TEXT("Update"),TEXT("Delete"),1,ipc.Config))
							{
								DeleteFile(local_arc);
							}
						}
						else
						{
							TextColor color(FOREGROUND_RED|FOREGROUND_INTENSITY);
							mprintf(TEXT("error\n"));
						}
					}
				}
				TCHAR exec[2048],execExp[4096];
				GetPrivateProfileString(TEXT("events"),TEXT("PostInstall"),TEXT(""),exec,ARRAYSIZE(exec),ipc.Config);
				if(*exec)
				{
					ExpandEnvironmentStrings(exec,execExp,ARRAYSIZE(execExp));
					STARTUPINFO si={sizeof(si)};
					PROCESS_INFORMATION pi;
					{
						TextColor color(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
						mprintf(TEXT("\nExecuting %-50.50s"),execExp);
					}
					if(CreateProcess(NULL,execExp,NULL,NULL,TRUE,NULL,NULL,ipc.PluginDirectory,&si,&pi))
					{
						{
							TextColor color(FOREGROUND_GREEN|FOREGROUND_INTENSITY);
							mprintf(TEXT("OK\n\n"));
						}
						WaitForSingleObject(pi.hProcess,INFINITE);
					}
					else
					{
						TextColor color(FOREGROUND_RED|FOREGROUND_INTENSITY);
						mprintf(TEXT("Error %d"),GetLastError());
					}
					mprintf(TEXT("\n"));
					CloseHandle(pi.hThread);
					CloseHandle(pi.hProcess);
				}
				if(Pos!=-1)
				{
					mi.wID=SC_CLOSE;
					SetMenuItemInfo(hMenu,Pos,MF_BYPOSITION,&mi);
					DrawMenuBar(GetConsoleWindow());
				}
				TextColor color(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
				mprintf(TEXT("\n%-60s"),TEXT("Starting FAR..."));
				STARTUPINFO si={sizeof(si)};
				PROCESS_INFORMATION pi;
				TCHAR FarCmd[2048];
				lstrcpy(FarCmd,ipc.FarModule);
				lstrcat(FarCmd,TEXT(" "));
				lstrcat(FarCmd,ipc.FarParams);
				if(CreateProcess(NULL,FarCmd,NULL,NULL,TRUE,NULL,NULL,NULL,&si,&pi))
				{
					TextColor color(FOREGROUND_GREEN|FOREGROUND_INTENSITY);
					mprintf(TEXT("OK"));
				}
				else
				{
					TextColor color(FOREGROUND_RED|FOREGROUND_INTENSITY);
					mprintf(TEXT("Error %d"),GetLastError());
				}
				mprintf(TEXT("\n"));

				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
			}
		}
	}
	LocalFree(argv);
	Clean();
}
