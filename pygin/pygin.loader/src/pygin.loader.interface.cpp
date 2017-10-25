/*
pygin.loader.interface.cpp

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

#include "pygin.loader.hpp"

static adapter Adapter;

BOOL WINAPI loader_Initialize(GlobalInfo* Info) noexcept
{
	return Adapter.ModuleInit() && Adapter.Initialize(Info);
}

BOOL WINAPI loader_IsPlugin(const wchar_t* FileName) noexcept
{
	return Adapter.IsPlugin(FileName);
}

HANDLE WINAPI loader_CreateInstance(const wchar_t* FileName) noexcept
{
	return Adapter.CreateInstance(FileName);
}

FARPROC WINAPI loader_GetFunctionAddress(HANDLE Instance, const wchar_t* FunctionName) noexcept
{
	return Adapter.GetFunctionAddress(Instance, FunctionName);
}

BOOL WINAPI loader_GetError(ErrorInfo* Info) noexcept
{
	return Adapter.GetError(Info);
}

BOOL WINAPI loader_DestroyInstance(HANDLE Instance) noexcept
{
	return Adapter.DestroyInstance(Instance);
}

void WINAPI loader_Free(const ExitInfo* Info) noexcept
{
	Adapter.Free(Info);
	Adapter.ModuleFree();
}