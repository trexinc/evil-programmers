#include "headers.hpp"

#include "py_common.hpp"

#include "py_object.hpp"
#include "py_import.hpp"
#include "py_list.hpp"
#include "py_string.hpp"

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

		PyObject* TypePtr, *ValuePtr, *TracebackPtr;
		PyErr_Fetch(&TypePtr, &ValuePtr, &TracebackPtr);
		object Type(TypePtr), Value(ValuePtr), Traceback(TracebackPtr);

		// Seems to be a Python core bug :(
		if (!strcmp(Value.type_name(), "str"))
		{
			Value = object(PyExc_Exception)(Value);
		}

		const auto TracebackModule = import::import("traceback"_py);
		const auto Formatter = TracebackModule[Traceback? "format_exception" : "format_exception_only"];
		const auto List = cast<list>(Traceback? Formatter(Type, Value, Traceback) : Formatter(Type, Value));

		std::string Message;
		for (size_t i = 0, size = List.size(); i != size; ++i)
		{
			Message += cast<string>(List[i]).to_string();
		}

		throw exception(Message);
	}
}

