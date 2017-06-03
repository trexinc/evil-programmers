#pragma once

namespace py
{
	class object;
	class type;
}

class types_cache
{
public:
	static const py::type& get_type(const std::function<py::object()>& Module, const std::string& TypeName);
	static void clear();

private:
	static std::unordered_map<std::string, py::type> m_TypesCache;
};

