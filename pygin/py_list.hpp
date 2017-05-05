#pragma once

#include "py_object.hpp"

namespace py
{
	class list: public object
	{
	public:
		explicit list(const object& Object);

		size_t size() const;
		object operator[](size_t index);
		void push_back(const object& Object);
		void insert(const object& Object, size_t index);
	};
}