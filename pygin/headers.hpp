#pragma once

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX

#include <windows.h>
#include <Rpc.h>

#include "plugin.hpp"

using namespace std::string_literals;

#define NONCOPYABLE(Type) \
Type(const Type&) = delete; \
Type& operator=(const Type&) = delete;

#define COPYABLE(Type) \
Type(const Type&) = default; \
Type& operator=(const Type&) = default;

#define MOVABLE(Type) \
Type(Type&&) = default; \
Type& operator=(Type&&) = default;
