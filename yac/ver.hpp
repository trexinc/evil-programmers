#include "build.hpp"

#define MAJOR_VER 0
#define MINOR_VER 0

#define _W(arg) L##arg
#define _STR(arg) _W(#arg)
#define STR(arg) _STR(arg)

#define ALLSTR STR(MAJOR_VER) L"." STR(MINOR_VER) L" build " STR(BUILD)

#define PRODUCTNAME L"Yet Another Completion"

#define MIN_FAR_MAJOR_VER 2
#define MIN_FAR_MINOR_VER 0
#define MIN_FAR_BUILD 1573
