#pragma once

#include "py_function.hpp"
#include "py_module.hpp"

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
	std::unique_ptr<module> create_module(const wchar_t* FileName) const;
	FARPROC WINAPI get_function(HANDLE Instance, const wchar_t* FunctionName) const;
	bool get_error(ErrorInfo* Info) const;

private:
	class python
	{
	public:
		NONCOPYABLE(python);
		python();
		~python();
	};

	python m_Python;
	py::module m_PyginModule;
	py::function m_PyginLoadPlugin;
};
