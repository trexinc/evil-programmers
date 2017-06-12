#pragma once
#include "py_object.hpp"

namespace py
{
	class tuple: public object
	{
	public:
		static auto type_name() { return "tuple"; }

		explicit tuple(size_t Size = 0);
		tuple(const std::initializer_list<object>& Args);
		template<typename... args>
		tuple(const args&... Args): tuple({ Args... }) {}
		tuple(cast_guard, const object& Object);

		class value_proxy
		{
		public:
			MOVABLE(value_proxy);

			value_proxy(tuple* Owner, size_t Index);
			value_proxy(const value_proxy& rhs);

			value_proxy& operator=(const object& value);
			value_proxy& operator=(const value_proxy& value);

			operator object() const;

		private:
			tuple* m_Owner;
			size_t m_Index;
		};

		value_proxy operator[](size_t Index);
		object operator[](size_t Index) const;
		void set_at(size_t Position, const object& Object);
		object get_at(size_t Position) const;
};
}
