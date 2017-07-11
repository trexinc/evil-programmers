#include "headers.hpp"

#include "py_dictionary.hpp"

#include "py_common.hpp"
#include "py_integer.hpp"

#include "types_cache.hpp"

#include "python.hpp"

namespace py
{
	const type& dictionary::get_type()
	{
		return types_cache::get_type(types::dictionary, []()
		{
			return type(cast_guard{}, dictionary());
		});
	}

	dictionary::dictionary():
		object(invoke(PyDict_New))
	{
	}

	dictionary::dictionary(cast_guard, const object& Object):
		object(Object)
	{
	}

	void dictionary::set_at(const object& Key, const object& Value)
	{
		invoke(PyDict_SetItem, get(), Key.get(), Value.get());
	}

	object dictionary::get_at(const object& Key) const
	{
		return from_borrowed(invoke(PyDict_GetItem, get(), Key.get()));
	}
}
