#pragma once

#define MAJOR_VER 0
#define MINOR_VER 0
#define BUILD 1

#define _W(arg) L##arg
#define _STR(arg) _W(#arg)
#define STR(arg) _STR(arg)

#ifdef _WIN64
#define PLATFORM L"x64"
#else
#define PLATFORM L"x86"
#endif

#define ALLSTR STR(MAJOR_VER) L"." STR(MINOR_VER) L" build " STR(BUILD) L" " PLATFORM

#define PRODUCTNAME L"XKeys"
