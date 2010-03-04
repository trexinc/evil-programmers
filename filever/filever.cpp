/**
	filever plugin for FAR Manager
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

#include "far_helper.hpp"

///========================================================================================== define
#define MIN_FAR_VERMAJOR  2
#define MIN_FAR_VERMINOR  0
#define MIN_FAR_BUILD     0

PCWSTR prefix = L"fver";

///========================================================================================== struct
enum		eMessage {
	MTitle,
	DTitle,
	DlgTitle,
	MbuttonOk,
	MbuttonCancel,

	MtxtFileFullVer,
	MtxtFileLang,

	MtxtFileComment,
	MtxtFileCompany,
	MtxtFileDesc,
	MtxtFileVer,
	MtxtFileInternal,
	MtxtFileCopyright,
	MtxtFileTrade,
	MtxtFileOriginal,
	MtxtFilePrivate,
	MtxtFileProductName,
	MtxtFileProductVer,
	MtxtFileSpecial,
};

///======================================================================================= implement
PluginStartupInfo		Info;
FarStandardFunctions 	FSF;
//PluginOptions					Options;

NamedValues<WORD>	Machines[] = {
	{ IMAGE_FILE_MACHINE_UNKNOWN, L"UNKNOWN" },
	{ IMAGE_FILE_MACHINE_I386, L"I386" },
	{ IMAGE_FILE_MACHINE_R4000, L"R4000" },
	{ IMAGE_FILE_MACHINE_WCEMIPSV2, L"WCEMIPSV2" },
	{ IMAGE_FILE_MACHINE_SH3, L"SH3" },
	{ IMAGE_FILE_MACHINE_SH3DSP, L"SH3DSP" },
	{ IMAGE_FILE_MACHINE_SH4, L"SH4" },
	{ IMAGE_FILE_MACHINE_SH5, L"SH5" },
	{ IMAGE_FILE_MACHINE_ARM, L"ARM" },
	{ IMAGE_FILE_MACHINE_THUMB, L"THUMB" },
	{ IMAGE_FILE_MACHINE_AM33, L"AM33" },
	{ IMAGE_FILE_MACHINE_POWERPC, L"POWERPC" },
	{ IMAGE_FILE_MACHINE_POWERPCFP, L"POWERPCFP" },
	{ IMAGE_FILE_MACHINE_IA64, L"IA64" },
	{ IMAGE_FILE_MACHINE_MIPS16, L"MIPS16" },
	{ IMAGE_FILE_MACHINE_MIPSFPU, L"MIPSFPU" },
	{ IMAGE_FILE_MACHINE_MIPSFPU16, L"MIPSFPU16" },
	{ IMAGE_FILE_MACHINE_EBC, L"EBC" },
	{ IMAGE_FILE_MACHINE_AMD64, L"AMD64" },
	{ IMAGE_FILE_MACHINE_M32R, L"M32R" },
};

class		FileVersion {
	WCHAR	m_ver[32];
	WCHAR	m_lng[32];
	WCHAR	m_lngId[16];
	PBYTE	m_data;
	WORD	m_MajorVersion, m_MinorVersion;
	WORD	m_BuildNumber, m_RevisionNumber;

	WORD	m_machine;
	WORD	m_flags;
public:
	~FileVersion() {
		WinMem::Free(m_data);
	}
	FileVersion(PCWSTR path) {
		WinMem::Zero(*this);

		DWORD	dwHandle, dwLen = ::GetFileVersionInfoSize(path, &dwHandle);
		if (dwLen) {
			if (WinMem::Alloc(m_data, dwLen)) {
				UINT	bufLen;
				VS_FIXEDFILEINFO	*info;
				if (::GetFileVersionInfo(path, dwHandle, dwLen, m_data)) {
					if (::VerQueryValue(m_data, (PWSTR)L"\\", (PVOID*)&info, &bufLen)) {
						m_MajorVersion	= HIWORD(info->dwFileVersionMS);
						m_MinorVersion	= LOWORD(info->dwFileVersionMS);
						m_BuildNumber		= HIWORD(info->dwFileVersionLS);
						m_RevisionNumber	= LOWORD(info->dwFileVersionLS);
						_snwprintf(m_ver, sizeofa(m_ver), L"%d.%d.%d.%d", m_MajorVersion, m_MinorVersion, m_BuildNumber, m_RevisionNumber);
					}
					struct LANGANDCODEPAGE {
						WORD wLanguage;
						WORD wCodePage;
					} *pTranslate;
					if (::VerQueryValue(m_data, (PWSTR)L"\\VarFileInfo\\Translation", (PVOID*)&pTranslate, &bufLen)) {
						::VerLanguageName(pTranslate->wLanguage, m_lng, sizeofa(m_lng));
						_snwprintf(m_lngId, sizeofa(m_lngId), L"%04x%04x", pTranslate->wLanguage, pTranslate->wCodePage);
					}
				}
			}
		} else {
			return;
		}

		FileMap		fmap(path);
		if (!fmap.IsOK() || (fmap.Size() < sizeof(IMAGE_DOS_HEADER))) {
			return;
		}
		fmap.Next();
		PIMAGE_DOS_HEADER	dosHeader = (PIMAGE_DOS_HEADER)fmap.data();
		if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
			return;
		}
		PIMAGE_NT_HEADERS	pPEHeader = (PIMAGE_NT_HEADERS)NTSIGNATURE(dosHeader);

		if (pPEHeader->Signature != IMAGE_NT_SIGNATURE) {
			return;
		}
		m_machine = pPEHeader->FileHeader.Machine;
		m_flags = pPEHeader->FileHeader.Characteristics;
	}
	PCWSTR			ver() const {
		return	m_ver;
	}
	PCWSTR			lng() const {
		return	m_lng;
	}
	PCWSTR			lngID() const {
		return	m_lngId;
	}
	bool			IsOK() const {
		return	m_data;
	}
	const PVOID		GetData() const {
		return	m_data;
	}
	WORD			machine() const {
		return	m_machine;
	}
	bool			Is64Bit() const {
		return	m_machine == IMAGE_FILE_MACHINE_IA64 || m_machine == IMAGE_FILE_MACHINE_AMD64;
	};
};

struct		FileInfo_ {
	PWSTR		data;
	PCWSTR		SubBlock;
	eMessage	msgTxt;
} FileInfo[] = {
	{NULL, L"FileDescription", MtxtFileDesc},
	{NULL, L"LegalCopyright", MtxtFileCopyright},
	{NULL, L"Comments", MtxtFileComment},
	{NULL, L"CompanyName", MtxtFileCompany},
	{NULL, L"FileVersion", MtxtFileVer},
	{NULL, L"InternalName", MtxtFileInternal},
	{NULL, L"LegalTrademarks", MtxtFileTrade},
	{NULL, L"OriginalFilename", MtxtFileOriginal},
	{NULL, L"PrivateBuild", MtxtFilePrivate},
	{NULL, L"ProductName", MtxtFileProductName},
	{NULL, L"ProductVersion", MtxtFileProductVer},
	{NULL, L"SpecialBuild", MtxtFileSpecial},
};

bool			InitDataArray(const FileVersion &in) {
	if (in.IsOK()) {
		WCHAR	QueryString[128] = {0};
		UINT	bufLen;
		for (size_t i = 0; i < sizeofa(FileInfo); ++i) {
			_snwprintf(QueryString, sizeofa(QueryString), L"\\StringFileInfo\\%s\\%s", in.lngID(), FileInfo[i].SubBlock);
			if (!::VerQueryValue(in.GetData(), QueryString, (PVOID*)&(FileInfo[i].data), &bufLen))
				FileInfo[i].data = (PWSTR)L"";
		}
	}
	return	false;
}

///========================================================================================== export
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

//	static PCWSTR	DiskStrings[1];
//	static int		DiskNumbers[1] = {6};
//	DiskStrings[0] = GetMsg(DTitle);
//	pi->DiskMenuStrings = DiskStrings;
//	pi->DiskMenuNumbers = DiskNumbers;
//	pi->DiskMenuStringsNumber = sizeofa(DiskStrings);
	pi->DiskMenuStringsNumber = 0;

	static PCWSTR MenuStrings[1];
	MenuStrings[0] = GetMsg(MTitle);
	pi->PluginMenuStrings = MenuStrings;
	pi->PluginMenuStringsNumber = sizeofa(MenuStrings);

//	pi->PluginConfigStrings = MenuStrings;
//	pi->PluginConfigStringsNumber = sizeofa(MenuStrings);
	pi->PluginConfigStringsNumber = 0;
	pi->CommandPrefix = prefix;
}
HANDLE	WINAPI	EXP_NAME(OpenFilePlugin)(const WCHAR *Name, const unsigned char *Data, int DataSize, int OpMode) {
	return	INVALID_HANDLE_VALUE;
}
HANDLE	WINAPI	EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item) {
	CStrW	cline;
	if (OpenFrom == OPEN_PLUGINSMENU) {
		PanelInfo	pi;
		if (Info.Control(PANEL_ACTIVE, FCTL_GETPANELINFO, sizeof(pi), (LONG_PTR)&pi)) {
			CStrW	buf(MAX_PATH_LENGTH + MAX_PATH + 1);
			FSF.GetCurrentDirectory(buf.capacity(), buf.buffer());
			if (!buf.empty())
				FSF.AddEndSlash(buf.buffer());

			PluginPanelItem PPI;
			Info.Control(PANEL_ACTIVE, FCTL_GETPANELITEM, pi.CurrentItem, (LONG_PTR)&PPI);
			buf += PPI.FindData.lpwszFileName;
			cline = buf;
		}
	} else if (OpenFrom == OPEN_COMMANDLINE) {
		cline = (PCWSTR)Item;
		FSF.Trim(cline.buffer());
		FSF.Unquote(cline.buffer());
	}

	FileVersion fv(cline);
	if (fv.IsOK()) {
		InitDataArray(fv);
		if (true) {
			size_t i = 0, x = 70, y = 2;
			InitDialogItem Items[] = {
				{DI_TEXT, 5, y, 26, 0, 0, 0, 0, 0, GetMsg(MtxtFileFullVer)},
				{DI_EDIT, 28, y++, x - 2, 0, 1, (DWORD_PTR)L"ProcessList.Computer", DIF_READONLY, 1, fv.ver()},
				{DI_TEXT, 5, y, 26, 0, 0, 0, 0, 0, GetMsg(MtxtFileLang)},
				{DI_EDIT, 28, y++, x - 2, 0, 1, (DWORD_PTR)L"ProcessList.Computer", DIF_READONLY, 1, fv.lng()},
				{DI_TEXT, 5, y, 26, 0, 0, 0, 0, 0, GetMsg(FileInfo[i].msgTxt)},
				{DI_EDIT, 28, y++, x - 2, 0, 1, (DWORD_PTR)L"ProcessList.Computer", DIF_READONLY, 1, FileInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0, 0, 0, 0, 0, GetMsg(FileInfo[i].msgTxt)},
				{DI_EDIT, 28, y++, x - 2, 0, 1, (DWORD_PTR)L"ProcessList.Computer", DIF_READONLY, 1, FileInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0, 0, 0, 0, 0, L"Machine"},
				{DI_EDIT, 28, y++, x - 2, 0, 1, (DWORD_PTR)L"ProcessList.Computer", DIF_READONLY, 1, NamedValues<WORD>::GetName(Machines, sizeofa(Machines), fv.machine())},
				{DI_TEXT, 5, y++, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, L""},
				{DI_TEXT, 5, y, 26, 0, 0, 0, 0, 0, GetMsg(FileInfo[i].msgTxt)},
				{DI_EDIT, 28, y++, x - 2, 0, 1, (DWORD_PTR)L"ProcessList.Computer", DIF_READONLY, 1, FileInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0, 0, 0, 0, 0, GetMsg(FileInfo[i].msgTxt)},
				{DI_EDIT, 28, y++, x - 2, 0, 1, (DWORD_PTR)L"ProcessList.Computer", DIF_READONLY, 1, FileInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0, 0, 0, 0, 0, GetMsg(FileInfo[i].msgTxt)},
				{DI_EDIT, 28, y++, x - 2, 0, 1, (DWORD_PTR)L"ProcessList.Computer", DIF_READONLY, 1, FileInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0, 0, 0, 0, 0, GetMsg(FileInfo[i].msgTxt)},
				{DI_EDIT, 28, y++, x - 2, 0, 1, (DWORD_PTR)L"ProcessList.Computer", DIF_READONLY, 1, FileInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0, 0, 0, 0, 0, GetMsg(FileInfo[i].msgTxt)},
				{DI_EDIT, 28, y++, x - 2, 0, 1, (DWORD_PTR)L"ProcessList.Computer", DIF_READONLY, 1, FileInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0, 0, 0, 0, 0, GetMsg(FileInfo[i].msgTxt)},
				{DI_EDIT, 28, y++, x - 2, 0, 1, (DWORD_PTR)L"ProcessList.Computer", DIF_READONLY, 1, FileInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0, 0, 0, 0, 0, GetMsg(FileInfo[i].msgTxt)},
				{DI_EDIT, 28, y++, x - 2, 0, 1, (DWORD_PTR)L"ProcessList.Computer", DIF_READONLY, 1, FileInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0, 0, 0, 0, 0, GetMsg(FileInfo[i].msgTxt)},
				{DI_EDIT, 28, y++, x - 2, 0, 1, (DWORD_PTR)L"ProcessList.Computer", DIF_READONLY, 1, FileInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0, 0, 0, 0, 0, GetMsg(FileInfo[i].msgTxt)},
				{DI_EDIT, 28, y++, x - 2, 0, 1, (DWORD_PTR)L"ProcessList.Computer", DIF_READONLY, 1, FileInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0, 0, 0, 0, 0, GetMsg(FileInfo[i].msgTxt)},
				{DI_EDIT, 28, y++, x - 2, 0, 1, (DWORD_PTR)L"ProcessList.Computer", DIF_READONLY, 1, FileInfo[i++].data},
//			{DI_TEXT, 5, y++, 26, 0, 0, 0, 0, 0, CurDir},
				{DI_BUTTON, 0, (++y)++, 0, 0, 0, 0, DIF_CENTERGROUP, 1, GetMsg(MbuttonOk)},
				{DI_DOUBLEBOX, 3, 1, x, y, 0, 0, 0, 0, GetMsg(DlgTitle)},
			};

			FarDialogItem FarItems[sizeofa(Items)];
			InitDialogItems(Items, FarItems, sizeofa(Items));
			HANDLE hDlg = Info.DialogInit(Info.ModuleNumber, -1, -1, x + 4, y + 2, L"Contents",
										  FarItems, sizeofa(Items), 0, 0, NULL, 0);
			Info.DialogRun(hDlg);
			Info.DialogFree(hDlg);
		}
	}
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
