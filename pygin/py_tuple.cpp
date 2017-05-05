#include "headers.hpp"

#include "py_tuple.hpp"

#include "python.hpp"

namespace py
{
	tuple::tuple(size_t Size):
		object(PyTuple_New(Size))
	{
	}

	void tuple::set_at(size_t Position, const object& Object)
	{
		// PyTuple_SetItem steals reference. Awesome design, guys, just awesome.
		Py_XINCREF(Object.get());
		PyTuple_SetItem(get(), Position, Object.get());
	}
}
