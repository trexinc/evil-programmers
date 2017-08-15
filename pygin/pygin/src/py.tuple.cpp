#include "headers.hpp"

#include "py.tuple.hpp"

#include "py.common.hpp"

#include "types_cache.hpp"

#include "python.hpp"

namespace py
{
	const type& tuple::get_type()
	{
		return types_cache::get_type(types::tuple, []
		{
			return type(tuple(0u));
		});
	}

	tuple::tuple(size_t Size):
		object(invoke(PyTuple_New, Size))
	{
	}

	tuple::tuple(const std::initializer_list<object>& Args):
		tuple(Args.size())
	{
		for (size_t i = 0; i != Args.size(); ++i)
		{
			set_at(i, Args.begin()[i]);
		}
	}

	tuple::tuple(cast_guard, const object& Object):
		object(Object)
	{
	}

	void tuple::set_at(size_t Position, const object& Object)
	{
		DONT_STEAL_REFERENCE(Object.get());
		invoke(PyTuple_SetItem, get(), Position, Object.get());
	}

	object tuple::get_at(size_t Position) const
	{
		return from_borrowed(invoke(PyTuple_GetItem, get(), Position));
	}

	size_t tuple::size() const
	{
		return invoke(PyTuple_Size, get());
	}
}
