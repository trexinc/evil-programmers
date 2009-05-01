#pragma once

#define MSG(ID) Info.GetMsg(Info.ModuleNumber,ID) 

#ifndef UNICODE
#define EXP_NAME(function) function
#else
#define EXP_NAME(function) function ## W
#endif

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

struct MODULES
{
	LPCTSTR DisplayName;
	LPCTSTR Path;
	LPCTSTR Module;
}
Modules[]=
{
	{TEXT("far"),      TEXT(""),                         TEXT("far.exe"),},

	{TEXT("align"),    TEXT("plugins\\editor\\align"),   TEXT("align.dll"),},
	{TEXT("autowrap"), TEXT("plugins\\editor\\autowrap"),TEXT("autowrap.dll"),},
	{TEXT("brackets"), TEXT("plugins\\editor\\brackets"),TEXT("brackets.dll"),},
	{TEXT("compare"),  TEXT("plugins\\compare"),         TEXT("compare.dll"),},
	{TEXT("drawline"), TEXT("plugins\\editor\\drawline"),TEXT("drawline.dll"),},
	{TEXT("editcase"), TEXT("plugins\\editor\\editcase"),TEXT("editcase.dll"),},
	{TEXT("emenu"),    TEXT("plugins\\emenu"),           TEXT("emenu.dll"),},
	{TEXT("farcmds"),  TEXT("plugins\\farcmds"),         TEXT("farcmds.dll"),},
	{TEXT("filecase"), TEXT("plugins\\filecase"),        TEXT("filecase.dll"),},
	{TEXT("ftp"),      TEXT("plugins\\ftp"),             TEXT("farftp.dll"),},
	{TEXT("hlfviewer"),TEXT("plugins\\hlfviewer"),       TEXT("hlfviewer.dll"),},
	{TEXT("macroview"),TEXT("plugins\\macroview"),       TEXT("macroview.dll"),},
	{TEXT("multiarc"), TEXT("plugins\\multiarc"),        TEXT("multiarc.dll"),},
	{TEXT("network"),  TEXT("plugins\\network"),         TEXT("network.dll"),},
	{TEXT("proclist"), TEXT("plugins\\proclist"),        TEXT("proclist.dll"),},
	{TEXT("tmppanel"), TEXT("plugins\\tmppanel"),        TEXT("tmppanel.dll"),},

	{TEXT("fexcept"),  TEXT(""),                         TEXT("FExcept.dll"),},
};

struct VerInfo
{
	DWORD Version;
	DWORD SubVersion;
	DWORD Build;
	DWORD FarBuild;
	VerInfo()
	{
		Version=0;
		SubVersion=0;
		Build=0;
		FarBuild=0;
	}
};

enum STATUS
{
	S_CANTCONNECT,
	S_UPTODATE,
	S_REQUIRED,
};
