#pragma once
#include "py_object.hpp"

namespace py
{
	class tuple: public object
	{
	public:
		TRIVIALLY_MOVABLE(tuple);

		explicit tuple(size_t Size);
		explicit tuple(const object& Object);

		class value_proxy
		{
		public:
			TRIVIALLY_MOVABLE(value_proxy);

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
		void set_at(size_t Position, const object& Object);
		object get_at(size_t Position) const;

		template<typename... args>
		static tuple make(const args&... Args)
		{
			tuple Result(sizeof...(args));
			set_tuple_items(Result, 0, Args...);
			return Result;
		}

	private:
		static void set_tuple_items(tuple&, size_t) {}

		template<typename arg, typename... args>
		static void set_tuple_items(tuple& Tuple, size_t CurrentIndex, const arg& Arg, const args&... Args)
		{
			Tuple.set_at(CurrentIndex, Arg);
			set_tuple_items(Tuple, CurrentIndex + 1, Args...);
		}
	};

	tuple as_tuple(PyObject* Object);
	tuple as_tuple(const object& Object);
}
