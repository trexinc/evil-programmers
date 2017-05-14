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
	// This one is unprotected, as otherwise it would reset error state
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

void WINAPI adapter_Free(const ExitInfo* info) noexcept
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