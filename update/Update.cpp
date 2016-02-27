#include "headers.hpp"

#include "Underscore.Archives.h"

#include <initguid.h>
#include "guid.hpp"
#include "Imports.hpp"

#include "update.hpp"
#include "lng.hpp"
#include "ver.hpp"


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
	void* Data;
	bool Self;
	bool *Result;
};

struct IPC
{
	wchar_t FarModule[MAX_PATH];
	wchar_t FarDirectory[MAX_PATH];
	wchar_t PluginModule[MAX_PATH];
	wchar_t PluginDirectory[MAX_PATH];
	wchar_t FarParams[MAX_PATH * 4];
	wchar_t TempDirectory[MAX_PATH];
	wchar_t Config[MAX_PATH];
	wchar_t SelfUpdateList[MAX_PATH];
	wchar_t FarUpdateList[MAX_PATH];
	wchar_t SevenZip[MAX_PATH];
	bool UseMsi;
	bool Self;
} ipc;

const wchar_t strVer[] = L"1.0";
const wchar_t Directory[] = L"FUTMP";

const wchar_t SelfRemoteSrv[] = L"www.idkfa.googlecode.com";
const wchar_t SelfRemotePath[] = L"/files";

const wchar_t FarRemoteSrv[] = L"www.farmanager.com";
const wchar_t FarRemotePath[] = L"/nightly";

const wchar_t FarUpdateFile[] = L"update3.php";

const wchar_t SelfUpdateFile[] = L"update.txt";

#ifdef _WIN64
#define PLATFORM_STR L"64"
#else
#define PLATFORM_STR L"32"
#endif

const wchar_t phpRequest[] = L"?p=" PLATFORM_STR;
const wchar_t SelfSection[] = L"UpdateW" PLATFORM_STR;

const wchar_t* FarSection = L"far";

bool bUseProxy;
wchar_t strProxyName[512];
wchar_t strProxyUser[512];
wchar_t strProxyPass[512];

bool NeedRestart = false;

SYSTEMTIME SavedTime;

HANDLE hThread = nullptr;

HANDLE hRunDll = nullptr;

HANDLE StopEvent = nullptr;
HANDLE UnlockEvent = nullptr;

CRITICAL_SECTION cs;

PluginStartupInfo Info;
FarStandardFunctions FSF;

DWORD Mode = 0;

int mprintf(const wchar_t* format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	wchar_t buff[1024];
	DWORD n = wvsprintf(buff, format, argptr);
	va_end(argptr);
	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), buff, n, &n, nullptr);
	return n;
}

template<class T>
T StringToNumber(const wchar_t* String)
{
	T Number = 0;
	for (const wchar_t* p = String; p&&*p; p++)
		if (*p >= L'0'&&*p <= L'9')
			Number = Number * 10 + (*p - L'0');
	return Number;
}

void GetNewModuleVersion(bool Self, wchar_t* Str, DWORD& NewMajor, DWORD& NewMinor, DWORD& NewBuild)
{
	const wchar_t* UpdateList = Self ? ipc.SelfUpdateList : ipc.FarUpdateList;
	const wchar_t* Section = Self ? SelfSection : FarSection;
	NewMajor = GetPrivateProfileInt(Section, L"major", -1, UpdateList);
	NewMinor = GetPrivateProfileInt(Section, L"minor", -1, UpdateList);
	NewBuild = GetPrivateProfileInt(Section, L"build", -1, UpdateList);
	if (Str)
	{
		FSF.sprintf(Str, Self ? L"Update (%d.%d build %d)" : L"Far Manager (%d.%d build %d)", NewMajor, NewMinor, NewBuild);
	}
}

bool NeedUpdate(bool Self)
{
	VersionInfo FarVersion = {};
	if (Self)
	{
		FarVersion.Major = MAJOR_VER;
		FarVersion.Minor = MINOR_VER;
		FarVersion.Build = BUILD;
	}
	else
	{
		Info.AdvControl(&MainGuid, ACTL_GETFARMANAGERVERSION, 0, &FarVersion);
	}

	DWORD NewMajor, NewMinor, NewBuild;
	GetNewModuleVersion(Self, nullptr, NewMajor, NewMinor, NewBuild);

	return (NewMajor > FarVersion.Major) ||
		((NewMajor == FarVersion.Major) && (NewMinor > FarVersion.Minor)) ||
		((NewMajor == FarVersion.Major) && (NewMinor == FarVersion.Minor) && (NewBuild > FarVersion.Build));
}

typedef BOOL(CALLBACK* DOWNLOADPROC)(DWORD);

BOOL CALLBACK DownloadProc(DWORD Percent)
{
	CursorPos cp;
	if (Percent)
	{
		mprintf(L"%d%%", Percent);
	}
	else
	{
		mprintf(L"    ");
	}
	return TRUE;
}

DWORD WINAPI WinInetDownload(const wchar_t* strSrv, const wchar_t* strURL, const wchar_t* strFile, DOWNLOADPROC Proc = nullptr)
{
	DWORD err = 0;

	DWORD ProxyType = INTERNET_OPEN_TYPE_DIRECT;
	if (bUseProxy)
		ProxyType = *strProxyName ? INTERNET_OPEN_TYPE_PROXY : INTERNET_OPEN_TYPE_PRECONFIG;

	HINTERNET hInternet = InternetOpen(L"Mozilla/5.0 (compatible; FAR Update)", ProxyType, strProxyName, nullptr, 0);
	if (hInternet)
	{
		BYTE Data[2048];
		DWORD dwBytesRead;
		BOOL bRead = FALSE;

		HINTERNET hConnect = InternetConnect(hInternet, strSrv, INTERNET_DEFAULT_HTTP_PORT, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 1);
		if (hConnect)
		{
			if (bUseProxy && *strProxyName)
			{
				if (*strProxyUser)
				{
					if (!InternetSetOption(hConnect, INTERNET_OPTION_PROXY_USERNAME, (LPVOID)strProxyUser, lstrlen(strProxyUser)))
					{
						err = GetLastError();
					}
				}
				if (*strProxyPass)
				{
					if (!InternetSetOption(hConnect, INTERNET_OPTION_PROXY_PASSWORD, (LPVOID)strProxyPass, lstrlen(strProxyPass)))
					{
						err = GetLastError();
					}
				}
			}
			HTTP_VERSION_INFO httpver = { 1, 1 };
			if (!InternetSetOption(hConnect, INTERNET_OPTION_HTTP_VERSION, &httpver, sizeof(httpver)))
			{
				err = GetLastError();
			}

			HINTERNET hRequest = HttpOpenRequest(hConnect, L"GET", strURL, L"HTTP/1.1", nullptr, 0, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 1);

			if (hRequest)
			{
				if (HttpSendRequest(hRequest, nullptr, 0, nullptr, 0))
				{
					HANDLE hFile = CreateFile(strFile, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
					if (hFile != INVALID_HANDLE_VALUE)
					{
						DWORD Size = 0;
						DWORD sz = sizeof(Size);
						if (!HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &Size, &sz, nullptr))
						{
							err = GetLastError();
						}
						if (Size != GetFileSize(hFile, nullptr))
						{
							SetEndOfFile(hFile);
							UINT BytesDone = 0;
							HideCursor hc;
							while ((bRead = InternetReadFile(hRequest, Data, sizeof(Data), &dwBytesRead)) != 0 && dwBytesRead)
							{
								BytesDone += dwBytesRead;
								if (Proc && Size)
								{
									Proc(BytesDone * 100 / Size);
								}
								DWORD dwWritten = 0;
								if (!WriteFile(hFile, Data, dwBytesRead, &dwWritten, nullptr))
								{
									err = GetLastError();
									break;
								}
							}
						}
						if (Proc)
						{
							Proc(0);
						}
						CloseHandle(hFile);
					}
					else
					{
						err = GetLastError();
					}
				}
				else
				{
					err = GetLastError();
				}
				InternetCloseHandle(hRequest);
			}
			InternetCloseHandle(hConnect);
		}
		InternetCloseHandle(hInternet);
	}
	return err;
}

bool DownloadFile(bool Self, const wchar_t* RemoteFile, const wchar_t* LocalName = nullptr, bool UseCallBack = false)
{
	wchar_t LocalFile[MAX_PATH];
	lstrcpy(LocalFile, ipc.TempDirectory);
	lstrcat(LocalFile, LocalName ? LocalName : FSF.PointToName(RemoteFile));
	return WinInetDownload(Self ? SelfRemoteSrv : FarRemoteSrv, RemoteFile, LocalFile, UseCallBack ? DownloadProc : nullptr) == 0;
}

DWORD CheckUpdates(bool Self)
{
	DWORD Ret = S_UPTODATE;
	CreateDirectory(ipc.TempDirectory, nullptr);
	wchar_t URL[1024];
	if (Self)
	{
		lstrcpy(URL, SelfRemotePath);
		lstrcat(URL, L"/");
		lstrcat(URL, SelfUpdateFile);
	}
	else
	{
		lstrcpy(URL, FarRemotePath);
		lstrcat(URL, L"/");
		lstrcat(URL, FarUpdateFile);
		lstrcat(URL, phpRequest);
	}
	if (DownloadFile(Self, URL, Self ? SelfUpdateFile : FarUpdateFile))
	{
		wchar_t sVer[MAX_PATH];
		GetPrivateProfileString(L"info", L"version", L"", sVer, ARRAYSIZE(sVer), Self ? ipc.SelfUpdateList : ipc.FarUpdateList);
		if (lstrcmp(sVer, strVer))
		{
			return S_CANTCONNECT;
		}
		if (NeedUpdate(Self))
		{
			Ret = S_REQUIRED;
		}
	}
	else
	{
		Ret = S_CANTCONNECT;
	}
	return Ret;
}

bool DownloadUpdates(bool Self, bool Silent)
{
	wchar_t URL[1024], arc[MAX_PATH];
	if (Self)
	{
		GetPrivateProfileString(SelfSection, L"arc", L"", arc, ARRAYSIZE(arc), ipc.SelfUpdateList);
	}
	else
	{
		GetPrivateProfileString(FarSection, ipc.UseMsi ? L"msi" : L"arc", L"", arc, ARRAYSIZE(arc), ipc.FarUpdateList);
	}
	FSF.sprintf(URL, L"%s/%s", Self ? SelfRemotePath : FarRemotePath, arc);
	if (!Silent)
	{
		mprintf(L"%s %-50s", MSG(MLoad), Self ? L"Update" : L"Far");
	}
	if (DownloadFile(Self, URL, nullptr, !Silent))
	{
		if (!Silent)
		{
			TextColor color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			mprintf(L"OK");
		}
		NeedRestart = true;
	}
	else
	{
		if (!Silent)
		{
			TextColor color(FOREGROUND_RED | FOREGROUND_INTENSITY);
			mprintf(L"download error %d", GetLastError());
		}
	}
	if (!Silent)
	{
		mprintf(L"\n");
	}
	return true;
}

bool Clean()
{
	DeleteFile(ipc.SelfUpdateList);
	DeleteFile(ipc.FarUpdateList);
	RemoveDirectory(ipc.TempDirectory);
	return true;
}

bool IsTime()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	EnterCriticalSection(&cs);
	bool Result = st.wYear != SavedTime.wYear || st.wMonth != SavedTime.wMonth || st.wDay != SavedTime.wDay;
	LeaveCriticalSection(&cs);
	return Result;
}

void SaveTime()
{
	EnterCriticalSection(&cs);
	GetLocalTime(&SavedTime);
	LeaveCriticalSection(&cs);
}

void StartUpdate(bool Self, bool Silent)
{
	DWORD RunDllExitCode = 0;
	GetExitCodeProcess(hRunDll, &RunDllExitCode);
	if (RunDllExitCode == STILL_ACTIVE)
	{
		if (!Silent)
		{
			TextColor color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			mprintf(L"\n%s\n", MSG(MExitFAR));
		}
	}
	else if (NeedRestart)
	{
		HANDLE ProcDup;
		DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(), GetCurrentProcess(), &ProcDup, 0, TRUE, DUPLICATE_SAME_ACCESS);

		wchar_t cmdline[MAX_PATH];

		int NumArgs = 0;
		auto Argv = CommandLineToArgvW(GetCommandLine(), &NumArgs);
		*ipc.FarParams = 0;
		for (int i = 1; i < NumArgs; i++)
		{
			lstrcat(ipc.FarParams, Argv[i]);
			if (i < NumArgs - 1)
				lstrcat(ipc.FarParams, L" ");
		}
		LocalFree(Argv);

		wchar_t WinDir[MAX_PATH];
		GetWindowsDirectory(WinDir, ARRAYSIZE(WinDir));
		BOOL IsWow64 = FALSE;
		ipc.Self = Self;
		FSF.sprintf(cmdline, L"%s\\%s\\rundll32.exe \"%s\", RestartFAR %I64d %I64d", WinDir, ifn.IsWow64Process(GetCurrentProcess(), &IsWow64) && IsWow64 ? L"SysWOW64" : L"System32", ipc.PluginModule, reinterpret_cast<INT64>(ProcDup), reinterpret_cast<INT64>(&ipc));

		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;

		BOOL Created = CreateProcess(nullptr, cmdline, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi);

		if (Created)
		{
			hRunDll = pi.hProcess;
			CloseHandle(pi.hThread);
			if (!Silent)
			{
				TextColor color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
				mprintf(L"\n%s\n", MSG(MExitFAR));
			}
			else
			{
				HANDLE hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
				EventStruct es = { E_DOWNLOADED, hEvent, Self };
				Info.AdvControl(&MainGuid, ACTL_SYNCHRO, 0, &es);
				WaitForSingleObject(hEvent, INFINITE);
				CloseHandle(hEvent);
			}
			SaveTime();
		}
		else
		{
			if (!Silent)
			{
				TextColor color(FOREGROUND_RED | FOREGROUND_INTENSITY);
				mprintf(L"%s - error %d", MSG(MCantCreateProcess), GetLastError());
			}
		}

	}
	else
	{
		if (!Silent)
		{
			TextColor color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			mprintf(MSG(MDone));
		}
	}
}

DWORD WINAPI ThreadProc(void* /*lpParameter*/)
{
	while (WaitForSingleObject(StopEvent, 0) != WAIT_OBJECT_0)
	{
		WaitForSingleObject(UnlockEvent, INFINITE);
		bool Time = false;

		Time = IsTime();

		if (Time)
		{
			//for(int i=0;i<2;i++)
			int i = 1;
			{
				switch (CheckUpdates(!i))
				{
				case S_REQUIRED:
				{
					ResetEvent(UnlockEvent);
					SaveTime();
					bool Load = (Mode == 2);
					if (!Load)
					{
						HANDLE hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
						EventStruct es = { E_ASKLOAD, hEvent, !i, &Load };
						Info.AdvControl(&MainGuid, ACTL_SYNCHRO, 0, &es);
						WaitForSingleObject(hEvent, INFINITE);
						CloseHandle(hEvent);
					}
					if (Load)
					{
						if (DownloadUpdates(!i, true))
						{
							StartUpdate(!i, true);
						}
					}
					else
					{
						Clean();
					}
					SetEvent(UnlockEvent);
				}
				break;
				case S_CANTCONNECT:
				{
					HANDLE hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
					bool Cancel = false;
					EventStruct es = { E_CONNECTFAIL, hEvent, !i, &Cancel };
					Info.AdvControl(&MainGuid, ACTL_SYNCHRO, 0, &es);
					WaitForSingleObject(hEvent, INFINITE);
					CloseHandle(hEvent);
					if (Cancel)
					{
						SaveTime();
					}
					Clean();
				}
				break;
				case S_UPTODATE:
				{
					SaveTime();
					Clean();
				}
				break;
				}
			}
		}
		Sleep(1000);
	}
	return 0;
}

void InitPaths()
{
	GetModuleFileName(nullptr, ipc.FarModule, ARRAYSIZE(ipc.FarModule));

	lstrcpy(ipc.FarDirectory, ipc.FarModule);
	*(StrRChr(ipc.FarDirectory, nullptr, L'\\') + 1) = 0;

	GetTempPath(ARRAYSIZE(ipc.TempDirectory), ipc.TempDirectory);
	lstrcat(ipc.TempDirectory, L"FarUpdate\\");

	lstrcpy(ipc.FarUpdateList, ipc.TempDirectory);
	lstrcat(ipc.FarUpdateList, FarUpdateFile);

	lstrcpy(ipc.SelfUpdateList, ipc.TempDirectory);
	lstrcat(ipc.SelfUpdateList, SelfUpdateFile);

	lstrcpy(ipc.PluginModule, Info.ModuleName);

	lstrcpy(ipc.PluginDirectory, ipc.PluginModule);
	*(StrRChr(ipc.PluginDirectory, nullptr, '\\') + 1) = 0;

	lstrcpy(ipc.Config, ipc.PluginModule);
	lstrcat(ipc.Config, L".config");

	lstrcpy(ipc.SevenZip, ipc.PluginDirectory);
	lstrcat(ipc.SevenZip, L"7zxr.dll");

}

void ReadSettings()
{
	Mode = GetPrivateProfileInt(L"update", L"Mode", 2, ipc.Config);
	ipc.UseMsi = (GetPrivateProfileInt(L"update", L"Msi", 0, ipc.Config) != 0);
	bUseProxy = GetPrivateProfileInt(L"connect", L"proxy", 0, ipc.Config) == 1;
	GetPrivateProfileString(L"connect", L"srv", L"", strProxyName, ARRAYSIZE(strProxyName), ipc.Config);
	GetPrivateProfileString(L"connect", L"user", L"", strProxyUser, ARRAYSIZE(strProxyUser), ipc.Config);
	GetPrivateProfileString(L"connect", L"pass", L"", strProxyPass, ARRAYSIZE(strProxyPass), ipc.Config);
}


int WINAPI GetMinFarVersionW()
{
#define MAKEFARVERSION(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))
	return MAKEFARVERSION(MIN_FAR_MAJOR_VER, MIN_FAR_MINOR_VER, MIN_FAR_BUILD);
#undef MAKEFARVERSION
}

void WINAPI GetGlobalInfoW(GlobalInfo *Info)
{
	Info->StructSize = sizeof(GlobalInfo);
	Info->MinFarVersion = MAKEFARVERSION(MIN_FAR_MAJOR_VER, MIN_FAR_MINOR_VER, 0, MIN_FAR_BUILD, VS_RELEASE);
	Info->Version = MAKEFARVERSION(MAJOR_VER, MINOR_VER, 0, BUILD, VS_RELEASE);
	Info->Guid = MainGuid;
	Info->Title = L"Update";
	Info->Description = L"Automatic update";
	Info->Author = L"Alex Alabuzhev";
}

void WINAPI SetStartupInfoW(const PluginStartupInfo* psInfo)
{
	ifn.Load();
	Info = *psInfo;
	FSF = *psInfo->FSF;
	Info.FSF = &FSF;

	bUseProxy = false;
	*strProxyName = 0;
	*strProxyUser = 0;
	*strProxyPass = 0;

	InitPaths();
	ReadSettings();
	InitializeCriticalSection(&cs);

	StopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	UnlockEvent = CreateEvent(nullptr, TRUE, TRUE, nullptr);
	if (Mode)
	{
		hThread = CreateThread(nullptr, 0, ThreadProc, nullptr, 0, nullptr);
	}
}

void WINAPI GetPluginInfoW(PluginInfo* pInfo)
{
	pInfo->StructSize = sizeof(PluginInfo);
	static const wchar_t* PluginMenuStrings[1], *PluginConfigStrings[1];
	PluginMenuStrings[0] = MSG(MName);
	pInfo->PluginMenu.Guids = &MenuGuid;
	pInfo->PluginMenu.Strings = PluginMenuStrings;
	pInfo->PluginMenu.Count = ARRAYSIZE(PluginMenuStrings);

	PluginConfigStrings[0] = MSG(MCfgName);
	pInfo->PluginConfig.Guids = &MenuGuid;
	pInfo->PluginConfig.Strings = PluginConfigStrings;
	pInfo->PluginConfig.Count = ARRAYSIZE(PluginConfigStrings);

	pInfo->Flags = PF_EDITOR | PF_VIEWER | PF_DIALOG | PF_PRELOAD;
	static const wchar_t* CommandPrefix = L"update";
	pInfo->CommandPrefix = CommandPrefix;
}

void WINAPI ExitFARW(ExitInfo* Info)
{
	SetEvent(StopEvent);
	WaitForSingleObject(hThread, INFINITE);
	DeleteCriticalSection(&cs);
	if (hThread)
	{
		CloseHandle(hThread);
	}

	CloseHandle(StopEvent);
	CloseHandle(UnlockEvent);

	if (hRunDll)
	{
		CloseHandle(hRunDll);
	}
}

HANDLE WINAPI OpenW(const OpenInfo* oInfo)
{
	if (WaitForSingleObject(UnlockEvent, 0) == WAIT_TIMEOUT)
	{
		return nullptr;
	}
	else
	{
		ResetEvent(UnlockEvent);
	}
	Console console;
	NeedRestart = false;

	//for(int i=0;i<2;i++)
	int i = 1;
	{
		switch (CheckUpdates(!i))
		{
		case S_REQUIRED:
		{
			const wchar_t* Items[6];
			wchar_t Str[128];
			Items[0] = MSG(MName);
			Items[1] = MSG(MAvailableUpdates);
			Items[2] = L"\x1";
			DWORD NewMajor, NewMinor, NewBuild;
			GetNewModuleVersion(!i, Str, NewMajor, NewMinor, NewBuild);
			Items[3] = Str;
			Items[4] = L"\x1";
			Items[5] = MSG(MAsk);

			if (!Info.Message(&MainGuid, nullptr, FMSG_MB_YESNO | FMSG_LEFTALIGN, nullptr, Items, ARRAYSIZE(Items), 2))
			{
				DownloadUpdates(!i, false);
				StartUpdate(!i, false);
			}
			else
			{
				TextColor color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
				mprintf(MSG(MCancelled));
				Clean();
			}
			break;
		}
		break;

		case S_UPTODATE:
		{
			TextColor color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			mprintf(L"%s %s", MSG(MSystem), MSG(MUpToDate));
			Clean();
		}
		break;

		default:
		{
			TextColor color(FOREGROUND_RED | FOREGROUND_INTENSITY);
			mprintf(MSG(MCantConnect));
		}
		break;
		}
	}

	SetEvent(UnlockEvent);
	SaveTime();
	mprintf(L"\n\n");

	return nullptr;
}

bool Extract(const wchar_t* lpArc, const wchar_t* lpPath, const wchar_t* lpDestDir)
{
	bool Result = false;
	if (!ipc.UseMsi)
	{
		SevenZipModuleManager ArchiveManager(lpArc);
		Result = ArchiveManager.Extract(lpPath, lpDestDir);
	}
	else
	{
		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;
		wchar_t cmdline[MAX_PATH];
		wsprintf(cmdline, L"msiexec.exe /promptrestart /qb /i \"%s\"", lpPath);
		if (CreateProcess(nullptr, cmdline, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
			DWORD ExitCode = 0;
			if (GetExitCodeProcess(pi.hProcess, &ExitCode) && ExitCode == ERROR_SUCCESS)
			{
				Result = true;
			}
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}
	}
	return Result;
}

intptr_t WINAPI ProcessSynchroEventW(const ProcessSynchroEventInfo *pInfo)
{
	switch (pInfo->Event)
	{
	case SE_COMMONSYNCHRO:
	{
		const auto es = reinterpret_cast<EventStruct*>(pInfo->Param);
		switch (es->Event)
		{
		case E_ASKLOAD:
		{
			wchar_t Str[128];
			DWORD NewMajor, NewMinor, NewBuild;
			GetNewModuleVersion(es->Self, Str, NewMajor, NewMinor, NewBuild);
			const wchar_t* Items[] = { MSG(MName), MSG(MAvailableUpdates), L"\x1", Str, L"\x1", MSG(MAsk) };
			if (!Info.Message(&MainGuid, nullptr, FMSG_MB_YESNO | FMSG_LEFTALIGN, nullptr, Items, ARRAYSIZE(Items), 2))
			{
				*es->Result = true;
			}
			SetEvent(reinterpret_cast<HANDLE>(es->Data));
		}
		break;
		case E_CONNECTFAIL:
		{
			const wchar_t* Items[] = { MSG(MName), MSG(MCantConnect) };
			if (Info.Message(&MainGuid, nullptr, FMSG_MB_RETRYCANCEL | FMSG_LEFTALIGN | FMSG_WARNING, nullptr, Items, ARRAYSIZE(Items), 2))
			{
				*es->Result = true;
			}
			SetEvent(reinterpret_cast<HANDLE>(es->Data));
		}
		break;
		case E_DOWNLOADED:
		{
			const wchar_t* Items[] = { MSG(MName), MSG(MUpdatesDownloaded), MSG(MExitFAR) };
			Info.Message(&MainGuid, nullptr, FMSG_MB_OK, nullptr, Items, ARRAYSIZE(Items), 0);
			SetEvent(reinterpret_cast<HANDLE>(es->Data));
		}
		break;
		}
	}
	break;
	}
	return 0;
}

EXTERN_C void WINAPI RestartFARW(HWND, HINSTANCE, const wchar_t* lpCmd, DWORD)
{
	ifn.Load();
	int argc = 0;
	const auto argv = CommandLineToArgvW(lpCmd, &argc);
	int n = 0;
	if (argc == 2 + n)
	{
		if (!ifn.AttachConsole(ATTACH_PARENT_PROCESS))
		{
			AllocConsole();
		}
		const auto ptr = StringToNumber<intptr_t>(argv[n + 0]);
		const auto hFar = static_cast<HANDLE>(reinterpret_cast<LPVOID>(ptr));
		if (hFar && hFar != INVALID_HANDLE_VALUE)
		{
			HMENU hMenu = GetSystemMenu(GetConsoleWindow(), FALSE);
			int Count = GetMenuItemCount(hMenu);
			int Pos = -1;
			for (int i = 0; i < Count; i++)
			{
				if (GetMenuItemID(hMenu, i) == SC_CLOSE)
				{
					Pos = i;
					break;
				}
			}
			MENUITEMINFO mi = { sizeof(mi), MIIM_ID, 0, 0, 0 };
			if (Pos != -1)
			{
				SetMenuItemInfo(hMenu, Pos, MF_BYPOSITION, &mi);
			}
			const auto IPCPtr = StringToNumber<intptr_t>(argv[n + 1]);
			if (ReadProcessMemory(hFar, reinterpret_cast<LPCVOID>(IPCPtr), &ipc, sizeof(IPC), nullptr))
			{
				WaitForSingleObject(hFar, INFINITE);

				CONSOLE_SCREEN_BUFFER_INFO csbi;
				GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
				while (csbi.dwSize.Y--)
				{
					mprintf(L"\n");
				}
				CloseHandle(hFar);

				mprintf(L"\n\n\n");

				wchar_t destpath[MAX_PATH];
				lstrcpy(destpath, ipc.FarDirectory);

				wchar_t arc[MAX_PATH];
				GetPrivateProfileString(L"far", ipc.UseMsi ? L"msi" : L"arc", L"", arc, ARRAYSIZE(arc), ipc.Self ? ipc.SelfUpdateList : ipc.FarUpdateList);
				if (*arc)
				{
					wchar_t local_arc[MAX_PATH];
					lstrcpy(local_arc, ipc.TempDirectory);
					lstrcat(local_arc, arc);
					if (GetFileAttributes(local_arc) != INVALID_FILE_ATTRIBUTES)
					{
						bool Result = false;
						while (!Result)
						{
							mprintf(L"Unpacking %-50s", arc);
							if (!Extract(ipc.SevenZip, local_arc, destpath))
							{
								{
									TextColor color(FOREGROUND_RED | FOREGROUND_INTENSITY);
									mprintf(L"\nUnpack error. Retry? (Y/N) ");
								}
								INPUT_RECORD ir = { 0 };
								DWORD n;
								while (!(ir.EventType == KEY_EVENT && !ir.Event.KeyEvent.bKeyDown && (ir.Event.KeyEvent.wVirtualKeyCode == L'Y' || ir.Event.KeyEvent.wVirtualKeyCode == L'N')))
								{
									ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &n);
									Sleep(1);
								}
								if (ir.Event.KeyEvent.wVirtualKeyCode == L'N')
								{
									mprintf(L"\n");
									break;
								}
								mprintf(L"\n");
							}
							else
							{
								Result = true;
							}
						}
						if (Result)
						{
							TextColor color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
							mprintf(L"OK\n");
							if (GetPrivateProfileInt(L"Update", L"Delete", 1, ipc.Config))
							{
								DeleteFile(local_arc);
							}
						}
						else
						{
							TextColor color(FOREGROUND_RED | FOREGROUND_INTENSITY);
							mprintf(L"error\n");
						}
					}
				}
				wchar_t exec[2048], execExp[4096];
				GetPrivateProfileString(L"events", L"PostInstall", L"", exec, ARRAYSIZE(exec), ipc.Config);
				if (*exec)
				{
					ExpandEnvironmentStrings(exec, execExp, ARRAYSIZE(execExp));
					STARTUPINFO si = { sizeof(si) };
					PROCESS_INFORMATION pi;
					{
						TextColor color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
						mprintf(L"\nExecuting %-50.50s", execExp);
					}
					if (CreateProcess(nullptr, execExp, nullptr, nullptr, TRUE, 0, nullptr, ipc.PluginDirectory, &si, &pi))
					{
						{
							TextColor color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
							mprintf(L"OK\n\n");
						}
						WaitForSingleObject(pi.hProcess, INFINITE);
					}
					else
					{
						TextColor color(FOREGROUND_RED | FOREGROUND_INTENSITY);
						mprintf(L"Error %d", GetLastError());
					}
					mprintf(L"\n");
					CloseHandle(pi.hThread);
					CloseHandle(pi.hProcess);
				}
				if (Pos != -1)
				{
					mi.wID = SC_CLOSE;
					SetMenuItemInfo(hMenu, Pos, MF_BYPOSITION, &mi);
					DrawMenuBar(GetConsoleWindow());
				}
				TextColor color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
				mprintf(L"\n%-60s", L"Starting FAR...");
				STARTUPINFO si = { sizeof(si) };
				PROCESS_INFORMATION pi;
				wchar_t FarCmd[2048];
				lstrcpy(FarCmd, ipc.FarModule);
				lstrcat(FarCmd, L" ");
				lstrcat(FarCmd, ipc.FarParams);
				if (CreateProcess(nullptr, FarCmd, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi))
				{
					TextColor color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
					mprintf(L"OK");
				}
				else
				{
					TextColor color(FOREGROUND_RED | FOREGROUND_INTENSITY);
					mprintf(L"Error %d", GetLastError());
				}
				mprintf(L"\n");

				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
			}
		}
	}
	LocalFree(argv);
	Clean();
}
