#pragma once
#include "py_object.hpp"

class module;
struct GlobalInfo;
class far_api;

namespace py
{
	class type;
}

class pygin
{
public:
	NONCOPYABLE(pygin);

	pygin(GlobalInfo* Info);
	~pygin();
	bool is_module(const wchar_t* FileName) const;
	std::unique_ptr<module> create_module(const wchar_t* FileName);
	FARPROC WINAPI get_function(HANDLE Instance, const wchar_t* FunctionName);
	bool get_error(ErrorInfo* Info) const;

private:
	py::object m_PyginModule;
};
