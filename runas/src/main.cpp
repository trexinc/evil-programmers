/**
	runas plugin for FAR Manager
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

#include "win_def.h"

#include <shlwapi.h>
#include <lm.h>
#include "plugin.hpp"
#include "CRT/crt.hpp"

///========================================================================================== define
#define MIN_FAR_VERMAJOR  2
#define MIN_FAR_VERMINOR  0
#define MIN_FAR_BUILD     0

extern "C" {
	WINADVAPI
	BOOL
	WINAPI
	IsTokenRestricted(HANDLE TokenHandle);

	WINADVAPI
	BOOL
	APIENTRY
	CreateRestrictedToken(
		HANDLE ExistingTokenHandle,
		DWORD Flags,
		DWORD DisableSidCount,
		PSID_AND_ATTRIBUTES SidsToDisable,
		DWORD DeletePrivilegeCount,
		PLUID_AND_ATTRIBUTES PrivilegesToDelete,
		DWORD RestrictedSidCount,
		PSID_AND_ATTRIBUTES SidsToRestrict,
		PHANDLE NewTokenHandle
	);
};

#define DISABLE_MAX_PRIVILEGE   0x1

///========================================================================================== struct
enum		{
	MTitle,
	DTitle,
	buttonOk,
	buttonCancel,
	MDialogTitle,
	MUsername,
	MPasword,
	MRestricted,
	MCommandLine,
	MError,
};

struct		InitDialogItem {
	unsigned char Type;
	unsigned char X1, Y1, X2, Y2;
	unsigned char Focus;
	DWORD_PTR Selected;
	unsigned int Flags;
	unsigned char DefaultButton;
	const TCHAR *Data;
};

///======================================================================================= implement
static PluginStartupInfo		Info;
static FarStandardFunctions 	FSF;
//PluginOptions					Options;

inline PCWSTR	GetMsg(int MsgId) {
	return	Info.GetMsg(Info.ModuleNumber, MsgId);
}
inline PCWSTR	GetDataPtr(HANDLE hDlg, size_t in) {
	return	(PCWSTR)Info.SendDlgMessage(hDlg, DM_GETCONSTTEXTPTR , in, 0);
}
inline bool		GetCheck(HANDLE hDlg, size_t in) {
	return	(bool)Info.SendDlgMessage(hDlg, DM_GETCHECK, in, 0);
}

void			InitDialogItems(struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber) {
	for (int i = 0; i < ItemsNumber; ++i) {
		Item[i].Type = Init[i].Type;
		Item[i].X1 = Init[i].X1;
		Item[i].Y1 = Init[i].Y1;
		Item[i].X2 = Init[i].X2;
		Item[i].Y2 = Init[i].Y2;
		Item[i].Focus = Init[i].Focus;
		Item[i].History = (const TCHAR *)Init[i].Selected;
		Item[i].Flags = Init[i].Flags;
		Item[i].DefaultButton = Init[i].DefaultButton;
		Item[i].MaxLen = 0;
		if ((DWORD_PTR)Init[i].Data < 2000)
			Item[i].PtrData = GetMsg((unsigned int)(DWORD_PTR)Init[i].Data);
		else
			Item[i].PtrData = Init[i].Data;
	}
}

CStrW			err2w(HRESULT in) {
	CStrW	Result(4096);
	PWSTR	buf = NULL;
	::FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		in,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(PWSTR)&buf, 0, NULL);
	Result = (buf) ? buf : L"Unknown error\r\n";
	::LocalFree(buf);
	size_t	len = Result.Len() - 2;
	if (len >= 0)
		Result[len] = L'\0';
	return	Result;
}

bool			InitUsers(FarList &users) {
	DWORD dwLevel = 3;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	NET_API_STATUS nStatus;

	USER_INFO_3 *info = NULL;
	nStatus = ::NetUserEnum(NULL, dwLevel,
							FILTER_NORMAL_ACCOUNT,
							(PBYTE*) & info,
							MAX_PREFERRED_LENGTH,
							&dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
	if (nStatus == NERR_Success) {
		users.ItemsNumber = 0;
		users.Items = (FarListItem*)WinMem::Alloc(sizeof(*users.Items) * dwEntriesRead);
		for (DWORD i = 0; i < dwEntriesRead; ++i) {
			if (!WinFlag<DWORD>::Check(info[i].usri3_flags, UF_ACCOUNTDISABLE)) {
				WinStr::Assign(users.Items[users.ItemsNumber].Text, info[i].usri3_name);
				if (info[i].usri3_priv == USER_PRIV_ADMIN) {
					WinFlag<DWORD>::Set(users.Items[users.ItemsNumber].Flags, LIF_CHECKED);
				}
				++users.ItemsNumber;
			}
		}
		::NetApiBufferFree(info);
	}
	return	nStatus == NERR_Success;
}
bool			FreeUsers(FarList &users) {
	for (int i = 0; i < users.ItemsNumber; ++i) {
		WinStr::Free(users.Items[i].Text);
	}
	WinMem::Free(users.Items);
	return	true;
}

HRESULT			ExecAsUser(PCWSTR app, PCWSTR user, PCWSTR pass) {
	CStrW	cmd(MAX_PATH_LENGTH + MAX_PATH + 1);
	WinFS::Expand(app, cmd);

	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);
//	si.wShowWindow = SW_HIDE;

	if (::CreateProcessWithLogonW(user, NULL, pass, LOGON_WITH_PROFILE, NULL, cmd,
								  CREATE_UNICODE_ENVIRONMENT | CREATE_DEFAULT_ERROR_MODE,
								  NULL, NULL, &si, &pi)) {
		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);
		return	NO_ERROR;
	}
	return	::GetLastError();
}
HRESULT			ExecRestricted(PCWSTR app) {
	CStrW	cmd(MAX_PATH_LENGTH + MAX_PATH + 1);
	WinFS::Expand(app, cmd);

	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;

	HANDLE	hOldToken = NULL;
	if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_QUERY | TOKEN_ADJUST_DEFAULT, &hOldToken)) {
		HANDLE	hToken = NULL;
		if (::CreateRestrictedToken(hOldToken, DISABLE_MAX_PRIVILEGE, 0, NULL, 0, NULL, 0, NULL, &hToken)) {
			if (::CreateProcessAsUserW(hToken, NULL, cmd, NULL, NULL, false, NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
				::CloseHandle(pi.hThread);
				::CloseHandle(pi.hProcess);
				::CloseHandle(hToken);
				return	NO_ERROR;
			}
			::CloseHandle(hToken);
		}
		::CloseHandle(hOldToken);
	}
	return	::GetLastError();
}

///========================================================================================== Export
void	WINAPI	EXP_NAME(ClosePlugin)(HANDLE hPlugin) {
//	delete(PList*)hPlugin;
}
int		WINAPI	EXP_NAME(Configure)(int) {
//	Options.Write();
	return	true;
}
void	WINAPI	EXP_NAME(ExitFAR)() {
}
int		WINAPI	EXP_NAME(GetMinFarVersion)() {
	return	MAKEFARVERSION(MIN_FAR_VERMAJOR, MIN_FAR_VERMINOR, MIN_FAR_BUILD);
}
void	WINAPI	EXP_NAME(GetPluginInfo)(PluginInfo *pi) {
	pi->StructSize = sizeof(PluginInfo);
	pi->Flags = PF_DIALOG;

//	static const TCHAR	*DiskStrings[1];
//	static int			DiskNumbers[1] = {6};
//	DiskStrings[0] = GetMsg(DTitle);
//	pi->DiskMenuStrings = DiskStrings;
//	pi->DiskMenuNumbers = DiskNumbers;
//	pi->DiskMenuStringsNumber = sizeofa(DiskStrings);
	pi->DiskMenuStringsNumber = 0;

	static const TCHAR	*MenuStrings[1];
	MenuStrings[0] = GetMsg(MTitle);
	pi->PluginMenuStrings = MenuStrings;
	pi->PluginMenuStringsNumber = sizeofa(MenuStrings);

//	pi->PluginConfigStrings = MenuStrings;
//	pi->PluginConfigStringsNumber = sizeofa(MenuStrings);
	pi->PluginConfigStringsNumber = 0;
	pi->CommandPrefix = L"runas";
}
HANDLE	WINAPI	EXP_NAME(OpenFilePlugin)(const TCHAR *Name, const unsigned char *Data, int DataSize, int OpMode) {
	return	INVALID_HANDLE_VALUE;
}
HANDLE	WINAPI	EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item) {
//	Options.Read();

	PCWSTR	cline = NULL;
	if (OpenFrom == OPEN_PLUGINSMENU) {
		PanelInfo pi;
		if (Info.Control(PANEL_ACTIVE, FCTL_GETPANELINFO, sizeof(pi), (LONG_PTR)&pi)) {
			CStrW buf(MAX_PATH_LENGTH + MAX_PATH + 1);
			Info.Control(PANEL_ACTIVE, FCTL_GETCURRENTDIRECTORY, buf.Size(), (LONG_PTR)buf.Data());
			if (buf.Len())
				::PathAddBackslash(buf);

			PluginPanelItem PPI;
			Info.Control(PANEL_ACTIVE, FCTL_GETPANELITEM, pi.CurrentItem, (LONG_PTR)&PPI);
			buf += PPI.FindData.lpwszFileName;
			WinStr::Assign(cline, buf);
		}
	} else if (OpenFrom == OPEN_COMMANDLINE) {
		WinStr::Assign(cline, (PCWSTR)Item);
	}
	if (!cline) {
		WinStr::Assign(cline, L"");
	}
	FarList	users;
	if (InitUsers(users)) {
		InitDialogItem Items[] = {
			{DI_DOUBLEBOX, 3, 1, 44, 12, 0, 0, 0, 0, GetMsg(MDialogTitle)},
			{DI_TEXT, 5, 2, 0, 0, 0, 0, 0, 0, GetMsg(MUsername)},
			{DI_COMBOBOX, 5, 3, 42, 0, 1, (DWORD_PTR)&users, DIF_SELECTONENTRY, 1, L""},
			{DI_TEXT, 5, 4, 0, 0, 0, 0, 0, 0, GetMsg(MPasword)},
			{DI_PSWEDIT, 5, 5, 42, 0, 0, 0, 0, 0, L""},
			{DI_CHECKBOX , 5, 6, 42, 0, 0, 0, 0, 0, GetMsg(MRestricted)},
			{DI_TEXT, 0, 7, 0, 0, 0, 0, DIF_SEPARATOR, 0, L""},
			{DI_TEXT, 5, 8, 0, 0, 0, 0, 0, 0, GetMsg(MCommandLine)},
			{DI_EDIT, 5, 9, 42, 0, 0, (DWORD_PTR)L"ProcessList.Username", DIF_HISTORY, 0, cline},
			{DI_TEXT, 5, 10, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, L""},
			{DI_BUTTON, 0, 11, 0, 0, 0, 0, DIF_CENTERGROUP, 1, GetMsg(buttonOk)},
			{DI_BUTTON, 0, 11, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(buttonCancel)},
		};
		size_t	size = sizeofa(Items);
		FarDialogItem FarItems[size];
		InitDialogItems(Items, FarItems, size);
		HANDLE hDlg = Info.DialogInit(Info.ModuleNumber, -1, -1, 48, 14, L"Contents",
									  FarItems, size, 0, 0, NULL, 0);
		if (hDlg != INVALID_HANDLE_VALUE) {
			HRESULT	err = NO_ERROR;
			while (true) {
				int		ret = Info.DialogRun(hDlg);
				if (ret == -1 || ret == 11)
					break;
				CStrW	cmd(GetDataPtr(hDlg, 8));
				if (GetCheck(hDlg, 5)) {
					err = ExecRestricted(cmd);
				} else {
					CStrW	user(GetDataPtr(hDlg, 2));
					CStrW	pass(GetDataPtr(hDlg, 4));
					err = ExecAsUser(cmd, user, pass);
				}
				if (err == NO_ERROR) {
					break;
				} else {
					CStrW	err_msg(err2w(err));
					PCWSTR Msg[] = {GetMsg(MError), L"", GetMsg(buttonOk), };
					::SetLastError(err);
					Info.Message(Info.ModuleNumber, FMSG_WARNING | FMSG_ERRORTYPE,
								 L"Contents", Msg, sizeofa(Msg), 1);
				}
			}
			Info.DialogFree(hDlg);
		}
		FreeUsers(users);
	}
	WinStr::Free(cline);

	return	INVALID_HANDLE_VALUE;
}
void	WINAPI	EXP_NAME(SetStartupInfo)(const PluginStartupInfo *psi) {
	Info	= *psi;
	FSF		= *psi->FSF;
	Info.FSF = &FSF;
}

///========================================================================================= WinMain
extern		"C" {
	BOOL		WINAPI	DllMainCRTStartup(HANDLE, DWORD, PVOID) {
		return	true;
	}
}
