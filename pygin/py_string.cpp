#include "headers.hpp"

#include "py_string.hpp"

#include "py_common.hpp"

#include "types_cache.hpp"

#include "python.hpp"

namespace py
{
	const type& string::get_type()
	{
		return types_cache::get_type(types::string, []()
		{
			return type(cast_guard{}, string(static_cast<const char*>(nullptr), 0));
		});
	}

	string::string(const char* Str, size_t Size):
		object(invoke(PyUnicode_FromStringAndSize, Str, Size))
	{
	}

	string::string(const wchar_t* Str, size_t Size):
		object(invoke(PyUnicode_FromWideChar, Str, Size))
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

	string::string(cast_guard, const object& Object):
		object(Object)
	{
	}

	bool string::operator==(const string& Other) const
	{
		return invoke(PyUnicode_Compare, get(), Other.get()) == 0;
	}

	string::operator std::string() const
	{
		const auto Utf8Str = object(invoke(PyUnicode_AsUTF8String, get()));
		const auto Data = invoke(PyBytes_AsString, Utf8Str.get());

		const auto Size = static_cast<size_t>(invoke(PyBytes_Size, Utf8Str.get()));

		return { Data, Size };
	}

	string::operator std::wstring() const
	{
		Py_ssize_t Size;
		const auto Data = invoke(PyUnicode_AsWideCharString, get(), &Size);

		const std::wstring Result(Data, static_cast<size_t>(Size));

		invoke(PyMem_Free, Data);

		return Result;
	}
}
