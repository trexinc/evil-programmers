#include "headers.hpp"

#include "py_object.hpp"
#include "python.hpp"

namespace py
{
	object::object(PyObject* Object):
		m_Object(Object)
	{
	}

	object::object(const object& Object):
		m_Object(Object.m_Object)
	{
		Py_XINCREF(m_Object);
	}

	object::object(object&& Object):
		m_Object(std::exchange(Object.m_Object, nullptr))
	{
	}

	object::object(std::nullptr_t):
		m_Object()
	{
	}

	object::~object()
	{
		Py_XDECREF(m_Object);
	}

	object& object::operator=(PyObject* Rhs)
	{
		Py_XDECREF(m_Object);
		m_Object = Rhs;
		return *this;
	}

	object& object::operator=(const object& Rhs)
	{
		Py_XINCREF(Rhs.m_Object);
		Py_XDECREF(m_Object);
		m_Object = Rhs.m_Object;
		return *this;
	}

	object& object::operator=(object&& Rhs)
	{
		m_Object = std::exchange(Rhs.m_Object, nullptr);
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

	bool object::set_attribute(const char* Name, const object& Value) const
	{
		return PyObject_SetAttrString(get(), Name, Value.get()) == 0;
	}

	bool object::set_attribute(const object& Name, const object& Value) const
	{
		return PyObject_SetAttr(get(), Name.get(), Value.get()) == 0;
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
