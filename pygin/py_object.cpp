#include "headers.hpp"

#include "py_object.hpp"

#include "py_tuple.hpp"

#include "python.hpp"
#include "error_handling.hpp"

namespace py
{
	object::object():
		m_Object()
	{
	}

	object::object(PyObject* Object):
		m_Object(Object)
	{
	}

	object::object(const object& Object):
		m_Object(Object.m_Object)
	{
		Py_XINCREF(m_Object);
	}

	object::object(object&& Object) noexcept:
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

	object& object::operator=(object&& Rhs) noexcept
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

	bool object::has_attribute(const char* Name) const
	{
		return invoke(PyObject_HasAttrString, get(), Name) != 0;
	}

	bool object::has_attribute(const object& Name) const
	{
		return invoke(PyObject_HasAttr, get(), Name.get()) != 0;
	}

	object object::get_attribute(const char* Name) const
	{
		return object(invoke(PyObject_GetAttrString, get(), Name));
	}

	object object::get_attribute(const object& Name) const
	{
		return object(invoke(PyObject_GetAttr, get(), Name.get()));
	}

	bool object::set_attribute(const char* Name, const object& Value) const
	{
		return invoke(PyObject_SetAttrString, get(), Name, Value.get()) == 0;
	}

	bool object::set_attribute(const object& Name, const object& Value) const
	{
		return invoke(PyObject_SetAttr, get(), Name.get(), Value.get()) == 0;
	}

	object object::operator()(const tuple& Args) const
	{
		return object(invoke(PyObject_CallObject, get(), Args.get()));
	}

	object object::from_borrowed(PyObject* Object)
	{
		Py_XINCREF(Object);
		return object(Object);
	}

	const char* object::type_name() const
	{
		return get()->ob_type->tp_name;
	}

	void object::validate_type_name(const char* TypeName) const
	{
		if (strcmp(TypeName, type_name()))
			throw MAKE_PYGIN_EXCEPTION(type_name() + " is not "s + TypeName);
	}
}
