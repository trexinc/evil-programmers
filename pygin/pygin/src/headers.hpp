#pragma once

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX

#include <windows.h>
#include <Rpc.h>

#include "../../Include/plugin.hpp"

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
