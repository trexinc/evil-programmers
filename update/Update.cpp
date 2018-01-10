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
#include "console.hpp"
#include "cursor_pos.hpp"
#include "hide_cursor.hpp"
#include "text_color.hpp"
#include "scope_exit.hpp"

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
	wchar_t SingletonName[MAX_PATH * 2];
	wchar_t UpdaterEventName[MAX_PATH * 2];
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

	std::wcscpy(ipc.FarDirectory, ipc.FarModule);
	*(std::wcsrchr(ipc.FarDirectory, L'\\') + 1) = 0;

	GetTempPath(ARRAYSIZE(ipc.TempDirectory), ipc.TempDirectory);
	std::wcscat(ipc.TempDirectory, L"FarUpdate\\");

	std::wcscpy(ipc.FarUpdateList, ipc.TempDirectory);
	std::wcscat(ipc.FarUpdateList, FarUpdateFile);

	std::wcscpy(ipc.SelfUpdateList, ipc.TempDirectory);
	std::wcscat(ipc.SelfUpdateList, SelfUpdateFile);

	std::wcscpy(ipc.PluginModule, PsInfo.ModuleName);

	std::wcscpy(ipc.PluginDirectory, ipc.PluginModule);
	*(std::wcsrchr(ipc.PluginDirectory, '\\') + 1) = 0;

	std::wcscpy(ipc.Config, ipc.PluginModule);
	std::wcscat(ipc.Config, L".config");

	std::wcscpy(ipc.SevenZip, ipc.PluginDirectory);
	std::wcscat(ipc.SevenZip, L"7zxr.dll");
}

auto GetPrivateProfileString(const wchar_t* AppName, const wchar_t* KeyName, const wchar_t* Default, const wchar_t* FileName)
{
	std::vector<wchar_t> Buffer(32768);
	DWORD size = ::GetPrivateProfileString(AppName, KeyName, Default, Buffer.data(), static_cast<DWORD>(Buffer.size()), FileName);
	return std::wstring(Buffer.data(), size);
}

using download_callback = void(*)(int);

auto GetProcessSpecificName(const std::wstring& BaseName)
{
	wchar_t ModuleName[MAX_PATH];
	GetModuleFileName(nullptr, ModuleName, ARRAYSIZE(ModuleName));
	auto ObjectName = BaseName + ModuleName;
	std::replace(ObjectName.begin(), ObjectName.end(), L'\\', L'/');
	return ObjectName;
}

auto GetSingleton()
{
	return handle(CreateMutex(nullptr, false, ipc.SingletonName));
}

auto GetUpdaterEvent()
{
	return handle(CreateEvent(nullptr, true, false, ipc.UpdaterEventName));
}

class update_plugin
{
public:
	NONCOPYABLE(update_plugin);

	update_plugin()
	{
		*ProxyName = 0;
		*ProxyUser = 0;
		*ProxyPass = 0;

		InitPaths();

		ReadSettings();

		m_ExitEvent.reset(CreateEvent(nullptr, TRUE, FALSE, nullptr));
		m_WaitEvent.reset(CreateEvent(nullptr, FALSE, TRUE, nullptr));

		std::wcscpy(ipc.SingletonName, GetProcessSpecificName(L"FarUpdateGuard_"s).data());
		std::wcscpy(ipc.UpdaterEventName, GetProcessSpecificName(L"FarUpdaterEvent_"s).data());

		m_Singleton = GetSingleton();

		if (m_Mode)
		{
			StartBackgroundJob();
		}
	}

	~update_plugin()
	{
		SetEvent(m_ExitEvent.get());
		WaitForSingleObject(m_Thread.get(), INFINITE);
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
	bool CheckAndQueueComponent(bool Self);
	update_status CheckUpdates(bool Self, bool Manual) const;
	bool DownloadUpdates(bool Self, bool Silent);
	bool DownloadFile(const wchar_t* ServerName, const wchar_t* RemoteFile, const wchar_t* LocalName, bool UseProxy, bool UseCallBack, bool DownloadAlways) const;
	DWORD WinInetDownload(const wchar_t* ServerName, const wchar_t* Url, const wchar_t* FileName, bool UseProxy, download_callback Callback) const;
	bool StartUpdate(bool Self, bool Silent);

	void ReadSettings();
	void ThreadProc();

	int m_Mode = 0;
	int m_Period = 0;
	bool m_UseProxy = false;
	handle m_Thread;
	handle m_ExitEvent;
	handle m_WaitEvent;
	handle m_Singleton;
	handle m_RunDll;
	handle m_CurrentProcess;

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

	INPUT_RECORD ir{};
	const auto& KeyEvent = ir.Event.KeyEvent;

	do
	{
		DWORD Read;
		if (!ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &Read) || Read != 1)
			return false;
	}
	while (!(ir.EventType == KEY_EVENT && !KeyEvent.bKeyDown && (KeyEvent.wVirtualKeyCode == L'Y' || KeyEvent.wVirtualKeyCode == L'N')));

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
	VersionInfo FarVersion {};
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
	const auto ProxyType = UseProxy?
		*ProxyName? INTERNET_OPEN_TYPE_PROXY : INTERNET_OPEN_TYPE_PRECONFIG :
		INTERNET_OPEN_TYPE_DIRECT;

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
			if (!InternetSetOption(Connection.get(), INTERNET_OPTION_PROXY_USERNAME, ProxyUser, static_cast<DWORD>(std::wcslen(ProxyUser))))
				return GetLastError();
		}
		if (*ProxyPass)
		{
			if (!InternetSetOption(Connection.get(), INTERNET_OPTION_PROXY_PASSWORD, ProxyPass, static_cast<DWORD>(std::wcslen(ProxyPass))))
				return GetLastError();
		}
	}

	HTTP_VERSION_INFO httpver { 1, 1 };
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

bool update_plugin::DownloadFile(const wchar_t* ServerName, const wchar_t* RemoteFile, const wchar_t* LocalName, bool UseProxy, bool UseCallback, bool DownloadAlways) const
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
		DownloadResult = WinInetDownload(ServerName, RemoteFile, LocalFile.data(), UseProxy, UseCallback? DownloadProc : nullptr);
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

	if (!DownloadFile(Self? SelfRemoteSrv : FarRemoteSrv, Url.data(), Self? SelfUpdateFile : FarUpdateFile, m_UseProxy, Manual, true))
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
	std::pair<std::wstring, std::wstring> SelfFiles[] =
	{
		{ L"Update", GetPrivateProfileString(SelfSection, L"arc", L"", ipc.SelfUpdateList) },
		{ L"Update pdb", GetPrivateProfileString(SelfSection, L"pdb", L"", ipc.FarUpdateList) },
	};

	std::pair<std::wstring, std::wstring> FarFiles[] =
	{
		{ L"Far", GetPrivateProfileString(FarSection, ipc.UseMsi? L"msi" : L"arc", L"", ipc.FarUpdateList) },
		{ L"Far pdb", GetPrivateProfileString(FarSection, L"pdb", L"", ipc.FarUpdateList) },
	};

	const auto RemoteServer = Self? SelfRemoteSrv : FarRemoteSrv;
	const auto RemotePath = Self? SelfRemotePath : FarRemotePath;

	const auto& DownloadUpdate = [&](const std::pair<std::wstring, std::wstring>& File)
	{
		if (!Silent)
		{
			mprintf(L"%-60s", (MSG(MLoad) + L" "s + File.first).data());
		}

		const auto Result = DownloadFile(RemoteServer, (RemotePath + L"/"s + File.second).data(), nullptr, m_UseProxy, !Silent, false);

		if (!Silent)
		{
			Result? mprintf(color::green, L"OK\n") : mprintf(color::red, L"download error %d\n", GetLastError());
		}
		
		return Result;
	};

	const auto& Files = Self? SelfFiles : FarFiles;
	return std::all_of(std::cbegin(Files), std::cend(Files), DownloadUpdate);
}

bool Clean()
{
	DeleteFile(ipc.SelfUpdateList);
	DeleteFile(ipc.FarUpdateList);
	RemoveDirectory(ipc.TempDirectory);
	return true;
}

bool update_plugin::StartUpdate(bool Self, bool Silent)
{
	if (!DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(), GetCurrentProcess(), &ptr_setter(m_CurrentProcess), 0, TRUE, DUPLICATE_SAME_ACCESS))
		return false;

	wchar_t cmdline[MAX_PATH];

	{
		int NumArgs = 0;
		const local_ptr<wchar_t*> Argv(CommandLineToArgvW(GetCommandLine(), &NumArgs));
		*ipc.FarParams = 0;
		for (int i = 1; i < NumArgs; i++)
		{
			std::wcscat(ipc.FarParams, Argv.get()[i]);
			if (i < NumArgs - 1)
				std::wcscat(ipc.FarParams, L" ");
		}
	}

	wchar_t WinDir[MAX_PATH];
	if (!GetWindowsDirectory(WinDir, ARRAYSIZE(WinDir)))
		return false;

	BOOL IsWow64 = FALSE;
	ipc.Self = Self;
	FSF.sprintf(cmdline, L"%s\\%s\\rundll32.exe \"%s\", RestartFAR %I64d %I64d", WinDir, ifn.IsWow64Process(GetCurrentProcess(), &IsWow64) && IsWow64? L"SysWOW64" : L"System32", ipc.PluginModule, reinterpret_cast<INT64>(m_CurrentProcess.get()), reinterpret_cast<INT64>(&ipc));

	STARTUPINFO si { sizeof si };
	PROCESS_INFORMATION pi;

	const auto UpdaterEvent = GetUpdaterEvent();
	ResetEvent(UpdaterEvent.get());

	if (!CreateProcess(nullptr, cmdline, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi))
	{
		if (!Silent)
		{
			mprintf(color::red, L"%s - error %d\n", MSG(MCantCreateProcess), GetLastError());
		}
		return false;
	}

	m_RunDll.reset(pi.hProcess);
	CloseHandle(pi.hThread);

	{
		HANDLE Events[] { UpdaterEvent.get(), m_ExitEvent.get(), m_RunDll.get() };
		if (WaitForMultipleObjects(ARRAYSIZE(Events), Events, false, INFINITE) != WAIT_OBJECT_0)
			return false;
	}

	// rundll32 will hold it now
	ReleaseMutex(m_Singleton.get());

	if (!Silent)
	{
		mprintf(color::yellow, L"\n%s\n", MSG(MExitFAR));
	}
	else
	{
		handle hEvent(CreateEvent(nullptr, FALSE, FALSE, nullptr));
		EventStruct es { E_DOWNLOADED, hEvent.get(), Self };
		PsInfo.AdvControl(&MainGuid, ACTL_SYNCHRO, 0, &es);

		HANDLE Handles[] { m_ExitEvent.get(), hEvent.get() };
		WaitForMultipleObjects(ARRAYSIZE(Handles), Handles, false, INFINITE);
	}
	return true;
}

void update_plugin::ThreadProc()
{
	for (;;)
	{
		const auto& Exiting = [this]
		{
			HANDLE Handles[] { m_ExitEvent.get(), m_WaitEvent.get() };
			return WaitForMultipleObjects(ARRAYSIZE(Handles), Handles, false, m_Period * 60 * 60 * 1000) == WAIT_OBJECT_0;
		};

		if (Exiting())
			return;

		const auto& Locked = [this]
		{
			HANDLE Handles[] { m_Singleton.get(), m_ExitEvent.get() };
			return WaitForMultipleObjects(ARRAYSIZE(Handles), Handles, false, INFINITE) == WAIT_OBJECT_0;
		};

		if (!Locked())
			continue;

		const auto& CheckComponentInBackground = [this](bool Self)
		{
			switch (CheckUpdates(Self, false))
			{
			case update_status::S_REQUIRED:
				{
					auto Load = m_Mode == 2;
					if (!Load)
					{
						const handle hEvent(CreateEvent(nullptr, FALSE, FALSE, nullptr));
						EventStruct es { E_ASKLOAD, hEvent.get(), Self, &Load };
						PsInfo.AdvControl(&MainGuid, ACTL_SYNCHRO, 0, &es);

						HANDLE Handles[]{ m_ExitEvent.get(), hEvent.get() };
						WaitForMultipleObjects(ARRAYSIZE(Handles), Handles, false, INFINITE);
					}

					if (Load && DownloadUpdates(Self, true) && StartUpdate(Self, true))
						return true;

					Clean();
					return false;
				}

			case update_status::S_CANTCONNECT:
			case update_status::S_INCOMPATIBLE:
			case update_status::S_UPTODATE:
			default:
				Clean();
				return false;
			}
		};

		if (!(/*CheckComponentInBackground(true) && */ CheckComponentInBackground(false)))
			ReleaseMutex(m_Singleton.get());
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

	switch (WaitForSingleObject(m_Singleton.get(), 0))
	{
	case WAIT_TIMEOUT:
		if (m_RunDll && WaitForSingleObject(m_RunDll.get(), 0) == WAIT_TIMEOUT)
			mprintf(color::yellow, L"%s\n", MSG(MExitFAR));
		else
			mprintf(color::white, L"%s\n", MSG(MInProgress));

		return;

	case WAIT_OBJECT_0:
		if (!(/*CheckAndQueueComponent(true) && */ CheckAndQueueComponent(false)))
			ReleaseMutex(m_Singleton.get());
		return;

	default:
		return;
	}
}

bool update_plugin::CheckAndQueueComponent(bool Self)
{
	mprintf(L"%-60s", (MSG(MChecking) + L" "s + (Self? L"Update"s : L"Far"s)).data());

	const auto UpdateStatus = CheckUpdates(Self, true);
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
				GetNewModuleVersion(Self, Str, NewMajor, NewMinor, NewBuild);
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
				MsgUptodate = MSG(Self? MPlugin : MFar) + L" "s + MSG(MUpToDate);
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
				if (DownloadUpdates(Self, false) && StartUpdate(Self, false))
					return true;
			}
			else
			{
				if (UpdateStatus == update_status::S_REQUIRED)
				{
					mprintf(color::yellow, L"%s\n", MSG(MCancelled));
				}
			}

			Clean();
			return false;
		}

	case update_status::S_CANTCONNECT:
		mprintf(color::red, L"Failed\n");
		mprintf(color::red, L"%s\n", MSG(MCantConnect));
		Clean();
		return false;

	case update_status::S_INCOMPATIBLE:
		mprintf(color::green, L"OK\n");
		mprintf(color::red, L"%s\n", MSG(MIncompatible));
		Clean();
		return false;

	default:
		Clean();
		return false;
	}
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
				const wchar_t* Items[] { MSG(MName), MSG(MAvailableUpdates), L"\x1", Str, L"\x1", MSG(MAsk) };
				if (!PsInfo.Message(&MainGuid, nullptr, FMSG_MB_YESNO | FMSG_LEFTALIGN, nullptr, Items, ARRAYSIZE(Items), 2))
				{
					*es->Result = true;
				}
				SetEvent(reinterpret_cast<HANDLE>(es->Data));
			}
			break;

			case E_DOWNLOADED:
			{
				const wchar_t* Items[] { MSG(MName), MSG(MUpdatesDownloaded), MSG(MExitFAR) };
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
	STARTUPINFO si { sizeof si };
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

static bool ExtractOneFile(const std::wstring& File)
{
	if (File.empty())
		return true;

	const auto local_file = ipc.TempDirectory + File;
	if (GetFileAttributes(local_file.data()) == INVALID_FILE_ATTRIBUTES)
		return false;

	for (;;)
	{
		mprintf(L"\n%-60s", (L"Processing "s + File).data());
		const auto Extracted = Extract(ipc.SevenZip, local_file.data(), ipc.FarDirectory);
		mprintf(L"\n%-60s", File.data());
		mprintf(Extracted? color::green : color::red, Extracted? L"OK\n" : L"Failed\n");
		if (Extracted)
		{
			if (GetPrivateProfileInt(L"Update", L"Delete", 1, ipc.Config))
				DeleteFile(local_file.data());

			return true;
		}

		if (!confirm_retry())
			return false;
	}
}

extern "C" void WINAPI RestartFARW(HWND, HINSTANCE, const wchar_t* Cmd, DWORD)
{
	ifn.Load();

	int argc = 0;
	local_ptr<wchar_t*> argv(CommandLineToArgvW(Cmd, &argc));

	int n = 0;
	if (argc != 2 + n)
		return;

	if (!ifn.AttachConsole(ATTACH_PARENT_PROCESS))
	{
		AllocConsole();
	}

	const auto ptr = std::wcstoull(argv.get()[n + 0], nullptr, 10);

	handle hFar(reinterpret_cast<HANDLE>(static_cast<uintptr_t>(ptr)));
	if (!hFar)
		return;

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

	MENUITEMINFO mi { sizeof mi, MIIM_ID, 0, 0, 0 };
	if (Pos != -1)
	{
		SetMenuItemInfo(hMenu, Pos, MF_BYPOSITION, &mi);
	}
	*/

	const auto IPCPtr = std::wcstoull(argv.get()[n + 1], nullptr, 10);
	if (!ReadProcessMemory(hFar.get(), reinterpret_cast<const void*>(static_cast<uintptr_t>(IPCPtr)), &ipc, sizeof ipc, nullptr))
		return;

	auto Singleton = GetSingleton();
	if (!Singleton)
		return;

	scope_exit_t ScopeExit([&]
	{
		Clean();
		ReleaseMutex(Singleton.get());
	});

	{
		const auto UpdaterEvent = GetUpdaterEvent();
		if (!UpdaterEvent)
			return;
		SetEvent(UpdaterEvent.get());
	}

	if (WaitForSingleObject(Singleton.get(), INFINITE) != WAIT_OBJECT_0)
		return;

	if (WaitForSingleObject(hFar.get(), INFINITE) != WAIT_OBJECT_0)
		return;

	hFar.reset();

	const std::wstring Files[] =
	{
		GetPrivateProfileString(L"far", ipc.UseMsi? L"msi" : L"arc", L"", ipc.Self? ipc.SelfUpdateList : ipc.FarUpdateList),
		GetPrivateProfileString(L"far", L"pdb", L"", ipc.Self? ipc.SelfUpdateList : ipc.FarUpdateList),
	};

	if (!std::all_of(std::cbegin(Files), std::cend(Files), ExtractOneFile))
		return;

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
