#include "headers.hpp"

#include "types_cache.hpp"

#include "py_object.hpp"
#include "py_type.hpp"


std::unordered_map<types, py::type> types_cache::m_TypesCache;
critical_section types_cache::m_Cs;

const py::type& types_cache::get_type(types TypeId, const std::function<py::type()>& Getter)
{
	std::lock_guard<critical_section> Lock(m_Cs);

	auto& Type = m_TypesCache[TypeId];
	if (!Type)
		Type = Getter();

	return Type;
}

void types_cache::clear()
{
	std::lock_guard<critical_section> Lock(m_Cs);

	m_TypesCache.clear();
}
