#pragma once

typedef struct _object PyObject;

namespace py
{
	class object
	{
	public:
		explicit object(PyObject* Object);
		object(const object& rhs);
		object(object&& Object);
		object(std::nullptr_t);

		~object();

		object& operator=(PyObject* rhs);
		object& operator=(const object& rhs);
		object& operator=(object&& Object);

		explicit operator bool() const;
		bool operator!() const;

		PyObject* get() const;

		object get_attribute(const char* Name) const;
		object get_attribute(const object& Name) const;

		object call(const object& Args) const;

		static object from_borrowed(PyObject* Object);

	private:
		PyObject* m_Object;
	};
}
