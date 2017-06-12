#pragma once

#include "py_object.hpp"

namespace py
{
	class dictionary: public object
	{
	public:
		static auto type_name() { return "dict"; }

		dictionary();

		dictionary(cast_guard, const object& Object);

		class value_proxy
		{
		public:
			MOVABLE(value_proxy);

			value_proxy(dictionary* Owner, const object& Key);
			value_proxy(const value_proxy& rhs);

			value_proxy& operator=(const char* value);
			value_proxy& operator=(const int value);
			value_proxy& operator=(const object& value);
			value_proxy& operator=(const value_proxy& value);

			operator object() const;

		private:
			dictionary* m_Owner;
			object m_Key;
		};

		value_proxy operator[](const object& Key);
		object operator[](const object& Key) const;
		void set_at(const object& Key, const object& Value);
		object get_at(const object& Key) const;
	};
}
