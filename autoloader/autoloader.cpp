#define UNICODE
#define _UNICODE

#include <string>
#include <sstream>
#include <plugin.hpp>
#include <initguid.h>

// {47FF3D06-78C0-48DB-9D28-267A7C583D67}
DEFINE_GUID(MainGuid, 0x47ff3d06, 0x78c0, 0x48db, 0x9d, 0x28, 0x26, 0x7a, 0x7c, 0x58, 0x3d, 0x67);


static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=MAKEFARVERSION(1, 0, 0, 0, VS_RELEASE);
	Info->Guid=MainGuid;
	Info->Title=L"autoloader";
	Info->Description=L"autoloader";
	Info->Author=L"Me";
}

HANDLE Thread = nullptr;
HANDLE hExitEvent = nullptr;

class Exception:public std::exception
{
public:
	Exception(const std::wstring& reason):m_reason(reason), m_error(GetLastError()){}
	const std::wstring Get() const
	{
		std::wstringstream ss;
		ss << m_error;
		return m_reason + L", Error: " + ss.str();
	}
private:
	std::wstring m_reason;
	DWORD m_error;
};

DWORD WINAPI ThreadProc(void*)
{
	HANDLE hDir = INVALID_HANDLE_VALUE;
	HANDLE hChangeEvent = nullptr;

	try
	{
		static char Buffer[65536];
		std::wstring Dir(Info.ModuleName);
		Dir = Dir.substr(0, Dir.rfind(L'\\') + 1);
		hDir = CreateFile(Dir.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED|FILE_FLAG_BACKUP_SEMANTICS, nullptr);
		if (hDir == INVALID_HANDLE_VALUE)
		{
			throw Exception(L"Can't open directory for reading");
		}

		hChangeEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!hChangeEvent)
		{
			throw Exception(L"Can't create ChangeEvent");
		}

		hExitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!hExitEvent)
		{
			throw Exception(L"Can't create ExitEvent");
		}

		OVERLAPPED Overlapped;
		Overlapped.hEvent = hChangeEvent;

		for(;;)
		{
			DWORD n;
			if (ReadDirectoryChangesW(hDir, Buffer, sizeof(Buffer), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE, &n, &Overlapped, nullptr))
			{
				HANDLE Handles[] = {hChangeEvent, hExitEvent};
				if (WaitForMultipleObjects(ARRAYSIZE(Handles), Handles, FALSE, INFINITE) == WAIT_OBJECT_0)
				{
					DWORD BytesReturned;
					if (GetOverlappedResult(hDir, &Overlapped, &BytesReturned, FALSE) && BytesReturned)
					{
						auto NotifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(Buffer);
						for(;;)
						{
							if (WaitForSingleObject(hExitEvent, 0) != WAIT_TIMEOUT)
								break;

							auto IsDll = [&NotifyInfo]()->bool
							{
								return !wcsnicmp(NotifyInfo->FileName + NotifyInfo->FileNameLength/sizeof(wchar_t) - 4, L".dll", 4);
							};

							auto FullNameString(Dir + std::wstring(NotifyInfo->FileName, NotifyInfo->FileNameLength/sizeof(wchar_t)));
							void* FullName = const_cast<wchar_t*>(FullNameString.c_str());

							switch (NotifyInfo->Action)
							{
							case FILE_ACTION_ADDED:
							case FILE_ACTION_RENAMED_NEW_NAME:
								if (IsDll())
								{
									Sleep(1000);
									Info.PluginsControl(INVALID_HANDLE_VALUE, PCTL_LOADPLUGIN, PLT_PATH, FullName);
								}
								break;

							case FILE_ACTION_RENAMED_OLD_NAME:
								if (IsDll())
								{
									HANDLE Plugin = reinterpret_cast<HANDLE>(Info.PluginsControl(INVALID_HANDLE_VALUE, PCTL_FINDPLUGIN, PFM_MODULENAME, FullName));
									if (Plugin)
									{
										Info.PluginsControl(Plugin, PCTL_UNLOADPLUGIN, 0, nullptr);
									}
								}
								break;

							}

							if (!NotifyInfo->NextEntryOffset)
								break;

							NotifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<char*>(NotifyInfo) + NotifyInfo->NextEntryOffset);
						}
					}
				}
				else
					break;
			}
		}
	}

	catch (Exception& e)
	{
		// TODO
	}

	catch (std::exception& e)
	{
		// TODO
	}

	catch (...)
	{
		// TODO
	}

	if (hExitEvent)
	{
		CloseHandle(hExitEvent);
	}

	if (hChangeEvent)
	{
		CloseHandle(hChangeEvent);
	}

	if (hDir != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hDir);
	}
	
	return 0;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *PSInfo)
{
	Info=*PSInfo;
	FSF=*PSInfo->FSF;
	Thread = CreateThread(nullptr, 0, ThreadProc, nullptr, 0, nullptr);
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=PF_PRELOAD;
}

void WINAPI ExitFARW(const struct ExitInfo *Info)
{
	if (hExitEvent)
	{
		SetEvent(hExitEvent);
	}

	if (Thread)
	{
		WaitForSingleObject(Thread, INFINITE);
		CloseHandle(Thread);
	}
}
