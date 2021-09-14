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

static void IncrementReference(PyObject* Object)
{
	Py_XINCREF(Object);
}

static void DecrementReference(PyObject* Object)
{
	Py_XDECREF(Object);
}

#ifdef _DEBUG
std::atomic_size_t g_ObjectsCount;
#endif

namespace py
{
	object::counter::counter()
	{
#ifdef _DEBUG
		++g_ObjectsCount;
#endif
	}

	object::counter::~counter()
	{
#ifdef _DEBUG
		--g_ObjectsCount;
#endif
	}

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
		IncrementReference(m_Object);
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
		DecrementReference(m_Object);
	}

	object object::none()
	{
		return from_borrowed(Py_None);
	}

	object& object::operator=(PyObject* Rhs) &
	{
		DecrementReference(m_Object);
		m_Object = Rhs;
		return *this;
	}

	object& object::operator=(const object& Rhs) &
	{
		IncrementReference(Rhs.m_Object);
		DecrementReference(m_Object);
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
		return m_Object && m_Object != Py_None;
	}

	PyObject* object::get() const
	{
		return m_Object;
	}

	PyObject* object::get_no_steal() const
	{
		IncrementReference(m_Object);
		return get();
	}

	PyObject* object::release()
	{
		return std::exchange(m_Object, nullptr);
	}

	bool object::has_attribute(std::string_view const Name) const
	{
		return has_attribute(string(Name));
	}


	bool object::has_attribute(std::wstring_view const Name) const
	{
		return has_attribute(string(Name));
	}

	bool object::has_attribute(const object& Name) const
	{
		return invoke(PyObject_HasAttr, get(), Name.get()) != 0;
	}

	object object::get_attribute(std::string_view const Name) const
	{
		return get_attribute(string(Name));
	}

	object object::get_attribute(std::wstring_view const Name) const
	{
		return get_attribute(string(Name));
	}

	object object::get_attribute(const object& Name) const
	{
		return object(invoke(PyObject_GetAttr, get(), Name.get()));
	}

	bool object::set_attribute(std::string_view const Name, const object& Value)
	{
		return set_attribute(string(Name), Value);
	}

	bool object::set_attribute(std::wstring_view const Name, const object& Value)
	{
		return set_attribute(string(Name), Value);
	}

	bool object::set_attribute(const object& Name, const object& Value)
	{
		return invoke(PyObject_SetAttr, get(), Name.get(), Value.get()) == 0;
	}

	bool object::set_at(std::string_view const Name, const object& Value)
	{
		return set_attribute(Name, Value);
	}

	iterator object::begin() const
	{
		return iterator(*this, false);
	}

	iterator object::end() const
	{
		return iterator(*this, true);
	}

	iterator object::cbegin() const
	{
		return begin();
	}

	iterator object::cend() const
	{
		return end();
	}

	object object::operator()(const std::initializer_list<object>& Args) const
	{
		return object(invoke(PyObject_CallObject, get(), tuple(Args).get()));
	}

	object object::from_borrowed(PyObject* Object)
	{
		IncrementReference(Object);
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
			const auto ThisTypeName = *this? cast<std::string>(type(*this)["__name__"sv]) : "None"s;
			throw MAKE_PYGIN_EXCEPTION(ThisTypeName + " is not "s + cast<std::string>(Type["__name__"sv]));
		}
	}

	iterator::iterator(const object& Container, bool const IsEnd):
		m_Container(Container.get()),
		m_Iterable(IsEnd? nullptr : py::invoke(PyObject_GetIter, m_Container)),
		m_Value(IsEnd? nullptr : py::invoke(PyIter_Next, m_Iterable.get()))
	{
		if (m_Iterable && !m_Value) // empty list
			m_Iterable = nullptr;
	}

	object iterator::operator*() const
	{
		return m_Value;
	}

	const object* iterator::operator->() const
	{
		return &m_Value;
	}

	iterator& iterator::operator++()
	{
		m_Value = py::invoke(PyIter_Next, m_Iterable.get());
		if (!m_Value)
		{
			m_Iterable = nullptr;
		}
		return *this;
	}

	bool iterator::operator==(const iterator& rhs) const
	{
		return
			m_Container == rhs.m_Container &&
			m_Iterable.get() == rhs.m_Iterable.get() &&
			m_Value.get() == rhs.m_Value.get();
	}

	bool iterator::operator!=(const iterator& rhs) const
	{
		return !(*this == rhs);
	}
}
