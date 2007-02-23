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

#ifndef ___MailViewPlugin_H___
#define ___MailViewPlugin_H___

#if defined(_MSC_VER)
# pragma pack(push,8)
# define _export
# if _MSC_VER > 1000
#   pragma once
# endif
#elif defined(__BORLANDC__)
# pragma option -a8
#elif defined(__GNUC__)
# pragma pack(8)
#else
# error You must set 8 bytes aligment for your compiler at this file
#endif

/* Invalid message id result */
#define BAD_MSG_ID                      (DWORD)-1

/* TMsgInfo Flags */
#define EMF_READED  ( 1 << 0 )
#define EMF_DELETED ( 1 << 1 )
#define EMF_REPLIED ( 1 << 2 )
#define EMF_FORWDED ( 1 << 3 )
#define EMF_FLAGGED ( 1 << 4 )

/* TMsgInfo Priority */
#define EMP_AUTO    0
#define EMP_HIGH    1
#define EMP_NORMAL    3
#define EMP_LOW     5

#define EMT_INET                1
#define EMT_FIDO                2

struct TMsgInfo
{
  DWORD    StructSize;
  DWORD    Flags;
  DWORD    Priority;
  FILETIME Sent;
  FILETIME Received;
  FILETIME Accessed;
};

typedef TMsgInfo * PMsgInfo;

enum MV_RESULT
{
  MV_OK         = 0x00000000,
  MV_FALSE      = 0x00000001,
  MV_NOTIMPL    = 0x80004001,
  MV_INVALIDARG = 0x80070057,
  MV_FAIL       = 0x80000008
};


//#if defined(_MSC_VER) || defined(__BORLANDC__)
# ifdef __cplusplus
extern "C"
  {
# endif

  //////////////////////////////////////////////////////////////////////////
  // Mailbox API

  /* Information */
  BOOL  _export WINAPI Mailbox_GetName( LPSTR lpName, LPDWORD lpSize );       // not used yet
  BOOL  _export WINAPI Mailbox_GetShortName( LPSTR lpName, LPDWORD lpSize );  // show in panel
  BOOL  _export WINAPI Mailbox_GetFilesMasks( LPSTR lpMasks, LPDWORD lpSize ); // if not exported then equal to *.*
  DWORD _export WINAPI Mailbox_GetMsgType();                                  // message type (EMT_xxx)

  /* Constructor/Destructor */
  HANDLE  _export WINAPI Mailbox_OpenMem( LPCVOID lpMem, DWORD dwSize );                  // readonly memory mapped file
  HANDLE  _export WINAPI Mailbox_OpenFile( LPCSTR szFileName );                           // no comment
  void  _export WINAPI Mailbox_Close( HANDLE hMailbox );                                // no comment

  /* Creation */
  HANDLE  _export WINAPI Mailbox_Create( LPCSTR szFileName );                // no comment

  /* Access */
  DWORD _export WINAPI Mailbox_GetNextMsg( HANDLE hMailbox, DWORD dwPrevID );  // BAD_MSG_ID for first
  BOOL  _export WINAPI Mailbox_GetMsgInfo( HANDLE hMailbox, DWORD dwMsgID, PMsgInfo lpInfo ); // no comment
  MV_RESULT _export WINAPI Mailbox_GetMsgHead( HANDLE hMailbox, DWORD dwMsgId, LPBYTE lpMsg, LPDWORD lpSize );
  BOOL  _export WINAPI Mailbox_GetMsg( HANDLE hMailbox, DWORD dwMsgID, LPBYTE lpMsg, LPDWORD lpSize ); // no comment

  /* Modification */
  BOOL  _export WINAPI Mailbox_SetMsgInfo( HANDLE hMailbox, DWORD dwMsgID, PMsgInfo lpInfo );
  DWORD _export WINAPI Mailbox_AddMsg( HANDLE hMailbox, const LPBYTE lpMsg, DWORD dwSize );     // not used yet
  BOOL  _export WINAPI Mailbox_DelMsg( HANDLE hMailbox, DWORD dwMsgId );

  /* Commands */
  MV_RESULT _export WINAPI Mailbox_Purge( HANDLE hMailbox ); // no comment

  //////////////////////////////////////////////////////////////////////////
  // Account API

# ifdef __cplusplus
};
# endif /* defined(__cplusplus) */
//#endif /* defined(__BORLANDC__) || defined(_MSC_VER) */
/*
#include <objbase.h>

DECLARE_INTERFACE_(IMailView, IUnknown)
{
  STDMETHOD(EnumMessage)(LPENUMVARIANT*) PURE;
  STDMETHOD(EnumMailbox)(LPENUMVARIANT*) PURE;
  STDMETHOD(EnumAccount)(LPENUMVARIANT*) PURE;
};

typedef IMailView * PMailView;

DECLARE_INTERFACE_(IMVAccount, IUnknown)
{
};

DECLARE_INTERFACE_(IMVMailboxInfo, IUnknown)
{
  STDMETHOD(GetName)( VARIANT_BOOL shortFormat, LPBSTR result ) PURE;
  STDMETHOD(GetFileMasks)( LPBSTR result ) PURE;
  //DWORD _export WINAPI Mailbox_GetMsgType();                                  // message type (EMT_xxx)
};


DECLARE_INTERFACE_(IMVPerson, IUnknown)
{
};

typedef IMVPerson * PMVPerson;

DECLARE_INTERFACE_(IMVMsgInfo, IUnknown)
{
  STDMETHOD(GetId)( LPBSTR result ) PURE;

  STDMETHOD(GetFrom)( PMVPerson * result ) PURE;
  STDMETHOD(GetTo)( PMVPerson * result ) PURE;
  STDMETHOD(GetCc)( PMVPerson * result ) PURE;
  STDMETHOD(GetBcc)( PMVPerson * result ) PURE;

  STDMETHOD(GetDate)( LPBSTR result ) PURE;

  STDMETHOD(GetSubject)( LPBSTR result ) PURE;

  STDMETHOD(GetFlags)( LPDWORD result ) PURE;

  STDMETHOD(GetPriority)( LPDWORD result ) PURE;

  STDMETHOD(GetEncoding)( LPLONG result ) PURE;
  STDMETHOD(SetEncoding)( LONG encoding ) PURE;
  STDMETHOD(SetEncoding)( BSTR encoding ) PURE;

  STDMETHOD(GetSent)( LPFILETIME result ) PURE;
  STDMETHOD(GetReceived)( LPFILETIME result ) PURE;
  STDMETHOD(GetAccessed)( LPFILETIME result ) PURE;

  //virtual FarString GetArea() = 0;
  //virtual FarString GetNewsgroups() = 0;
  //virtual FarString GetMailer() = 0;
  //virtual FarString GetOrganization() = 0;

  // первый элемент должен содержать InReplyTo значение
  STDMETHOD(GetReferences)( PMVStrings result ) PURE;

  STDMETHOD(GetAttchmentsCount)( LPDWORD result ) PURE;
};

typedef IMVMsgInfo * PMVMsgInfo;

DECLARE_INTERFACE_(IMVData, IUnknown)
{
  STDMETHOD(GetData)() PURE;
  STDMETHOD(GetSize)() PURE;
};

typedef IMVData * PMVData;

// implements IMVMailboxInfo
DECLARE_INTERFACE_(IMVMailbox, IUnknown)
{
  STDMETHOD(Open)( BSTR fileName, VARIANT_BOOL create ) PURE;
  STDMETHOD(Close)() PURE;

  // Access
  STDMETHOD(GetNextMsg)( DWORD prevId, LPDWORD result ) PURE;
  STDMETHOD(GetMsgInfo)( DWORD msgId, PMVMsgInfo * result ) PURE;
  STDMETHOD(GetMsgHead)( DWORD msgId, LPBYTE data, LPDWORD size ) PURE;
  STDMETHOD(GetMsg)( DWORD msgId, PMVData * result ) PURE;

  // Modification
  STDMETHOD(SetMsgInfo)( DWORD msgId, PMVMsgInfo info ) PURE;
  STDMETHOD(AddMsg)( PMVData data, LPDWORD result ) PURE;
  STDMETHOD(DelMsg)( DWORD msgId );

  // Commands
  STDMETHOD(Purge)();
};

typedef IMVMailbox * PMVMailbox;

// implements IMVData
DECLARE_INTERFACE_(IMVMsgPart, IUnknown)
{
  STDMETHOD(GetMessage)( PMVMessage * result ) PURE;

  STDMETHOD(EnumMsgPart)( PMVEnumMsgPart * result ) PURE;

  STDMETHOD(GetKludge)( BSTR name, LPBSTR result ) PURE;
  STDMETHOD(GetKludges)( PMVStrings result ) PURE;
};

typedef IMVMsgPart * PMVMsgPart;

DECLARE_INTERFACE_(IMVStrings, IUnknown)
{
};

typedef IMVStrings * PMVStrings;

DECLARE_INTERFACE_(IMVEnumMsgPart, IUnknown)
{
  STDMETHOD(Next)( ULONG celt, PMVMsgPart * rgelt, ULONG * pceltFetched ) PURE;
  STDMETHOD(Skip)( ULONG celt ) PURE;
  STDMETHOD(Reset)(void) PURE;
  STDMETHOD(Clone)( IMVEnumMsgPart ** ppenum ) PURE;
};

typedef IMVEnumMsgPart * PMVEnumMsgPart;

// implements IMVData, IMVMsgInfo, IMVMsgPart
DECLARE_INTERFACE_(IMVMessage, IUnknown)
{
  STDMETHOD(Read)( BSTR fileName, LONG encoding, VARIANT_BOOL headOnly ) PURE;
  STDMETHOD(Read)( PMVMailbox mailbox, LONG encoding, VARIANT_BOOL headOnly ) PURE;

  STDMETHOD(GetFmtName)( LPBSTR result ) PURE;
};

typedef IMVMessage * PMVMessage;
*/

#if defined(_MSC_VER)
# pragma pack(pop)
#elif defined(__BORLANDC__)
# pragma option -a.
#elif defined(__GNUC__)
# pragma pack()
#else
# error You must restore aligment for your compiler at this file
#endif

#endif /* !defined(___MailViewPlugin_H___) */
