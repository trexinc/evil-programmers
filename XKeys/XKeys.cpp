#include <plugin.hpp>
#include <farkeys.hpp>
#include "version.hpp"
#include "lng.hpp"

LPVOID WINAPIV memcpy(LPVOID dst,LPCVOID src, size_t count)
{
	LPVOID ret=dst;
	while(count--)
	{
		*static_cast<LPBYTE>(dst)=*static_cast<LPCBYTE>(src);
		dst=static_cast<LPBYTE>(dst)+1;
		src=static_cast<LPCBYTE>(src)+1;
	}
	return ret;
}

enum KEY_ID
{
	ID_ALTTAB,
	ID_ALTESC,
	ID_ALTSPACE,
	ID_ALTENTER,
	ID_ALTPRTSC,
	ID_PRTSC,
	ID_CTRLESC,
};

struct Keys
{
	INT iCode;
	INT fCode;
	wchar_t fName[64];
}
Keys[]=
{
	{1<<ID_ALTTAB,  KEY_ALT|KEY_TAB,     0},
	{1<<ID_ALTESC,  KEY_ALT|KEY_ESC,     0},
	{1<<ID_ALTSPACE,KEY_ALT|KEY_SPACE,   0},
	{1<<ID_ALTENTER,KEY_ALT|KEY_ENTER,   0},
	{1<<ID_ALTPRTSC,KEY_ALT|KEY_PRNTSCRN,0},
	{1<<ID_PRTSC,   KEY_PRNTSCRN,        0},
	{1<<ID_CTRLESC, KEY_CTRL|KEY_ESC,    0},
};

#define KEYSMASK 0x7F;

BYTE regKeys;
wchar_t RegKey[MAX_PATH];

EXTERN_C WINBASEAPI BOOL WINAPI SetConsoleKeyShortcuts(BOOL bSet, BYTE bReserveKeys,LPVOID,DWORD);

#ifdef _DEBUG
#define MSG(i) *Info.GetMsg(Info.ModuleNumber,i)?Info.GetMsg(Info.ModuleNumber,i):STR(i)
#else
#define MSG(i) Info.GetMsg(Info.ModuleNumber,i)
#endif

PluginStartupInfo Info;
FarStandardFunctions FSF;

VOID WINAPI SetStartupInfoW(const PluginStartupInfo *psInfo)
{
	Info=*psInfo;
	FSF=*psInfo->FSF;
	Info.FSF=&FSF;
	FSF.snprintf(RegKey,ARRAYSIZE(RegKey),L"%s\\%s",Info.RootKey,L"XKeys");
	HKEY hKey;
	if(RegOpenKeyEx(HKEY_CURRENT_USER,RegKey,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
	{
		DWORD Data=0;
		DWORD Size=sizeof(Data);
		if(RegQueryValueEx(hKey,L"Keys",NULL,NULL,reinterpret_cast<LPBYTE>(&Data),&Size)==ERROR_SUCCESS)
		{
			regKeys=Data&KEYSMASK;
		}
		RegCloseKey(hKey);
	}
	SetConsoleKeyShortcuts(TRUE,regKeys,NULL,0);
}

VOID WINAPI GetPluginInfoW(PluginInfo* pInfo)
{
	pInfo->StructSize=sizeof(PluginInfo);
	static LPCTSTR PluginConfigStrings[1];
	PluginConfigStrings[0]=MSG(L_NAME);
	pInfo->PluginConfigStrings=PluginConfigStrings;
	pInfo->PluginConfigStringsNumber=ARRAYSIZE(PluginConfigStrings);
	pInfo->Flags=PF_PRELOAD;
}

INT WINAPI ConfigureW(INT ItemNumber)
{
	enum{dlgX=30,dlgY=13,};
	for(size_t i=0;i<ARRAYSIZE(Keys);i++)
	{
		FSF.FarKeyToName(Keys[i].fCode,Keys[i].fName,ARRAYSIZE(Keys[i].fName));
	}
	enum
	{
		DLG_DOUBLEBOX,
		DLG_CHECKBOX_ALTTAB,
		DLG_CHECKBOX_ALTESC,
		DLG_CHECKBOX_ALTSPACE,
		DLG_CHECKBOX_ALTENTER,
		DLG_CHECKBOX_ALTPRTSC,
		DLG_CHECKBOX_PRTSC,
		DLG_CHECKBOX_CTRLESC,
		DLG_SEPARATOR,
		DLG_BUTTON_OK,
		DLG_BUTTON_CANCEL,
	};

	FarDialogItem DialogItems[]=
	{
		{DI_DOUBLEBOX,3, 1,dlgX-4,dlgY-2,1,0,                                                           0,                           0,MSG(L_NAME)},
		{DI_CHECKBOX, 5, 2,     5,     2,0,regKeys&Keys[ID_ALTTAB].iCode?BSTATE_CHECKED:BST_UNCHECKED,  0,                           0,Keys[ID_ALTTAB].fName},
		{DI_CHECKBOX, 5, 3,     5,     3,0,regKeys&Keys[ID_ALTESC].iCode?BSTATE_CHECKED:BST_UNCHECKED,  0,                           0,Keys[ID_ALTESC].fName},
		{DI_CHECKBOX, 5, 4,     5,     4,0,regKeys&Keys[ID_ALTSPACE].iCode?BSTATE_CHECKED:BST_UNCHECKED,0,                           0,Keys[ID_ALTSPACE].fName},
		{DI_CHECKBOX, 5, 5,     5,     5,0,regKeys&Keys[ID_ALTENTER].iCode?BSTATE_CHECKED:BST_UNCHECKED,0,                           0,Keys[ID_ALTENTER].fName},
		{DI_CHECKBOX, 5, 6,     5,     6,0,regKeys&Keys[ID_ALTPRTSC].iCode?BSTATE_CHECKED:BST_UNCHECKED,0,                           0,Keys[ID_ALTPRTSC].fName},
		{DI_CHECKBOX, 5, 7,     5,     7,0,regKeys&Keys[ID_PRTSC].iCode?BSTATE_CHECKED:BST_UNCHECKED,   0,                           0,Keys[ID_PRTSC].fName},
		{DI_CHECKBOX, 5, 8,     5,     8,0,regKeys&Keys[ID_CTRLESC].iCode?BSTATE_CHECKED:BST_UNCHECKED, 0,                           0,Keys[ID_CTRLESC].fName},
		{DI_TEXT,     3, dlgY-4,0,dlgY-4,0,                                                             0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L""},
		{DI_BUTTON,   0, dlgY-3,0,dlgY-3,0,                                                             0,DIF_CENTERGROUP,           1,MSG(L_OK)},
		{DI_BUTTON,   0, dlgY-3,0,dlgY-3,0,                                                             0,DIF_CENTERGROUP,           0,MSG(L_CANCEL)},
	};
	HANDLE hDlg=Info.DialogInit(Info.ModuleNumber,-1,-1,dlgX,dlgY,NULL,DialogItems,ARRAYSIZE(DialogItems),0,0,NULL,NULL);
	if(Info.DialogRun(hDlg)==DLG_BUTTON_OK)
	{
		for(int i=DLG_CHECKBOX_ALTTAB;i<=DLG_CHECKBOX_CTRLESC;i++)
		{
			Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0)==BSTATE_CHECKED?regKeys|=Keys[i-1].iCode:regKeys&=~Keys[i-1].iCode;
		}
		regKeys&=KEYSMASK;
		HKEY hKey;
		if(RegOpenKeyEx(HKEY_CURRENT_USER,RegKey,0,KEY_SET_VALUE,&hKey)==ERROR_SUCCESS)
		{
			DWORD Data=regKeys;
			RegSetValueEx(hKey,L"Keys",NULL,REG_DWORD,reinterpret_cast<LPCBYTE>(&Data),sizeof(Data));
			RegCloseKey(hKey);
		}
		SetConsoleKeyShortcuts(TRUE,regKeys,NULL,0);
	}
	Info.DialogFree(hDlg);
	return TRUE;
}

VOID WINAPI ExitFARW()
{
	SetConsoleKeyShortcuts(TRUE,0,NULL,0);
}