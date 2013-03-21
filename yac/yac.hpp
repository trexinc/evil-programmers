#pragma once
#include "stringstack.hpp"

struct CMP
{
	const wchar_t* name;
	DWORD Value;
}
CmplRules[]=
{
	{L"FIL",0x00000001}, // files/directories
	{L"PTH",0x00000002}, // programs in %PATH%
	{L"SRV",0x00000004}, // hosts
	{L"SHR",0x00000008}, // shares
	{L"ENV",0x00000010}, // environment variables
	{L"DRV",0x00000020}, // logical drives
	{L"ADS",0x00000040}, // NTFS streams
	{L"REG",0x00000080}, // registry
	{L"PIP",0x00000100}, // named pipes
	{L"VOL",0x00000200}, // volumes
	{L"PRC",0x00000400}, // processes
	{L"SVC",0x00000800}, // services
	{L"CMD",0x00001000}, // shell commands
	{L"ALS",0x00002000}, // aliases
	{L"SAL",0x00004000}, // system aliases
	{L"PRF",0x00008000}, // plugin prefixes
	{L"KEY",0x00010000}, // keys/parameters
	{L"USR",0x00020000}, // users
	{L"PNL",0x00040000}, // plugin panel items
	{L"HST",0x00080000}, // input history
	//+ 0x80000000 - namespaces (T_NMS)
};

#define CMP(x) CmplRules[x].Value
int CMPNUM(DWORD CMPVAL)
{
	for(int i=0;i<ARRAYSIZE(CmplRules);i++)
		if(CmplRules[i].Value&CMPVAL)
			return i;
	return -1;
}

enum WORKSPACES
{
	W_SHELL,
	W_EDITOR,
	W_DIALOG
};

struct Options
{
	struct CMP
	{
		DWORD CmdCompletion,ParamCompletion;
	}
	Cmd,Edit,Dlg;
	DWORD ShowResults;
	DWORD Beep;
	DWORD AddEndSlash;
	DWORD MaxFilterSize;
	BOOL SmartFilter;
	BOOL Hints;
	BOOL ShowType;
	BOOL Multicolumn;
	BOOL Pausable;
	BOOL AddSpace;
	BOOL SaveTail;
	BOOL Preload;
	BOOL UsePathExt;
	BYTE DlgColor;
	string *DirCmds;
	string *Prefix;
}
Opt;

struct TmpOptions
{
	Options::CMP Cmd,Edit,Dlg;
	DWORD MaxFilterSize;
	BYTE DlgColor;
	BOOL SmartFilter;
	BOOL Hints;

	BOOL Multicolumn;
	BOOL Pausable;
}
TmpOpt;


struct _rootkey
{
	const wchar_t* Name;
	HKEY key;
}
ROOTS[]=
{
	{L"HKEY_CLASSES_ROOT\\",  HKEY_CLASSES_ROOT  },
	{L"HKEY_CURRENT_CONFIG\\",HKEY_CURRENT_CONFIG},
	{L"HKEY_CURRENT_USER\\",  HKEY_CURRENT_USER  },
	{L"HKEY_LOCAL_MACHINE\\", HKEY_LOCAL_MACHINE },
	{L"HKEY_USERS\\",         HKEY_USERS         },
	{L"HKCR\\",               HKEY_CLASSES_ROOT  },
	{L"HKCC\\",               HKEY_CURRENT_CONFIG},
	{L"HKCU\\",               HKEY_CURRENT_USER  },
	{L"HKLM\\",               HKEY_LOCAL_MACHINE },
	{L"HKU\\",                HKEY_USERS         },
};

const wchar_t* PluginKey=L"YAC",*ContextKey=L"Context";

struct RegistryStr
{
	const wchar_t* sss;
	//const wchar_t* Completion;
	const wchar_t* CmdCmdCompletion;
	const wchar_t* CmdParamCompletion;
	const wchar_t* EditCmdCompletion;
	const wchar_t* EditParamCompletion;
	const wchar_t* DlgCmdCompletion;
	const wchar_t* DlgParamCompletion;
	const wchar_t* ShowType;
	const wchar_t* Multicolumn;
	const wchar_t* Pausable;
	const wchar_t* MaxFilterSize;
	const wchar_t* SmartFilter;
	const wchar_t* Hints;
	const wchar_t* ShowResults;
	const wchar_t* Beep;
	const wchar_t* AddEndSlash;
	const wchar_t* AddSpace;
	const wchar_t* SaveTail;
	const wchar_t* Preload;
	const wchar_t* UsePathExt;
	const wchar_t* DlgColor;
	const wchar_t* DirCmds;
	const wchar_t* filter;
	const wchar_t* keys;
	const wchar_t* Prefix;
}
RegStr=
{
	L"%s%s%s",
//	L"Completion",
	L"Cmd.Cmd.Completion",
	L"Cmd.Param.Completion",
	L"Edt.Cmd.Completion",
	L"Edt.Param.Completion",
	L"Dlg.Cmd.Completion",
	L"Dlg.Param.Completion",
	L"ShowType",
	L"Multicolumn",
	L"Pausable",
	L"MaxFilterSize",
	L"SmartFilter",
	L"Hints",
	L"ShowResults",
	L"Beep",
	L"AddEndSlash",
	L"AddSpace",
	L"SaveTail",
	L"Preload",
	L"UsePathExt",
	L"DlgColor",
	L"DirCmds",
	L"filter",
	L"keys",
	L"Prefix",
};

struct resumedata
{
	string prefix,rootcmd,pattern,postfix,result,CleanPart;
	int ItemIndex;
	void Clean()
	{
		prefix=L"";
		rootcmd=L"";
		pattern=L"";
		postfix=L"";
		result=L"";
		CleanPart=L"";
		ItemIndex=-1;
	}
}
*ResumeData;

struct state
{
	string line;
	string PanelDir;
	size_t pos;
	size_t selstart;
	size_t selend;
	int Workspace;
	HANDLE hDlg;
	state()
	{
		pos = selstart = selend	= 0;
		Workspace = 0;
		hDlg = NULL;
	};
}
*ThisState,*PrevState;

string *DlgModStr,*FarName,*MaskFilter,*DlgItemHistory;
size_t startlen;
bool IsDirCmd=false;

typedef BOOL(WINAPI *PCredWriteW)(PCREDENTIALW Credential,DWORD Flags);
PCredWriteW _CredWriteW;

struct ThParams
{
	int Type; // 0==srv; 1==share; 2==user
	const wchar_t* ServerName;
	NET_API_STATUS *res;
	LONG_PTR Data;
	LPDWORD entriesread,totalentries,resume_handle;
	HANDLE *hDlg;
};

bool NeedMove;

SMALL_RECT CmpDlgPos;
short VerticalOffset;
COORD NewCmpDlgPos;

DWORD ResultDlgExitNum=0;

FarListItem *ResultListItem=0,*ResultListInitItem=0;

bool NeedFirstQuote=false;

int ID;

int PrevCount=0;

StringStack* SStack;

bool SilentEnum=false;

void ReadConfig();
void WriteConfig();

#ifdef _DEBUG
long CallMallocFree=0;
long CallNewDelete=0;
#endif //_DEBUG
