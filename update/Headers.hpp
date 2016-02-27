#pragma once

#undef WINVER
#define WINVER 0x0600

#define WIN32_LEAN_AND_MEAN

#include <plugin.hpp>
#include <psapi.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <wininet.h>
#include "memory.hpp"

extern PluginStartupInfo Info;

