/*
pygin.interface.cpp

*/
/*
Copyright 2017 Alex Alabuzhev
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"

#include "pygin.hpp"
#include "module.hpp"

#include "error_handling.hpp"

static std::unique_ptr<pygin> PyginInstance;

BOOL WINAPI adapter_Initialize(GlobalInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		PyginInstance = std::make_unique<pygin>(Info);
		return TRUE;
	},
	[]
	{
		return FALSE;
	});
}

BOOL WINAPI adapter_IsPlugin(const wchar_t* FileName) noexcept
{
	return try_call(
	[&]
	{
		return PyginInstance->is_module(FileName);
	},
	[]
	{
		return false;
	});
}

HANDLE WINAPI adapter_CreateInstance(const wchar_t* FileName) noexcept
{
	return try_call(
	[&]
	{
		return PyginInstance->create_module(FileName).release();
	},
	[]
	{
		return static_cast<module*>(nullptr);
	});
}

FARPROC WINAPI adapter_GetFunctionAddress(HANDLE Instance, const wchar_t* FunctionName) noexcept
{
	return try_call(
	[&]
	{
		return PyginInstance->get_function(Instance, FunctionName);
	},
	[]
	{
		return static_cast<FARPROC>(nullptr);
	});
}

BOOL WINAPI adapter_GetError(ErrorInfo* Info) noexcept
{
	return PyginInstance->get_error(Info);
}

BOOL WINAPI adapter_DestroyInstance(HANDLE Instance) noexcept
{
	return try_call(
	[&]
	{
		std::unique_ptr<module>(static_cast<module*>(Instance));
		return TRUE;
	},
	[]
	{
		return FALSE;
	});
}

void WINAPI adapter_Free(const ExitInfo* Info) noexcept
{
	return try_call(
	[&]
	{
		PyginInstance.reset();
	},
	[]
	{
	});
}
