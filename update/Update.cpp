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

enum class update_status
{
	S_CANTCONNECT,
	S_UPTODATE,
	S_REQUIRED,
};

enum EVENT
{
	E_ASKLOAD,
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

wchar_t strProxyName[512];
wchar_t strProxyUser[512];
wchar_t strProxyPass[512];

PluginStartupInfo PsInfo;
FarStandardFunctions FSF;

void InitPaths()
{
	GetModuleFileName(nullptr, ipc.FarModule, ARRAYSIZE(ipc.FarModule));

	wcscpy(ipc.FarDirectory, ipc.FarModule);
	*(wcsrchr(ipc.FarDirectory, L'\\') + 1) = 0;

	GetTempPath(ARRAYSIZE(ipc.TempDirectory), ipc.TempDirectory);
	wcscat(ipc.TempDirectory, L"FarUpdate\\");

	wcscpy(ipc.FarUpdateList, ipc.TempDirectory);
	wcscat(ipc.FarUpdateList, FarUpdateFile);

	wcscpy(ipc.SelfUpdateList, ipc.TempDirectory);
	wcscat(ipc.SelfUpdateList, SelfUpdateFile);

	wcscpy(ipc.PluginModule, PsInfo.ModuleName);

	wcscpy(ipc.PluginDirectory, ipc.PluginModule);
	*(wcsrchr(ipc.PluginDirectory, '\\') + 1) = 0;

	wcscpy(ipc.Config, ipc.PluginModule);
	wcscat(ipc.Config, L".config");

	wcscpy(ipc.SevenZip, ipc.PluginDirectory);
	wcscat(ipc.SevenZip, L"7zxr.dll");
}

class update_plugin
{
public:
	update_plugin()
	{
		wchar_t ModuleName[MAX_PATH];
		GetModuleFileName(nullptr, ModuleName, ARRAYSIZE(ModuleName));
		m_GuardName = L"FarUpdateGuard_"s + ModuleName;
		std::replace(m_GuardName.begin(), m_GuardName.end(), L'\\', L'/');

		*strProxyName = 0;
		*strProxyUser = 0;
		*strProxyPass = 0;

		InitPaths();

		ReadSettings();

		InitializeCriticalSection(&m_Cs);
		m_ExitEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		m_WaitEvent = CreateEvent(nullptr, FALSE, TRUE, nullptr);
		m_SingletonEvent = CreateEvent(nullptr, TRUE, TRUE, m_GuardName.data());

		if (m_Mode)
		{
			StartBackgroundJob();
		}

		m_PluginMenuStrings[0] = MSG(MName);
		m_PluginConfigStrings[0] = MSG(MCfgName);
	}

	~update_plugin()
	{
		SetEvent(m_ExitEvent);
		WaitForSingleObject(m_Thread, INFINITE);
		DeleteCriticalSection(&m_Cs);
		if (m_Thread)
		{
			CloseHandle(m_Thread);
		}
		CloseHandle(m_SingletonEvent);
		CloseHandle(m_WaitEvent);
		CloseHandle(m_ExitEvent);

		if (m_RunDll)
		{
			CloseHandle(m_RunDll);
		}
	}

	void GetPluginInfo(PluginInfo& Info) const
	{
		Info.StructSize = sizeof Info;
		Info.PluginMenu.Guids = &MenuGuid;
		Info.PluginMenu.Strings = m_PluginMenuStrings;
		Info.PluginMenu.Count = ARRAYSIZE(m_PluginMenuStrings);

		Info.PluginConfig.Guids = &MenuGuid;
		Info.PluginConfig.Strings = m_PluginConfigStrings;
		Info.PluginConfig.Count = ARRAYSIZE(m_PluginConfigStrings);

		Info.Flags = PF_EDITOR | PF_VIEWER | PF_DIALOG | PF_PRELOAD;
		Info.CommandPrefix = m_CommandPrefix.data();
	}

	void ManualCheck();

	void StartBackgroundJob()
	{
		m_Thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, [](void* Parameter)
		{
			static_cast<update_plugin*>(Parameter)->ThreadProc();
			return 0u;
		}, this, 0, nullptr));
	}

private:
	update_status CheckUpdates(bool Self);
	bool DownloadUpdates(bool Self, bool Silent);
	void StartUpdate(bool Self, bool Silent);

	void ReadSettings();
	void ThreadProc();

	int m_Mode = 0;
	int m_Period = 0;
	bool m_UseProxy = false;
	bool NeedRestart = false;
	std::wstring m_GuardName;
	CRITICAL_SECTION m_Cs;
	HANDLE m_Thread = nullptr;
	HANDLE m_ExitEvent = nullptr;
	HANDLE m_WaitEvent = nullptr;
	HANDLE m_SingletonEvent = nullptr;
	HANDLE m_RunDll = nullptr;

	std::wstring m_CommandPrefix = L"update";
	const wchar_t* m_PluginMenuStrings[1];
	const wchar_t* m_PluginConfigStrings[1];
};

std::unique_ptr<update_plugin> PluginInstance;

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
		PsInfo.AdvControl(&MainGuid, ACTL_GETFARMANAGERVERSION, 0, &FarVersion);
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

DWORD WINAPI WinInetDownload(const wchar_t* strSrv, const wchar_t* strURL, const wchar_t* strFile, bool UseProxy, DOWNLOADPROC Proc)
{
	DWORD err = 0;

	DWORD ProxyType = INTERNET_OPEN_TYPE_DIRECT;
	if (UseProxy)
		ProxyType = *strProxyName ? INTERNET_OPEN_TYPE_PROXY : INTERNET_OPEN_TYPE_PRECONFIG;

	HINTERNET hInternet = InternetOpen(L"Mozilla/5.0 (compatible; FAR Update)", ProxyType, strProxyName, nullptr, 0);
	if (hInternet)
	{
		BYTE Data[2048];
		DWORD dwBytesRead;

		HINTERNET hConnect = InternetConnect(hInternet, strSrv, INTERNET_DEFAULT_HTTP_PORT, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 1);
		if (hConnect)
		{
			if (UseProxy && *strProxyName)
			{
				if (*strProxyUser)
				{
					if (!InternetSetOption(hConnect, INTERNET_OPTION_PROXY_USERNAME, strProxyUser, static_cast<DWORD>(wcslen(strProxyUser))))
					{
						err = GetLastError();
					}
				}
				if (*strProxyPass)
				{
					if (!InternetSetOption(hConnect, INTERNET_OPTION_PROXY_PASSWORD, strProxyPass, static_cast<DWORD>(wcslen(strProxyPass))))
					{
						err = GetLastError();
					}
				}
			}
			HTTP_VERSION_INFO httpver = { 1, 1 };
			if (!InternetSetOption(hConnect, INTERNET_OPTION_HTTP_VERSION, &httpver, sizeof httpver))
			{
				err = GetLastError();
			}

			HINTERNET hRequest = HttpOpenRequest(hConnect, L"GET", strURL, L"HTTP/1.1", nullptr, nullptr, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 1);

			if (hRequest)
			{
				if (HttpSendRequest(hRequest, nullptr, 0, nullptr, 0))
				{
					HANDLE hFile = CreateFile(strFile, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
					if (hFile != INVALID_HANDLE_VALUE)
					{
						DWORD Size = 0;
						DWORD sz = sizeof Size;
						if (!HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &Size, &sz, nullptr))
						{
							err = GetLastError();
						}
						if (Size != GetFileSize(hFile, nullptr))
						{
							SetEndOfFile(hFile);
							UINT BytesDone = 0;
							HideCursor hc;
							while (InternetReadFile(hRequest, Data, sizeof Data, &dwBytesRead) && dwBytesRead)
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

bool DownloadFile(bool Self, const wchar_t* RemoteFile, const wchar_t* LocalName, bool UseProxy, bool UseCallBack)
{
	const auto LocalFile = std::wstring(ipc.TempDirectory) + (LocalName? LocalName : FSF.PointToName(RemoteFile));
	return WinInetDownload(Self? SelfRemoteSrv : FarRemoteSrv, RemoteFile, LocalFile.data(), UseProxy, UseCallBack? DownloadProc : nullptr) == 0;
}

update_status update_plugin::CheckUpdates(bool Self)
{
	auto Ret = update_status::S_UPTODATE;
	CreateDirectory(ipc.TempDirectory, nullptr);

	const auto URL = Self?
		SelfRemotePath + L"/"s + SelfUpdateFile :
		FarRemotePath + L"/"s + FarUpdateFile + phpRequest;

	if (DownloadFile(Self, URL.data(), Self ? SelfUpdateFile : FarUpdateFile, m_UseProxy, false))
	{
		wchar_t sVer[MAX_PATH];
		GetPrivateProfileString(L"info", L"version", L"", sVer, ARRAYSIZE(sVer), Self ? ipc.SelfUpdateList : ipc.FarUpdateList);
		if (wcscmp(sVer, strVer)) // BUGBUG
		{
			return update_status::S_CANTCONNECT;
		}
		if (NeedUpdate(Self))
		{
			Ret = update_status::S_REQUIRED;
		}
	}
	else
	{
		Ret = update_status::S_CANTCONNECT;
	}
	return Ret;
}

bool update_plugin::DownloadUpdates(bool Self, bool Silent)
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
	if (DownloadFile(Self, URL, nullptr, m_UseProxy, !Silent))
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

void update_plugin::StartUpdate(bool Self, bool Silent)
{
	DWORD RunDllExitCode = 0;
	GetExitCodeProcess(m_RunDll, &RunDllExitCode);
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
			wcscat(ipc.FarParams, Argv[i]);
			if (i < NumArgs - 1)
				wcscat(ipc.FarParams, L" ");
		}
		LocalFree(Argv);

		wchar_t WinDir[MAX_PATH];
		GetWindowsDirectory(WinDir, ARRAYSIZE(WinDir));
		BOOL IsWow64 = FALSE;
		ipc.Self = Self;
		FSF.sprintf(cmdline, L"%s\\%s\\rundll32.exe \"%s\", RestartFAR %I64d %I64d", WinDir, ifn.IsWow64Process(GetCurrentProcess(), &IsWow64) && IsWow64 ? L"SysWOW64" : L"System32", ipc.PluginModule, reinterpret_cast<INT64>(ProcDup), reinterpret_cast<INT64>(&ipc));

		STARTUPINFO si = { sizeof si };
		PROCESS_INFORMATION pi;

		if (CreateProcess(nullptr, cmdline, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi))
		{
			m_RunDll = pi.hProcess;
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
				PsInfo.AdvControl(&MainGuid, ACTL_SYNCHRO, 0, &es);
				WaitForSingleObject(hEvent, INFINITE);
				CloseHandle(hEvent);
			}
		}
		else
		{
			if (!Silent)
			{
				TextColor color(FOREGROUND_RED | FOREGROUND_INTENSITY);
				mprintf(L"%s - error %d\n", MSG(MCantCreateProcess), GetLastError());
			}
		}

	}
	else
	{
		if (!Silent)
		{
			TextColor color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			mprintf(L"%s\n", MSG(MDone));
		}
	}
}

void update_plugin::ThreadProc()
{
	while (WaitForSingleObject(m_ExitEvent, 0) != WAIT_OBJECT_0)
	{
		{
			HANDLE Handles[] = { m_ExitEvent, m_SingletonEvent };
			if (WaitForMultipleObjects(ARRAYSIZE(Handles), Handles, false, INFINITE) == WAIT_OBJECT_0)
				return;
		}

		HANDLE Handles[] = { m_ExitEvent, m_WaitEvent };
		const auto Result = WaitForMultipleObjects(ARRAYSIZE(Handles), Handles, false, m_Period * 60 * 60 * 1000);
		if (Result == WAIT_OBJECT_0)
			return;

		if (Result == WAIT_OBJECT_0 + 1)
		{
			//for(int i=0;i<2;i++)
			int i = 1;
			{
				switch (CheckUpdates(!i))
				{
				case update_status::S_REQUIRED:
					{
						ResetEvent(m_SingletonEvent);
						auto Load = m_Mode == 2;
						if (!Load)
						{
							const auto hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
							EventStruct es = { E_ASKLOAD, hEvent, !i, &Load };
							PsInfo.AdvControl(&MainGuid, ACTL_SYNCHRO, 0, &es);
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
						SetEvent(m_SingletonEvent);
					}
					break;

				case update_status::S_CANTCONNECT:
					{
						Clean();
					}
					break;

				case update_status::S_UPTODATE:
					{
						Clean();
					}
					break;
				}
			}
		}
	}
}


void update_plugin::ReadSettings()
{
	m_Mode = GetPrivateProfileInt(L"update", L"Mode", 2, ipc.Config);
	m_Period = GetPrivateProfileInt(L"update", L"Period", 60, ipc.Config);
	ipc.UseMsi = (GetPrivateProfileInt(L"update", L"Msi", 0, ipc.Config) != 0);
	m_UseProxy = GetPrivateProfileInt(L"connect", L"proxy", 0, ipc.Config) == 1;
	GetPrivateProfileString(L"connect", L"srv", L"", strProxyName, ARRAYSIZE(strProxyName), ipc.Config);
	GetPrivateProfileString(L"connect", L"user", L"", strProxyUser, ARRAYSIZE(strProxyUser), ipc.Config);
	GetPrivateProfileString(L"connect", L"pass", L"", strProxyPass, ARRAYSIZE(strProxyPass), ipc.Config);
}


extern "C" int WINAPI GetMinFarVersionW()
{
#define MAKEFARVERSION(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))
	return MAKEFARVERSION(MIN_FAR_MAJOR_VER, MIN_FAR_MINOR_VER, MIN_FAR_BUILD);
#undef MAKEFARVERSION
}

extern "C" void WINAPI GetGlobalInfoW(GlobalInfo *Info)
{
	Info->StructSize = sizeof *Info;
	Info->MinFarVersion = MAKEFARVERSION(MIN_FAR_MAJOR_VER, MIN_FAR_MINOR_VER, 0, MIN_FAR_BUILD, VS_RELEASE);
	Info->Version = MAKEFARVERSION(MAJOR_VER, MINOR_VER, 0, BUILD, VS_RELEASE);
	Info->Guid = MainGuid;
	Info->Title = L"Update";
	Info->Description = L"Automatic update";
	Info->Author = L"Alex Alabuzhev";
}

extern "C" void WINAPI SetStartupInfoW(const PluginStartupInfo* psInfo)
{
	try
	{
		PsInfo = *psInfo;
		FSF = *psInfo->FSF;
		PsInfo.FSF = &FSF;

		ifn.Load();

		PluginInstance = std::make_unique<update_plugin>();
	}
	catch(...)
	{
	}
}

extern "C" void WINAPI GetPluginInfoW(PluginInfo* Info)
{
	try
	{
		PluginInstance->GetPluginInfo(*Info);
	}
	catch(...)
	{
	}
}

extern "C" void WINAPI ExitFARW(const ExitInfo*)
{
	try
	{
		PluginInstance.reset();
	}
	catch(...)
	{
	}
}

extern "C" HANDLE WINAPI OpenW(const OpenInfo*)
{
	try
	{
		PluginInstance->ManualCheck();
	}
	catch (...)
	{
	}
	return nullptr;
}

void update_plugin::ManualCheck()
{
	Console console;

	if (WaitForSingleObject(m_SingletonEvent, 0) == WAIT_TIMEOUT)
	{
		TextColor color(FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		mprintf(L"%s\n", MSG(MInProgress));
		return;
	}
	ResetEvent(m_SingletonEvent);

	//for(int i=0;i<2;i++)
	int i = 1;
	{
		switch (CheckUpdates(!i))
		{
		case update_status::S_REQUIRED:
			{
				DWORD NewMajor, NewMinor, NewBuild;
				wchar_t Str[128];
				GetNewModuleVersion(!i, Str, NewMajor, NewMinor, NewBuild);

				const wchar_t* Items[] =
				{
					MSG(MName),
					MSG(MAvailableUpdates),
					L"\x1",
					Str,
					L"\x1",
					MSG(MAsk)
				};

				if (!PsInfo.Message(&MainGuid, nullptr, FMSG_MB_YESNO | FMSG_LEFTALIGN, nullptr, Items, ARRAYSIZE(Items), 2))
				{
					if (DownloadUpdates(!i, false))
					{
						StartUpdate(!i, false);
					}
				}
				else
				{
					TextColor color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
					mprintf(L"%s\n", MSG(MCancelled));
					Clean();
				}
			}
			break;

		case update_status::S_UPTODATE:
			{
				TextColor color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
				mprintf(L"%s %s\n", MSG(MSystem), MSG(MUpToDate));
				Clean();
			}
			break;

		case update_status::S_CANTCONNECT:
			{
				TextColor color(FOREGROUND_RED | FOREGROUND_INTENSITY);
				mprintf(L"%s\n", MSG(MCantConnect));
			}
			break;
		}
	}

	SetEvent(m_SingletonEvent);
}

bool Extract(const wchar_t* lpArc, const wchar_t* lpPath, const wchar_t* lpDestDir)
{
	bool Result = false;
	if (!ipc.UseMsi)
	{
		Result = SevenZipModuleManager(lpArc).Extract(lpPath, lpDestDir);
	}
	else
	{
		STARTUPINFO si { sizeof si };
		PROCESS_INFORMATION pi;
		auto CmdLine = L"msiexec.exe /promptrestart /qb /i \""s + lpPath + L"\""s;
		if (CreateProcess(nullptr, &CmdLine[0], nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
			DWORD ExitCode = 0;
			Result = GetExitCodeProcess(pi.hProcess, &ExitCode) && ExitCode == ERROR_SUCCESS;
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}
	}
	return Result;
}

extern "C" intptr_t WINAPI ProcessSynchroEventW(const ProcessSynchroEventInfo *pInfo)
{
	try
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
				if (!PsInfo.Message(&MainGuid, nullptr, FMSG_MB_YESNO | FMSG_LEFTALIGN, nullptr, Items, ARRAYSIZE(Items), 2))
				{
					*es->Result = true;
				}
				SetEvent(reinterpret_cast<HANDLE>(es->Data));
			}
			break;

			case E_DOWNLOADED:
			{
				const wchar_t* Items[] = { MSG(MName), MSG(MUpdatesDownloaded), MSG(MExitFAR) };
				PsInfo.Message(&MainGuid, nullptr, FMSG_MB_OK, nullptr, Items, ARRAYSIZE(Items), 0);
				SetEvent(reinterpret_cast<HANDLE>(es->Data));
			}
			break;
			}
		}
		break;
		}
	}
	catch (...)
	{
	}
	return 0;
}

extern "C" void WINAPI RestartFARW(HWND, HINSTANCE, const wchar_t* lpCmd, DWORD)
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
		const auto ptr = std::wcstoull(argv[n + 0], nullptr, 10);
		const auto hFar = static_cast<HANDLE>(reinterpret_cast<LPVOID>(ptr));
		if (hFar && hFar != INVALID_HANDLE_VALUE)
		{
			/*
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
			MENUITEMINFO mi = { sizeof mi, MIIM_ID, 0, 0, 0 };
			if (Pos != -1)
			{
				SetMenuItemInfo(hMenu, Pos, MF_BYPOSITION, &mi);
			}
			*/
			const auto IPCPtr = std::wcstoull(argv[n + 1], nullptr, 10);
			if (ReadProcessMemory(hFar, reinterpret_cast<LPCVOID>(IPCPtr), &ipc, sizeof ipc, nullptr))
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

				wchar_t arc[MAX_PATH];
				GetPrivateProfileString(L"far", ipc.UseMsi ? L"msi" : L"arc", L"", arc, ARRAYSIZE(arc), ipc.Self ? ipc.SelfUpdateList : ipc.FarUpdateList);
				if (*arc)
				{
					const auto local_arc = std::wstring(ipc.TempDirectory) + arc;
					if (GetFileAttributes(local_arc.data()) != INVALID_FILE_ATTRIBUTES)
					{
						bool Result = false;
						while (!Result)
						{
							mprintf(L"Unpacking %-50s", arc);
							if (!Extract(ipc.SevenZip, local_arc.data(), ipc.FarDirectory))
							{
								{
									TextColor color(FOREGROUND_RED | FOREGROUND_INTENSITY);
									mprintf(L"\nUnpack error. Retry? (Y/N) ");
								}
								INPUT_RECORD ir = { 0 };
								while (!(ir.EventType == KEY_EVENT && !ir.Event.KeyEvent.bKeyDown && (ir.Event.KeyEvent.wVirtualKeyCode == L'Y' || ir.Event.KeyEvent.wVirtualKeyCode == L'N')))
								{
									DWORD BytesRead;
									ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &BytesRead);
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
								DeleteFile(local_arc.data());
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
					STARTUPINFO si = { sizeof si };
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
				/*
				if (Pos != -1)
				{
					mi.wID = SC_CLOSE;
					SetMenuItemInfo(hMenu, Pos, MF_BYPOSITION, &mi);
					DrawMenuBar(GetConsoleWindow());
				}
				*/

				{
					TextColor color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
					mprintf(L"\n%-60s", L"Starting FAR...");
				}
				STARTUPINFO si = { sizeof si };
				PROCESS_INFORMATION pi;
				auto FarCmd = ipc.FarModule + L" "s + ipc.FarParams;
				if (CreateProcess(nullptr, &FarCmd[0], nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi))
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
