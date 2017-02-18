#include "headers.hpp"
#pragma hdrstop

#include <initguid.h>

#include "seven_zip_module_manager.hpp"

#include "guid.hpp"
#include "imported_functions.hpp"

#include "update.hpp"
#include "lng.hpp"
#include "ver.hpp"

#include "system.hpp"
#include "Console.hpp"
#include "cursor_pos.hpp"
#include "hide_cursor.hpp"
#include "text_color.hpp"

enum class update_status
{
	S_CANTCONNECT,
	S_INCOMPATIBLE,
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

wchar_t ProxyName[512];
wchar_t ProxyUser[512];
wchar_t ProxyPass[512];

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

auto GetPrivateProfileString(const wchar_t* AppName, const wchar_t* KeyName, const wchar_t* Default, const wchar_t* FileName)
{
	std::vector<wchar_t> Buffer(32768);
	DWORD size = ::GetPrivateProfileString(AppName, KeyName, Default, Buffer.data(), static_cast<DWORD>(Buffer.size()), FileName);
	return std::wstring(Buffer.data(), size);
}

using download_callback = void(*)(int);

class update_plugin
{
public:
	update_plugin()
	{
		wchar_t ModuleName[MAX_PATH];
		GetModuleFileName(nullptr, ModuleName, ARRAYSIZE(ModuleName));
		m_GuardName = L"FarUpdateGuard_"s + ModuleName;
		std::replace(m_GuardName.begin(), m_GuardName.end(), L'\\', L'/');

		*ProxyName = 0;
		*ProxyUser = 0;
		*ProxyPass = 0;

		InitPaths();

		ReadSettings();

		InitializeCriticalSection(&m_Cs);
		m_ExitEvent.reset(CreateEvent(nullptr, TRUE, FALSE, nullptr));
		m_WaitEvent.reset(CreateEvent(nullptr, FALSE, TRUE, nullptr));
		m_SingletonEvent.reset(CreateEvent(nullptr, TRUE, TRUE, m_GuardName.data()));

		if (m_Mode)
		{
			StartBackgroundJob();
		}
	}

	~update_plugin()
	{
		SetEvent(m_ExitEvent.get());
		WaitForSingleObject(m_Thread.get(), INFINITE);
		DeleteCriticalSection(&m_Cs);
	}

	void GetPluginInfo(PluginInfo& Info)
	{
		Info.StructSize = sizeof Info;

		Info.PluginMenu.Guids = &MenuGuid;
		m_PluginMenuStrings[0] = MSG(MName);
		Info.PluginMenu.Strings = m_PluginMenuStrings;
		Info.PluginMenu.Count = ARRAYSIZE(m_PluginMenuStrings);

		/*
		Info.PluginConfig.Guids = &MenuGuid;
		m_PluginConfigStrings[0] = MSG(MCfgName);
		Info.PluginConfig.Strings = m_PluginConfigStrings;
		Info.PluginConfig.Count = ARRAYSIZE(m_PluginConfigStrings);
		*/

		Info.Flags = PF_EDITOR | PF_VIEWER | PF_DIALOG | PF_PRELOAD;
		Info.CommandPrefix = m_CommandPrefix.data();
	}

	void ManualCheck();

	void StartBackgroundJob()
	{
		m_Thread.reset(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, [](void* Parameter)
		{
			static_cast<update_plugin*>(Parameter)->ThreadProc();
			return 0u;
		}, this, 0, nullptr)));
	}

private:
	update_status CheckUpdates(bool Self, bool Manual) const;
	bool DownloadUpdates(bool Self, bool Silent);
	bool DownloadFile(bool Self, const wchar_t* RemoteFile, const wchar_t* LocalName, bool UseProxy, bool UseCallBack, bool DownloadAlways) const;
	DWORD WinInetDownload(const wchar_t* ServerName, const wchar_t* Url, const wchar_t* FileName, bool UseProxy, download_callback Callback) const;
	void StartUpdate(bool Self, bool Silent);

	void ReadSettings();
	void ThreadProc();

	int m_Mode = 0;
	int m_Period = 0;
	bool m_UseProxy = false;
	bool NeedRestart = false;
	std::wstring m_GuardName;
	CRITICAL_SECTION m_Cs;
	handle m_Thread;
	handle m_ExitEvent;
	handle m_WaitEvent;
	handle m_SingletonEvent;
	handle m_RunDll;

	std::wstring m_CommandPrefix = L"update";
	const wchar_t* m_PluginMenuStrings[1];
	const wchar_t* m_PluginConfigStrings[1];
};

std::unique_ptr<update_plugin> PluginInstance;

namespace detail
{
	void check_arg(){}

	template<typename arg, typename... args>
	void check_arg(arg, args... Args)
	{
		static_assert(std::is_fundamental<std::remove_pointer_t<arg>>::value, "Must be a fundamental type");
		check_arg(Args...);
	}
}

template<typename... args>
int mprintf(color Color, const wchar_t* Format, args... Args)
{
	text_color TextColor(Color);
	return mprintf(Format, std::forward<args>(Args)...);
}

template<typename... args>
int mprintf(const wchar_t* Format, args... Args)
{
	detail::check_arg(Args...);
	return mprintf_impl(Format, std::forward<args>(Args)...);
}

int mprintf_impl(const wchar_t* Format, ...)
{
	va_list argptr;
	va_start(argptr, Format);

	std::unique_ptr<wchar_t[]> Buffer;
	size_t size = 128;
	int Length;
	do
	{
		Buffer = std::make_unique<wchar_t[]>(size *= 2);
		Buffer[size - 1] = 0;
		Length = _vsnwprintf(Buffer.get(), size - 1, Format, argptr);
	}
	while (Length < 0);

	va_end(argptr);

	DWORD n;
	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), Buffer.get(), Length, &n, nullptr);
	return n;
}

static bool get_response(const wchar_t* Question)
{
	mprintf(color::yellow, (L"\n"s + Question + L" (Y/N) "s).data());

	INPUT_RECORD ir = { 0 };
	while (!(ir.EventType == KEY_EVENT && !ir.Event.KeyEvent.bKeyDown && (ir.Event.KeyEvent.wVirtualKeyCode == L'Y' || ir.Event.KeyEvent.wVirtualKeyCode == L'N')))
	{
		DWORD BytesRead;
		ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &BytesRead);
		Sleep(1);
	}

	mprintf(L"\n");
	return ir.Event.KeyEvent.wVirtualKeyCode == L'Y';
}

static bool confirm_retry()
{
	return get_response(L"Retry?");
}

static bool CallbackHandler(callback_mode Type, const wchar_t* Str)
{
	switch (Type)
	{
	case notification:
		//mprintf(L"%s", Str);
		return true;

	case error:
		mprintf(color::red, L"%s", Str);
		return true;

	case retry_confirmation:
		return confirm_retry();

	default:
		return true;
	}
}
void GetNewModuleVersion(bool Self, wchar_t* Str, DWORD& NewMajor, DWORD& NewMinor, DWORD& NewBuild)
{
	const auto UpdateList = Self? ipc.SelfUpdateList : ipc.FarUpdateList;
	const auto Section = Self? SelfSection : FarSection;
	NewMajor = GetPrivateProfileInt(Section, L"major", -1, UpdateList);
	NewMinor = GetPrivateProfileInt(Section, L"minor", -1, UpdateList);
	NewBuild = GetPrivateProfileInt(Section, L"build", -1, UpdateList);
	if (Str)
	{
		FSF.sprintf(Str, Self? L"Update (%d.%d build %d)" : L"Far Manager (%d.%d build %d)", NewMajor, NewMinor, NewBuild);
	}
}

template<typename T>
bool ApiDynamicStringReceiver(std::wstring& Destination, const T& Callable)
{
	wchar_t Buffer[MAX_PATH];
	auto Size = Callable(Buffer, std::size(Buffer));
	if (!Size)
		return false;

	if (Size < std::size(Buffer))
	{
		Destination.assign(Buffer, Size);
		return true;
	}

	for (;;)
	{
		const auto BufferSize = Size;
		auto vBuffer = std::make_unique<wchar_t[]>(BufferSize);
		Size = Callable(vBuffer.get(), BufferSize);
		if (!Size)
			return false;
		if (Size < BufferSize)
		{
			Destination.assign(vBuffer.get(), Size);
			return true;
		}
	}
}

std::wstring expand_strings(const wchar_t* Str)
{
	std::wstring Result;
	if (!ApiDynamicStringReceiver(Result, [&](wchar_t* Buffer, size_t Size)
	{
		auto ReturnedSize = ::ExpandEnvironmentStrings(Str, Buffer, static_cast<DWORD>(Size));
		// This pesky function includes the terminating null character even upon success, breaking the usual pattern
		if (ReturnedSize <= Size)
			--ReturnedSize;
		return ReturnedSize;
	}))
	{
		Result = Str;
	}
	return Result;
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

void DownloadProc(int Percent)
{
	static int LastPercent = 0;
	cursor_pos cp;
	if (Percent)
	{
		if (Percent != LastPercent)
		{
			mprintf(L"%d%%", Percent);
			LastPercent = Percent;
		}
	}
	else
	{
		mprintf(L"    ");
		LastPercent = 0;
	}
}

namespace detail
{
	struct internet_handle_closer
	{
		void operator()(HINTERNET Handle) const
		{
			InternetCloseHandle(Handle);
		}
	};
}

using internet_ptr = std::unique_ptr<std::remove_pointer_t<HINTERNET>, detail::internet_handle_closer>;

DWORD update_plugin::WinInetDownload(const wchar_t* ServerName, const wchar_t* Url, const wchar_t* FileName, bool UseProxy, download_callback Callback) const
{
	auto ProxyType = UseProxy? *ProxyName? INTERNET_OPEN_TYPE_PROXY : INTERNET_OPEN_TYPE_PRECONFIG : INTERNET_OPEN_TYPE_DIRECT;

	internet_ptr Internet(InternetOpen(L"Mozilla/5.0 (compatible; FAR Update)", ProxyType, ProxyName, nullptr, 0));
	if (!Internet)
		return GetLastError();
		
	BYTE Data[2048];
	DWORD BytesRead;

	internet_ptr Connection(InternetConnect(Internet.get(), ServerName, INTERNET_DEFAULT_HTTP_PORT, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 1));
	if (!Connection)
		return GetLastError();

	if (UseProxy && *ProxyName)
	{
		if (*ProxyUser)
		{
			if (!InternetSetOption(Connection.get(), INTERNET_OPTION_PROXY_USERNAME, ProxyUser, static_cast<DWORD>(wcslen(ProxyUser))))
				return GetLastError();
		}
		if (*ProxyPass)
		{
			if (!InternetSetOption(Connection.get(), INTERNET_OPTION_PROXY_PASSWORD, ProxyPass, static_cast<DWORD>(wcslen(ProxyPass))))
				return GetLastError();
		}
	}

	HTTP_VERSION_INFO httpver = { 1, 1 };
	if (!InternetSetOption(Connection.get(), INTERNET_OPTION_HTTP_VERSION, &httpver, sizeof httpver))
		return GetLastError();

	internet_ptr Request(HttpOpenRequest(Connection.get(), L"GET", Url, L"HTTP/1.1", nullptr, nullptr, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 1));
	if (!Request)
		return GetLastError();

	if (!HttpSendRequest(Request.get(), nullptr, 0, nullptr, 0))
		return GetLastError();

	handle File(normalize_handle(CreateFile(FileName, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr)));
	if (!File)
		return GetLastError();

	DWORD Status;
	DWORD StatusSize = sizeof(Status);
	if (!HttpQueryInfo(Request.get(), HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &Status, &StatusSize, nullptr))
		return GetLastError();

	switch(Status)
	{
	case 200:
		break;
	case 404:
		return ERROR_INTERNET_ITEM_NOT_FOUND;
	case 500:
	default:
		return ERROR_GEN_FAILURE;
	}

	DWORD Size;
	DWORD SizeSize = sizeof(Size);
	if (!HttpQueryInfo(Request.get(), HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &Size, &SizeSize, nullptr))
		return GetLastError();

	if (Size == GetFileSize(File.get(), nullptr))
		return 0;

	SetEndOfFile(File.get());
	UINT BytesDone = 0;
	hide_cursor hc;
	while (InternetReadFile(Request.get(), Data, sizeof Data, &BytesRead) && BytesRead)
	{
		if (WaitForSingleObject(m_ExitEvent.get(), 0) == WAIT_OBJECT_0)
			return ERROR_CANCELLED;

		DWORD Written = 0;
		if (!WriteFile(File.get(), Data, BytesRead, &Written, nullptr) || Written != BytesRead)
		{
			return GetLastError();
		}
		BytesDone += BytesRead;
		if (Callback && Size)
		{
			Callback(BytesDone * 100 / Size);
		}
	}

	if (Callback)
		Callback(0);

	return 0;
}

bool update_plugin::DownloadFile(bool Self, const wchar_t* RemoteFile, const wchar_t* LocalName, bool UseProxy, bool UseCallback, bool DownloadAlways) const
{
	const auto FileName = LocalName? LocalName : FSF.PointToName(RemoteFile);
	const auto LocalFile = std::wstring(ipc.TempDirectory) + FileName;
	if (DownloadAlways && GetFileAttributes(LocalFile.data()) != INVALID_FILE_ATTRIBUTES)
	{
		DeleteFile(LocalFile.data());
	}

	DWORD DownloadResult;
	for (;;)
	{
		DownloadResult = WinInetDownload(Self? SelfRemoteSrv : FarRemoteSrv, RemoteFile, LocalFile.data(), UseProxy, UseCallback? DownloadProc : nullptr);
		if (DownloadResult == ERROR_SUCCESS)
			break;

		if (!UseCallback)
			break;

		auto Message = L" Error downloading "s + FileName + L":\n  "s + GetLastErrorMessage(DownloadResult) + L"\n"s;
		CallbackHandler(error, Message.data());
		if (!CallbackHandler(retry_confirmation, L""))
			break;
	}

	return DownloadResult == ERROR_SUCCESS;
}

update_status update_plugin::CheckUpdates(bool Self, bool Manual) const
{
	CreateDirectory(ipc.TempDirectory, nullptr);

	const auto Url = Self?
		SelfRemotePath + L"/"s + SelfUpdateFile :
		FarRemotePath + L"/"s + FarUpdateFile + phpRequest;

	if (!DownloadFile(Self, Url.data(), Self? SelfUpdateFile : FarUpdateFile, m_UseProxy, Manual, true))
		return update_status::S_CANTCONNECT;

	const auto Version = GetPrivateProfileString(L"info", L"version", L"", Self? ipc.SelfUpdateList : ipc.FarUpdateList);
	if (Version != strVer)
	{
		return update_status::S_INCOMPATIBLE;
	}
	if (NeedUpdate(Self))
	{
		return update_status::S_REQUIRED;
	}

	return update_status::S_UPTODATE;
}

bool update_plugin::DownloadUpdates(bool Self, bool Silent)
{
	std::wstring Url, File[2];
	if (Self)
	{
		File[0] = GetPrivateProfileString(SelfSection, L"arc", L"", ipc.SelfUpdateList);
		File[1] = GetPrivateProfileString(SelfSection, L"pdb", L"", ipc.FarUpdateList);
	}
	else
	{
		File[0]  =GetPrivateProfileString(FarSection, ipc.UseMsi? L"msi" : L"arc", L"", ipc.FarUpdateList);
		File[1] = GetPrivateProfileString(FarSection, L"pdb", L"", ipc.FarUpdateList);
	}

	for (size_t i = 0; i != ARRAYSIZE(File); ++i)
	{
		if (!Silent)
		{
			mprintf(L"%-60s", (MSG(MLoad) + L" "s + (Self? L"Update"s : L"Far"s) + (!i? L""s : L" pdb"s)).data());
		}
		Url = (Self? SelfRemotePath : FarRemotePath) + L"/"s + File[i];
		if (DownloadFile(Self, Url.data(), nullptr, m_UseProxy, !Silent, false))
		{
			if (!Silent)
			{
				mprintf(color::green, L"OK\n");
			}
			NeedRestart = true;
		}
		else
		{
			if (!Silent)
			{
				mprintf(color::red, L"download error %d\n", GetLastError());
			}
			return false;
		}
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
	GetExitCodeProcess(m_RunDll.get(), &RunDllExitCode);
	if (RunDllExitCode == STILL_ACTIVE)
	{
		if (!Silent)
		{
			mprintf(color::yellow, L"\n%s\n", MSG(MExitFAR));
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
		FSF.sprintf(cmdline, L"%s\\%s\\rundll32.exe \"%s\", RestartFAR %I64d %I64d", WinDir, ifn.IsWow64Process(GetCurrentProcess(), &IsWow64) && IsWow64? L"SysWOW64" : L"System32", ipc.PluginModule, reinterpret_cast<INT64>(ProcDup), reinterpret_cast<INT64>(&ipc));

		STARTUPINFO si = { sizeof si };
		PROCESS_INFORMATION pi;

		if (CreateProcess(nullptr, cmdline, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi))
		{
			m_RunDll.reset(pi.hProcess);
			CloseHandle(pi.hThread);
			if (!Silent)
			{
				mprintf(color::yellow, L"\n%s\n", MSG(MExitFAR));
			}
			else
			{
				handle hEvent(CreateEvent(nullptr, FALSE, FALSE, nullptr));
				EventStruct es = { E_DOWNLOADED, hEvent.get(), Self };
				PsInfo.AdvControl(&MainGuid, ACTL_SYNCHRO, 0, &es);
				WaitForSingleObject(hEvent.get(), INFINITE);
			}
		}
		else
		{
			if (!Silent)
			{
				mprintf(color::red, L"%s - error %d\n", MSG(MCantCreateProcess), GetLastError());
			}
		}

	}
	else
	{
		if (!Silent)
		{
			mprintf(color::white, L"%s\n", MSG(MDone));
		}
	}
}

void update_plugin::ThreadProc()
{
	while (WaitForSingleObject(m_ExitEvent.get(), 0) != WAIT_OBJECT_0)
	{
		{
			HANDLE Handles[] = { m_ExitEvent.get(), m_SingletonEvent.get() };
			if (WaitForMultipleObjects(ARRAYSIZE(Handles), Handles, false, INFINITE) == WAIT_OBJECT_0)
				return;
		}

		HANDLE Handles[] = { m_ExitEvent.get(), m_WaitEvent.get() };
		const auto Result = WaitForMultipleObjects(ARRAYSIZE(Handles), Handles, false, m_Period * 60 * 60 * 1000);
		if (Result == WAIT_OBJECT_0)
			return;

		if (Result == WAIT_OBJECT_0 + 1)
		{
			//for(int i=0;i<2;i++)
			int i = 1;
			{
				switch (CheckUpdates(!i, false))
				{
				case update_status::S_REQUIRED:
					{
						ResetEvent(m_SingletonEvent.get());
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
						SetEvent(m_SingletonEvent.get());
					}
					break;

				case update_status::S_CANTCONNECT:
					{
						Clean();
					}
					break;

				case update_status::S_INCOMPATIBLE:
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
	GetPrivateProfileString(L"connect", L"srv", L"", ProxyName, ARRAYSIZE(ProxyName), ipc.Config);
	GetPrivateProfileString(L"connect", L"user", L"", ProxyUser, ARRAYSIZE(ProxyUser), ipc.Config);
	GetPrivateProfileString(L"connect", L"pass", L"", ProxyPass, ARRAYSIZE(ProxyPass), ipc.Config);
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
	console console;

	if (WaitForSingleObject(m_SingletonEvent.get(), 0) == WAIT_TIMEOUT)
	{
		mprintf(color::white, L"%s\n", MSG(MInProgress));
		return;
	}
	ResetEvent(m_SingletonEvent.get());

	//for(int i=0;i<2;i++)
	int i = 1;
	{
		mprintf(L"%-60s", (MSG(MChecking) + L" "s + (!i? L"Update"s : L"Far"s)).data());

		const auto UpdateStatus = CheckUpdates(!i, true);
		switch (UpdateStatus)
		{
		case update_status::S_UPTODATE:
		case update_status::S_REQUIRED:
			{
				mprintf(color::green, L"OK\n");
				DWORD NewMajor, NewMinor, NewBuild;
				wchar_t Str[128];

				std::wstring MsgUptodate;
				std::vector<const wchar_t*> Items;
				if (UpdateStatus == update_status::S_REQUIRED)
				{
					GetNewModuleVersion(!i, Str, NewMajor, NewMinor, NewBuild);
					Items =
					{
						MSG(MName),
						MSG(MAvailableUpdates),
						L"\x1",
						Str,
						L"\x1",
						MSG(MAsk)
					};
				}
				else
				{
					MsgUptodate = MSG(!i? MPlugin : MFar) + L" "s + MSG(MUpToDate);
					Items =
					{
						MSG(MName),
						MsgUptodate.data(),
						MSG(MPluginsNote),
						MSG(MAsk)
					};
				}

				bool Download;
				{
					cursor_pos CursorPos;
					Download = !PsInfo.Message(&MainGuid, nullptr, FMSG_MB_YESNO | FMSG_LEFTALIGN, nullptr, Items.data(), Items.size(), 2);
				}
				if (Download)
				{
					if (DownloadUpdates(!i, false))
					{
						StartUpdate(!i, false);
					}
				}
				else
				{
					if (UpdateStatus == update_status::S_REQUIRED)
					{
						mprintf(color::yellow, L"%s\n", MSG(MCancelled));
					}
					Clean();
				}
			}
			break;

		case update_status::S_CANTCONNECT:
			{
				mprintf(color::red, L"Failed\n");
				mprintf(color::red, L"%s\n", MSG(MCantConnect));
			}
			break;

		case update_status::S_INCOMPATIBLE:
			{
				mprintf(color::green, L"OK\n");
				mprintf(color::red, L"%s\n", MSG(MIncompatible));
			}
			break;
		}
	}

	SetEvent(m_SingletonEvent.get());
}

bool Extract(const wchar_t* Arc, const wchar_t* Path, const wchar_t* DestDir)
{
	auto Result = false;
	if (!ipc.UseMsi)
	{
		mprintf(L"\n");
		Result = seven_zip_module_manager(Arc).extract(Path, DestDir, CallbackHandler);
	}
	else
	{
		STARTUPINFO si { sizeof si };
		PROCESS_INFORMATION pi;
		auto CmdLine = L"msiexec.exe /promptrestart /qb /i \""s + Path + L"\""s;
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

static void create_process_interactive(const std::wstring &Command, const wchar_t* Directory, bool Wait)
{
	STARTUPINFO si = { sizeof si };
	PROCESS_INFORMATION pi;

	for (;;)
	{
		if (CreateProcess(nullptr, const_cast<wchar_t*>(Command.data()), nullptr, nullptr, TRUE, 0, nullptr, Directory, &si, &pi))
		{
			mprintf(color::green, L"OK\n");
			if (Wait)
				WaitForSingleObject(pi.hProcess, INFINITE);
			break;
		}

		mprintf(color::red, L"Failed\n");
		const auto Message = L" Error starting "s + Command + L":\n  "s + GetLastErrorMessage(GetLastError()) + L"\n"s;
		mprintf(color::red, Message.data());
		if (!confirm_retry())
			break;
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}


extern "C" void WINAPI RestartFARW(HWND, HINSTANCE, const wchar_t* Cmd, DWORD)
{
	ifn.Load();
	int argc = 0;
	local_ptr<wchar_t*> argv(CommandLineToArgvW(Cmd, &argc));
	int n = 0;
	if (argc == 2 + n)
	{
		if (!ifn.AttachConsole(ATTACH_PARENT_PROCESS))
		{
			AllocConsole();
		}
		const auto ptr = std::wcstoull(argv.get()[n + 0], nullptr, 10);
		handle hFar(reinterpret_cast<HANDLE>(static_cast<uintptr_t>(ptr)));
		if (hFar)
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
			const auto IPCPtr = std::wcstoull(argv.get()[n + 1], nullptr, 10);
			if (ReadProcessMemory(hFar.get(), reinterpret_cast<const void*>(static_cast<uintptr_t>(IPCPtr)), &ipc, sizeof ipc, nullptr))
			{
				WaitForSingleObject(hFar.get(), INFINITE);

				hFar.reset();

				std::wstring File[2];
				File[0] = GetPrivateProfileString(L"far", ipc.UseMsi? L"msi" : L"arc", L"", ipc.Self? ipc.SelfUpdateList : ipc.FarUpdateList);
				File[1] = GetPrivateProfileString(L"far", L"pdb", L"", ipc.Self? ipc.SelfUpdateList : ipc.FarUpdateList);
				for(size_t i = 0; i != ARRAYSIZE(File); ++i)
				{
					if (!File[i].empty())
					{
						const auto local_file = ipc.TempDirectory + File[i];
						if (GetFileAttributes(local_file.data()) != INVALID_FILE_ATTRIBUTES)
						{
							for (;;)
							{
								mprintf(L"\n%-60s", (L"Processing "s + File[i]).data());
								const auto Result = Extract(ipc.SevenZip, local_file.data(), ipc.FarDirectory);
								mprintf(L"\n%-60s", File[i].data());
								mprintf(Result? color::green : color::red, Result? L"OK\n" : L"Failed\n");
								if (Result)
								{
									if (GetPrivateProfileInt(L"Update", L"Delete", 1, ipc.Config))
										DeleteFile(local_file.data());

									break;
								}

								if (!confirm_retry())
									break;
							}
						}
					}
				}

				const auto UserCommand = expand_strings(GetPrivateProfileString(L"events", L"PostInstall", L"", ipc.Config).data());
				if (!UserCommand.empty())
				{
					mprintf(color::white, L"\n%-60s", L"Starting user command");
					create_process_interactive(UserCommand, ipc.PluginDirectory, true);
				}
				/*
				if (Pos != -1)
				{
					mi.wID = SC_CLOSE;
					SetMenuItemInfo(hMenu, Pos, MF_BYPOSITION, &mi);
					DrawMenuBar(GetConsoleWindow());
				}
				*/

				const auto FarCommand = ipc.FarModule + (*ipc.FarParams? L" "s + ipc.FarParams : L""s);
				mprintf(color::white, L"\n%-60s", L"Starting Far");
				create_process_interactive(FarCommand, nullptr, false);
			}
		}
	}
	Clean();
}
