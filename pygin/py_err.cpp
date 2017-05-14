#include "headers.hpp"

#include "py_err.hpp"

#include "py_import.hpp"
#include "py_list.hpp"
#include "py_string.hpp"
#include "py_tuple.hpp"

#include "python.hpp"

namespace py
{
	namespace err
	{
		object occurred()
		{
			return object::from_borrowed(PyErr_Occurred());
		}

		void clear()
		{
			PyErr_Clear();
		}

		void print()
		{
			if (PyErr_Occurred())
				PyErr_Print();
		}

		void raise_exception_if_needed()
		{
			if (!PyErr_Occurred())
				return;

			PyObject* TypePtr, * ValuePtr, * TracebackPtr;
			PyErr_Fetch(&TypePtr, &ValuePtr, &TracebackPtr);
			object Type(TypePtr), Value(ValuePtr), Traceback(TracebackPtr);

			// Seems to be a Python core bug :(
			if (!strcmp(Value.type_name(), "str"))
			{
				Value = object(PyExc_Exception).call(Value);
			}

			const auto TracebackModule = import::import("traceback"_py);

			const char* FormatterName;
			tuple Args(0);

			if (Traceback)
			{
				FormatterName = "format_exception";
				Args = tuple::make(Type, Value, Traceback);
			}
			else
			{
				FormatterName = "format_exception_only";
				Args = tuple::make(Type, Value);
			}

			const auto Formatter = TracebackModule.get_attribute(FormatterName);
			const auto List = cast<list>(Formatter.call(Args));

			std::string Message;
			for (size_t i = 0, size = List.size(); i != size; ++i)
			{
				Message += cast<string>(List[i]).to_string();
			}

			throw exception(Message);
		}
	}
}
