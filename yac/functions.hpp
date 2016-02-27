#pragma once
#include "unicodestring.hpp"

inline bool IsSlash(wchar_t x);
inline bool IsSpace(wchar_t x);
inline bool IsEol(wchar_t x);

#define strcmpi(s1,s2)    (CompareStringW(LOCALE_USER_DEFAULT,NORM_IGNORECASE,s1,-1,s2,-1)-2)
#define strcmpin(s1,s2,n) (CompareStringW(LOCALE_USER_DEFAULT,NORM_IGNORECASE,s1,(int)n,s2,(int)n)-2)
#define strcmpn(s1,s2,n)  (CompareStringW(LOCALE_USER_DEFAULT,0,s1,(int)n,s2,(int)n)-2)

const wchar_t* GetFileName(const wchar_t* fullpath);
bool CmdSeparator(const wchar_t c);

BOOL AddEndSlash(wchar_t* Path, wchar_t TypeSlash);
BOOL AddEndSlash(string &strPath,wchar_t TypeSlash);
BOOL WINAPI AddEndSlash(wchar_t* Path);
BOOL AddEndSlash(string &strPath);

wchar_t* RemoveLeadingSpaces(wchar_t* Str);
string& RemoveLeadingSpaces(string &strStr);
wchar_t* RemoveTrailingSpaces(wchar_t* Str);
string& RemoveTrailingSpaces(string &strStr);
wchar_t* WINAPI RemoveExternalSpaces(wchar_t* Str);
string& WINAPI RemoveExternalSpaces(string &strStr);

bool CheckQuotedSymbols(const wchar_t* Str);
bool IsQuotedSymbol(wchar_t c);

DWORD apiExpandEnvironmentStrings(const wchar_t* src,string &strDest);
void Unquote(wchar_t* Str);
void Unquote(string &Str);

bool TokenVirtualRoot(const wchar_t* str,string& out);

bool apiGetModuleFileName(HMODULE hModule, string& strFilename);

#define CtrlMask(x) (x&(LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_ALT_PRESSED|RIGHT_CTRL_PRESSED|SHIFT_PRESSED))

inline bool IsShift(DWORD Mod)
{
	return CtrlMask(Mod) == SHIFT_PRESSED;
}

inline bool IsAlt(DWORD Mod)
{
	return (CtrlMask(Mod) == LEFT_CTRL_PRESSED) || (CtrlMask(Mod) == RIGHT_CTRL_PRESSED);
}

inline bool IsCtrl(DWORD Mod)
{
	return (CtrlMask(Mod) == LEFT_ALT_PRESSED) || (CtrlMask(Mod) == RIGHT_ALT_PRESSED);
}

inline bool IsAltShift(DWORD Mod)
{
	return CtrlMask(Mod) == (LEFT_ALT_PRESSED|SHIFT_PRESSED) ||
		CtrlMask(Mod) == (RIGHT_ALT_PRESSED|SHIFT_PRESSED);
}

inline bool IsCtrlAlt(DWORD Mod)
{
	return CtrlMask(Mod) == (LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED) ||
		CtrlMask(Mod) == (LEFT_ALT_PRESSED|RIGHT_CTRL_PRESSED) ||
		CtrlMask(Mod) == (RIGHT_ALT_PRESSED|LEFT_CTRL_PRESSED) ||
		CtrlMask(Mod) == (RIGHT_ALT_PRESSED|RIGHT_CTRL_PRESSED);
}

inline bool IsCtrlShift(DWORD Mod)
{
	return CtrlMask(Mod) == (LEFT_CTRL_PRESSED|SHIFT_PRESSED) ||
		CtrlMask(Mod) == (RIGHT_CTRL_PRESSED|SHIFT_PRESSED);
}

inline bool IsCtrlAltShift(DWORD Mod)
{
	return CtrlMask(Mod) == (LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED) ||
		CtrlMask(Mod) == (LEFT_ALT_PRESSED|RIGHT_CTRL_PRESSED|SHIFT_PRESSED) ||
		CtrlMask(Mod) == (RIGHT_ALT_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED) ||
		CtrlMask(Mod) == (RIGHT_ALT_PRESSED|RIGHT_CTRL_PRESSED|SHIFT_PRESSED);
}
