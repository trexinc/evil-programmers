/**
	bci.cpp
	Copyright (C) 2005 WhiteDragon
	Copyright (C) 2009 DrKnS
	Copyright (C) 2009 Std
	Copyright (C) 2009 GrAnD

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
**/
#define DynamicLink

#include "win_def.h"
#include "bcCommon.h"
#include "bci.h"

#include <mmsystem.h>
#include <wchar.h>

///========================================================================================== define
#include "strings.h"

enum			puMenuCommands {
	cAbout = 1,
	cInfo,
	cStop,
	cPause,
	cExit,
};

/// ============================================================================================ var
//#define WM_TASKICON (WM_USER+27)

WORD a[16];//={0,0,0,0,0xffff,0,0,0,0,0,0xffff,0,0,0,0,0xffff};
WORD x[16];//={0,0,0,0,0,	 0,0,0,0,0,0,	 0,0,0,0,0};

BYTE n[11][5] = {
	{6, 9, 9, 9, 6},
	{2, 6, 2, 2, 7},
	{6, 9, 2, 4, 15},
	{6, 9, 2, 9, 6},
	{3, 5, 9, 15, 1},
	{15, 8, 14, 1, 14},
	{7, 8, 14, 9, 6},
	{15, 1, 2, 4, 8},
	{6, 9, 6, 9, 6},
	{6, 9, 7, 1, 14},
	{25, 18, 4, 9, 19},
};

BYTE t[5][4] = {
	{0x30, 0x40, 0x40, 0x30},
	{0x44, 0x7c, 0x54, 0x44},
	{0x44, 0x54, 0x54, 0x28},
	{0x70, 0x48, 0x48, 0x70},
	{0x30, 0x48, 0x78, 0x48},
};

PCWSTR			WindowClass = L"BCI2Class";
PCWSTR			WindowName = L"BCI2";
PCWSTR			EventName = L"Global\\BCI2";
PCWSTR			WavFile = L"";

#ifdef DynamicLink
namespace	DynLink {
	HINSTANCE		hNtDll = NULL;
	dl__snwprintf	my_snprintf = NULL;
	bool			InitLib() {
		hNtDll = ::LoadLibrary(L"ntdll.dll");
		if (hNtDll) {
			my_snprintf = (dl__snwprintf) ::GetProcAddress(hNtDll, "_snwprintf");
		} else {
			::MessageBox(NULL, L"Can`t load ntdll.dll", L"Critical error", MB_ICONERROR);
			ExitThread(::GetLastError());
			return	false;
		}
		return	true;
	}
	void			FreeLib() {
		::FreeLibrary(hNtDll);
	}
}
#endif

HWND			hWnd = NULL;
HANDLE			hEvent = NULL;
HMENU			puMenu = NULL;
LCID			lang = 1033;

UINT const		WM_TASKICON = WM_USER + 27;
UINT			uiTIMER = 500;

size_t			TimeOut = 60 * 1000;	// play sound timeout
DWORD			SpeakerFreq = 1000;		// speaker frequency
DWORD			SpeakerDur = 1000;		// speaker duration
bool			bColorWhite = false;	// icon color
bool			bSendExit = false;
bool			bNoBalloon = false;
bool			bEnglishForced = false;
bool			bShowAbout = false;

PCWSTR			*MsgMenu;
PCWSTR			*MsgOut;
PCWSTR			*MsgAsk;
PCWSTR			MsgInfo;
PCWSTR			MsgAboutTitle;
PCWSTR			MsgAbout;
PCWSTR			InfoTemplate;

/// ================================================================================================
void			TrimCopy(PWSTR buf, size_t bufsize, PCWSTR filepath) {
	PCWSTR			dots = L"...";
	const size_t	dots_len = WinStr::Len(dots);
	size_t			filepath_len = WinStr::Len(filepath);
	if (filepath_len < bufsize || filepath_len == 0) {
		WinStr::Copy(buf, filepath);
		return;
	}

	if (bufsize < (dots_len + 1)) { // "...\0"
		if (bufsize > 1)
			*buf++ = L'*';
		if (bufsize)
			*buf++ = 0;
		return;
	}
	--bufsize;

	// looking for a prefix, such as "D:\", "\\?\D:\", "\\Host\"
	int prefix_len = 0;
	if (filepath[0] == L'\\' && filepath[1] == L'\\') {
		prefix_len += 2;
		if (filepath[2] == L'?' && filepath[3] == L'\\')
			prefix_len += 2;
	}
	while (filepath[prefix_len] != 0 && filepath[prefix_len] != L'\\')
		++prefix_len;
	++prefix_len;

	// search for a filename
	PCWSTR		p = filepath + filepath_len - 1;
	while (p > filepath && *p != L'\\')
		--p;
	size_t filename_len = filepath + filepath_len - p;

	// check if <prefix> + "..." + <at least filename of filepath> is fit to the buffer
	if ((prefix_len + dots_len + filename_len) >= bufsize) {
		// overrun: don't use prefix
		WinMem::Copy(buf, dots, dots_len * sizeof(WCHAR));
		buf += dots_len;
		bufsize -= dots_len;
	} else {
		WinMem::Copy(buf, filepath, prefix_len * sizeof(WCHAR));
		buf += prefix_len;
		WinMem::Copy(buf, dots, dots_len * sizeof(WCHAR));
		buf += dots_len;
		bufsize -= prefix_len + dots_len;

		filepath += prefix_len;
		filepath_len -= prefix_len;
	}

	filepath += filepath_len;
	if (filepath_len > bufsize)
		filepath_len = bufsize;
	WinMem::Copy(buf, filepath - filepath_len, bufsize * sizeof(WCHAR));
	buf[bufsize] = 0; // was "--bufsize"ed to fit a zero
}

void			FormatHint(PWSTR buf, size_t bufsize, PCWSTR action, PCWSTR filename, DWORD pr_value) {
	int len = my_snwprintf(buf, bufsize, L"%s - %d%%\n", action, pr_value);
	if (len == -1)
		return;
	TrimCopy(buf + len, bufsize - len - 1, filename);
	buf[bufsize-1] = 0;
}

/// ========================================================================================== bcopy
namespace bcopy {
	void		Command(DWORD Command, DWORD ThreadId) {
		DWORD send[3] = {OPERATION_INFO, Command, ThreadId};
		HANDLE hPipe = ::CreateFile(PIPE_NAMEW, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hPipe != INVALID_HANDLE_VALUE) {
			DWORD dwBytesWritten;
			::WriteFile(hPipe, send, sizeof(send), &dwBytesWritten, NULL);
			::CloseHandle(hPipe);
		}
	}
	bool		GetList(SmallInfoRec* &InfoList, DWORD &size) {
		bool Result = false;
		size = 0;
		InfoList = NULL;
		DWORD send[2] = {OPERATION_INFO, INFOFLAG_ALL};
		DWORD dwBytesRead, dwBytesWritten, rec_size;
		HANDLE hPipe = ::CreateFile(PIPE_NAMEW, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hPipe != INVALID_HANDLE_VALUE) {
			if (::WriteFile(hPipe, send, sizeof(send), &dwBytesWritten, NULL))
				if (::ReadFile(hPipe, &size, sizeof(size), &dwBytesRead, NULL) &&
						::ReadFile(hPipe, &rec_size, sizeof(rec_size), &dwBytesRead, NULL)) {
					Result = true;
					if (size) {
						InfoList = (SmallInfoRec*)WinMem::Alloc(sizeof(SmallInfoRec) * size);
						if ((!(InfoList)) || (!::ReadFile(hPipe, InfoList, sizeof(SmallInfoRec)*(size), &dwBytesRead, NULL))) {
							WinMem::Free(InfoList);
							InfoList = NULL;
							size = 0;
							Result = false;
						}
					}
				}
			::CloseHandle(hPipe);
		}
		return	Result;
	}
	void		FreeList(SmallInfoRec* &InfoList) {
		WinMem::Free(InfoList);
		InfoList = NULL;
	}

	bool		GetInfo(InfoRec *receive, DWORD ThreadId) {
		bool Result = false;
		DWORD dwBytesRead, dwBytesWritten, dwSize;
		DWORD send[3] = {OPERATION_INFO, INFOFLAG_BYHANDLE, ThreadId};
		receive->info.type = INFOTYPE_INVALID;
		HANDLE hPipe = ::CreateFile(PIPE_NAMEW, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hPipe != INVALID_HANDLE_VALUE) {
			::WriteFile(hPipe, send, sizeof(send), &dwBytesWritten, NULL);
			if (::ReadFile(hPipe, &dwSize, sizeof(dwSize), &dwBytesRead, NULL))
				::ReadFile(hPipe, receive, sizeof(InfoRec), &dwBytesRead, NULL);
			::CloseHandle(hPipe);
		} else if (::GetLastError() == ERROR_PIPE_BUSY) {
			Result = true;
		}
		return	Result;
	}
}

///=================================================================================== TaskBar Icons
size_t const	MAX_ICONS = 64;
class TbIcons {
	size_t		Size;
	IconItem	IconList[MAX_ICONS+1];

	bool		ShowBalloon(UINT uID, PCWSTR szInfoTitle, PCWSTR szInfo) {
		NOTIFYICONDATA tnid;
		WinMem::Zero(tnid);
		tnid.cbSize = sizeof(NOTIFYICONDATA);
		tnid.hWnd = hWnd;
		tnid.uID = uID;
		tnid.uVersion = NOTIFYICON_VERSION;
		tnid.uFlags = NIF_INFO;
		tnid.dwInfoFlags = NIIF_INFO;
		WinStr::Copy(tnid.szInfoTitle, szInfoTitle, sizeofa(tnid.szInfoTitle) - 1);
		WinStr::Copy(tnid.szInfo, szInfo, sizeofa(tnid.szInfo) - 1);
		return	::Shell_NotifyIcon(NIM_MODIFY, &tnid) != 0;
	}
	bool		TrayRefresh(UINT uID, HICON hIcon, PCWSTR szTip) {
		bool	Result = false;
		static HICON hPrevIcon = NULL;
		if (hPrevIcon)
			::DestroyIcon(hPrevIcon);
		hPrevIcon = hIcon;

		NOTIFYICONDATA tnid;
		WinMem::Zero(tnid);
		tnid.cbSize = sizeof(NOTIFYICONDATA);
		tnid.hWnd = hWnd;
		tnid.uID = uID;
		if (hIcon) {
			tnid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
			tnid.uCallbackMessage = WM_TASKICON;
			tnid.hIcon = hIcon;
			WinStr::Copy(tnid.szTip, szTip, sizeofa(tnid.szTip) - 1);
			WinStr::Copy(tnid.szInfoTitle, L"", sizeofa(tnid.szInfoTitle) - 1);
			WinStr::Copy(tnid.szInfo, L"", sizeofa(tnid.szInfo) - 1);
			Result = ::Shell_NotifyIcon(NIM_MODIFY, &tnid) != 0;
			if (!Result)
				Result = ::Shell_NotifyIcon(NIM_ADD, &tnid) != 0;
		}
		return	Result;
	}
	bool		TrayDelete(UINT id) {
		NOTIFYICONDATA nid;
		WinMem::Zero(nid);
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = hWnd;
		nid.uID = id;
		return	::Shell_NotifyIcon(NIM_DELETE, &nid) != 0;
	}

	void		Refresh(const SmallInfoRec &InfoItem, size_t i) {
		WCHAR	szTip[128];
		WinMem::Zero(szTip, sizeofa(szTip));
		BYTE percent = (BYTE)(InfoItem.percent + 128 * InfoItem.pause);
		if ((IconList[i].pr != percent)) {
			IconList[i].pr = percent;
			WinMem::Zero(x, sizeof(x));
			x[12] = (0xffff >> (InfoItem.percent * 145 / 1000 + 1)) & 0x7ffe;
			XchgByte(x[12]);
			x[13] = x[12];
			a[0] = a[1] = a[2] = a[3] = 0xffff - 0x1b00 * InfoItem.pause;
			for (size_t k = 0; k < 4; ++k)
				a[k] -= t[InfoItem.type - 1][k];
			for (size_t k = 0; k < 5; ++k) {
				a[k+5] = 0xffff - n[InfoItem.percent / 10][k] * 0x1000 - n[InfoItem.percent % 10][k] * 0x80 - n[10][k];
				XchgByte(a[k+5]);
			}
			if (bColorWhite)
				for (size_t k = 0; k < 16; ++k)
					x[k] = ~(a[k] | x[k]);

			FormatHint(szTip, sizeofa(szTip), MsgOut[InfoItem.type - 1], InfoItem.Src, InfoItem.percent);
			HBITMAP	ba = ::CreateBitmap(16, 16, 1, 1, a);
			HBITMAP	bx = ::CreateBitmap(16, 16, 1, 1, x);
			ICONINFO ii = {TRUE, 0, 0, ba, bx};
			HICON hIcon = ::CreateIconIndirect(&ii);
			::DeleteObject(ba);
			::DeleteObject(bx);
			TrayRefresh(IconList[i].id, hIcon, szTip);
			if (!bNoBalloon && InfoItem.Ask) {
				WCHAR	szInfo[3*MAX_PATH];
				InfoRec info;
				WinMem::Zero(info);
				bcopy::GetInfo(&info, InfoItem.ThreadId);
				if (InfoItem.Ask == 2)
					my_snwprintf(szInfo, sizeofa(szInfo), L"%s\n%s", MsgAsk[InfoItem.Ask], info.Src);
				else
					my_snwprintf(szInfo, sizeofa(szInfo), L"%s\n%s", MsgAsk[InfoItem.Ask], info.Dest);
				ShowBalloon(IconList[i].id, MsgAsk[0], szInfo);
			}
		}
	}
	void		Create(const SmallInfoRec &InfoItem) {
		if (Size < MAX_ICONS) {
			IconItem ii(InfoItem.ThreadId, ::GetTickCount());
			IconList[Size] = ii;
			Refresh(InfoItem, Size++);
		}

	}
	size_t		Find(const SmallInfoRec &InfoItem) {
		for (size_t i = 0; i < Size; ++i) {
			if (IconList[i].id == InfoItem.ThreadId) {
				return	i;
			}
		}
		return	(size_t) - 1; //0xFFFFFFFF
	}
	void		Delete(size_t i) {
		TrayDelete(IconList[i].id);
		if ((::GetTickCount() - IconList[i].start) >= TimeOut) {
			if (!WinStr::Eqi(WavFile, L"")) {
				if (WinStr::Eqi(WavFile, L"beep")) {
					::Beep(SpeakerFreq, SpeakerDur);
				} else {
					::PlaySound(WavFile, NULL, SND_FILENAME | SND_NODEFAULT);
				}
			}
			/*
			::Beep(294, 1000 / 8);
			::Beep(440, 1000 / 4);
			::Beep(262*2, 1000 / 4);
			::Beep(330*2, 1000 / 4);
			::Beep(415, 1000 / 8);
			::Beep(440, 1000);
			*/
		}
		IconList[i] = IconList[--Size];
	}
	void		DeleteEnded(SmallInfoRec* InfoList, size_t InfoSize) {
		for (size_t i = 0; i < Size; ++i) {
			bool found = false;
			for (size_t j = 0; j < InfoSize; ++j)
				if (IconList[i].id == InfoList[j].ThreadId) {
					found = true;
					break;
				}
			if (!found) {
				Delete(i);
			}
		}
	}
public:
	TbIcons(): Size(0) {}
	void		Refresh(SmallInfoRec* InfoList, size_t InfoSize) {
		DeleteEnded(InfoList, InfoSize);
		for (size_t i = 0; i < InfoSize; ++i) {
			size_t	pos = Find(InfoList[i]);
			if (pos == (size_t)(-1))
				Create(InfoList[i]);
			else
				Refresh(InfoList[i], pos);
		}
	}
	void		DeleteAll() {
		for (size_t i = 0; i < Size; ++i) {
			TrayDelete(IconList[i].id);
		}
		Size = 0;
	}
	size_t		GetSize() const {
		return	Size;
	}
} Icons;

/// ================================================================================================
void			CreatePopUpMenu() {
	puMenu = ::CreatePopupMenu();
	::AppendMenu(puMenu, 0, cAbout, MsgMenu[cAbout]);
	::AppendMenu(puMenu, 0, cInfo, MsgMenu[cInfo]);
	::AppendMenu(puMenu, MF_SEPARATOR, 0, NULL);
	::AppendMenu(puMenu, 0, cStop, MsgMenu[cStop]);
	::AppendMenu(puMenu, 0, cPause, MsgMenu[cPause]);
	::AppendMenu(puMenu, MF_SEPARATOR, 0, NULL);
	::AppendMenu(puMenu, 0, cExit, MsgMenu[cExit]);
}
void			ShowAbout() {
	::MessageBox(NULL, MsgAbout, MsgAboutTitle, MB_ICONINFORMATION);
}
void			ShowInfo(DWORD id) {
	WCHAR	szInfo[5*MAX_PATH];
	WinMem::Zero(szInfo, sizeofa(szInfo));
	InfoRec info;
	WinMem::Zero(info);
	bcopy::GetInfo(&info, id);
	my_snwprintf(szInfo, sizeofa(szInfo), InfoTemplate,
				 MsgOut[info.info.type - 1],
				 info.Src,
				 info.info.Src,
				 info.info.DestDir);
	::MessageBox(NULL, szInfo, MsgInfo, MB_ICONINFORMATION);
}
void			ParseCommandLine(size_t argc, PWSTR argv[]) {
	for (size_t i = 1; i < argc; ++i) {
		if (WinStr::Eqi(argv[i], L"/s") && i < (argc - 1)) {
			WavFile = argv[i + 1];
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/t") && i < (argc - 1)) {
			TimeOut = WinStr::AsULong(argv[i + 1]) * 1000;
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/spf") && i < (argc - 1)) {
			SpeakerFreq = WinStr::AsULong(argv[i + 1]);
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/spd") && i < (argc - 1)) {
			SpeakerDur = WinStr::AsULong(argv[i + 1]);
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/w")) {
			bColorWhite = true;
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/r")) {
			bSendExit = true;
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/e")) {
			bEnglishForced = true;
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/nb")) {
			bNoBalloon = true;
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/?")) {
			bShowAbout = true;
			continue;
		}
	}
}

/// ======================================================================================= CallBack
BOOL CALLBACK	WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_TASKICON:
			switch (lParam) {
				case WM_LBUTTONDOWN:
					bcopy::Command(INFOFLAG_PAUSE, static_cast<DWORD>(wParam));
					break;
				case WM_RBUTTONDOWN:
					POINT	cur;
					::GetCursorPos(&cur);
					::SetForegroundWindow(hWnd);
					switch (::TrackPopupMenu(puMenu, ::GetSystemMetrics(SM_MENUDROPALIGNMENT) | TPM_RETURNCMD | TPM_NONOTIFY, cur.x, cur.y, 0, hWnd, 0)) {
						case cAbout:
							ShowAbout();
							break;
						case cInfo:
							ShowInfo(static_cast<DWORD>(wParam));
							break;
						case cStop:
							bcopy::Command(INFOFLAG_STOP, static_cast<DWORD>(wParam));
							break;
						case cPause:
							bcopy::Command(INFOFLAG_PAUSE, static_cast<DWORD>(wParam));
							break;
						case cExit:
							::PostQuitMessage(0);
					}
//					::PostMessage(hWnd, WM_NULL, 0, 0);
			}
			break;
		case WM_TIMER: {
			SmallInfoRec	*InfoList;
			DWORD			InfoSize;
			if (bcopy::GetList(InfoList, InfoSize)) {
				Icons.Refresh(InfoList, InfoSize);
				bcopy::FreeList(InfoList);
			}
			if (Icons.GetSize() == 0) {
				::KillTimer(hWnd, 0);
				::ResetEvent(hEvent);
			}
		}
	}
	return	::DefWindowProc(hWnd, uMsg, wParam, lParam) != 0;
}

/// =========================================================================================== main
int APIENTRY	wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int) {
	DynLink::InitLib();
	a[4] = a[10] = a[15] = 0xffff;

	int		argc = 0;
	PWSTR	*argv = ::CommandLineToArgvW(pCmdLine, &argc);
	ParseCommandLine(argc, argv);

// language setting
	MsgMenu	= MsgMenu1033;
	MsgOut	= MsgOut1033;
	MsgAsk	= MsgAsk1033;
	MsgInfo	= MsgInfo1033;
	MsgAboutTitle	= MsgAboutTitle1033;
	MsgAbout		= MsgAbout1033;
	InfoTemplate	= InfoTemplate1033;
	if (!bEnglishForced) {
		lang = ::GetUserDefaultLCID();
		if (lang == 1049) {
			MsgMenu	= MsgMenu1049;
			MsgOut	= MsgOut1049;
			MsgAsk	= MsgAsk1049;
			MsgInfo	= MsgInfo1049;
			MsgAboutTitle	= MsgAboutTitle1049;
			MsgAbout		= MsgAbout1049;
			InfoTemplate	= InfoTemplate1049;
		}
	}

	if (bShowAbout) {
		ShowAbout();
	} else {
		hWnd = ::FindWindow(NULL, WindowName);
		if (hWnd) {
			::PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
		if (!bSendExit) {
			hEvent = ::CreateEvent(NULL, true, false, EventName);
			if (!hEvent)
				hEvent = ::OpenEvent(EVENT_MODIFY_STATE, false, EventName);
			if (!hEvent)
				return	1;

			CreatePopUpMenu();
			if (puMenu == NULL)
				return	2;
			WNDCLASS wndClass;
			WinMem::Zero(wndClass);
			wndClass.lpfnWndProc = (WNDPROC)WndProc;
			wndClass.hInstance = hInstance;
			wndClass.lpszClassName = WindowClass;
			::RegisterClass(&wndClass);

			hWnd = ::CreateWindow(WindowClass, WindowName, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
			if (!hWnd)
				return	3;

			::SetTimer(hWnd, 0, uiTIMER, 0);

			MSG		msg;
			HANDLE	pHandles[] = {hEvent};
			DWORD	nCount = sizeofa(pHandles);
			bool	bNeedExit = false;
			while (!bNeedExit) {
				DWORD reason = ::MsgWaitForMultipleObjects(nCount, pHandles, false, INFINITE, QS_ALLINPUT);
				if (reason == WAIT_OBJECT_0 + 0) {
					if (Icons.GetSize() == 0)
						::SetTimer(hWnd, 0, uiTIMER, 0);
					::ResetEvent(hEvent);
				} else if (reason == WAIT_OBJECT_0 + nCount) {
					while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
						if (msg.message == WM_QUIT || msg.message == WM_CLOSE)
							bNeedExit = true;
						else {
							::TranslateMessage(&msg);
							::DispatchMessage(&msg);
						}
					}
				}
			}
			::KillTimer(hWnd, 0);
			Icons.DeleteAll();
			if (hEvent)
				::CloseHandle(hEvent);
			::DestroyWindow(hWnd);
			::DestroyMenu(puMenu);
		}
	}
	::LocalFree(argv); // do not replace
	DynLink::FreeLib();
	return	0;
}

/// ========================================================================== Startup (entry point)
extern "C" int	WinMainCRTStartup() {
	int		Result;
	STARTUPINFO StartupInfo = {sizeof(STARTUPINFO), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	::GetStartupInfo(&StartupInfo);

	Result = wWinMain(::GetModuleHandle(NULL), NULL, ::GetCommandLine(),
					  StartupInfo.dwFlags & STARTF_USESHOWWINDOW ? StartupInfo.wShowWindow : SW_SHOWDEFAULT);
	::ExitProcess(Result);
	return	Result;
}
