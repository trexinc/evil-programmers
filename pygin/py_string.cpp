#include "headers.hpp"

#include "py_string.hpp"

#include "python.hpp"
#include "py_err.hpp"

namespace py
{
	string::string(const char* Str, size_t Size):
		object(PyUnicode_FromStringAndSize(Str, Size))
	{
	}

	string::string(const wchar_t* Str, size_t Size):
		object(PyUnicode_FromWideChar(Str, Size))
	{
	}

	string::string(const char* Str):
		string(Str, strlen(Str))
	{
	}

	string::string(const wchar_t* Str):
		string(Str, wcslen(Str))
	{
	}

	string::string(const std::string& Str):
		string(Str.data(), Str.size())
	{
	}

	string::string(const std::wstring& Str):
		string(Str.data(), Str.size())
	{
	}

	string::string(const object& Object):
		object(Object)
	{
	}

	int string::compare(const object& Other) const
	{
		return PyUnicode_Compare(get(), Other.get());
	}

	string as_string(PyObject* Object)
	{
		return string(object::from_borrowed(Object));
	}

	string as_string(const object& Object)
	{
		return string(Object);
	}

	std::string string::to_string() const
	{
		const auto Utf8Str = object(PyUnicode_AsUTF8String(get()));
		if (!Utf8Str)
		{
			err::print_if_any();
			throw std::runtime_error("PyUnicode_AsUTF8String");
		}

		const auto Data = PyBytes_AsString(Utf8Str.get());
		if (!Data)
		{
			err::print_if_any();
			throw std::runtime_error("PyBytes_AsString");
		}

		const auto Size = static_cast<size_t>(PyBytes_Size(Utf8Str.get()));

		return { Data, Size };
	}

	std::wstring string::to_wstring() const
	{
		Py_ssize_t Size;
		const auto Data = PyUnicode_AsWideCharString(get(), &Size);
		if (!Data)
		{
			err::print_if_any();
			throw std::runtime_error("PyUnicode_AsWideCharString");
		}

		const std::wstring Result(Data, static_cast<size_t>(Size));

		PyMem_Free(Data);

		return Result;
	}
}
