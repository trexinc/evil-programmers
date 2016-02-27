#include "headers.hpp"
#include "functions.hpp"

#include "UnicodeString.hpp"
#include "yac.hpp"
#include "Container.hpp"
#include "tokenizer.hpp"
#include "CmdTokens.hpp"

#include "lng.hpp"
#include "ver.hpp"
#include "registry.hpp"

PluginStartupInfo Info;
FarStandardFunctions FSF;

bool CfgMode=false,CmpMode=false;

DWORD LocalCompletion=0xffffffff;

Container *GlobalContainer=0;

#include "func.cpp"

#include "farwrap.cpp"

#include "parser.cpp"
#include "network.cpp"
#include "enums.cpp"
#include "maxmatch.cpp"
#include "printitems.cpp"
#include "config.cpp"
#include "resultdlg.cpp"
#include "completion.cpp"

void InitModuleName()
{
	if(!FarName)
	{
		FarName=new string;
		if(FarName)
			apiGetModuleFileName(NULL,*FarName);
	}
}

void MsgUnknownCmd(const wchar_t* cmd)
{
	printfW(L"%s: %s\n\n",GetMsg(MUnknownCmd),cmd);
}

int WINAPI GetMinFarVersionW()
{
	#define MAKEFARVERSION(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))
	return MAKEFARVERSION(MIN_FAR_MAJOR_VER,MIN_FAR_MINOR_VER,MIN_FAR_BUILD);
}

void WINAPI SetStartupInfoW(const PluginStartupInfo* psInfo)
{
	Info=*psInfo;
	FSF=*psInfo->FSF;
	Info.FSF=&FSF;

	InitModuleName();

	Opt.DirCmds=new string;
	Opt.Prefix=new string;

	MaskFilter=new string;
	DlgItemHistory=new string;
	ThisState=new state;
	PrevState=new state;

	ResumeData=new resumedata;
	ResumeData->ItemIndex=-1;

	SStack=new StringStack;

	ReadConfig();
}

void WINAPI GetPluginInfoW(PluginInfo* pInfo)
{
	pInfo->StructSize=sizeof(PluginInfo);
	static const wchar_t* PluginMenuStrings[1],*PluginConfigStrings[1];
	PluginMenuStrings[0]=GetMsg(MName);
	PluginConfigStrings[0]=GetMsg(MName);
	pInfo->PluginMenu.Strings=PluginMenuStrings;
	pInfo->PluginConfig.Strings=PluginConfigStrings;
	pInfo->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
	pInfo->PluginConfig.Count=ARRAYSIZE(PluginConfigStrings);
	WindowType wi={sizeof(wi)};
	Info.AdvControl(YacGuid, ACTL_GETWINDOWTYPE, 0, &wi);
	pInfo->Flags=PF_EDITOR|(CmpMode?0:PF_DIALOG)|((wi.Type!=WTYPE_VIEWER)?PF_VIEWER:0)|(Opt.Preload?PF_PRELOAD:0);
	pInfo->CommandPrefix=*Opt.Prefix;
}

int WINAPI ConfigureW(int ItemNumber)
{
	return Config(ItemNumber);
}

HANDLE WINAPI OpenPluginW(int OpenFrom,INT_PTR Item)
{
	if(OpenFrom==OPEN_COMMANDLINE)
	{
		string cmd(reinterpret_cast<const wchar_t*>(Item));
		RemoveExternalSpaces(cmd);

		const wchar_t* cmdptr=cmd;
		CmdTokens T;
		switch(T.NextToken(cmdptr))
		{
		case T.TK_CFG:
			{
				const wchar_t* curcmd=cmdptr;
				switch(T.NextToken(cmdptr))
				{
				case T.TK_EMPTY:
					Config(0);
					break;
				case T.TK_SAVE:
					WriteConfig();
					break;
				case T.TK_LOAD:
					ReadConfig();
					break;
				case T.TK_UNKNOWN:
					MsgUnknownCmd(curcmd);
				}
				
			break;
			}
		case T.TK_UNLOAD:
			return 0;
		case T.TK_HOME:
			{
				string strHome(Info.ModuleName);
				size_t SlashPos=0;
				if(strHome.RPos(SlashPos,L'\\'))
					strHome.SetLength(SlashPos);
				FarPanelDirectory dir = {sizeof(dir)};
				dir.Name = strHome;
				Info.PanelControl(PANEL_ACTIVE,FCTL_SETPANELDIRECTORY,0,&dir);
				Info.AdvControl(YacGuid, ACTL_REDRAWALL,0,0);
			}
			break;
		case T.TK_HELP:
			Info.ShowHelp(Info.ModuleName,NULL,NULL);
			break;
		case T.TK_INFO:
			printfW(L"%s\n%s\n",PRODUCTNAME,ALLSTR);
			printfW(L"configuration:"
#ifdef _WIN64
				L" x64"
#else
				L" x86"
#endif
#ifdef _DEBUG
				L" Debug"
#else
				L" Release"
#endif
			);
			break;
		case T.TK_UNKNOWN:
			MsgUnknownCmd(cmd);
			break;
		}
		return INVALID_HANDLE_VALUE;
	}

	if(!GetState(Item))
		return INVALID_HANDLE_VALUE;

	if(ThisState->Workspace==W_EDITOR)
	{
		EditorInfo ei;
		Info.EditorControl(-1, ECTL_GETINFO,0, &ei);
		if(ei.CurState&ECSTATE_LOCKED)
			return INVALID_HANDLE_VALUE;
	}

	FarMenuItem MenuItems[]=
	{
		{MIF_NONE, GetMsg(MNext)},
		{MIF_NONE, GetMsg(MPrev)},
	};

	FarKey BreakKeys[] =
	{
		{VK_F9, SHIFT_PRESSED|LEFT_ALT_PRESSED},
		{VK_F9, SHIFT_PRESSED|RIGHT_ALT_PRESSED},
	};
	int MenuCode=-1,BreakCode=0;
	while(BreakCode!=-1)
	{
		if(CfgMode)
			BreakCode=-1;
		MenuCode=Info.Menu(YacGuid, nullptr, -1,-1,0,FMENU_WRAPMODE,GetMsg(MName),0,L"Contents",CfgMode?0:BreakKeys,CfgMode?0:&BreakCode,MenuItems,ARRAYSIZE(MenuItems));
		if(!BreakCode)
			ConfigureW(0);
	}
	if(ThisState->line.Empty()||(!ThisState->pos && MenuCode<2))
		return INVALID_HANDLE_VALUE;

	switch(MenuCode)
	{
	case -2:
	case -1:
		break;
	case 0:
	case 1:
		Complete(MenuCode!=0);
		break;
	default:
		break;
	}
	return INVALID_HANDLE_VALUE;
}

void WINAPI ExitFARW()
{
	WriteConfig();
	Clean();
	FreeRegRootString();
	delete SStack;
	delete FarName;
	delete MaskFilter;
	delete DlgItemHistory;
	delete Opt.DirCmds;
	delete Opt.Prefix;
	delete ThisState;
	delete PrevState;
	delete ResumeData;
}
