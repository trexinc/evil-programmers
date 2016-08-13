// headers.hpp : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.hpp"

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
// Windows Header Files:
#include <windows.h>
#include <Rpc.h>

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <string>

#include "plugin.hpp"
#include "python_cpp.hpp"

using namespace std::string_literals;

// TODO: reference additional headers your program requires here
