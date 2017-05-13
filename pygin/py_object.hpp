#pragma once

#include "py_common.hpp"

namespace py
{
	class tuple;

	class cast_guard {};

	class object
	{
	public:
		object();
		explicit object(PyObject* Object);
		object(const object& rhs);
		object(object&& Object) noexcept;
		object(std::nullptr_t);

		~object();

		object& operator=(PyObject* rhs);
		object& operator=(const object& rhs);
		object& operator=(object&& Object) noexcept;

		explicit operator bool() const;
		bool operator!() const;

		PyObject* get() const;

		bool has_attribute(const char* Name) const;
		bool has_attribute(const object& Name) const;

		object get_attribute(const char* Name) const;
		object get_attribute(const object& Name) const;

		bool set_attribute(const char* Name, const object& Value) const;
		bool set_attribute(const object& Name, const object& Value) const;

		template<typename... args>
		object call(const args&... Args) const
		{
			return call(tuple::make(Args...));
		}

		object call(const tuple& Args) const;

		static object from_borrowed(PyObject* Object);
		const char* type_name() const;
		void validate_type_name(const char* TypeName) const;

	private:
		PyObject* m_Object;
	};
}
