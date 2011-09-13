
#include "FarCall.h"
static PluginStartupInfo    Info;
static FarStandardFunctions FSF;

static char ModuleName[MAX_PATH * 2];
static char ModulePath[sizeof( ModuleName )];
static bool connected = false;

#define EOL "\xd\xa"

const char* batch = 
  "@call %s" EOL
  "@rundll32.exe \"%s\",Export %s" EOL;



BOOL APIENTRY DllMain( HINSTANCE hinstDLL,
  DWORD     fdwReason,
  LPVOID    /*lpReserved*/ )
{
  if( !hinstDLL )
    return FALSE;

  char* name;

  switch( fdwReason )
  {
  case DLL_PROCESS_ATTACH:
    DebugString( "process attach" );
    heap = HeapCreate( HEAP_GENERATE_EXCEPTIONS, 10 * 1024, 0 );

    // get module name & path
    GetModuleFileName( hinstDLL, ModuleName, sizeof( ModuleName ) );
    GetFullPathName( ModuleName, sizeof( ModulePath ), ModulePath, &name );
    assert( name );
    *name = 0;
    DebugString( ModuleName );
    DebugString( ModulePath );
    break;
  case DLL_THREAD_ATTACH:
    DebugString( "thread attach" );
    break;
  case DLL_THREAD_DETACH:
    DebugString( "thread detach" );
    break;
  case DLL_PROCESS_DETACH:
    DebugString( "process detach" );
    if( heap )
    {
      HeapDestroy( heap );
      heap = 0;
    }
    break;
  }

  return TRUE;
}

/**
Global information about the plugin
*/
void WINAPI GetGlobalInfoW(struct GlobalInfo *gInfo)
{
  gInfo->StructSize = sizeof(GlobalInfo);
  gInfo->MinFarVersion = FARMANAGERVERSION;
  gInfo->Version = MAKEFARVERSION(1,0,0,5,VS_BETA);
  gInfo->Guid = MainGuid;
  gInfo->Title = L"FarCall";
  gInfo->Description =L"Plugin allows to call external batch files & import environment variables to Far process";
  gInfo->Author = L"hoopoepg, Dobrunov Aleksey";
}

void WINAPI GetPluginInfoW( struct PluginInfo *pInfo )
{
  if( !pInfo )
    return;

  static wchar_t* config[] = {L"Far Call"};
  static wchar_t* call = L"call";

  memset(pInfo, 0, sizeof(*pInfo));
  pInfo->StructSize = sizeof( *pInfo );
  pInfo->Flags = PF_NONE;
  pInfo->PluginConfig.Count = 0;
  pInfo->DiskMenu.Count = 0;
  pInfo->PluginMenu.Count = 1;
  pInfo->PluginMenu.Strings = config;
  pInfo->CommandPrefix = call;
  pInfo->PluginMenu.Guids = &PluginMenu;
}

void WINAPI SetStartupInfoW( const struct PluginStartupInfo *info )
{
  Info = *info;
  FSF = *Info.FSF;
  Info.FSF = &FSF;
}

static DWORD WINAPI ListenEnv( LPVOID lpParameter )
{
  HANDLE pipe = (HANDLE)lpParameter;

  connected = true;
  ConnectNamedPipe( pipe, 0 );

  char buffer[MAX_PATH];
  char* envs = 0;

  DWORD read = 0;
  int len = 0;

  while( ReadFile( pipe, buffer, MAX_PATH, &read, 0 ) || read )
  {
    envs = (char*)realloc( envs, len + read );
    lmemcpy( envs + len, buffer, read );
    len += read;
    read = 0;
  }

  if( envs )
  {
    wchar_t* var = (wchar_t*)envs;
    while( *var )
    {
      int len = lstrlenW( var );
      if( lstrchr( var + 1, '=' ) > 0 )
      {
        int pos = lstrchr( var + 1, '=' ) + 1;
        var[pos] = 0;
        while( lstrlenW( var ) && var[lstrlenW( var ) - 1] <= ' ' )
          var[lstrlenW( var ) - 1] = 0;
        wchar_t* val = var + pos + 1;
        while( *val && *val <= ' ' )
          val++;
        SetEnvironmentVariableW( var, val );
      }
      var += len + 1;
    }

    free( envs );
  }

  return 0;
}

HANDLE WINAPI OpenW( const struct OpenInfo *oInfo)
{
  char* filename = 0;

  switch( oInfo->OpenFrom )
  {
  case OPEN_COMMANDLINE:
    {
      wchar_t* cmdline = (wchar_t*)oInfo->Data;
      if( !cmdline )
        return INVALID_HANDLE_VALUE;

      while( *cmdline && *cmdline <= ' ' )
        cmdline++;

      if( *cmdline )
        filename = w2a( cmdline );
      else
        return INVALID_HANDLE_VALUE;
      break;
    }
  case OPEN_PLUGINSMENU:
    {
      size_t Size=Info.PanelControl( PANEL_ACTIVE, FCTL_GETCURRENTPANELITEM, 0, NULL);
      PluginPanelItem *PPI=(PluginPanelItem*)malloc(Size);

      if (PPI)
      {
        FarGetPluginPanelItem FGPPI={Size,PPI};
        Info.PanelControl( PANEL_ACTIVE, FCTL_GETCURRENTPANELITEM, 0, &FGPPI );
        filename = w2a(FGPPI.Item->FileName);
        free(PPI);
      }
      else
        return INVALID_HANDLE_VALUE;
      break;
    }
  default:
    return INVALID_HANDLE_VALUE;
  }

  if( !filename )
    return INVALID_HANDLE_VALUE;

  wchar_t comspec[MAX_PATH * 2];
  if( !GetEnvironmentVariableW( L"COMSPEC", comspec, sizeofa( comspec ) ) )
    lstrcpyW( comspec, L"cmd.exe" );

  char pipename[100];
  wsprintf( pipename, "\\\\.\\pipe\\FarCall%d", GetCurrentProcessId() );

  char* batchstr = (char*)malloc( 10000 );
  wsprintf( batchstr, batch, filename, ModuleName, pipename );

  // obtaining temp file name
  wchar_t tmp[MAX_PATH * 10];
  GetTempPathW( sizeofa( tmp ), tmp );
  GetTempFileNameW( tmp, L"", 0, tmp );
  DeleteFileW( tmp );

  lstrcatW( tmp, L".bat" );

  HANDLE file = CreateFileW( tmp, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0 );
  if( !file || file == INVALID_HANDLE_VALUE )
  {
    DeleteFileW( tmp );
    free( filename );
    free( batchstr );
    return INVALID_HANDLE_VALUE;
  }

  DWORD written;
  WriteFile( file, batchstr, lstrlen( batchstr ), &written, 0 );
  CloseHandle( file );

  wchar_t cmd[MAX_PATH * 10] = L"\"";
  lstrcatW( lstrcatW( lstrcatW( lstrcatW( cmd, comspec ), L"\" /c \"" ), tmp ), L"\"");

  STARTUPINFOW sinfo;
  ZeroMemory( &sinfo, sizeof( sinfo ) );
  sinfo.cb = sizeof( sinfo );

  PROCESS_INFORMATION pinfo;

  Handle np( CreateNamedPipe( pipename, PIPE_ACCESS_DUPLEX, PIPE_WAIT,
    PIPE_UNLIMITED_INSTANCES, 100, 100, 0, 0 ) );

  connected = false;

  DWORD id;
  Handle thread( CreateThread( 0, 0, ListenEnv, np, 0, &id ) );

  while( !connected )
    Sleep( 100 );

  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &csbi );

#ifndef LIGHTGRAY
#define LIGHTGRAY 7
#endif

  wchar_t Blank[1024];
  FSF.sprintf(Blank,L"%*s",csbi.dwSize.X,L"");
  FarColor Color;
  Color.BackgroundColor=0;
  Color.ForegroundColor = LIGHTGRAY;
  Color.Flags=FCF_4BITMASK;
  Color.Reserved=0;
  for (int Y=0;Y<csbi.dwSize.Y;Y++)
            Info.Text(0,Y,&Color,Blank);
  Info.Text(0,0,0,NULL);

  COORD C;
  C.X=0;
  C.Y=csbi.dwCursorPosition.Y;
  SetConsoleCursorPosition( GetStdHandle( STD_OUTPUT_HANDLE ), C );

  const int CUR_DIR_SIZE = 10000;
  wchar_t* curdir = (wchar_t*)malloc(CUR_DIR_SIZE * sizeof(*curdir));
  ZeroMemory(curdir, CUR_DIR_SIZE * sizeof(*curdir));

  FSF.GetCurrentDirectory(CUR_DIR_SIZE,curdir);

  if( np && CreateProcessW( comspec, cmd, 0, 0, TRUE, 0, 0, curdir, &sinfo, &pinfo ) )
  {
    CloseHandle( pinfo.hThread );

    HANDLE ar[] = {pinfo.hProcess, np};

    WaitForMultipleObjects( 2, ar, TRUE, INFINITE );

    CloseHandle( pinfo.hProcess );

    SMALL_RECT src;
    COORD dest;
    CHAR_INFO fill;
    src.Left=0;
    src.Top=2;
    src.Right=csbi.dwSize.X;
    src.Bottom=csbi.dwSize.Y;
    dest.X=dest.Y=0;
    fill.Char.AsciiChar=' ';
    fill.Attributes=7;
    ScrollConsoleScreenBuffer( GetStdHandle( STD_OUTPUT_HANDLE ), &src, NULL, dest, &fill);

    Info.AdvControl(&MainGuid, ACTL_REDRAWALL,0, 0);
  }
  else
    Handle onp( CreateFile( pipename, GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0 ) );


  free( filename );
  free( batchstr );
  free( curdir );
  DeleteFileW( tmp );

  return INVALID_HANDLE_VALUE;
}

void WINAPI ClosePluginW( HANDLE /*hPlugin*/ )
{
  return;
}

void CALLBACK Export( HWND, HINSTANCE, LPTSTR lpCmdLine, int )
{
  if( !lpCmdLine )
    ExitProcess( 0 );
  Handle onp( CreateFile( lpCmdLine, GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0 ) );

  if( !onp || onp == INVALID_HANDLE_VALUE )
    ExitProcess( 0 );

  wchar_t* env = GetEnvironmentStringsW();
  int i;
  for( i = 0; *(int*)(env + i); i++ );

  i *= sizeof(*env);
  i += 2 * sizeof(*env);

  DWORD written;

  WriteFile( onp, env, i, &written, 0 );

  FreeEnvironmentStringsW( env );

  return;
}

// in order to not fall when it starts in far2 
int WINAPI GetMinFarVersionW(void)
{
#define MAKEFARVERSION_OLD(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))

  return MAKEFARVERSION_OLD(FARMANAGERVERSION_MAJOR,FARMANAGERVERSION_MINOR,FARMANAGERVERSION_BUILD);
}






