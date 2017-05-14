#pragma once
#include "py_object.hpp"

namespace py
{
	namespace err
	{
		object occurred();
		void clear();
		void print();

		class exception: public std::runtime_error
		{
			using runtime_error::runtime_error;
		};

		void raise_exception_if_needed();
	}
}
