#pragma once
#include "py_object.hpp"

namespace py
{
	class tuple: public object
	{
	public:
		TRIVIALLY_MOVABLE(tuple);

		explicit tuple(size_t Size);

		void set_at(size_t Position, const object& Object);

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
}
