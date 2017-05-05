#include "headers.hpp"

#include "py_object.hpp"
#include "python.hpp"

namespace py
{
	object::object(PyObject* Object):
		m_Object(Object)
	{
	}

	object::object(const object& rhs):
		m_Object(rhs.m_Object)
	{
		Py_XINCREF(m_Object);
	}

	object::object(std::nullptr_t):
		m_Object()
	{
	}

	object::~object()
	{
		Py_XDECREF(m_Object);
	}

	object& object::operator=(PyObject* rhs)
	{
		Py_XDECREF(m_Object);
		m_Object = rhs;
		return *this;
	}

	object& object::operator=(const object& rhs)
	{
		Py_XINCREF(rhs.m_Object);
		Py_XDECREF(m_Object);
		m_Object = rhs.m_Object;
		return *this;
	}

	object::operator bool() const
	{
		return m_Object != nullptr;
	}

	bool object::operator!() const
	{
		return !m_Object;
	}

	PyObject* object::get() const
	{
		return m_Object;
	}

	object object::get_attribute(const char* Name) const
	{
		return object(PyObject_GetAttrString(get(), Name));
	}

	object object::get_attribute(const object& Name) const
	{
		return object(PyObject_GetAttr(get(), Name.get()));
	}

	object object::call(const object& Args) const
	{
		return object(PyObject_CallObject(get(), Args.get()));
	}

	object object::from_borrowed(PyObject* Object)
	{
		Py_XINCREF(Object);
		return object(Object);
	}
}
