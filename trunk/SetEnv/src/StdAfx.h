// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__58B30425_523A_4AEB_A091_0A048F64003F__INCLUDED_)
#define AFX_STDAFX_H__58B30425_523A_4AEB_A091_0A048F64003F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers

#define UNICODE
#define _UNICODE

#include <Windows.h>
#include <TChar.h>
#include <TlHelp32.h>
#include <ShellAPI.h>

#include "memory.hpp"

#ifdef _DEBUG
#define MyMain() int main(int argc, char* argv[])
#else //_DEBUG
#define MyMain() int MyWinMain(void)
#endif //_DEBUG

#ifndef _DEBUG
#pragma comment(linker,"/ENTRY:MyWinMain")
#pragma comment(linker,"/NODEFAULTLIB")
//#pragma comment(linker,"/opt:nowin98")
#pragma comment(linker, "/merge:.rdata=.text")
#pragma comment(linker, "/section:.text,RWE")
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__58B30425_523A_4AEB_A091_0A048F64003F__INCLUDED_)
