#pragma once

#include "critical_section.hpp"

namespace py
{
	class object;
	class type;
}

enum class types
{
	boolean,
	dictionary,
	function,
	integer,
	list,
	module,
	string,
	tuple,
	type,
	uuid,
};

class types_cache
{
public:
	static const py::type& get_type(types TypeId, const std::function<py::type()>& Getter);
	static void clear();

private:
	static std::unordered_map<types, py::type> m_TypesCache;
	static critical_section m_Cs;
};

