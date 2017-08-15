#include "headers.hpp"

#include "py.common.hpp"

#include "py.import.hpp"
#include "py.list.hpp"
#include "py.object.hpp"
#include "py.string.hpp"

#include "python.hpp"

namespace py
{
	void initialize()
	{
		Py_Initialize();
	}

	void finalize()
	{
		Py_Finalize();
	}

	bool callable_check(const object& Object)
	{
		return invoke(PyCallable_Check, Object.get()) != 0;
	}

	void raise_exception_if_needed()
	{
		if (!PyErr_Occurred())
			return;

		PyObject* TypePtr, * ValuePtr, * TracebackPtr;
		PyErr_Fetch(&TypePtr, &ValuePtr, &TracebackPtr);
		PyErr_NormalizeException(&TypePtr, &ValuePtr, &TracebackPtr);

		object Type(TypePtr), Value(ValuePtr), Traceback(TracebackPtr);

		const auto TracebackModule = import::import("traceback"_py);

		std::string Message;

		{
			const auto List = cast<list>(TracebackModule["format_exception_only"](Type, Value));
			for (size_t i = 0, size = List.size(); i != size; ++i)
			{
				Message += cast<string>(List[i]);
			}
		}

		if (Traceback)
		{
			Message += "\1\nTraceback (most recent call last):\n"s;

			const auto List = cast<list>(TracebackModule["format_tb"](Traceback));
			for (size_t i = 0, size = List.size(); i != size; ++i)
			{
				Message += cast<string>(List[i]);
			}
		}

		throw exception(Message);
	}
}

