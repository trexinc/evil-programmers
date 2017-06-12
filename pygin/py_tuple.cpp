#include "headers.hpp"

#include "py_tuple.hpp"
#include "py_common.hpp"

#include "python.hpp"

namespace py
{
	tuple::tuple(size_t Size):
		object(invoke(PyTuple_New, Size))
	{
	}

	tuple::tuple(const std::initializer_list<object>& Args):
		tuple(Args.size())
	{
		for (size_t i = 0; i != Args.size(); ++i)
		{
			set_at(i, Args.begin()[i]);
		}
	}

	tuple::tuple(cast_guard, const object& Object):
		object(Object)
	{
	}

	tuple::value_proxy::value_proxy(tuple* Owner, size_t Index):
		m_Owner(Owner),
		m_Index(Index)
	{
	}

	tuple::value_proxy::value_proxy(const value_proxy& rhs):
		m_Owner(rhs.m_Owner),
		m_Index(rhs.m_Index)
	{
	}

	tuple::value_proxy& tuple::value_proxy::operator=(const object& value)
	{
		m_Owner->set_at(m_Index, value);
		return *this;
	}

	tuple::value_proxy& tuple::value_proxy::operator=(const value_proxy& value)
	{
		m_Owner->set_at(m_Index, value);
		return *this;
	}

	tuple::value_proxy::operator object() const
	{
		return m_Owner->get_at(m_Index);
	}

	tuple::value_proxy tuple::operator[](size_t Index)
	{
		return { this, Index };
	}

	object tuple::operator[](size_t Index) const
	{
		return get_at(Index);
	}

	void tuple::set_at(size_t Position, const object& Object)
	{
		DONT_STEAL_REFERENCE(Object.get());
		invoke(PyTuple_SetItem, get(), Position, Object.get());
	}

	object tuple::get_at(size_t Position) const
	{
		return from_borrowed(invoke(PyTuple_GetItem, get(), Position));
	}
}
