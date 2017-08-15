#include "headers.hpp"

#include "pygin.loader.hpp"

extern "C" IMAGE_DOS_HEADER __ImageBase;

bool adapter::ModuleInit()
{
	m_Activated = true;

	wchar_t AdaptherPath[MAX_PATH];
	if (!GetModuleFileName(reinterpret_cast<HINSTANCE>(&__ImageBase), AdaptherPath, static_cast<DWORD>(std::size(AdaptherPath))))
		return false;

	std::wcscpy(wcsrchr(AdaptherPath, L'\\') + 1, L"pygin.dll");

	// TODO: customisation point here

	m_Adapter = LoadLibrary(AdaptherPath);
	if (!m_Adapter)
		return false;

#define INIT_IMPORT(name) \
	m_##name = reinterpret_cast<decltype(m_##name)>(GetProcAddress(m_Adapter, "adapter_" #name)); \
	if (!m_##name) \
	{ \
		Cleanup(); \
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

	Info->StructSize = sizeof(Info);
	Info->Summary = L"Error loading pygin.dll";
	Info->Description = L"Unknown error";
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

	FreeLibrary(m_Adapter);
	m_Adapter = nullptr;
}
