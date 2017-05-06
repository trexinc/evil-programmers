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

template<typename callable, typename fallback>
auto try_call(const callable& Callable, const fallback& Fallback) noexcept
{
	try
	{
		return Callable();
	}
	catch (...)
	{
		return Fallback();
	}
}

#define TRIVIALLY_MOVABLE(Type) \
Type(Type&&) = default; \
Type& operator=(Type&&) = default;
