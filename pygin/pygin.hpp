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

	const py::type& api_type(const std::string& TypeName) const;
	using type_factory = std::function<const py::type&(const std::string&)>;
	type_factory api_type_factory() const;

private:
	py::object m_PyginModule;
	mutable std::unordered_map<std::string, py::type> m_ApiTypes;
};
