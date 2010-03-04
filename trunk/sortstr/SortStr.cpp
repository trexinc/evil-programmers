#include "win_def.h"

#include "far/far_helper.hpp"
#include "far/farkeys.hpp"

#include <vector>
#include <map>
#include <algorithm>
using namespace std;

enum		{
	MTitle,
	DlgTitle,
	buttonOk,
	buttonCancel,
	cbSelected,
	cbInvert,
	cbSensitive,
};

///======================================================================================= implement
PluginStartupInfo		Info;
FarStandardFunctions	FSF;

typedef	pair<CStrW, intmax_t> sortpair;
class	cSortPairsCS {
public:
	bool	operator()(sortpair lhs, sortpair rhs) {
		return	wcscoll(lhs.first.c_str(), rhs.first.c_str()) < 0;
	}
} SortPairsCS;
class	cSortPairsCI {
public:
	bool	operator()(sortpair lhs, sortpair rhs) {
//		return	_wcsicoll(lhs.first.c_str(), rhs.first.c_str()) < 0;
		return	FSF.LStricmp(lhs.first.c_str(), rhs.first.c_str()) < 0;
	}
} SortPairsCI;

bool				ProcessEditor(bool sel, bool inv, bool cs) {
	EditorInfo ei;
	Info.EditorControl(ECTL_GETINFO, &ei);
	size_t	lineFirst = 0;
	if (sel) {
		if (ei.BlockType != BTYPE_STREAM && ei.BlockType != BTYPE_COLUMN) {
			return	false;
		} else {
			lineFirst = ei.BlockStartLine;
		}
	}
	vector<CStrW>		data;
	vector<sortpair>	sortdata;

	for (intmax_t i = lineFirst; i < ei.TotalLines; ++i) {
		static EditorGetString	egs = {0};
		egs.StringNumber = i;
		Info.EditorControl(ECTL_GETSTRING, &egs);

		if (sel && (egs.SelStart == -1 || egs.SelStart == egs.SelEnd))
			break;
		if (i == (ei.TotalLines - 1) && WinStr::Empty(egs.StringText))
			break;
		CStrW	tmp(egs.StringText, egs.StringLength);
		data.push_back(tmp);

		if (sel) {
			size_t	SelLen = ((egs.SelEnd - egs.SelStart) <= 0) ? egs.StringLength : egs.SelEnd - egs.SelStart;
			CStrW	tmp(egs.StringText + egs.SelStart, SelLen);
			pair<CStrW, intmax_t>	tp(tmp, i - lineFirst);
			sortdata.push_back(tp);
		} else {
			CStrW	tmp(egs.StringText, egs.StringLength);
			pair<CStrW, intmax_t>	tp(tmp, i - lineFirst);
			sortdata.push_back(tp);
		}
	}

	if (cs) {
		sort(sortdata.begin(), sortdata.end(), SortPairsCS);
	} else {
		sort(sortdata.begin(), sortdata.end(), SortPairsCI);
	}

	if (inv) {
		vector<sortpair>::reverse_iterator it = sortdata.rbegin();
		for (intmax_t i = lineFirst; it != sortdata.rend(); ++i) {
			if ((lineFirst + it->second) == i) {
				++it;
				continue;
			}
			static	EditorSetString	ess = {0};
			ess.StringNumber = i;
			ess.StringText = data[it->second].c_str();
			ess.StringLength = data[it->second].capacity();
			Info.EditorControl(ECTL_SETSTRING, &ess);
			++it;
		}
	} else {
		vector<sortpair>::iterator it = sortdata.begin();
		for (intmax_t i = lineFirst; it != sortdata.end(); ++i) {
			if ((lineFirst + it->second) == i) {
				++it;
				continue;
			}
			static	EditorSetString	ess = {0};
			ess.StringNumber = i;
			ess.StringText = data[it->second].c_str();
			ess.StringLength = data[it->second].capacity();
			Info.EditorControl(ECTL_SETSTRING, &ess);
			++it;
		}
	}

	EditorSelect	es = {0};
	es.BlockType = BTYPE_NONE;
	Info.EditorControl(ECTL_SELECT, &es);

	Info.EditorControl(ECTL_REDRAW, NULL);
	return	true;
}
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓                                             ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ╔════════════ Sort strings ═══════════╗   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ║ [ ] Only selected                   ║   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ║ [ ] Invert sort                     ║   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ║ [ ] Case sensitive                  ║   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓   ╚═════════════════════════════════════╝   ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓                                             ░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓

///========================================================================================== Export
void WINAPI			EXP_NAME(GetPluginInfo)(PluginInfo *Info) {
	Info->StructSize = sizeof(PluginInfo);
	Info->Flags = PF_DISABLEPANELS | PF_EDITOR;
	static PCWSTR	PluginMenuStrings[1];
	PluginMenuStrings[0] = GetMsg(MTitle);
	Info->PluginMenuStrings = PluginMenuStrings;
	Info->PluginMenuStringsNumber = 1;
}
HANDLE WINAPI		EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item) {
	EditorInfo ei;
	Info.EditorControl(ECTL_GETINFO, &ei);
	InitDialogItem	InitItems[] = {
		{DI_DOUBLEBOX, 3, 1, 41, 7, 0, 0, 0, 0, GetMsg(DlgTitle)},
		{DI_CHECKBOX,  5, 2, 0,  0, 0, ei.BlockType != BTYPE_NONE, 0, 0, GetMsg(cbSelected)},
		{DI_CHECKBOX,  5, 3, 0,  0, 0, 0, 0, 0, GetMsg(cbInvert)},
		{DI_CHECKBOX,  5, 4, 0,  0, 0, 0, 0, 0, GetMsg(cbSensitive)},
		{DI_TEXT,      0, 5, 0,  0, 0, 0, DIF_SEPARATOR, 0, L""},
		{DI_BUTTON,    0, 6, 0,  0, 0, 0, DIF_CENTERGROUP, 1, GetMsg(buttonOk)},
		{DI_BUTTON,    0, 6, 0,  0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(buttonCancel)},
	};
	FarDialogItem	Items[sizeofa(InitItems)];
	InitDialogItems(InitItems, Items, sizeofa(InitItems));

	HANDLE hDlg = Info.DialogInit(Info.ModuleNumber, -1, -1, 45, 9, NULL, Items, sizeofa(Items), 0, 0, NULL, 0);
	if (hDlg != INVALID_HANDLE_VALUE) {
		int		ret = Info.DialogRun(hDlg);
		if (ret == 5) {
			ProcessEditor(GetCheck(hDlg, 1), GetCheck(hDlg, 2), GetCheck(hDlg, 3));
		}
		Info.DialogFree(hDlg);
	}
	return	INVALID_HANDLE_VALUE;
}
void WINAPI			EXP_NAME(SetStartupInfo)(const PluginStartupInfo *Info) {
	::Info = *Info;
	FSF = *Info->FSF;
	::Info.FSF = &FSF;
}

///========================================================================================= WinMain
/*
extern "C" {
	BOOL WINAPI		DllMainCRTStartup(HANDLE hDll, DWORD dwReason, LPVOID lpReserved) {
		return	TRUE;
	}
}
*/
