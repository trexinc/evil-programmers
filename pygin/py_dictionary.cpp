#include "headers.hpp"

#include "py_dictionary.hpp"
#include "py_integer.hpp"
#include "py_string.hpp"

#include "python.hpp"

namespace py
{
	dictionary::dictionary():
		object(PyDict_New())
	{
	}

	dictionary::dictionary(const object& Object):
		object(Object)
	{
	}

	dictionary::value_proxy::value_proxy(dictionary* Owner, const object& Key):
		m_Owner(Owner),
		m_Key(Key)
	{
	}

	dictionary::value_proxy::value_proxy(const value_proxy& rhs):
		m_Owner(rhs.m_Owner),
		m_Key(rhs.m_Key)
	{
	}

	dictionary::value_proxy& dictionary::value_proxy::operator=(const char* value)
	{
		m_Owner->set_at(m_Key, string(value));
		return *this;
	}

	dictionary::value_proxy& dictionary::value_proxy::operator=(int value)
	{
		m_Owner->set_at(m_Key, py::integer(value));
		return *this;
	}

	dictionary::value_proxy& dictionary::value_proxy::operator=(const object& value)
	{
		m_Owner->set_at(m_Key, value);
		return *this;
	}

	dictionary::value_proxy& dictionary::value_proxy::operator=(const value_proxy& value)
	{
		m_Owner->set_at(m_Key, value);
		return *this;
	}

	dictionary::value_proxy::operator object() const
	{
		return m_Owner->get_at(m_Key);
	}

	dictionary::value_proxy dictionary::operator[](const char* Key)
	{
		return{ this, string(Key) };
	}

	void dictionary::set_at(const object& Key, const object& Value)
	{
		PyDict_SetItem(get(), Key.get(), Value.get());
	}

	object dictionary::get_at(const object& Key)
	{
		return from_borrowed(PyDict_GetItem(get(), Key.get()));
	}
}
