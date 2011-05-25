// FarCall.cpp : Defines the entry point for the DLL application.
//

#define malloc __malloc
#define free __free
#define realloc __realloc

#include <windows.h>

#ifndef EXCLUDE_ANSI_API
#  include "plugin.hpp"
#endif // EXCLUDE_ANSI_API

namespace W
{
    typedef DWORD* DWORD_PTR;
    typedef LONG*  LONG_PTR;
#include "pluginW.hpp"
}

#undef malloc
#undef free
#undef realloc

#ifdef DEBUG
#define DebugString(x) {OutputDebugString( __STR__(__LINE__)": " );OutputDebugString( x );OutputDebugString( "\n" );}
#define __STR2__(x) #x
#define __STR__(x) __STR2__(x)
#define assert( x ) if( !(x) ) DebugString( "Assertion "#x" in line "__STR__(__LINE__) )
#else
#define DebugString(x)
#define assert( x )
#endif

#ifdef ZeroMemory
#undef ZeroMemory
#endif

#define ZeroMemory( ptr, size ) {for( int i = 0; i < size; i++ ) *((char*)(ptr) + i) = 0;}

#define sizeofa( x ) (sizeof( x ) / sizeof( *x ))

static HANDLE heap = 0;

static void* malloc( size_t size )
{
    assert( heap );
    return HeapAlloc( heap, HEAP_ZERO_MEMORY, size );
}

static void free( void* block )
{
    assert( heap );
    if( block )
        HeapFree( heap, 0, block );
    return;
}

static void* realloc( void* block, size_t size )
{
    assert( heap );

    if( block && !size )
    {
        HeapFree( heap, 0, block );
        return 0;
    }
    else if( block && size )
        return HeapReAlloc( heap, HEAP_ZERO_MEMORY, block, size );
    else if( !block && size )
        return HeapAlloc( heap, HEAP_ZERO_MEMORY, size );
    else // !block && !size
        return 0;
}

static char* lstrdup( const char* str )
{
    if( !str )
        return 0;
    char* dup = (char*)malloc( (lstrlen( str ) + 1) * sizeof(*str) );
    if( !dup )
        return 0;
    lstrcpy( dup, str );
    return dup;
}

static wchar_t* lstrdup( const wchar_t* str )
{
    if( !str )
        return 0;
    wchar_t* dup = (wchar_t*)malloc( (lstrlenW( str ) + 1) * sizeof(*str) );
    if( !dup )
        return 0;
    lstrcpyW( dup, str );
    return dup;
}

static int lstrchr( const wchar_t* str, wchar_t sym )
{
    for( int i = 0; str[i]; i++ )
        if( str[i] == sym )
            return i;
    return -1;
}

static void lmemcpy( void* dst, void* src, int len )
{
    if( !dst || !src || !len )
        return;

    char* _src = (char*)src;
    char* _dst = (char*)dst;
    for( int i = 0; i < len; i++, _dst++, _src++ )
        *_dst = *_src;
}

static char* w2a(const char* s, int cp = CP_ACP)
{
    return lstrdup(s);
}

static char* w2a(const wchar_t* s, int cp = CP_ACP)
{
    char* res = 0;
    size_t size = 0;

    if (!s)
        return NULL;

    size = WideCharToMultiByte(cp, 0, s, -1, 0, 0, NULL, NULL) + 1;

    if (size)
    {
        res = (char*)malloc(size * sizeof(*res));
        WideCharToMultiByte(cp, 0, s, -1, res, size, NULL, NULL);
    }

    return res;
}

static wchar_t* a2w(const wchar_t* s, int cp = CP_ACP)
{
    return lstrdup(s);
}

static wchar_t* a2w(const char* s, int cp = CP_ACP)
{
    wchar_t* res = 0;
    size_t size = 0;

    if (!s)
        return NULL;

    size = MultiByteToWideChar(cp, 0, s, -1, 0, 0) + 1;

    if (size)
    {
        res = (wchar_t*)malloc(size * sizeof(*res));
        MultiByteToWideChar(cp, 0, s, -1, res, size);
    }

    return res;
}


static char ModuleName[MAX_PATH * 2];
static char ModulePath[sizeof( ModuleName )];


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

#ifndef EXCLUDE_ANSI_API
void WINAPI GetPluginInfo( struct PluginInfo *Info )
{
    if( !Info )
        return;
    ZeroMemory( Info, sizeof( *Info ) );

    static char* config[] = {"Far Call"};
    
    Info->StructSize = sizeof( *Info );
    Info->PluginMenuStrings = config;
    Info->PluginMenuStringsNumber = 1;
    Info->CommandPrefix = "call";
}
#endif // EXCLUDE_ANSI_API

void WINAPI GetPluginInfoW( struct W::PluginInfo *Info )
{
    if( !Info )
        return;

    static wchar_t* config[] = {L"Far Call"};
    static wchar_t* call = L"call";
    
    Info->StructSize = sizeof( *Info );
    Info->PluginMenuStrings = config;
    Info->PluginMenuStringsNumber = 1;
    Info->CommandPrefix = call;
}

#ifndef EXCLUDE_ANSI_API
static PluginStartupInfo    Info;
static FarStandardFunctions FSF;
#endif // EXCLUDE_ANSI_API

static W::PluginStartupInfo    InfoW;
static W::FarStandardFunctions FSFW;

#ifndef EXCLUDE_ANSI_API
void WINAPI SetStartupInfo( const struct PluginStartupInfo *info )
{
    if( info )
    {
        Info = *info;
        if(Info.FSF)
            FSF = *Info.FSF;
        Info.FSF = &FSF;
    }
}
#endif // EXCLUDE_ANSI_API

void WINAPI SetStartupInfoW( const struct W::PluginStartupInfo *info )
{
    if( info )
    {
        InfoW = *info;
        FSFW = *InfoW.FSF;
        InfoW.FSF = &FSFW;
        //InfoW.FSF = InfoW.FSF;
    }
}

class Handle
{
private:
    HANDLE m_Handle;
    bool   m_NoClose;
public:
    Handle() { m_Handle = 0; m_NoClose = false; }
    Handle( HANDLE handle, bool noclose = false )
        { m_Handle = handle; m_NoClose = noclose; }
    ~Handle()
        { if( m_Handle && m_Handle != INVALID_HANDLE_VALUE && ! m_NoClose ) CloseHandle( m_Handle ); m_Handle = 0; }
    operator HANDLE() { return m_Handle; }
    HANDLE* operator &() { return &m_Handle; }
    Handle& operator =(HANDLE src) {m_Handle = src; return *this;}
    bool NoClose( bool noclose ) { return m_NoClose = noclose; }
};

static bool connected = false;

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


#define EOL "\xd\xa"

const char* batch = //"@cls" EOL
                    "@call %s" EOL
                    "@rundll32.exe \"%s\",Export %s" EOL;
//                    "@call \"%s\"" EOL
//                    "@rundll32.exe \"%s\",Export %s" EOL;

#ifndef EXCLUDE_ANSI_API
HANDLE WINAPI OpenPlugin( int OpenFrom, int item )
{
    char* filename = 0;

    switch( OpenFrom )
    {
    case OPEN_COMMANDLINE:
        {
            char* cmdline = (char*)item;
            if( !cmdline )
                return INVALID_HANDLE_VALUE;

            while( *cmdline && *cmdline <= ' ' )
                cmdline++;

            if( *cmdline )
                filename = lstrdup( cmdline );
            else
                return INVALID_HANDLE_VALUE;
            break;
        }
    case OPEN_PLUGINSMENU:
        {
            PanelInfo pinfo;
            Info.Control( INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &pinfo );
            if( pinfo.CurrentItem > 0 )
            {
                filename = lstrdup( pinfo.PanelItems[pinfo.CurrentItem].FindData.cFileName );
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

    char comspec[MAX_PATH * 2];
    if( !GetEnvironmentVariable( "COMSPEC", comspec, sizeofa( comspec ) ) )
        lstrcpy( comspec, "cmd.exe" );

    char pipename[100];
    wsprintf( pipename, "\\\\.\\pipe\\FarCall%d", GetCurrentProcessId() );

    char* batchstr = (char*)malloc( 10000 );
    wsprintf( batchstr, batch, filename, ModuleName, pipename );

    // obtaining temp file name
    char tmp[MAX_PATH * 10];
    GetTempPath( sizeofa( tmp ), tmp );
    GetTempFileName( tmp, "", 0, tmp );
    DeleteFile( tmp );

    lstrcat( tmp, ".bat" );

    HANDLE file = CreateFile( tmp, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0 );
    if( !file || file == INVALID_HANDLE_VALUE )
    {
        DeleteFile( tmp );
        free( filename );
        free( batchstr );
        return INVALID_HANDLE_VALUE;
    }

    DWORD written;
    WriteFile( file, batchstr, lstrlen( batchstr ), &written, 0 );
    CloseHandle( file );

    char cmd[MAX_PATH * 10] = "\"";
    lstrcat( lstrcat( lstrcat( lstrcat( cmd, comspec ), "\" /c \"" ), tmp ), "\"");

    STARTUPINFO sinfo;
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

    char Blank[1024];
    FSF.sprintf(Blank,"%*s",csbi.dwSize.X,"");
    for (int Y=0;Y<csbi.dwSize.Y;Y++)
        Info.Text(0,Y,LIGHTGRAY,Blank);
    Info.Text(0,0,0,NULL);
    
    COORD C;
    C.X=0;
    C.Y=csbi.dwCursorPosition.Y;
    SetConsoleCursorPosition( GetStdHandle( STD_OUTPUT_HANDLE ), C );

    if( np && CreateProcess( comspec, cmd, 0, 0, TRUE, 0, 0, 0, &sinfo, &pinfo ) )
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

        Info.Control( INVALID_HANDLE_VALUE, FCTL_SETUSERSCREEN, 0 );
    }
    else
        Handle onp( CreateFile( pipename, GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0 ) );

    
    free( filename );
    free( batchstr );
    DeleteFile( tmp );
    
    return INVALID_HANDLE_VALUE;
}
#endif // EXCLUDE_ANSI_API

HANDLE WINAPI OpenPluginW( int OpenFrom, int item )
{
    char* filename = 0;

    switch( OpenFrom )
    {
    case W::OPEN_COMMANDLINE:
        {
            wchar_t* cmdline = (wchar_t*)item;
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
    case W::OPEN_PLUGINSMENU:
        {
            W::PluginPanelItem pinfo;
            if(InfoW.Control( PANEL_ACTIVE, W::FCTL_GETCURRENTPANELITEM, 0, (long*)&pinfo ))
            {
                filename = w2a( pinfo.FindData.lpwszFileName );
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
    FSFW.sprintf(Blank,L"%*s",csbi.dwSize.X,L"");
    for (int Y=0;Y<csbi.dwSize.Y;Y++)
        InfoW.Text(0,Y,LIGHTGRAY,Blank);
    InfoW.Text(0,0,0,NULL);
    
    COORD C;
    C.X=0;
    C.Y=csbi.dwCursorPosition.Y;
    SetConsoleCursorPosition( GetStdHandle( STD_OUTPUT_HANDLE ), C );

    const int CUR_DIR_SIZE = 10000;
    wchar_t* curdir = (wchar_t*)malloc(CUR_DIR_SIZE * sizeof(*curdir));
    ZeroMemory(curdir, CUR_DIR_SIZE * sizeof(*curdir));

    FSFW.GetCurrentDirectory(CUR_DIR_SIZE,curdir);
    //InfoW.Control(PANEL_ACTIVE, W::FCTL_GETPANELDIR, CUR_DIR_SIZE, (long*)curdir);

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

        InfoW.AdvControl(0, W::ACTL_REDRAWALL, 0);
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

#ifndef EXCLUDE_ANSI_API
void WINAPI ClosePlugin( HANDLE /*hPlugin*/ )
{
    return;
}
#endif // EXCLUDE_ANSI_API

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





