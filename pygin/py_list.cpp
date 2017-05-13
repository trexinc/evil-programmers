#include "headers.hpp"

#include "py_list.hpp"

#include "python.hpp"

namespace py
{
	list::list(cast_guard, const object& Object):
		object(Object)
	{
	}

	list::value_proxy::value_proxy(list* Owner, size_t Index):
		m_Owner(Owner),
		m_Index(Index)
	{
	}

	list::value_proxy::value_proxy(const value_proxy& rhs):
		m_Owner(rhs.m_Owner),
		m_Index(rhs.m_Index)
	{
	}

	list::value_proxy& list::value_proxy::operator=(const object& value)
	{
		m_Owner->set_at(m_Index, value);
		return *this;
	}

	list::value_proxy& list::value_proxy::operator=(const value_proxy& value)
	{
		m_Owner->set_at(m_Index, value);
		return *this;
	}

	list::value_proxy::operator object() const
	{
		return m_Owner->get_at(m_Index);
	}

	list::value_proxy list::operator[](size_t Index)
	{
		return { this, Index };
	}

	object list::operator[](size_t Index) const
	{
		return get_at(Index);
	}

	void list::set_at(size_t Index, const object& Value)
	{
		DONT_STEAL_REFERENCE(Value.get());
		invoke(PyList_SetItem, get(), Index, Value.get());
	}

	object list::get_at(size_t Index) const
	{
		return from_borrowed(invoke(PyList_GetItem, get(), Index));
	}

	size_t list::size() const
	{
		return invoke(PyList_Size, get());
	}

	void list::push_back(const object& Object)
	{
		invoke(PyList_Append, get(), Object.get());
	}

	void list::insert(const object& Object, size_t index)
	{
		invoke(PyList_Insert, get(), index, Object.get());
	}
}
