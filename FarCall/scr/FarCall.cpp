// FarCall.cpp : Defines the entry point for the DLL application.
//

#define malloc __malloc
#define free __free
#define realloc __realloc

#include <windows.h>

namespace W
{
    typedef DWORD* DWORD_PTR;
    typedef LONG*  LONG_PTR;
}

#include "plugin.hpp"

static const GUID PluginGuid = // {7CD88BB2-9C2D-4A46-A39A-15629816932C}
{ 0x7cd88bb2, 0x9c2d, 0x4a46, { 0xa3, 0x9a, 0x15, 0x62, 0x98, 0x16, 0x93, 0x2c } };

static const GUID MenuGuid = // {7CD88BB2-9C2D-4A46-A39A-15629816932D}
{ 0x7cd88bb2, 0x9c2d, 0x4a46, { 0xa3, 0x9a, 0x15, 0x62, 0x98, 0x16, 0x93, 0x2d } };

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

static char* w2a(const wchar_t* s, int cp = CP_ACP)
{
    char* res = 0;
    int size = 0;

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

void WINAPI GetGlobalInfoW( struct GlobalInfo* Info )
{
    if(Info)
    {
        ZeroMemory( Info, sizeof( *Info ) );
        Info->StructSize = sizeof(*Info);
        Info->MinFarVersion = FARMANAGERVERSION;
        Info->Version = MAKEFARVERSION(3, 0, 1, 1, VS_RELEASE);
        Info->Guid = PluginGuid;
        Info->Title = L"Far Call";
        Info->Description = L"Plugin allows execute any batch file & import output environment variables";
        Info->Author = L"hoopoepg :)";
    }
}


void WINAPI GetPluginInfoW( struct PluginInfo *Info )
{
    if( !Info )
        return;

    static wchar_t* config[] = {L"Far Call"};

    static PluginMenuItem item = {&MenuGuid, config, 1};
    
    ZeroMemory( Info, sizeof( *Info ) );
    Info->StructSize = sizeof( *Info );
    Info->PluginMenu = item;
    Info->CommandPrefix = L"call";
}

static PluginStartupInfo    InfoW;
static FarStandardFunctions FSFW;

void WINAPI SetStartupInfoW( const struct PluginStartupInfo *info )
{
    if( info )
    {
        InfoW = *info;
        FSFW = *InfoW.FSF;
        InfoW.FSF = &FSFW;
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
    DisconnectNamedPipe(pipe);
    return 0;
}


#define EOL "\xd\xa"

const char* batch = //"@cls" EOL
                    "@call %s" EOL
                    "@rundll32.exe \"%s\",Export %s" EOL;
//                    "@call \"%s\"" EOL
//                    "@rundll32.exe \"%s\",Export %s" EOL;

HANDLE WINAPI OpenW(const struct OpenInfo *Info)
{
    const int CUR_DIR_SIZE = 100000;
    char* filename = 0;

    switch( Info->OpenFrom )
    {
    case OPEN_COMMANDLINE:
        {
            OpenCommandLineInfo* cinfo = (OpenCommandLineInfo*)Info->Data;

            const wchar_t* cmdline = cinfo->CommandLine;
            if( !cmdline )
                return INVALID_HANDLE_VALUE;

            while( *cmdline && *cmdline <= ' ' )
                cmdline++;

            if( *cmdline )
                filename = w2a( cmdline );
            else
                return 0;
            break;
        }
    case OPEN_PLUGINSMENU:
        {
            FarGetPluginPanelItem pinfo;
            PluginPanelItem* pitem = (PluginPanelItem*)malloc(CUR_DIR_SIZE);
            ZeroMemory(&pinfo, sizeof(pinfo));
            ZeroMemory(pitem, CUR_DIR_SIZE);
            pinfo.StructSize = sizeof(pinfo);
            pinfo.Size = CUR_DIR_SIZE;
            pinfo.Item = pitem;

            if(InfoW.PanelControl( PANEL_ACTIVE, FCTL_GETCURRENTPANELITEM, 0, &pinfo ))
            {
                filename = w2a( pinfo.Item->FileName );
                free(pitem);
            }
            else
            {
                free(pitem);
                return 0;
            }
            break;
        }
    default:
        return 0;
    }
    
    if( !filename )
        return 0;

    wchar_t comspec[MAX_PATH * 2];
    if( !GetEnvironmentVariableW( L"COMSPEC", comspec, sizeofa( comspec ) ) )
        lstrcpyW( comspec, L"cmd.exe" );

    char pipename[100];
    wsprintf( pipename, "\\\\.\\pipe\\FarCall%ul", GetCurrentProcessId() );

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
    FarColor fc = {FCF_NONE, LIGHTGRAY, 0, 0};
    for (int Y=0;Y<csbi.dwSize.Y;Y++)
        InfoW.Text(0,Y,&fc,Blank);
    InfoW.Text(0,0,0,NULL);
    
    COORD C;
    C.X=0;
    C.Y=csbi.dwCursorPosition.Y;
    SetConsoleCursorPosition( GetStdHandle( STD_OUTPUT_HANDLE ), C );

    wchar_t* curr_dir = (wchar_t*)malloc(CUR_DIR_SIZE);
    
    FSFW.GetCurrentDirectory(CUR_DIR_SIZE, curr_dir);

    if( np && CreateProcessW( NULL, cmd, 0, 0, TRUE, 0, 0, curr_dir[0] ? curr_dir : 0, &sinfo, &pinfo ) )
    {

        HANDLE ar[] = {pinfo.hProcess, np};

        WaitForMultipleObjects( 2, ar, TRUE, INFINITE );

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

        InfoW.AdvControl(0, ACTL_REDRAWALL, 0, 0);
    }
    else
        Handle onp( CreateFile( pipename, GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0 ) );

    
    free( filename );
    free( batchstr );
    free( curr_dir );
    DeleteFileW( tmp );
    
    return 0;
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





