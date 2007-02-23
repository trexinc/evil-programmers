/*************************************************************************
  FarDbg.cpp

  (c) 2001-02 Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>

  Revision: 1.23 [04/10/2002]

*************************************************************************/

#include <Windows.h>
//#define _CRTDBG_MAP_ALLOC
//#include <CrtDbg.h>
#ifndef _msize
#define _msize(p) _msize_dbg(p, _NORMAL_BLOCK)
#endif

#include "FarPlus.h"
#include "FarDbg.h"

#ifdef FARPLUS_TRACE

void far_ods( char const * const Fmt, ... )
{
	// здесь не используется create/delete, на случай, если
	// захочется попользовать в CDbgReportInit::AllocHook(),
	// иначе, по понятным причинам, получим переполнение стека :)
	HANDLE Heap = GetProcessHeap();

	char * Tmp = (char*)HeapAlloc( Heap, 0, 0x1000 );

	va_list argPtr;
	va_start( argPtr, Fmt );

	wvsprintf( Tmp, Fmt, argPtr );

	va_end( argPtr );

	OutputDebugString( Tmp );

	HeapFree( Heap, 0, Tmp );
}

#endif

extern "C" int __cdecl _purecall(void)
{
	if ( Far::IsInitialized() )
	{
		FarMessage msg( FMSG_WARNING );
		msg.AddLine( "Error" );
		msg.AddLine( "Pure virtual function call" );
		msg.AddSeparator();
		msg.AddButton( "&Termitate" );
#ifdef _DEBUG
		msg.AddButton( "&Debug" );
#endif
		if ( msg.Show() == 1 )
			asm("int $3");
		else
			ExitProcess( 0 );
	}
	else
	{
		if ( MessageBox( NULL, NULL, "Pure virtual function call"
#ifdef _DEBUG
			"\n\nPress Cancel to debug the application - JIT must be enabled"
			, MB_OKCANCEL|
#else
			, MB_OK|
#endif
			MB_TASKMODAL|MB_ICONERROR|MB_SETFOREGROUND
			) == IDCANCEL )
			asm("int $3");
		else
			ExitProcess( 0 );
	}

	return 0;
}

int show_assert_message( const char * FileName, const int Line, const char * Expression, bool bShowWinError, bool bDumpStack )
{
	static LONG AssertBusy = -1;

	if ( InterlockedIncrement( &AssertBusy ) > 0 )
	{
		InterlockedDecrement( &AssertBusy );
		far_trace( "DEBUG: assert within assert (examine call stack to determine first one)" );
		asm("int $3");
	}

	int Result = 0;

	if ( Far::IsInitialized() )
	{
		FarString FN = FileName;
		FarMessage Msg( FMSG_LEFTALIGN|FMSG_WARNING|(bShowWinError?FMSG_ERRORTYPE:0) );
		Msg.AddLine( "Debug Assertion Failed!" );
		Msg.AddFmt( "Plugin    : %s", FarSF::PointToName( Far::GetModuleName() ) );
		Msg.AddFmt( "File      : %hs, Line %d", FarSF::TruncPathStr( FN.GetBuffer(), 40 ), Line );
		Msg.AddFmt( "Expression: %s", Expression );
		Msg.AddSeparator();
		Msg.AddButton( "&Abort" );
		Msg.AddButton( "&Debug" );
		Msg.AddButton( "&Ignore" );

		if ( bDumpStack )
			Msg.AddButton( "Dump &Stack" );

		Result = Msg.Show();
	}
	else
	{
		char buf[ 4096 ];
		FarSF::snprintf( buf, sizeof( buf ), "File: %hs, Line %d\nExpression: %s\n\n"
			"Press Retry to debug the application - JIT must be enabled",
			FileName, Line, Expression );

		Result = MessageBox( NULL, buf, "Debug Assertion Failed!",
			MB_ABORTRETRYIGNORE|MB_TASKMODAL|MB_ICONERROR|MB_SETFOREGROUND );

		switch ( Result )
		{
		case IDIGNORE:
			Result = 2;
			break;
		case IDRETRY:
			Result = 1;
			break;
		default:
			Result = 0;
			break;
		}
	}

	InterlockedDecrement( &AssertBusy );

	return Result;
}

int far_assert_message( const char * FileName, const int Line, const char * Expression, bool bShowWinError )
{
	bool bDumpStack = true;

	while ( true )
	{
		int result = show_assert_message( FileName, Line, Expression, bShowWinError, bDumpStack );

		switch ( result ) // abort
		{
			case 0: // abort
				ExitThread( 0 );

			case 1: // debug
				return 1;

			case 2: // ignore
				return -1;

			case 3: // dump stack
				bDumpStack = false;
				//FarDumpStack();
				break;
		}
	}

	return 1; // never
}

#ifdef _MSC_VER

inline bool dbg_isspace( char c )
{
	return c == '\n' || c == '\r' || c == '\x20';
}

int far_dbg_report( int reportType, char * userMessage, int * retVal )
{
	char * RptTypes[] = { "Warning", "Error", "Assertion Failed" };

	while ( dbg_isspace( *(userMessage + strlen( userMessage ) - 1) ) )
		*(userMessage + strlen( userMessage ) - 1) = '\0';

	if ( reportType == _CRT_ASSERT )
	{
		char * p = strchr( userMessage, '(' );
		if ( p )
		{
			*p = 0;
			int Line = strtol( ++p, &p, 10 );
			if ( p && *p == ')' )
			{
				p ++;

				p += strlen( RptTypes[ reportType ] ) + sizeof ( " : " );

				*p = 0;

				p ++;

				*retVal = far_assert_message( userMessage, Line, p, false );

				return *retVal;
			}
		}
	}

	if ( Far::IsInitialized() )
	{
		FarMessage Msg( FMSG_WARNING );
		Msg.AddFmt( "Debug %s!", RptTypes[ reportType ] );
		Msg.AddLine( "" );

		char * p = strstr( userMessage, ") : " );
		if ( p && isdigit( *(p-1) ) )
		{
			p ++;

			*p = 0;

			p+= 3;

			Msg.AddLine( userMessage );
			Msg.AddLine( p );
		}
		else
		{
			Msg.AddLine( userMessage );
		}

		Msg.AddSeparator();

		Msg.AddButton( "&Abort" );
		Msg.AddButton( "&Debug" );
		Msg.AddButton( "&Ignore" );

		*retVal = Msg.Show();
	}
	else
	{
		char cap[ 256 ];
		FarSF::snprintf( cap, sizeof( cap ), "Debug %s!", RptTypes[ reportType ] );

		char buf[ 4096 ];

		char * p = strstr( userMessage, ") : " );
		if ( p && isdigit( *(p-1) ) )
		{
			p ++;

			*p = 0;

			p+= 3;

			strcpy( buf, userMessage );
			strcat( buf, "\n" );
			strcat( buf, p );
		}
		else
		{
			strcpy( buf, userMessage );
		}

		strcat( buf, "\n\nPress Retry to debug the application - JIT must be enabled" );

		*retVal = MessageBox( NULL, buf, cap,
			MB_ABORTRETRYIGNORE|MB_TASKMODAL|MB_ICONERROR|MB_SETFOREGROUND );

		switch ( *retVal )
		{
		case IDIGNORE:
			*retVal = 2;
			break;
		case IDRETRY:
			*retVal = 1;
			break;
		default:
			*retVal = 0;
			break;
		}

	}

	if ( *retVal == 0 ) // abort
	{
		ExitThread( 0 );
	}

	if ( *retVal == 2 ) // ignore
	{
		*retVal = -1;
	}

	return *retVal;
}

#ifdef _DEBUG

class CDbgReportInit
{
private:
	static int m_MemDelta;
	static _CRT_ALLOC_HOOK m_OrigAllocHook;

	_CRT_REPORT_HOOK m_OrigReportHook;
public:
	static int __cdecl AllocHook( int nAllocType, void * pvData, size_t nSize,
		int nBlockUse, long lRequest, const BYTE * szFileName, int nLine )
	{
		switch ( nAllocType )
		{
		case _HOOK_ALLOC:
			m_MemDelta += nSize;
			break;

		case _HOOK_REALLOC:
			m_MemDelta -= _msize( pvData );
			m_MemDelta += nSize;
			break;

		case _HOOK_FREE:
			nSize = _msize( pvData );
			m_MemDelta -= nSize;
			break;

		default:
			asm("int $3"); // !!! unknown memory operation !!!
			break;
		}

		if ( m_OrigAllocHook )
			return m_OrigAllocHook( nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine );

		return 1; // allow all allocs/reallocs/frees
	}

	CDbgReportInit()
	{
		// хочется считать память лично :)
		m_OrigAllocHook = _CrtSetAllocHook( CDbgReportInit::AllocHook );

		// и выводить всякую лабуду :)
		m_OrigReportHook = _CrtSetReportHook( far_dbg_report );

		// показывать все в диалоге
		_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG|_CRTDBG_MODE_WNDW );
		_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG|_CRTDBG_MODE_WNDW );
		_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG|_CRTDBG_MODE_WNDW );

		// проверка памяти
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF );
	}
	~CDbgReportInit()
	{
		// восстанавливаем стандартый обработчик сообщений
		_CrtSetReportHook( m_OrigReportHook );

		// переключаем вывод сообщений из диалога Far'а в ODS
		_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
		_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
		_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );

		if ( m_MemDelta > 0 )
		{
			// где-то забыли почиститься, скажем это.
			far_ods( "-----------------------------------\n" );
			far_ods( "Detected memory leaks: %d bytes\n", m_MemDelta );
			far_ods( "-----------------------------------\n" );
		}

		_CrtSetAllocHook( m_OrigAllocHook );
	}
};

int CDbgReportInit::m_MemDelta = 0;

_CRT_ALLOC_HOOK CDbgReportInit::m_OrigAllocHook = NULL;

CDbgReportInit g_DbgReportInit;

#endif

#include <imagehlp.h>
//#pragma comment( lib, "imagehlp.lib" )

/////////////////////////////////////////////////////////////////////////////
// Routine to produce stack dump
#define MODULE_NAME_LEN 64
#define SYMBOL_NAME_LEN 128

struct FARPLUS_SYMBOL_INFO
{
	DWORD dwAddress;
	DWORD dwOffset;
	CHAR  szModule[ MODULE_NAME_LEN ];
	CHAR  szSymbol[ SYMBOL_NAME_LEN ];
};

static DWORD __stdcall GetModuleBase( HANDLE hProcess, DWORD dwReturnAddress )
{
	IMAGEHLP_MODULE moduleInfo;

	if ( SymGetModuleInfo( hProcess, dwReturnAddress, &moduleInfo ) )
		return moduleInfo.BaseOfImage;

	MEMORY_BASIC_INFORMATION mbInfo;
	if ( VirtualQueryEx( hProcess, (LPCVOID)dwReturnAddress, &mbInfo, sizeof( mbInfo ) ) )
	{
		char szFile[ MAX_PATH ] = { 0 };
		DWORD cch = GetModuleFileName( (HINSTANCE)mbInfo.AllocationBase, szFile, MAX_PATH );

		// Ignore the return code since we can't do anything with it.
		if ( SymLoadModule( hProcess, NULL, cch ? szFile : NULL, NULL,
			(DWORD)mbInfo.AllocationBase, 0 ) )

		return (DWORD) mbInfo.AllocationBase;
	}

	return SymGetModuleBase( hProcess, dwReturnAddress );
}

// determine number of elements in an array (not bytes)
#define _countof( array ) ( sizeof( array ) / sizeof( array[ 0 ] ) )

static void ResolveSymbol( HANDLE hProcess, DWORD dwAddress, FARPLUS_SYMBOL_INFO &siSymbol )
{
	union
	{
		char rgchSymbol[ sizeof( IMAGEHLP_SYMBOL ) + 255 ];
		IMAGEHLP_SYMBOL sym;
	};

	char  szUndec     [ 256 ];
	char  szWithOffset[ 256 ];

	LPSTR pszSymbol = NULL;

	IMAGEHLP_MODULE mi;
	mi.SizeOfStruct = sizeof( IMAGEHLP_MODULE );

	ZeroMemory( &siSymbol, sizeof( FARPLUS_SYMBOL_INFO ) );

	siSymbol.dwAddress = dwAddress;

	if ( SymGetModuleInfo( hProcess, dwAddress, &mi ) )
		strncpy( siSymbol.szModule, FarSF::PointToName( mi.ImageName ), _countof( siSymbol.szModule ) );
	else
		strcpy( siSymbol.szModule, "<no module>" );

	__try
	{
		sym.SizeOfStruct  = sizeof( IMAGEHLP_SYMBOL );
		sym.Address       = dwAddress;
		sym.MaxNameLength = 255;

		if ( SymGetSymFromAddr( hProcess, dwAddress, &(siSymbol.dwOffset), &sym ) )
		{
			pszSymbol = sym.Name;

			if ( UnDecorateSymbolName( sym.Name, szUndec, _countof( szUndec ),
				UNDNAME_NO_MS_KEYWORDS|UNDNAME_NO_ACCESS_SPECIFIERS ) )
				pszSymbol = szUndec;
			else if ( SymUnDName( &sym, szUndec, _countof( szUndec ) ) )
				pszSymbol = szUndec;

			if ( siSymbol.dwOffset != 0 )
			{
				FarSF::sprintf( szWithOffset, "%s + %d bytes", pszSymbol, siSymbol.dwOffset );
				pszSymbol = szWithOffset;
			}
		}
		else
			pszSymbol = "<no symbol>";
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		pszSymbol = "<EX: no symbol>";
		siSymbol.dwOffset = dwAddress - mi.BaseOfImage;
	}

	strncpy( siSymbol.szSymbol, pszSymbol, _countof( siSymbol.szSymbol ) );
}

class CTraceClipboardData
{
private:
	FarString m_Data;
	DWORD     m_dwTarget;

	char    * m_Tmp;

public:
	CTraceClipboardData( DWORD dwTarget ) : m_dwTarget( dwTarget )
	{
		m_Tmp = new char[ 0x1000 ];
	}
	~CTraceClipboardData()
	{
		delete [] m_Tmp;

		if ( m_Data.IsEmpty() ) return;

		// far_trace можно переопределить
		if ( m_dwTarget & FAR_STACK_DUMP_TARGET_TRACE )
			far_trace( m_Data.c_str() );

		// far_ods всегда выводит в ODS
		//if ( m_dwTarget & FAR_STACK_DUMP_TARGET_ODS )
		//	far_ods( m_Data.c_str() );

		if ( m_dwTarget & FAR_STACK_DUMP_TARGET_CLIPBOARD )
			FarSF::CopyToClipboard( m_Data );

	}
	void SendOut( char const * const Fmt, ... )
	{
		va_list argPtr;
		va_start( argPtr, Fmt );
		wvsprintf( m_Tmp, Fmt, argPtr );
		m_Data += m_Tmp;
		va_end( argPtr );
	}
};

void FarDumpStack( unsigned long dwTarget)
{
	CTraceClipboardData clipboardData( dwTarget );

	clipboardData.SendOut( "===== begin FarDumpStack output =====\r\n" );

	FarIntArray adwAddress/*( 16 )*/;
	HANDLE hProcess = ::GetCurrentProcess();
	if ( SymInitialize( hProcess, NULL, FALSE ) )
	{
		// force undecorated names to get params
		DWORD dw = SymGetOptions();
		dw &= ~SYMOPT_UNDNAME;
		SymSetOptions(dw);

		HANDLE hThread = ::GetCurrentThread();
		CONTEXT threadContext;

		threadContext.ContextFlags = CONTEXT_FULL;

		if ( ::GetThreadContext(hThread, &threadContext ) )
		{
			STACKFRAME stackFrame;
			ZeroMemory( &stackFrame, sizeof( stackFrame ) );
			stackFrame.AddrPC.Mode = AddrModeFlat;

			stackFrame.AddrPC.Offset    = threadContext.Eip;
			stackFrame.AddrStack.Offset = threadContext.Esp;
			stackFrame.AddrStack.Mode   = AddrModeFlat;
			stackFrame.AddrFrame.Offset = threadContext.Ebp;
			stackFrame.AddrFrame.Mode   = AddrModeFlat;

			for ( int nFrame = 0; nFrame < 1024; nFrame++ )
			{
				if ( StackWalk( IMAGE_FILE_MACHINE_I386, hProcess, hProcess,
					&stackFrame, &threadContext, NULL,
					SymFunctionTableAccess, GetModuleBase, NULL) == 0 )
					break;
				adwAddress.Add( stackFrame.AddrPC.Offset );
			}
		}
	}
	else
	{
		clipboardData.SendOut( "FarDumpStack Error: IMAGEHLP.DLL wasn't found. "
			"GetLastError() returned 0x%8.8X\r\n", GetLastError() );
	}

	// dump it out now
	for ( int nAddress = 0; nAddress < adwAddress.Count(); nAddress++ )
	{
		FARPLUS_SYMBOL_INFO info;

		clipboardData.SendOut( "%8.8X: ", adwAddress[ nAddress ] );

		ResolveSymbol( hProcess, adwAddress[ nAddress ], info );

		clipboardData.SendOut( info.szModule );
		clipboardData.SendOut( "! " );
		clipboardData.SendOut( info.szSymbol );

		clipboardData.SendOut( "\r\n" );
	}

	clipboardData.SendOut( "====== end FarDumpStack output ======\r\n" );
}

#endif // defined(_MSC_VER)
