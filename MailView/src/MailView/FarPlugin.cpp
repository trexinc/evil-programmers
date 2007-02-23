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
#include "StdAfx.h"

#include "FarPlugin.h"

//////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
// SetStartupInfo build by DECLARE_PLUGIN
#pragma comment( linker, "/export:SetStartupInfo=_SetStartupInfo@4" )
#pragma comment( linker, "/export:ExitFAR=_ExitFAR@0" )
#pragma comment( linker, "/export:GetPluginInfo=_GetPluginInfo@4" )
#pragma comment( linker, "/export:GetMinFarVersion=_GetMinFarVersion@0" )
#endif

FarCustomPlugin * Plugin = NULL;

void WINAPI ExitFAR()
{
  if ( Plugin )
  {
    delete Plugin;
    Plugin = NULL;
  }

  // destrioy FarPlus
  Far::Done();
}

void WINAPI GetPluginInfo( PluginInfo * pInfo )
{
  // initialize from FarPlus
  Far::GetPluginInfo( pInfo );
}

int WINAPI GetMinFarVersion(void)
{
  //far_assert( Plugin != NULL );
  return FarCustomPlugin::GetMinFarVersion();
}

//////////////////////////////////////////////////////////////////////////

int WINAPI Configure( int ItemNumber )
{
  far_assert( Plugin != NULL );
  return Plugin->Configure( ItemNumber ) ? TRUE : FALSE;
}

HANDLE WINAPI OpenPlugin( const int OpenFrom, const int Item )
{
  far_assert( Plugin != NULL );
  return Plugin->Open( OpenFrom, Item );
}

HANDLE WINAPI OpenFilePlugin( char * Name, const unsigned char * Data, int DataSize )
{
  far_assert( Plugin != NULL );
  return Plugin->OpenFile( Name, Data, DataSize );
}

//////////////////////////////////////////////////////////////////////////

void WINAPI ClosePlugin( HANDLE hPlugin )
{
  far_assert( Plugin != NULL );
  Plugin->Close( hPlugin );
}

int WINAPI Compare( HANDLE hPlugin, const PluginPanelItem * Item1, const PluginPanelItem * Item2, UINT Mode )
{
  far_assert( Plugin != NULL );
  return Plugin->Compare( hPlugin, Item1, Item2, Mode );
}

int WINAPI DeleteFiles( HANDLE hPlugin, PluginPanelItem * PanelItem, int ItemsNumber, int OpMode )
{
  far_assert( Plugin != NULL );
  FarPluginPanelItems items( PanelItem, ItemsNumber );
  return Plugin->DeleteFiles( hPlugin, &items, OpMode );
}

void WINAPI FreeFindData( HANDLE hPlugin, PluginPanelItem * pPanelItem, int ItemsNumber )
{
  far_assert( Plugin != NULL );
  if ( pPanelItem )
  {
    FarPluginPanelItems items( pPanelItem, ItemsNumber, true );
    Plugin->FreeFindData( hPlugin, &items );
  }
}

void WINAPI FreeVirtualFindData( HANDLE hPlugin, PluginPanelItem * PanelItem, int ItemsNumber )
{
  far_assert( Plugin != NULL );
  FarPluginPanelItems items( PanelItem, ItemsNumber, true );
  Plugin->FreeVirtualFindData( hPlugin, &items );
}

int WINAPI GetFiles( HANDLE hPlugin, PluginPanelItem * PanelItems, int ItemsNumber, int Move, PSTR DestPath, int OpMode )
{
  far_assert( Plugin != NULL );
  FarPluginPanelItems items( PanelItems, ItemsNumber );
  return Plugin->GetFiles( hPlugin, &items, Move != 0, DestPath, OpMode );
}

int WINAPI GetFindData( HANDLE hPlugin, PluginPanelItem ** ppPanelItems, LPINT pItemsNumber, int OpMode )
{
  far_assert( Plugin != NULL );

  FarPluginPanelItems Items( 16 );

  bool Result = Plugin->GetFindData( hPlugin, &Items, OpMode );

  *pItemsNumber = Items.Count();
  *ppPanelItems = Items.Detach();

  return Result ? TRUE : FALSE;
}

void WINAPI GetOpenPluginInfo( HANDLE hPlugin, OpenPluginInfo * pInfo )
{
  far_assert( Plugin != NULL );
  pInfo->StructSize = sizeof( OpenPluginInfo );
  Plugin->GetOpenInfo( hPlugin, pInfo );
}

int WINAPI GetVirtualFindData( HANDLE hPlugin, PluginPanelItem ** ppPanelItems, PINT pItemsNumber, LPCSTR Path )
{
  far_assert( Plugin != NULL );
  FarPluginPanelItems Items( 16 );

  bool Result = Plugin->GetVirtualFindData( hPlugin, &Items, Path );

  *pItemsNumber = Items.Count();
  *ppPanelItems = Items.Detach();

  return Result ? TRUE : FALSE;
}

int WINAPI MakeDirectory( HANDLE hPlugin, PSTR Name, int OpMode )
{
  far_assert( Plugin != NULL );
  return Plugin->MakeDirectory( hPlugin, Name, OpMode );
}

int WINAPI ProcessEvent( HANDLE hPlugin, int Event, PVOID Param )
{
  far_assert( Plugin != NULL );
  return Plugin->ProcessEvent( hPlugin, (FAR_EVENTS)Event, Param ) ? TRUE : FALSE;
}

int WINAPI ProcessHostFile( HANDLE hPlugin, PluginPanelItem * PanelItem, int ItemsNumber, int OpMode )
{
  far_assert( Plugin != NULL );
  FarPluginPanelItems items( PanelItem, ItemsNumber );
  return Plugin->ProcessHostFile( hPlugin, &items, OpMode ) ? TRUE : FALSE;
}

int WINAPI ProcessKey( HANDLE hPlugin, int Key, UINT ControlState )
{
  far_assert( Plugin != NULL );
  return Plugin->ProcessKey( hPlugin, Key, ControlState ) ? TRUE : FALSE;
}

int WINAPI PutFiles( HANDLE hPlugin, PluginPanelItem * PanelItem, int ItemsNumber, int Move, int OpMode )
{
  far_assert( Plugin != NULL );
  FarPluginPanelItems items( PanelItem, ItemsNumber );
  return Plugin->PutFiles( hPlugin, &items, Move != 0, OpMode ) ? TRUE : FALSE;
}

int WINAPI SetDirectory( HANDLE hPlugin, LPCSTR Dir, int OpMode )
{
  far_assert( Plugin != NULL );
  return Plugin->SetDirectory( hPlugin, Dir, OpMode ) ? TRUE : FALSE;
}

int WINAPI SetFindList( HANDLE hPlugin, PluginPanelItem * PanelItems, int ItemsNumber )
{
  far_assert( Plugin != NULL );

  FarPluginPanelItems Items( PanelItems, ItemsNumber );

  bool Result = Plugin->SetFindList( hPlugin, &Items );

  return Result ? TRUE : FALSE;
}

#ifdef _MSC_VER
#pragma comment(linker, "/export:ClosePlugin=_ClosePlugin@4")
#pragma comment(linker, "/export:Compare=_Compare@16")
#pragma comment(linker, "/export:Configure=_Configure@4")
#pragma comment(linker, "/export:DeleteFiles=_DeleteFiles@16")
#pragma comment(linker, "/export:FreeFindData=_FreeFindData@12")
//#pragma comment(linker, "/export:FreeVirtualFindData=_FreeVirtualFindData@12")
#pragma comment(linker, "/export:GetFiles=_GetFiles@24")
#pragma comment(linker, "/export:GetFindData=_GetFindData@16")
#pragma comment(linker, "/export:GetOpenPluginInfo=_GetOpenPluginInfo@8")
//#pragma comment(linker, "/export:GetVirtualFindData=_GetVirtualFindData@16")
#pragma comment(linker, "/export:MakeDirectory=_MakeDirectory@12")
#pragma comment(linker, "/export:OpenFilePlugin=_OpenFilePlugin@12")
#pragma comment(linker, "/export:OpenPlugin=_OpenPlugin@8")
//#pragma comment(linker, "/export:ProcessEditorEvent=_ProcessEditorEvent@8")
//#pragma comment(linker, "/export:ProcessEditorInput=_ProcessEditorInput@4")
#pragma comment(linker, "/export:ProcessEvent=_ProcessEvent@12")
#pragma comment(linker, "/export:ProcessHostFile=_ProcessHostFile@16")
#pragma comment(linker, "/export:ProcessKey=_ProcessKey@12")
#pragma comment(linker, "/export:PutFiles=_PutFiles@20")
#pragma comment(linker, "/export:SetDirectory=_SetDirectory@12")
//#pragma comment(linker, "/export:SetFindList=_SetFindList@12")
#endif


void DbgMsgV( char const * const Title, char const * const Fmt, va_list argList )
{
  FarString Tmp;
  Tmp.Format( "%s\n%s", Title, Tmp.FormatV( Fmt, argList ).c_str() );

  FarMessage::Show( FMSG_WARNING|FMSG_ALLINONE|FMSG_MB_OK, NULL, (LPCSTR const*)Tmp.c_str(), 0, 0 );
}

void DbgMsg( char const * const Title, char const * const Fmt, ... )
{
  va_list argPtr;
  va_start( argPtr, Fmt );
  DbgMsgV( Title, Fmt, argPtr );
  va_end( argPtr );
}

void DbgMsg( char const * const Fmt, ... )
{
  va_list argPtr;
  va_start( argPtr, Fmt );
  DbgMsgV( "", Fmt, argPtr );
  va_end( argPtr );
}
