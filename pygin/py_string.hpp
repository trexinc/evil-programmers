#pragma once

#include "py_object.hpp"

namespace py
{
	class string: public object
	{
	public:
		static auto type_name() { return "str"; }

		string(const char* Str, size_t Size);
		string(const wchar_t* Str, size_t Size);
		explicit string(const char* Str);
		explicit string(const wchar_t* Str);
		explicit string(const std::string& Str);
		explicit string(const std::wstring& Str);
		string(cast_guard, const object& Object);

		bool operator==(const string& Other) const;

		std::string to_string() const;
		std::wstring to_wstring() const;
	};

	inline namespace literals
	{
		inline auto operator ""_py(const char* Data, size_t Size) noexcept
		{
			return string(Data, Size);
		}

		inline auto operator ""_py(const wchar_t* Data, size_t Size) noexcept
		{
			return string(Data, Size);
		}
	}
}
