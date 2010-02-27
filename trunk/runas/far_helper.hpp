/**
    far_helper.h
    Copyright (C) 2008 zg
    © 2010 Andrew Grechkin

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

#ifndef __FAR_HELPER_H__
#define __FAR_HELPER_H__

#include "plugin.hpp"

///======================================================================================== external
extern PluginStartupInfo	Info;
extern FarStandardFunctions FSF;

///====================================================================================== definition
#ifndef EXP_NAME
#define EXP_NAME(p) _export p ## W
#endif

struct		InitDialogItem {
	int Type;
	int X1, Y1, X2, Y2;
	int Focus;
	DWORD_PTR Reserved;
	DWORD	Flags;
	int		DefaultButton;
	PCWSTR	Data;
};

///=================================================================================================
inline PCWSTR		GetMsg(int MsgId) {
	return	Info.GetMsg(Info.ModuleNumber, MsgId);
}
inline PCWSTR		GetDataPtr(HANDLE hDlg, size_t in) {
	return	(PCWSTR)Info.SendDlgMessage(hDlg, DM_GETCONSTTEXTPTR , in, 0);
}
inline bool			GetCheck(HANDLE hDlg, size_t in) {
	return	(bool)Info.SendDlgMessage(hDlg, DM_GETCHECK, in, 0);
}
inline void			InitDialogItems(struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber) {
	for (int i = 0; i < ItemsNumber; ++i) {
		Item[i].Type = Init[i].Type;
		Item[i].X1 = Init[i].X1;
		Item[i].Y1 = Init[i].Y1;
		Item[i].X2 = Init[i].X2;
		Item[i].Y2 = Init[i].Y2;
		Item[i].Focus = Init[i].Focus;
		Item[i].Reserved = Init[i].Reserved;
		Item[i].Flags = Init[i].Flags;
		Item[i].DefaultButton = Init[i].DefaultButton;
		Item[i].MaxLen = 0;
		if ((DWORD_PTR)Init[i].Data < 2000)
			Item[i].PtrData = GetMsg((unsigned int)(DWORD_PTR)Init[i].Data);
		else
			Item[i].PtrData = Init[i].Data;
	}
}

///====================================================================================== CFarDialog
class		CFarDialog {
	HANDLE hDlg;
public:
	~CFarDialog() {
		Info.DialogFree(hDlg);
	}
	CFarDialog(): hDlg(INVALID_HANDLE_VALUE) {
		Info.DialogFree(hDlg);
	}
	int				Execute(INT_PTR PluginNumber, int X1, int Y1, int X2, int Y2, PCWSTR HelpTopic, FarDialogItem* Item,
				   int ItemsNumber, DWORD Reserved, DWORD Flags, FARWINDOWPROC DlgProc, LONG_PTR Param) {
		hDlg = Info.DialogInit(PluginNumber, X1, Y1, X2, Y2, HelpTopic, Item, ItemsNumber, Reserved, Flags, DlgProc, Param);
		return	Info.DialogRun(hDlg);
	}
	HANDLE			Handle() {
		return hDlg;
	};
	int				Check(int index) {
		return	(int)Info.SendDlgMessage(hDlg, DM_GETCHECK, index, 0);
	}
	PCWSTR			Str(int index) {
		return (PCWSTR)Info.SendDlgMessage(hDlg, DM_GETCONSTTEXTPTR, index, 0);
	}
	DWORD			Flags(int index) {
		FarDialogItem DialogItem;
		if (Info.SendDlgMessage(hDlg, DM_GETDLGITEMSHORT, index, (LONG_PTR)&DialogItem)) return DialogItem.Flags;
		return 0;
	};
	DWORD			Type(int index) {
		FarDialogItem DialogItem;
		if (Info.SendDlgMessage(hDlg, DM_GETDLGITEMSHORT, index, (LONG_PTR)&DialogItem)) return DialogItem.Type;
		return 0;
	};
	int				ListPos(int index) {
		return Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, index, 0);
	};
};

///======================================================================================= CFarPanel
class		CFarPanel {
	PanelInfo	m_pi;
	HANDLE		m_hPlug;
	PWSTR		m_CurDir;
	PluginPanelItem* m_ppi;

	int		m_Result;
	size_t	m_CurDirSize;
	size_t	m_ppiSize;

	CFarPanel();
public:
	~CFarPanel() {
		WinMem::Free(m_CurDir);
		WinMem::Free(m_ppi);
	}
	CFarPanel(HANDLE aPlugin, int cmd = FCTL_GETPANELINFO): m_hPlug(aPlugin), m_CurDir(NULL), m_ppi(NULL) {
		m_CurDirSize = m_ppiSize = 0;
		m_Result = Info.Control(aPlugin, cmd, 0, (LONG_PTR) & m_pi);
	}

	bool		IsOK() {
		return	m_Result != 0;
	}
	int			PanelType() {
		return	m_pi.PanelType;
	};
	int			Plugin() {
		return	m_pi.Plugin;
	};
	int			ItemsNumber() {
		return	m_pi.ItemsNumber;
	};
	int			SelectedItemsNumber() {
		return	m_pi.SelectedItemsNumber;
	};
	int			CurrentItem() {
		return	m_pi.CurrentItem;
	};
	DWORD		Flags() {
		return	m_pi.Flags;
	};
	PWSTR		CurDir() {
		m_CurDirSize = Info.Control(m_hPlug, FCTL_GETPANELDIR, 0, 0);
		if (WinMem::Realloc(m_CurDir, m_CurDirSize)) {
			Info.Control(m_hPlug, FCTL_GETPANELDIR, m_CurDirSize, (LONG_PTR)m_CurDir);
		}
		return	m_CurDir;
	}
	PluginPanelItem& operator[](size_t index) {
		m_ppiSize = Info.Control(m_hPlug, FCTL_GETPANELITEM, index, 0);
		if (WinMem::Realloc(m_ppi, m_ppiSize)) {
			Info.Control(m_hPlug, FCTL_GETPANELITEM, index, (LONG_PTR)m_ppi);
		}
		return *m_ppi;
	}
	PluginPanelItem& Selected(size_t index) {
		m_ppiSize = Info.Control(m_hPlug, FCTL_GETSELECTEDPANELITEM, index, 0);
		if (WinMem::Realloc(m_ppi, m_ppiSize)) {
			Info.Control(m_hPlug, FCTL_GETSELECTEDPANELITEM, index, (LONG_PTR)m_ppi);
		}
		return *m_ppi;
	}
	void		StartSelection() {
		Info.Control(m_hPlug, FCTL_BEGINSELECTION, 0, 0);
	}
	void		Select(size_t index, bool in) {
		Info.Control(m_hPlug, FCTL_SETSELECTION, index, (LONG_PTR)in);
	}
	void		CommitSelection() {
		Info.Control(m_hPlug, FCTL_ENDSELECTION, 0, 0);
	}
};

#endif
