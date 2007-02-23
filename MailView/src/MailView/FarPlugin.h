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
#ifndef ___FarPlugin_H___
#define ___FarPlugin_H___

#if _MSC_VER >= 1000
#pragma once
#endif

#include <FarPlus.h>

#include "../plugin.hpp"
#include "../farkeys.hpp"
#include "../FarColor.hpp"

#include <FarDbg.h>

enum TColor
{
  clBlack     = 0x00,      //  Черный
  clBlue      = 0x01,      //  Синий
  clGreen     = 0x02,      //  Зеленый
  clCyan      = 0x03,      //  Голубой
  clRed       = 0x04,      //  Красный
  clMagenta   = 0x05,      //  Сиреневый
  clBrown     = 0x06,      //  Коричневый
  clLtGray    = 0x07,      //  Белый
  clLtGrey    = clLtGray,
  clDkGray    = 0x08,      //  Серый
  clDkGrey    = clDkGray,
  clLtBlue    = 0x09,      //  Ярко-синий
  clLtGreen   = 0x0A,      //  Ярко-зеленый
  clLtCyan    = 0x0B,      //  Ярко-голубой
  clLtRed     = 0x0C,      //  Ярко-красный
  clLtMagenta = 0x0D,      //  Ярко-сиреневый
  clYellow    = 0x0E,      //  Желтый
  clWhite     = 0x0F       //  Ярко-белый
};

#define MAKE_COLOR(background,foreground) (((background&0xF)<<4)|(foreground&0xF))


class FarCustomPanelPlugin
{
private:
  int FCTL( int command, void * param )
  {
    return Far::m_Info.Control( this, command, param );
  }

protected:
  FarFileName m_HostFileName;

public:
  void close( const char * path )
  {
    FCTL( FCTL_CLOSEPLUGIN, (void*)path );
  }

  int getInfo( PanelInfo * panelInfo )
  {
    return FCTL( FCTL_GETPANELINFO, panelInfo );
  }

  int getAnotherInfo( PanelInfo * panelInfo )
  {
    return FCTL( FCTL_GETANOTHERPANELINFO, panelInfo );
  }

  PanelInfo getInfo()
  {
    PanelInfo panelInfo;
    getInfo( &panelInfo );
    return panelInfo;
  }

  PanelInfo getAnotherInfo()
  {
    PanelInfo panelInfo;
    getAnotherInfo( &panelInfo );
    return panelInfo;
  }

  int getShortInfo( PanelInfo * panelInfo )
  {
    return FCTL( FCTL_GETPANELSHORTINFO, panelInfo );
  }

  PanelInfo getShortInfo()
  {
    PanelInfo panelInfo;
    getShortInfo( &panelInfo );
    return panelInfo;
  }

  int update( bool clearSelection = false )
  {
    return FCTL( FCTL_UPDATEPANEL, clearSelection ? NULL : (void*)1 );
  }

  int updateAnother( bool clearSelection = false )
  {
    return FCTL( FCTL_UPDATEANOTHERPANEL, clearSelection ? NULL : (void*)1 );
  }

  int redraw( PanelRedrawInfo * redrawInfo = NULL )
  {
    return FCTL( FCTL_REDRAWPANEL, redrawInfo );
  }

  int redrawAnother( PanelRedrawInfo * redrawInfo = NULL )
  {
    return FCTL( FCTL_REDRAWANOTHERPANEL, redrawInfo );
  }

  int setDirectory( const char * path )
  {
    return FCTL( FCTL_SETPANELDIR, (void*)path );
  }

  int getCmdLine( char * buf )
  {
    return FCTL( FCTL_GETCMDLINE, buf );
  }

  FarString getCmdLine()
  {
    FarString cmdLine;
    getCmdLine( cmdLine.GetBuffer( 2048 ) );
    cmdLine.ReleaseBuffer();
    return cmdLine;
  }

  int setCmdLine( const char * cmdLine )
  {
    return FCTL( FCTL_SETCMDLINE, (void*)cmdLine );
  }

  int getCmdLineSelectedText( char * buf )
  {
    return FCTL( FCTL_GETCMDLINESELECTEDTEXT, buf );
  }

  FarString getCmdLineSelectedText()
  {
    FarString cmdLineSelectedText;
    getCmdLineSelectedText( cmdLineSelectedText.GetBuffer() );
    cmdLineSelectedText.ReleaseBuffer();
    return cmdLineSelectedText;
  }

  int getCmdLineSelection( CmdLineSelect * cmdLineSelect )
  {
    return FCTL( FCTL_GETCMDLINESELECTION, cmdLineSelect );
  }

  CmdLineSelect getCmdLineSelection()
  {
    CmdLineSelect cmdLineSelect;
    getCmdLineSelection( &cmdLineSelect );
    return cmdLineSelect;
  }

  int setCmdLineSelection( CmdLineSelect * cmdLineSelect )
  {
    return FCTL( FCTL_SETCMDLINESELECTION, cmdLineSelect );
  }

  int getCmdLinePos()
  {
    int pos = -1;
    return FCTL( FCTL_GETCMDLINEPOS, &pos ) ? pos : -1;
  }

  int setCmdLinePos( int pos )
  {
    return FCTL( FCTL_GETCMDLINEPOS, (void*)pos );
  }

  int insertCmdLine( const char * cmdLine )
  {
    return FCTL( FCTL_INSERTCMDLINE, (void*)cmdLine );
  }

  int setSelection( PanelInfo * panelInfo )
  {
    return FCTL( FCTL_SETSELECTION, panelInfo );
  }

  int setViewMode( int viewMode )
  {
    return FCTL( FCTL_SETVIEWMODE, (void*)viewMode );
  }

  int setUserScreen()
  {
    return FCTL( FCTL_SETUSERSCREEN, NULL );
  }

  int setSortMode( int sortMode )
  {
    return FCTL( FCTL_SETSORTMODE, (void*)sortMode );
  }

  int setSortOrder( int sortOrder )
  {
    return FCTL( FCTL_SETSORTMODE, (void*)sortOrder );
  }


public:
  FarCustomPanelPlugin( const FarFileName& HostFileName )
    : m_HostFileName( HostFileName )
  {
  }
  virtual ~FarCustomPanelPlugin()
  {
  }

  virtual void GetOpenInfo( OpenPluginInfo * pInfo )
  {
    pInfo->HostFile = m_HostFileName;
  }

  virtual bool GetFindData( FarPluginPanelItems *Items, int OpMode )
  {
    return false;
  }

  virtual void FreeFindData( FarPluginPanelItems *Items )
  {
  }

  virtual int Compare( const PluginPanelItem * Item1, const PluginPanelItem * Item2, UINT Mode )
  {
    return -2;
  }

  virtual int GetFiles( FarPluginPanelItems *Items, bool bMove, PSTR DestPath, int OpMode )
  {
    return 0;
  }

  virtual bool PutFiles( const FarPluginPanelItems *Items, bool bMove, int OpMode )
  {
    return false;
  }

  virtual bool ProcessKey( int Key, UINT ControlState )
  {
    return false;
  }

  virtual bool DeleteFiles( FarPluginPanelItems *Items, int OpMode )
  {
    return false;
  }

  virtual void FreeVirtualFindData( FarPluginPanelItems *Items )
  {
  }

  virtual bool GetVirtualFindData( FarPluginPanelItems *Items, LPCSTR Path )
  {
    return false;
  }

  virtual int MakeDirectory( LPCSTR Name, int OpMode )
  {
    return 0;
  }

  virtual bool ProcessEvent( FAR_EVENTS Event, PVOID Param )
  {
    return false;
  }

  virtual bool ProcessHostFile( const FarPluginPanelItems *Items, int OpMode )
  {
    return false;
  }

  virtual bool SetDirectory( LPCSTR Dir, int OpMode )
  {
    return false;
  }

  virtual bool SetFindList( const FarPluginPanelItems *Items )
  {
    return false;
  }

};

typedef FarCustomPanelPlugin * PFarCustomPanelPlugin;

class FarCustomPlugin
{
public:
  FarCustomPlugin()
  {
  }

  virtual ~FarCustomPlugin()
  {
  }

  //////////////////////////////////////////////////////////////////////////

  static int GetMinFarVersion()
  {
    //return FARMANAGERVERSION;
    return MAKEFARVERSION( 1,70,1282 );
    //return MAKEFARVERSION( 1,70,591 );
  }

  virtual HANDLE Open( const int OpenFrom, int Item )
  {
    return INVALID_HANDLE_VALUE;
  }

  virtual HANDLE OpenFile( LPCSTR Name, const BYTE * Data, int DataSize )
  {
    return INVALID_HANDLE_VALUE;
  }

  virtual void Close( HANDLE hPlugin )
  {
    far_assert( hPlugin != NULL );
    delete (FarCustomPanelPlugin*)hPlugin;
  }

  virtual void GetOpenInfo( HANDLE hPlugin, OpenPluginInfo * pInfo )
  {
    far_assert( hPlugin != NULL );
    ((FarCustomPanelPlugin*)hPlugin)->GetOpenInfo( pInfo );
  }

  virtual bool GetFindData( HANDLE hPlugin, FarPluginPanelItems *Items, int OpMode )
  {
    far_assert( hPlugin != NULL );
    return ((FarCustomPanelPlugin*)hPlugin)->GetFindData( Items, OpMode );
  }

  virtual void FreeFindData( HANDLE hPlugin, FarPluginPanelItems *Items )
  {
    far_assert( hPlugin != NULL );
    ((FarCustomPanelPlugin*)hPlugin)->FreeFindData( Items );
  }

  virtual int Compare( HANDLE hPlugin, const PluginPanelItem * Item1, const PluginPanelItem * Item2, UINT Mode )
  {
    far_assert( hPlugin != NULL );
    return ((FarCustomPanelPlugin*)hPlugin)->Compare( Item1, Item2, Mode );
  }

  virtual int GetFiles( HANDLE hPlugin, FarPluginPanelItems *Items, bool bMove, LPSTR DestPath, int OpMode )
  {
    far_assert( hPlugin != NULL );
    return ((FarCustomPanelPlugin*)hPlugin)->GetFiles( Items, bMove, DestPath, OpMode );
  }

  virtual bool PutFiles( HANDLE hPlugin, const FarPluginPanelItems *Items, bool bMove, int OpMode )
  {
    far_assert( hPlugin != NULL );
    return ((FarCustomPanelPlugin*)hPlugin)->PutFiles( Items, bMove, OpMode );
  }

  virtual bool ProcessKey( HANDLE hPlugin, int Key, UINT ControlState )
  {
    far_assert( hPlugin != NULL );
    return ((FarCustomPanelPlugin*)hPlugin)->ProcessKey( Key, ControlState );
  }

  virtual bool DeleteFiles( HANDLE hPlugin, FarPluginPanelItems *Items, int OpMode )
  {
    far_assert( hPlugin != NULL );
    return ((FarCustomPanelPlugin*)hPlugin)->DeleteFiles( Items, OpMode );
  }

  virtual void FreeVirtualFindData( HANDLE hPlugin, FarPluginPanelItems *Items )
  {
    far_assert( hPlugin != NULL );
    ((FarCustomPanelPlugin*)hPlugin)->FreeVirtualFindData( Items );
  }

  virtual bool GetVirtualFindData( HANDLE hPlugin, FarPluginPanelItems *Items, LPCSTR Path )
  {
    far_assert( hPlugin != NULL );
    return ((FarCustomPanelPlugin*)hPlugin)->GetVirtualFindData( Items, Path );
  }

  virtual int MakeDirectory( HANDLE hPlugin, LPCSTR Name, int OpMode )
  {
    far_assert( hPlugin != NULL );
    return ((FarCustomPanelPlugin*)hPlugin)->MakeDirectory( Name, OpMode );
  }

  virtual bool ProcessEvent( HANDLE hPlugin, FAR_EVENTS Event, PVOID Param )
  {
    far_assert( hPlugin != NULL );
    return ((FarCustomPanelPlugin*)hPlugin)->ProcessEvent( Event, Param );
  }

  virtual bool ProcessHostFile( HANDLE hPlugin, const FarPluginPanelItems *Items, int OpMode )
  {
    far_assert( hPlugin != NULL );
    return ((FarCustomPanelPlugin*)hPlugin)->ProcessHostFile( Items, OpMode );
  }

  virtual bool SetDirectory( HANDLE hPlugin, LPCSTR Dir, int OpMode )
  {
    far_assert( hPlugin != NULL );
    return ((FarCustomPanelPlugin*)hPlugin)->SetDirectory( Dir, OpMode );
  }

  virtual bool SetFindList( HANDLE hPlugin, const FarPluginPanelItems *Items )
  {
    far_assert( hPlugin != NULL );
    return ((FarCustomPanelPlugin*)hPlugin)->SetFindList( Items );
  }

  virtual bool Configure( int ItemNumber )
  {
    return false;
  }

};

extern FarCustomPlugin * Plugin;

#define DECLARE_PLUGIN( ClassName ) extern "C" void WINAPI SetStartupInfo( const PluginStartupInfo * Info ) { Far::Init( Info ); Plugin = create ClassName; }

void DbgMsg( char const * const Title, char const * const Fmt, ... );
void DbgMsg( char const * const Fmt, ... );

#endif // !defined(___WPlugin_H___)
