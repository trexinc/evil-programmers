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