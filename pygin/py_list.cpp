#include "headers.hpp"

#include "py_list.hpp"
#include "python.hpp"

namespace py
{
	list::list(const object& Object):
		object(Object)
	{
	}

	size_t list::size() const
	{
		return PyList_Size(get());
	}

	void list::push_back(const object& Object)
	{
		PyList_Append(get(), Object.get());
	}

	void list::insert(const object& Object, size_t index)
	{
		PyList_Insert(get(), index, Object.get());
	}

	object list::operator[](size_t index)
	{
		return from_borrowed(PyList_GetItem(get(), index));
	}
}
