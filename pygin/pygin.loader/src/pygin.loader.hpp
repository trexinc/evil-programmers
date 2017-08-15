#include "headers.hpp"

class adapter
{
public:
	bool ModuleInit();
	void ModuleFree();

	BOOL Initialize(GlobalInfo* Info) const noexcept;
	BOOL IsPlugin(const wchar_t* FileName) const noexcept;
	HANDLE CreateInstance(const wchar_t* FileName) const noexcept;
	FARPROC GetFunctionAddress(HANDLE Instance, const wchar_t* FunctionName) const noexcept;
	BOOL GetError(ErrorInfo* Info) const noexcept;
	BOOL DestroyInstance(HANDLE Instance) const noexcept;
	void Free(const ExitInfo* Info) const noexcept;

private:
	void Cleanup();

	HMODULE m_Adapter{};

	BOOL (WINAPI *m_Initialize)(GlobalInfo*){};
	BOOL (WINAPI *m_IsPlugin)(const wchar_t*){};
	HANDLE (WINAPI *m_CreateInstance)(const wchar_t*){};
	FARPROC (WINAPI *m_GetFunctionAddress)(HANDLE, const wchar_t*){};
	BOOL (WINAPI *m_GetError)(ErrorInfo*){};
	BOOL (WINAPI *m_DestroyInstance)(HANDLE){};
	void (WINAPI *m_Free)(const ExitInfo*){};

	bool m_Activated{};
};
