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
#ifndef ___MsgLib_H___
#define ___MsgLib_H___

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MSGAPI_INLINE     inline

typedef class  FarStringArray * PKludges;
typedef struct TMemoryBlock   * PContent;

typedef class CMessageT * PMessage;
typedef class CMsgPartT * PMsgPart;

#include <FarString.h>

#include "FarMemoryBlock.h"

#include "Message.h"
#include "MsgPart.h"

#endif //!defined(___MsgLib_H___)
