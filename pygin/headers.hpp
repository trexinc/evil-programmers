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

using namespace std::string_literals;
