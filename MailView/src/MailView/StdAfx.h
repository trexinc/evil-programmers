/*
 MailView plugin for FAR Manager
 Copyright (C) 2005 Alex Yaroslavsky
 Copyright (C) 2002-2003 Dennis Trachuk

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__B3CC8BA2_9FC8_11D5_8640_FEA0D4826270__INCLUDED_)
#define AFX_STDAFX_H__B3CC8BA2_9FC8_11D5_8640_FEA0D4826270__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers
//#define _MFC_OVERRIDES_NEW

#include <windows.h>

#define FARPLUS_DEBUG_MEMORY
#ifdef _DEBUG
#include <CrtDbg.h>
#endif
// TODO: reference additional headers your program requires here
#include <String.h>
#include <StdLib.h>
#include <FarDbg.h>

extern LPCSTR STR_EmptyStr;

#define STR_DateFmt "dd MMM yy"
#define STR_TimeFmt "hh:mm"

#define LCID_DateTime MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), SORT_DEFAULT)


#ifndef USE_FAR_170
#error undefined USE_FAR_170
#endif

#define for if ( false ); else for

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__B3CC8BA2_9FC8_11D5_8640_FEA0D4826270__INCLUDED_)
