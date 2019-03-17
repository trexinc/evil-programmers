/*
pygin.loader.cpp

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

extern "C" IMAGE_DOS_HEADER __ImageBase;

bool adapter::ModuleInit()
{
	m_Summary = L"Pygin Adapter Loader"s;
	m_Description = L"Unknown error"s;

	m_Activated = true;

	wchar_t AdaptherPath[MAX_PATH];
	if (!GetModuleFileName(reinterpret_cast<HINSTANCE>(&__ImageBase), AdaptherPath, static_cast<DWORD>(std::size(AdaptherPath))))
	{
		m_Description = GetLastErrorMessage(GetLastError());
		return false;
	}

	std::wcscpy(wcsrchr(AdaptherPath, L'\\') + 1, L"pygin.dll");

	// TODO: customisation point here

	m_Adapter.reset(LoadLibrary(AdaptherPath));
	if (!m_Adapter)
	{
		m_Description = GetLastErrorMessage(GetLastError()) + L"\n\1\nHint: Make sure you have Python directory (e.g. C:\\Program Files\\Python37) in PATH"s;
		return false;
	}

#define INIT_IMPORT(name) \
	m_##name = reinterpret_cast<decltype(m_##name)>(GetProcAddress(m_Adapter.get(), "adapter_" #name)); \
	if (!m_##name) \
	{ \
		Cleanup(); \
		m_Description = GetLastErrorMessage(GetLastError()); \
		return false; \
	}

	INIT_IMPORT(Initialize)
	INIT_IMPORT(IsPlugin)
	INIT_IMPORT(CreateInstance)
	INIT_IMPORT(GetFunctionAddress)
	INIT_IMPORT(GetError)
	INIT_IMPORT(DestroyInstance)
	INIT_IMPORT(Free)

#undef INIT_IMPORT

	return true;
}

void adapter::ModuleFree()
{
	Cleanup();
	m_Activated = false;
}

BOOL adapter::Initialize(GlobalInfo* Info) const noexcept
{
	return m_Initialize(Info);
}

BOOL adapter::IsPlugin(const wchar_t* FileName) const noexcept
{
	return m_IsPlugin(FileName);
}

HANDLE adapter::CreateInstance(const wchar_t* FileName) const noexcept
{
	return m_CreateInstance(FileName);
}

FARPROC adapter::GetFunctionAddress(HANDLE Instance, const wchar_t* FunctionName) const noexcept
{
	return m_GetFunctionAddress(Instance, FunctionName);
}

BOOL adapter::GetError(ErrorInfo* Info) const noexcept
{
	if (!m_Activated)
		return false;

	if (m_GetError)
		return m_GetError(Info);

	Info->StructSize = sizeof(*Info);
	Info->Summary = m_Summary.c_str();
	Info->Description = m_Description.c_str();
	return true;
}

BOOL adapter::DestroyInstance(HANDLE Instance) const noexcept
{
	return m_DestroyInstance(Instance);
}

void adapter::Free(const ExitInfo* Info) const noexcept
{
	return m_Free(Info);
}

void adapter::Cleanup()
{
	m_Initialize = nullptr;
	m_IsPlugin = nullptr;
	m_CreateInstance = nullptr;
	m_GetFunctionAddress = nullptr;
	m_GetError = nullptr;
	m_DestroyInstance = nullptr;
	m_Free = nullptr;
}
