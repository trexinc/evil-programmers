#include "headers.hpp"

#include "py_string.hpp"

#include "python.hpp"

namespace py
{
	string::string(const char* Str):
		object(PyUnicode_FromString(Str))
	{
	}

	string::string(const std::string& Str):
		object(PyUnicode_FromString(Str.data()))
	{
	}

	string::string(const wchar_t* Str):
		object(PyUnicode_FromUnicode(Str, wcslen(Str)))
	{
	}

	string::string(const std::wstring& Str):
		object(PyUnicode_FromUnicode(Str.data(), Str.size()))
	{
	}

	int string::compare(const object& Other) const
	{
		return PyUnicode_Compare(get(), Other.get());
	}

	std::string string::to_string(const object& Object)
	{
		const auto Utf8Str = object(PyUnicode_AsUTF8String(Object.get()));
		if (!Utf8Str)
			throw std::runtime_error("PyUnicode_AsUTF8String");

		const auto Data = PyBytes_AsString(Utf8Str.get());
		if (!Data)
			throw std::runtime_error("PyBytes_AsString");

		const auto Size = static_cast<size_t>(PyBytes_Size(Utf8Str.get()));

		return{ Data, Size };
	}

	std::wstring string::to_wstring(const object& Object)
	{
		Py_ssize_t Size;
		const auto Data = PyUnicode_AsWideCharString(Object.get(), &Size);
		if (!Data)
			throw std::runtime_error("PyUnicode_AsWideCharString");

		const std::wstring Result(Data, static_cast<size_t>(Size));

		PyMem_Free(Data);

		return Result;
	}

}
