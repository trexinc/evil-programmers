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

TCHAR update_list[MAX_PATH];
TCHAR basepath[MAX_PATH];
TCHAR tmpdir[MAX_PATH];
TCHAR config[MAX_PATH];

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
bool Force=false;

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

bool GetCurrentModuleVersion(LPCTSTR Module,VerInfo &vi)
{
	bool ret=false;
	DWORD dwHandle;
	DWORD dwSize=GetFileVersionInfoSize(Module,&dwHandle);
	if(dwSize)
	{
		LPVOID Data=malloc(dwSize);
		if(Data)
		{
			if(GetFileVersionInfo(Module,NULL,dwSize,Data))
			{
				VS_FIXEDFILEINFO *ffi;
				UINT Len;
				LPVOID lplpBuffer;
				if(VerQueryValue(Data,TEXT("\\"),&lplpBuffer,&Len))
				{
					ffi=static_cast<VS_FIXEDFILEINFO*>(lplpBuffer);
					if(ffi->dwFileType==VFT_APP || ffi->dwFileType==VFT_DLL)
					{
						vi.Version=LOBYTE(HIWORD(ffi->dwFileVersionMS));
						vi.SubVersion=LOBYTE(LOWORD(ffi->dwFileVersionMS));
						vi.Build=LOWORD(ffi->dwFileVersionLS);
						vi.FarBuild=LOWORD(ffi->dwProductVersionLS);
						ret=true;
					}
				}
			}
			free(Data);
		}
	}
	if(!ret && Force)
	{
		vi.Version=0;
		vi.SubVersion=0;
		vi.Build=0;
		vi.FarBuild=0;
		ret=true;
	}	
	return ret;
}

bool GetNewModuleVersion(LPCTSTR Name,VerInfo &vi)
{
	vi.Version=GetPrivateProfileInt(Name,TEXT("major"),-1,update_list);
	vi.SubVersion=GetPrivateProfileInt(Name,TEXT("minor"),-1,update_list);
	vi.Build=GetPrivateProfileInt(Name,TEXT("build"),-1,update_list);
	vi.FarBuild=GetPrivateProfileInt(Name,TEXT("farbuild"),vi.Build,update_list);
	return vi.Version!=-1 && vi.SubVersion!=-1 && vi.Build!=-1 && vi.FarBuild!=-1;
}

bool NeedUpdate(VerInfo &vi_old,VerInfo &vi_new)
{                                                         
	return (vi_new.Version>vi_old.Version) || 
	((vi_new.Version==vi_old.Version)&&(vi_new.SubVersion>vi_old.SubVersion))||
	((vi_new.Version==vi_old.Version)&&(vi_new.SubVersion==vi_old.SubVersion)&&((vi_new.Build>vi_old.Build)||(vi_new.FarBuild>vi_old.FarBuild)));
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
	FSF.sprintf(LocalFile,TEXT("%s\\%s"),tmpdir,LocalName?LocalName:FSF.PointToName(RemoteFile));

	TCHAR strReq[512];
	FSF.sprintf(strReq, TEXT("%s/%s"),RemotePath,RemoteFile);

	return WinInetDownload(RemoteSrv,RemoteFile,LocalFile,UseCallBack?DownloadProc:NULL)==0;
}

DWORD CheckUpdates(DWORD *Mask=NULL)
{
	DWORD Ret=S_UPTODATE;
	CreateDirectory(tmpdir,NULL);
	TCHAR URL[1024];
	FSF.sprintf(URL,TEXT("%s/%s%s"),RemotePath,phpFile,phpRequest);
	if(DownloadFile(URL,phpFile))
	{
		TCHAR sVer[MAX_PATH];
		GetPrivateProfileString(TEXT("info"),TEXT("version"),TEXT(""),sVer,ARRAYSIZE(sVer),update_list);
		if(lstrcmp(sVer, strVer))
		{
			return S_CANTCONNECT;
		}
		for(size_t i=0;i<ARRAYSIZE(Modules);i++)
		{
			VerInfo vi_old,vi_new;
			TCHAR RealName[MAX_PATH];
			BOOL bEmpty = !Modules[i].Path || lstrlen(Modules[i].Path) == 0;
			FSF.sprintf(RealName,TEXT("%s\\%s%s%s"),basepath,Modules[i].Path, bEmpty?TEXT(""):TEXT("\\"), Modules[i].Module);
			if(GetCurrentModuleVersion(RealName,vi_old))
			{
				if(GetNewModuleVersion(Modules[i].DisplayName,vi_new))
				{
					if(NeedUpdate(vi_old,vi_new))
					{
						Ret=S_REQUIRED;
						if(Mask)
						{
							*Mask|=(1<<i);
						}
						else
						{
							break;
						}
					}
				}
			}
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
	for(size_t i=0;i<ARRAYSIZE(Modules);i++)
	{
		VerInfo vi_old,vi_new;
		TCHAR RealName[MAX_PATH];
		BOOL bEmpty = !Modules[i].Path || !*Modules[i].Path;
		FSF.sprintf(RealName,TEXT("%s\\%s%s%s"),basepath,Modules[i].Path, bEmpty?TEXT(""):TEXT("\\"), Modules[i].Module);
		if(GetCurrentModuleVersion(RealName,vi_old))
		{
			if(GetNewModuleVersion(Modules[i].DisplayName,vi_new))
			{
				if(NeedUpdate(vi_old,vi_new))
				{
					TCHAR URL[1024],arc[MAX_PATH];
					GetPrivateProfileString(Modules[i].DisplayName,TEXT("arc"),TEXT(""),arc,ARRAYSIZE(arc),update_list);
					FSF.sprintf(URL,TEXT("%s/%s"),RemotePath,arc);
					if(!Silent)
						mprintf(TEXT("%s %-50s"),MSG(MLoad),Modules[i].DisplayName);
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
				}
				else
				{
					if(!Silent)
					{
						mprintf(TEXT("%s %s"),Modules[i].DisplayName,MSG(MUpToDate));
					}
				}
			}
			else
			{
				if(!Silent)
				{
					TextColor color(FOREGROUND_RED|FOREGROUND_INTENSITY);
					mprintf(TEXT("%s %s"),MSG(MCantFindInfo),Modules[i].DisplayName);
				}
			}
			if(!Silent)
			{
				mprintf(TEXT("\n"));
			}
		}
	}
	return true;
}

bool Clean()
{
	DeleteFile(update_list);
	RemoveDirectory(tmpdir);
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

		LPTSTR cmdline=new TCHAR[32*1024];

		TCHAR FarExe[MAX_PATH];
		GetModuleFileName(NULL,FarExe,ARRAYSIZE(FarExe));
		size_t NumArgs=0;
		LPTSTR *Argv=CommandLineToArgv(GetCommandLine(),reinterpret_cast<PINT>(&NumArgs));
		TCHAR Params[MAX_PATH*4]={0};
		for(size_t i=1;i<NumArgs;i++)
		{
			lstrcat(Params,Argv[i]);
			if(i<NumArgs-1)
				lstrcat(Params,TEXT(" "));
		}
		LocalFree(Argv);
		TCHAR DllPath[MAX_PATH];
		lstrcpy(DllPath,Info.ModuleName);
		lstrcpy(const_cast<LPTSTR>(FSF.PointToName(DllPath)),TEXT("7zxr.dll"));

		FSF.sprintf(cmdline,TEXT("rundll32 \"%s\", RestartFAR %I64d %d \"%s\" \"%s\" \"%s\" \"%s\""),Info.ModuleName,reinterpret_cast<INT64>(ProcDup),!Silent,FarExe,Params,basepath,DllPath);

		STARTUPINFO si={sizeof(si)};
		PROCESS_INFORMATION pi;

		BOOL Created=CreateProcess(NULL,cmdline,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);

		delete[] cmdline;
		if(Created)
		{
			hRunDll=pi.hProcess;
			CloseHandle(pi.hThread);
			if(!Silent)
			{
				TextColor color(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY);
				mprintf(TEXT("\n%s\n"),MSG(MExitFAR));
			}
			EnterCriticalSection(&cs);
			SaveTime();
			exitfar=true;
			LeaveCriticalSection(&cs);
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
			DWORD Mask=0;
			switch(CheckUpdates(&Mask))
			{
			case S_REQUIRED:
				{
					TCHAR Msg[2048];
					lstrcpy(Msg,MSG(MAvailableUpdates));
#ifndef UNICODE
					OemToCharBuff(Msg,Msg,lstrlen(Msg));
#endif
					lstrcat(Msg,TEXT("\n\n"));
					for(size_t i=0;i<ARRAYSIZE(Modules);i++)
					{
						if(Mask&(1<<i))
						{
							lstrcat(Msg,Modules[i].DisplayName);
							VerInfo vi;
							if(GetNewModuleVersion(Modules[i].DisplayName,vi))
							{
								TCHAR Version[MAX_PATH];
								wsprintf(Version,TEXT(" (%d.%d build %d)"),vi.Version,vi.SubVersion,vi.Build);
								lstrcat(Msg,Version);
							}
							lstrcat(Msg,TEXT("\n"));
						}
					}
					lstrcat(Msg,TEXT("\n"));
#ifndef UNICODE
					TCHAR Tmp[1024];
					lstrcpy(Tmp,MSG(MAsk2));
					OemToCharBuff(Tmp,Tmp,lstrlen(Tmp));
					lstrcat(Msg,Tmp);
#else
					lstrcat(Msg,MSG(MAsk2));
#endif
					EnterCriticalSection(&cs);
					SaveTime();
					lock=true;
					LeaveCriticalSection(&cs);
					Info.Message(Info.ModuleNumber,FMSG_MB_YESNO|FMSG_LEFTALIGN,TEXT("iddqd"),NULL,1,2);
					bool Load=(Mode==2)||MessageBox(NULL,Msg,TEXT("FAR Update"),MB_ICONINFORMATION|MB_YESNO)==IDYES;
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
#else
					LPCWSTR Msg=MSG(MCantConnect);
#endif
					if(MessageBox(NULL,Msg,TEXT("FAR Update"),MB_ICONERROR|MB_RETRYCANCEL)==IDCANCEL)
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
VOID InitPaths(LPCTSTR base)
{
	wsprintf(tmpdir,TEXT("%s\\FUTMP"),base);
	wsprintf(update_list,TEXT("%s\\%s"),tmpdir,phpFile);
	TCHAR ModuleName[MAX_PATH];
	if(GetModuleFileName(reinterpret_cast<HINSTANCE>(&__ImageBase),ModuleName,ARRAYSIZE(ModuleName)))
		wsprintf(config,TEXT("%s.config"),ModuleName);
}

VOID ReadUserPaths(LPCTSTR cfgfile)
{
	static TCHAR UserPaths[ARRAYSIZE(Modules)][MAX_PATH];
	for(size_t i=1;i<ARRAYSIZE(Modules);i++)
	{
		GetPrivateProfileString(TEXT("paths"),Modules[i].DisplayName,Modules[i].Path,UserPaths[i],ARRAYSIZE(UserPaths[i]),cfgfile);
		Modules[i].Path=UserPaths[i];
	}
}
VOID ReadConnectSettings(LPCTSTR cfgfile)
{
	bUseProxy = GetPrivateProfileInt(TEXT("connect"),TEXT("proxy"), 0, cfgfile) == 1;
	GetPrivateProfileString(TEXT("connect"),TEXT("srv"),TEXT(""),strProxyName,ARRAYSIZE(strProxyName),cfgfile);
	GetPrivateProfileString(TEXT("connect"),TEXT("user"), TEXT(""),strProxyUser,ARRAYSIZE(strProxyUser),cfgfile);
	GetPrivateProfileString(TEXT("connect"),TEXT("pass"), TEXT(""),strProxyPass,ARRAYSIZE(strProxyPass),cfgfile);
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

	GetModuleFileName(NULL,basepath,ARRAYSIZE(basepath));
	*StrRChr(basepath,NULL,TEXT('\\'))=0;
	InitPaths(basepath);
#ifndef UNICODE
	VerInfo vi;
	TCHAR RealName[MAX_PATH];
	FSF.sprintf(RealName,TEXT("%s\\%s\\%s"),basepath,Modules[0].Path,Modules[0].Module);
	GetCurrentModuleVersion(RealName,vi);
	if(vi.Version>1)
	{
		wrapper=true;
		return;
	}
#endif
	ReadConnectSettings(config);
	ReadUserPaths(config);
	InitializeCriticalSection(&cs);

	Force=(GetPrivateProfileInt(TEXT("update"),TEXT("Force"),1,config)!=0);
	Mode=GetPrivateProfileInt(TEXT("update"),TEXT("Mode"),2,config);
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
	DWORD Mask=0;
	switch(CheckUpdates(&Mask))
	{
	case S_REQUIRED:
		{
			LPCTSTR Items[5+ARRAYSIZE(Modules)];
			TCHAR Str[ARRAYSIZE(Modules)][128];
			Items[0]=MSG(MName);
			Items[1]=MSG(MAvailableUpdates);
			Items[2]=TEXT("\x1");
			size_t count=3;
			for(size_t i=0;i<ARRAYSIZE(Modules);i++)
			{
				if(Mask&(1<<i))
				{
					VerInfo vi;
					if(GetNewModuleVersion(Modules[i].DisplayName,vi))
					{
						FSF.sprintf(Str[i],TEXT("%s (%d.%d build %d)"),Modules[i].DisplayName,vi.Version,vi.SubVersion,vi.Build);
						Items[count++]=Str[i];
					}
					else
					{
						Items[count++]=Modules[i].DisplayName;
					}
				}
			}
			Items[count++]=TEXT("\x1");
			Items[count++]=MSG(MAsk);

			if(!Info.Message(Info.ModuleNumber,FMSG_MB_YESNO|FMSG_LEFTALIGN,NULL,Items,(int)count,2))
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
	SevenZipModuleManager ArchiveManager(lpArc);
	return ArchiveManager.Extract(lpPath,lpDestDir);
}

EXTERN_C VOID WINAPI EXP_NAME(RestartFAR)(HWND,HINSTANCE,LPCTSTR lpCmd,DWORD)
{
	ifn.Load();
	INT argc=0;
	LPTSTR *argv=CommandLineToArgv(lpCmd,&argc);
	INT n=0;
	if(argc==6+n)
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
			int Silent=StringToNumber(argv[n+1],Silent);
			if(!Silent)
			{
				MessageBox(NULL,TEXT("Updates are downloaded.\n\nExit from FAR to continue."),TEXT("Update"),MB_ICONINFORMATION);
			}
			WaitForSingleObject(hFar,INFINITE);

			CONSOLE_SCREEN_BUFFER_INFO csbi;
			GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&csbi);
			while(csbi.dwSize.Y--)
			{
				mprintf(TEXT("\n"));
			}
			CloseHandle(hFar);

			mprintf(TEXT("\n\n\n"));

			InitPaths(argv[n+4]);
			ReadUserPaths(config);
			for(size_t i=0;i<ARRAYSIZE(Modules);i++)
			{
				TCHAR destpath[MAX_PATH];
				wsprintf(destpath,TEXT("%s\\%s"),argv[n+4],Modules[i].Path);
				if (destpath[lstrlen(destpath)-1]!=TEXT('\\'))
				{
					lstrcat(destpath,TEXT("\\"));
				}

				TCHAR arc[MAX_PATH];
				GetPrivateProfileString(Modules[i].DisplayName,TEXT("arc"),TEXT(""),arc,ARRAYSIZE(arc),update_list);
				if(*arc)
				{
					TCHAR local_arc[MAX_PATH];
					wsprintf(local_arc,TEXT("%s\\%s"),tmpdir,arc);
					if(GetFileAttributes(local_arc)!=INVALID_FILE_ATTRIBUTES)
					{
						mprintf(TEXT("Unpacking %-50s"),arc);
						bool Result=false;
						while(!Result)
						{
							if(!Extract(argv[n+5],local_arc,destpath))
							{
								if(MessageBox(NULL,TEXT("unpack error"),TEXT("FAR Update"),MB_ICONERROR|MB_RETRYCANCEL)==IDCANCEL)
								{
									break;
								}
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
							DeleteFile(local_arc);
						}
						else
						{
							TextColor color(FOREGROUND_RED|FOREGROUND_INTENSITY);
							mprintf(TEXT("error\n"));
						}
					}
				}
			}
			TCHAR exec[2048];
			GetPrivateProfileString(TEXT("events"),TEXT("PostInstall"),TEXT(""),exec,ARRAYSIZE(exec),config);
			if(*exec)
			{
				STARTUPINFO si={sizeof(si)};
				PROCESS_INFORMATION pi;
				{
					TextColor color(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
					mprintf(TEXT("\nExecuting %-50.50s"),exec);
				}
				if(CreateProcess(NULL,exec,NULL,NULL,TRUE,NULL,NULL,NULL,&si,&pi))
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
			mprintf(TEXT("\n%-60s"),TEXT("Starting FAR again..."));
			STARTUPINFO si={sizeof(si)};
			PROCESS_INFORMATION pi;
			TCHAR FarCmd[2048];
			lstrcpy(FarCmd,argv[n+2]);
			lstrcat(FarCmd,TEXT(" "));
			lstrcat(FarCmd,argv[n+3]);
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
			
			// шаманство для x64
			DWORD ProcList[2];
			for(;;)
			{
				DWORD ProcCount=ifn.GetConsoleProcessList(ProcList,2);
				if(Count==1)
				{
					Sleep(1);
				}
				else
				{
					break;
				}
			}
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}
	}
	LocalFree(argv);
	Clean();
}
