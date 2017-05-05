#pragma once

#include "py_object.hpp"

namespace py
{
	class string: public object
	{
	public:
		explicit string(const char* Str);
		explicit string(const wchar_t* Str);
		explicit string(const std::string& Str);
		explicit string(const std::wstring& Str);

		int compare(const object& Other) const;

		static std::string to_string(const object& Object);
		static std::wstring to_wstring(const object& Object);
	};
}
