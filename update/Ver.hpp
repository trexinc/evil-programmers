#pragma once

#define MAJOR_VER 3
#define MINOR_VER 0
#define BUILD 20

#define _W(arg) L##arg
#define _STR(arg) _W(#arg)
#define STR(arg) _STR(arg)

#define FARVER L"3"

#ifdef _WIN64
#define PLATFORM L"x64"
#else
#define PLATFORM L"x86"
#endif

#define ALLSTR STR(MAJOR_VER) L"." STR(MINOR_VER) L" build " STR(BUILD) " " PLATFORM

#define PRODUCTNAME L"Update"
