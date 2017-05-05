#pragma once

#include "py_object.hpp"

namespace py
{
	class dictionary: public object
	{
	public:
		dictionary();

		explicit dictionary(const object& Object);

		class value_proxy
		{
		public:
			value_proxy(dictionary* Owner, const object& Key);
			value_proxy(const value_proxy& rhs);

			value_proxy& operator=(const char* value);
			value_proxy& operator=(const int value);
			value_proxy& operator=(const object& value);
			value_proxy& operator=(const value_proxy& value);

			operator object() const;

		private:
			object m_Key;
			dictionary* m_Owner;
		};

		value_proxy operator[](const char* Key);
		void set_at(const object& Key, const object& Value);
		object get_at(const object& Key);
	};
}
