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
#define UNICODE
#define _UNICODE
#define _WIN32_IE 0x0600

#include <windows.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <wchar.h>
#include "bcCommon.h"
#include "bci.h"

///========================================================================================== define
enum puMenuCommands {
	cInfo = 1,
	cStop,
	cPause,
	cExit
};

const wchar_t *puMenuMsg[] = {
	L"",
	L"Информация",
	L"Отменить",
	L"Пауза/Продолжить",
	L"Выход"
};

const wchar_t *OutMsg[] = {
//	L"Invalid",
	L"Копирование",
	L"Перенос",
	L"Стирание",
	L"Удаление",
	L"Установка атрибутов"
};

const wchar_t *AskMsg[] = {
	L"Внимание",
	L"Такой файл уже существует",
	L"ASKGROUP_RETRY",
	L"ASKGROUP_RETRYONLY",
	L"ASKGROUP_LINK"
};

/// ============================================================================================ var
//#define WM_TASKICON (WM_USER+27)

WORD a[16];//={0,0,0,0,0xffff,0,0,0,0,0,0xffff,0,0,0,0,0xffff};
WORD x[16];//={0,0,0,0,0,     0,0,0,0,0,0,     0,0,0,0,0};

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
	{25, 18, 4, 9, 19}
};

BYTE t[5][4] = {
	{0x30, 0x40, 0x40, 0x30},
	{0x44, 0x7c, 0x54, 0x44},
	{0x44, 0x54, 0x54, 0x28},
	{0x70, 0x48, 0x48, 0x70},
	{0x30, 0x48, 0x78, 0x48}
};

PCWSTR			WindowClass = L"BCI2Class";
PCWSTR			WindowName = L"BCI2";
PCWSTR			EventName = L"Global\\BCI2";
PCWSTR			WavFile = L"";

HWND			hWnd = 0;
HANDLE			hEvent = 0;
HMENU			puMenu = 0;

UINT const		WM_TASKICON = WM_USER + 27;
UINT 			uiTIMER = 500;

size_t			TimeOut = 5 * 1000;		// play sound timeout
bool			ColorWhite = false;		// icon color
bool			bSendExit = false;
bool			bNoBalloon = false;

void TrimCopy(wchar_t* buf, size_t bufsize, const wchar_t* filepath) {
	const wchar_t* dots = L"...";
	const size_t dots_len = 3;
	size_t filepath_len = wcslen(filepath);
	if (filepath_len < bufsize || filepath_len == 0) {
		wcscpy(buf, filepath);
		return;
	}

	if (bufsize < (dots_len + 1)) {    // "...\0"
		if (bufsize > 1)
			*buf++ = '*';
		if (bufsize)
			*buf++ = 0;
		return;
	}
	--bufsize;

	// looking fora prefix, such as "D:\", "\\?\D:\", "\\Host\"
	int prefix_len = 0;
	if (filepath[0] == '\\' && filepath[1] == '\\') {
		prefix_len += 2;
		if (filepath[2] == '?' && filepath[3] == '\\')
			prefix_len += 2;
	}
	while (filepath[prefix_len] != 0 && filepath[prefix_len] != '\\')
		++prefix_len;
	++prefix_len;

	// search for a filename
	const wchar_t* p = filepath + filepath_len - 1;
	while (p > filepath && *p != '\\')
		--p;
	int filename_len = filepath + filepath_len - p;

	// check if <prefix> + "..." + <at least filename of filepath> is fit to the buffer
	if ((prefix_len + dots_len + filename_len) >= bufsize) {
		// overrun: don't use prefix
		memcpy(buf, dots, dots_len * sizeof(wchar_t));
		buf += dots_len;
		bufsize -= dots_len;
	} else {
		memcpy(buf, filepath, prefix_len * sizeof(wchar_t));
		buf += prefix_len;
		memcpy(buf, dots, dots_len * sizeof(wchar_t));
		buf += dots_len;
		bufsize -= prefix_len + dots_len;

		filepath += prefix_len;
		filepath_len -= prefix_len;
	}

	filepath += filepath_len;
	if (filepath_len > bufsize)
		filepath_len = bufsize;
	memcpy(buf, filepath - filepath_len, bufsize * sizeof(wchar_t));
	buf[bufsize] = 0; // was "--bufsize"ed to fit a zero
}

void FormatHint(TCHAR *buf, size_t bufsize, const TCHAR *action, const TCHAR *filename, DWORD pr_value) {
	int len = _snwprintf(buf, bufsize, L"%s - %d%%\n", action, pr_value);
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
						InfoList = (SmallInfoRec*)MemAlloc(sizeof(SmallInfoRec) * size);
						if ((!(InfoList)) || (!::ReadFile(hPipe, InfoList, sizeof(struct SmallInfoRec)*(size), &dwBytesRead, NULL))) {
							MemFree(InfoList);
							InfoList = NULL;
							size = 0;
							Result = false;
						}
					}
				}
			::CloseHandle(hPipe);
		}
		return Result;
	}
	void		FreeList(SmallInfoRec* &InfoList) {
		MemFree(InfoList);
	}

	bool			GetInfo(InfoRec *receive, DWORD ThreadId) {
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
		return Result;
	}
}

///=================================================================================== TaskBar Icons
size_t const	MAX_ICONS = 64;
class TbIcons {
	size_t		Size;
	IconItem	IconList[MAX_ICONS+1];

	bool		ShowBalloon(UINT uID, PCWSTR szInfoTitle, PCWSTR szInfo) {
		NOTIFYICONDATA tnid;
		MemZero(tnid);
		tnid.cbSize = sizeof(NOTIFYICONDATA);
		tnid.hWnd = hWnd;
		tnid.uID = uID;
		tnid.uVersion = NOTIFYICON_VERSION;
		tnid.uFlags = NIF_INFO;
		tnid.dwInfoFlags = NIIF_INFO;
		lstrcpyn(tnid.szInfoTitle, szInfoTitle, sizeofa(tnid.szInfoTitle) - 1);
		lstrcpyn(tnid.szInfo, szInfo, sizeofa(tnid.szInfo) - 1);
		return ::Shell_NotifyIcon(NIM_MODIFY, &tnid);
	}
	bool		TrayRefresh(UINT uID, HICON hIcon, PCWSTR szTip) {
		bool	Result = false;
		static HICON hPrevIcon = NULL;
		if (hPrevIcon)
			::DestroyIcon(hPrevIcon);
		hPrevIcon = hIcon;

		NOTIFYICONDATA tnid;
		MemZero(tnid);
		tnid.cbSize = sizeof(NOTIFYICONDATA);
		tnid.hWnd = hWnd;
		tnid.uID = uID;
		if (hIcon) {
			tnid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
			tnid.uCallbackMessage = WM_TASKICON;
			tnid.hIcon = hIcon;
			lstrcpyn(tnid.szTip, szTip, sizeofa(tnid.szTip) - 1);
			lstrcpyn(tnid.szInfoTitle, L"", sizeofa(tnid.szInfoTitle) - 1);
			lstrcpyn(tnid.szInfo, L"", sizeofa(tnid.szInfo) - 1);
			Result = Shell_NotifyIcon(NIM_MODIFY, &tnid);
			if (!Result)
				Result = Shell_NotifyIcon(NIM_ADD, &tnid);
		}
		return Result;
	}
	bool		TrayDelete(UINT id) {
		NOTIFYICONDATA nid;
		MemZero(nid);
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = hWnd;
		nid.uID = id;
		return	::Shell_NotifyIcon(NIM_DELETE, &nid);
	}

	void		Refresh(const SmallInfoRec &InfoItem, size_t i) {
		TCHAR	szTip[128] = {0};
		BYTE percent = InfoItem.percent + 128 * InfoItem.pause;
		if ((IconList[i].pr != percent)) {
			IconList[i].pr = percent;
			MemZero(x, sizeof(x));
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
			if (ColorWhite)
				for (size_t k = 0; k < 16; ++k)
					x[k] = ~(a[k] | x[k]);

			FormatHint(szTip, sizeofa(szTip), OutMsg[InfoItem.type - 1], InfoItem.Src, InfoItem.percent);
			HBITMAP	ba = ::CreateBitmap(16, 16, 1, 1, a);
			HBITMAP	bx = ::CreateBitmap(16, 16, 1, 1, x);
			ICONINFO ii = {TRUE, 0, 0, ba, bx};
			HICON hIcon = ::CreateIconIndirect(&ii);
			::DeleteObject(ba);
			::DeleteObject(bx);
			TrayRefresh(IconList[i].id, hIcon, szTip);
			if (!bNoBalloon && InfoItem.Ask) {
				TCHAR	szInfo[3*MAX_PATH];
				InfoRec info;
				MemZero(info);
				bcopy::GetInfo(&info, InfoItem.ThreadId);
				_snwprintf(szInfo, sizeofa(szInfo), L"%s\n%s", AskMsg[InfoItem.Ask], info.Dest);
				ShowBalloon(IconList[i].id, AskMsg[0], szInfo);
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
				return i;
			}
		}
		return -1; //0xFFFFFFFF
	}
	void		Delete(size_t i) {
		TrayDelete(IconList[i].id);
		if ((::GetTickCount() - IconList[i].start) >= TimeOut && ::lstrcmp(WavFile, L"") != 0)
			::PlaySound(WavFile, NULL, SND_FILENAME | SND_NODEFAULT);
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
		for (size_t i = 0; i < InfoSize; ++i) {
			size_t	pos = Find(InfoList[i]);
			if (pos == (size_t)(-1))
				Create(InfoList[i]);
			else
				Refresh(InfoList[i], pos);
		}
		DeleteEnded(InfoList, InfoSize);
	}
	void		DeleteAll() {
		for (size_t i = 0; i < Size; ++i) {
			TrayDelete(IconList[i].id);
		}
		Size = 0;
	}
	size_t		GetSize() const {
		return Size;
	}
} Icons;

/// ================================================================================================
void			CreatePopUpMenu() {
	puMenu = ::CreatePopupMenu();
	::AppendMenu(puMenu, 0, cInfo, puMenuMsg[cInfo]);
	::AppendMenu(puMenu, MF_SEPARATOR, 0, NULL);
	::AppendMenu(puMenu, 0, cStop, puMenuMsg[cStop]);
	::AppendMenu(puMenu, 0, cPause, puMenuMsg[cPause]);
	::AppendMenu(puMenu, MF_SEPARATOR, 0, NULL);
	::AppendMenu(puMenu, 0, cExit, puMenuMsg[cExit]);
}
void			ShowInfo(DWORD id) {
	TCHAR szInfo[4*MAX_PATH] = {0};
	SmallInfoRec	*InfoList;
	DWORD			InfoSize;
	if (bcopy::GetList(InfoList, InfoSize)) {
		if (InfoSize) {
			for (size_t i = 0; i < InfoSize; ++i) {
				if (InfoList[i].ThreadId == id) {
					_snwprintf(szInfo, sizeofa(szInfo), L"%s\n\n%s\nв\n%s",
							   OutMsg[InfoList[i].type - 1],
							   InfoList[i].Src,
							   InfoList[i].DestDir);
					::MessageBox(NULL, szInfo, (PCWSTR)L"Информация", MB_ICONINFORMATION);
					break;
				}
			}
		}
		bcopy::FreeList(InfoList);
	}
}

/// ======================================================================================= CallBack
BOOL CALLBACK	WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_TASKICON:
			switch (lParam) {
				case WM_LBUTTONDOWN:
					bcopy::Command(INFOFLAG_PAUSE, wParam);
					break;
				case WM_RBUTTONDOWN:
					POINT cur;
					::GetCursorPos(&cur);
					::SetForegroundWindow(hWnd);
					switch (::TrackPopupMenu(puMenu, ::GetSystemMetrics(SM_MENUDROPALIGNMENT) | TPM_RETURNCMD | TPM_NONOTIFY, cur.x, cur.y, 0, hWnd, 0)) {
						case cInfo:
							ShowInfo(wParam);
							break;
						case cStop:
							bcopy::Command(INFOFLAG_STOP, wParam);
							break;
						case cPause:
							bcopy::Command(INFOFLAG_PAUSE, wParam);
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
				if (!InfoSize) {
					::KillTimer(hWnd, 0);
					::ResetEvent(hEvent);
				}
				Icons.Refresh(InfoList, InfoSize);
				bcopy::FreeList(InfoList);
			}
		}
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/// =========================================================================================== main
#ifdef __GNUC__
#ifdef __cplusplus
extern "C" {
#endif
	int WinMainCRTStartup(void);
#ifdef __cplusplus
};
int WinMainCRTStartup(void)
#endif
#else
int APIENTRY	wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
#endif
{
	a[4] = a[10] = a[15] = 0xffff;
	HINSTANCE hInstance = ::GetModuleHandleW(NULL);
	PCWSTR pCmdLine = ::GetCommandLine();

	int argc = 0;
	PWSTR* argv = ::CommandLineToArgvW(pCmdLine, &argc);
	for (int i = 1; i < argc; ++i) {
		if ((::lstrcmp(argv[i], L"/s") == 0) && i < (argc - 1)) {
			WavFile = argv[i + 1];
			continue;
		}
		if ((::lstrcmp(argv[i], L"/t") == 0) && i < (argc - 1)) {
			TimeOut = _wtoi(argv[i + 1]) * 1000;
			continue;
		}
		if ((::lstrcmp(argv[i], L"/w") == 0)) {
			ColorWhite = true;
			continue;
		}
		if ((::lstrcmp(argv[i], L"/r") == 0)) {
			bSendExit = true;
			continue;
		}
		if ((::lstrcmp(argv[i], L"/nb") == 0)) {
			bNoBalloon = true;
			continue;
		}
	}

	hWnd = ::FindWindow(NULL, WindowName);
	if (!hWnd && !bSendExit) {
		hEvent = ::CreateEvent(NULL, true, false, EventName);
		if (!hEvent)
			hEvent = ::OpenEvent(EVENT_MODIFY_STATE, false, EventName);
		if (!hEvent)
			return 1;

		CreatePopUpMenu();

		WNDCLASS wndClass;
		MemZero(wndClass);
		wndClass.lpfnWndProc = (WNDPROC)WndProc;
		wndClass.hInstance = hInstance;
		wndClass.lpszClassName = WindowClass;
		::RegisterClass(&wndClass);

		hWnd = ::CreateWindow(WindowClass, WindowName, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
		if (!hWnd)
			return 1;

		::SetTimer(hWnd, 0, uiTIMER, 0);

		MSG		msg;
		HANDLE	pHandles[] = {hEvent};
		DWORD	nCount = 1;
		bool	bRun = true;

		while (bRun) {
			DWORD reason = ::MsgWaitForMultipleObjects(nCount, pHandles, false, INFINITE, QS_ALLINPUT);
			if (reason == WAIT_OBJECT_0 + 0) {
				if (Icons.GetSize() == 0)
					::SetTimer(hWnd, 0, uiTIMER, 0);
				::ResetEvent(hEvent);
			} else if (reason == WAIT_OBJECT_0 + nCount) {
				while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
					if (msg.message == WM_QUIT || msg.message == WM_CLOSE)
						bRun = false;
					else {
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
				}
			}
		}
		Icons.DeleteAll();
		::KillTimer(hWnd, 0);
		if (hEvent)
			::CloseHandle(hEvent);
	} else if (bSendExit) {
		::PostMessage(hWnd, WM_CLOSE, 0, 0);
	}
	::LocalFree(argv); // do not replace
	::ExitProcess(0);
	return 0;
}
