/*
py.object.cpp

*/
/*
Copyright 2017 Alex Alabuzhev
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"

#include "py.object.hpp"

#include "py.string.hpp"
#include "py.tuple.hpp"
#include "py.type.hpp"

#include "error_handling.hpp"

#include "python.hpp"

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

	object& object::operator=(PyObject* Rhs) &
	{
		Py_XDECREF(m_Object);
		m_Object = Rhs;
		return *this;
	}

	object& object::operator=(const object& Rhs) &
	{
		Py_XINCREF(Rhs.m_Object);
		Py_XDECREF(m_Object);
		m_Object = Rhs.m_Object;
		return *this;
	}

	object& object::operator=(object&& Rhs) & noexcept
	{
		m_Object = std::exchange(Rhs.m_Object, nullptr);
		return *this;
	}

	object::operator bool() const
	{
		return !!*this;
	}

	bool object::operator!() const
	{
		return !m_Object || m_Object == Py_None;
	}

	PyObject* object::get() const
	{
		return m_Object;
	}

	PyObject* object::release()
	{
		return std::exchange(m_Object, nullptr);
	}

	bool object::has_attribute(const char* Name) const
	{
		return invoke(PyObject_HasAttrString, get(), Name) != 0;
	}

	bool object::has_attribute(const std::string& Name) const
	{
		return has_attribute(Name.data());
	}

	bool object::has_attribute(const object& Name) const
	{
		return invoke(PyObject_HasAttr, get(), Name.get()) != 0;
	}

	object object::get_attribute(const char* Name) const
	{
		return object(invoke(PyObject_GetAttrString, get(), Name));
	}

	object object::get_attribute(const std::string& Name) const
	{
		return get_attribute(Name.data());
	}

	object object::get_attribute(const object& Name) const
	{
		return object(invoke(PyObject_GetAttr, get(), Name.get()));
	}

	object object::get_at(const char* Name) const
	{
		return get_attribute(Name);
	}

	object object::get_at(const std::string& Name) const
	{
		return get_at(Name.data());
	}

	bool object::set_attribute(const char* Name, const object& Value) const
	{
		return invoke(PyObject_SetAttrString, get(), Name, Value.get()) == 0;
	}

	bool object::set_attribute(const object& Name, const object& Value) const
	{
		return invoke(PyObject_SetAttr, get(), Name.get(), Value.get()) == 0;
	}

	bool object::set_at(const char* Name, const object& Value) const
	{
		return set_attribute(Name, Value);
	}

	object object::operator()(const std::initializer_list<object>& Args) const
	{
		return object(invoke(PyObject_CallObject, get(), tuple(Args).get()));
	}

	object object::from_borrowed(PyObject* Object)
	{
		Py_XINCREF(Object);
		return object(Object);
	}

	bool object::check_type(const type& Type) const
	{
		return py::invoke(PyObject_IsInstance, get(), Type.get()) != 0 || (*this && Type && get()->ob_type == Type.get()->ob_type);
	}

	void object::ensure_type(const type& Type) const
	{
		if (!check_type(Type))
		{
			const auto ThisTypeName = *this? cast<std::string>(type(*this)["__name__"]) : "None"s;
			throw MAKE_PYGIN_EXCEPTION(ThisTypeName + " is not "s + cast<std::string>(Type["__name__"]));
		}
	}
}
