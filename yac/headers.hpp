#pragma once
#define _WIN32_WINNT 0x0600

#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
//#define NOVIRTUALKEYCODES // VK_*
#define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS      // SM_*
#define NOMENUS           // MF_*
#define NOICONS           // IDI_*
#define NOKEYSTATES       // MK_*
#define NOSYSCOMMANDS     // SC_*
#define NORASTEROPS       // Binary and Tertiary raster ops
#define NOSHOWWINDOW      // SW_*
#define OEMRESOURCE       // OEM Resource values
#define NOATOM            // Atom Manager routines
#define NOCLIPBOARD       // Clipboard routines
#define NOCOLOR           // Screen colors
#define NOCTLMGR          // Control and Dialog routines
#define NODRAWTEXT        // DrawText() and DT_*
#define NOGDI             // All GDI defines and routines
//#define NOKERNEL          // All KERNEL defines and routines
//#define NOUSER            // All USER defines and routines
//#define NONLS             // All NLS defines and routines
//#define NOMB              // MB_* and MessageBox()
#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE        // typedef METAFILEPICT
//#define NOMINMAX          // Macros min(a,b) and max(a,b)
#define NOMSG             // typedef MSG and associated routines
#define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL          // SB_* and scrolling routines
#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND           // Sound driver routines
#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
#define NOWH              // SetWindowsHook and WH_*
#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
#define NOCOMM            // COMM driver routines
#define NOKANJI           // Kanji support stuff.
#define NOHELP            // Help engine interface.
#define NOPROFILER        // Profiler interface.
#define NODEFERWINDOWPOS  // DeferWindowPos routines
#define NOMCX             // Modem Configuration Extensions

#define _INC_STDLIB
//#define _INC_EXCPT
#define _INC_CTYPE

#pragma pack(push)
#pragma pack(8)

#include <new>
#include <stdio.h>
#include <windows.h>
#include <WinCred.h>
#include <Lm.h>
#include <Psapi.h>
#include <winternl.h>
#include <winscard.h>

#pragma pack (pop)

#include <plugin.hpp>
#include <farcolor.hpp>

extern PluginStartupInfo Info;
extern FarStandardFunctions FSF;

template <class T>
inline const T& Min(const T& a,const T& b){return a<b?a:b;}

template <class T>
inline const T& Max(const T& a,const T& b){return a<b?b:a;}

// {ADE3CE69-2D5D-48D4-A964-255A5A2BBE1D}
DEFINE_GUID(_YacGuid, 0xade3ce69, 0x2d5d, 0x48d4, 0xa9, 0x64, 0x25, 0x5a, 0x5a, 0x2b, 0xbe, 0x1d);
const GUID* YacGuid = &_YacGuid;
#undef min
#undef max
