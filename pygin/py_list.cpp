#include "headers.hpp"

#include "py_list.hpp"

#include "py_common.hpp"

#include "python.hpp"

namespace py
{
	list::list(cast_guard, const object& Object):
		object(Object)
	{
	}

	void list::set_at(size_t Index, const object& Value)
	{
		DONT_STEAL_REFERENCE(Value.get());
		invoke(PyList_SetItem, get(), Index, Value.get());
	}

	object list::get_at(size_t Index) const
	{
		return from_borrowed(invoke(PyList_GetItem, get(), Index));
	}

	size_t list::size() const
	{
		return invoke(PyList_Size, get());
	}

	void list::push_back(const object& Object)
	{
		invoke(PyList_Append, get(), Object.get());
	}

	void list::insert(const object& Object, size_t index)
	{
		invoke(PyList_Insert, get(), index, Object.get());
	}
}
