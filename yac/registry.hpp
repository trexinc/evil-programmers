#pragma once
#include "headers.hpp"
#include "unicodestring.hpp"

void SetRegRootKey(HKEY hRootKey);
string GetRegRootString();
void SetRegRootString(const wchar_t* Key);
void FreeRegRootString(); //BUGBUG

long SetRegKey(const wchar_t* Key,const wchar_t* ValueName,const wchar_t* const ValueData, int SizeData, DWORD Type);
long SetRegKey(const wchar_t* Key,const wchar_t* ValueName,const wchar_t* const ValueData);
long SetRegKey(const wchar_t* Key,const wchar_t* ValueName,DWORD ValueData);
long SetRegKey(const wchar_t* Key,const wchar_t* ValueName,LPCBYTE ValueData,DWORD ValueSize);
int GetRegKey(const wchar_t* Key,const wchar_t* ValueName,string &strValueData,const wchar_t* Default,LPDWORD pType=NULL);
int GetRegKey(const wchar_t* Key,const wchar_t* ValueName,LPBYTE ValueData,LPCBYTE Default,DWORD DataSize,LPDWORD pType=NULL);
int GetRegKey(const wchar_t* Key,const wchar_t* ValueName,DWORD &ValueData,DWORD Default);
int GetRegKey(const wchar_t* Key,const wchar_t* ValueName,DWORD Default);
HKEY CreateRegKey(const wchar_t* Key);
HKEY OpenRegKey(const wchar_t* Key);
int GetRegKeySize(const wchar_t* Key,const wchar_t* ValueName);
int GetRegKeySize(HKEY hKey,const wchar_t* ValueName);
int CheckRegKey(const wchar_t* Key);
int CheckRegValue(const wchar_t* Key,const wchar_t* ValueName);
int DeleteEmptyKey(HKEY hRoot, const wchar_t* FullKeyName);
int EnumRegKey(const wchar_t* Key,DWORD Index,string &strDestName,bool full=true);

void UseSameRegKey();
void CloseSameRegKey();
long CloseRegKey(HKEY hKey);

int RegQueryStringValueEx (HKEY hKey, const wchar_t* lpwszValueName, string &strData, const wchar_t* lpwszDefault=L"");
int RegQueryStringValue (HKEY hKey, const wchar_t* lpwszSubKey, string &strData, const wchar_t* lpwszDefault=L"");
bool IsRegKeyExist(const wchar_t* Key);
