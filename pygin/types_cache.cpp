#include "headers.hpp"

#include "types_cache.hpp"

#include "py_common.hpp"
#include "py_object.hpp"
#include "py_type.hpp"

std::unordered_map<std::string, py::type> types_cache::m_TypesCache;

const py::type& types_cache::get_type(const std::function<py::object()>& Module, const std::string& TypeName)
{
	auto& Type = m_TypesCache[TypeName];
	if (!Type)
	{
		Type = py::cast<py::type>(Module()[TypeName.data()]);
	}
	return Type;
}

void types_cache::clear()
{
	m_TypesCache.clear();
}
