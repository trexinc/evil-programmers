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

#if !defined(AFX_STDAFX_H__5233C318_502F_4FD3_800F_9C6C1B0858F0__INCLUDED_)
#define AFX_STDAFX_H__5233C318_502F_4FD3_800F_9C6C1B0858F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers
//#define _MFC_OVERRIDES_NEW

// TODO: reference additional headers your program requires here

//#define _CRTDBG_MAP_ALLOC
#define FARPLUS_DEBUG_MEMORY
#ifdef _DEBUG
#include <CrtDbg.h>
#endif

#include <Windows.h>
#include <FarPlus.h>
#ifndef USE_FAR_170
#error undefined USE_FAR_170
#endif
#include <FarDbg.h>
//#include <String.h>
//#include <Memory.h>
#include <FarArray.h>
#include <FarString.h>

#include "MsgLib.h"


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__5233C318_502F_4FD3_800F_9C6C1B0858F0__INCLUDED_)
