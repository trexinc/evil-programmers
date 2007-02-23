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
class CHyperViewConverter
{
  typedef HANDLE (__cdecl *THV_Document_Create)(PluginStartupInfo*,LPCSTR);
  typedef void   (__cdecl *THV_Document_Destroy)(HANDLE);
  typedef LPCSTR (__cdecl *THV_Document_MakeTxtFile)(HANDLE,LPCSTR,COORD);

  THV_Document_Create      docCreate      = NULL;
  THV_Document_Destroy     docDestroy     = NULL;
  THV_Document_MakeTxtFile docMakeTxtFile = NULL;

  PluginStartupInfo info;

  HMODULE hModule;
  HANDLE  hDocument;
public:
  CHyperViewConverter( PluginStartupInfo info, LPCSTR dllFileName = NULL, LPCSTR cfgFileName = NULL )
  {
    if ( !info )
    {
      hModule = NULL;
      return;
    }

    this.info = info;

    hModule = LoadLibrary( dllFileName ? dllFileName : "HyperView.dll" );
    if ( hModule )
    {
      docCreate      = (THV_Document_Create)GetProcAddress( hMod, "HV_Document_Create" );
      docDestroy     = (THV_Document_Destroy)GetProcAddress( hMod, "HV_Document_Destroy" );
      docMakeTxtFile = (THV_Document_MakeTxtFile)GetProcAddress( hMod, "HV_Document_MakeTxtFile" );
      if ( !docCreate || !docDestroy || !docMakeTxtFile )
      {
        FreeLibrary( hModule );
        hModule = NULL;
      }
    }
  }
  ~CHyperViewConverter()
  {
    if ( hModule )
    {
      FreeLibrary( hModule );
    }
  }

  LPCSTR makeTextFile( LPCSTR htmlFileName, short width = 0 )
  {
    if ( hModule == NULL )
    {
      return htmlFileName;
    }

    COORD windowSize = { width, 0 };

    if ( hDocument )
    {
      docDestroy( hDocument );
      hDocument = NULL;
    }
  }
};
  HANDLE  hDoc = NULL;
  HMODULE hMod = NULL;

  CMimeContent mc;
  /*!!!pMsg->SaveBodyToFile( TmpFileName );*/
  PMsgPart TextPart = pMsg->GetTextPart();
  if ( TextPart )
  {
    TextPart->SaveToFile( TmpFileName );
    mc.Assign( TextPart->GetKludge( K_RFC_ContentType ) );
  }

  if ( mc.Type && FarSF::LStricmp( mc.Type, "text" ) == 0 && FarSF::LStricmp( mc.SubType, "html" ) == 0 )
  {
    hMod = LoadLibrary( ViewCfg()->GetHVLib() );
    if ( hMod )
    {
      typedef HANDLE (__cdecl *THV_Document_Create)(PluginStartupInfo*,LPCSTR);
      typedef void   (__cdecl *THV_Document_Destroy)(HANDLE);
      typedef LPCSTR (__cdecl *THV_Document_MakeTxtFile)(HANDLE,LPCSTR,COORD);
      docCreate      = (THV_Document_Create)GetProcAddress( hMod, "HV_Document_Create" );
      docDestroy     = (THV_Document_Destroy)GetProcAddress( hMod, "HV_Document_Destroy" );
      docMakeTxtFile = (THV_Document_MakeTxtFile)GetProcAddress( hMod, "HV_Document_MakeTxtFile" );
      if ( docCreate && docDestroy && docMakeTxtFile )
      {
        hDoc = docCreate( Far::GetPluginStartupInfo(), ViewCfg()->GetHVCfg().data() );
        if ( hDoc )
        {
          COORD c = { GetWidth(), GetHeight() };
          LPCSTR TxtFileName = docMakeTxtFile( hDoc, TmpFileName, c );
          if ( TxtFileName )
          {
            SetFileAttributes( m_FileName, FILE_ATTRIBUTE_NORMAL );
            DeleteFile( TmpFileName );
            TmpFileName = TxtFileName;
          }
          docDestroy( hDoc );
        }
      }
      FreeLibrary( hMod );
    }
  }

  m_XFTNOrigin   = pMsg->GetKludge( K_RFC_XFTNOrigin );
  m_XFTNTearline = pMsg->GetKludge( K_RFC_XFTNTearline );
  SetEncoding( GetMsgEncoding( pMsg, TextPart ), false );
  LoadFromFile( TmpFileName, true );
