#pragma once

class module;
struct GlobalInfo;
class far_api;

class pygin
{
public:
	NONCOPYABLE(pygin);

	pygin(GlobalInfo* Info);
	~pygin();
	bool is_module(const wchar_t* FileName) const;
	std::unique_ptr<module> create_module(const wchar_t* FileName);
	FARPROC WINAPI get_function(HANDLE Instance, const wchar_t* FunctionName);

private:
	std::unique_ptr<module> m_PyginModule;
};
