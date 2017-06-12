#pragma once

typedef struct _object PyObject;

namespace py
{
	class object;

	class exception: public std::runtime_error
	{
		using runtime_error::runtime_error;
	};

	void raise_exception_if_needed();

	template<typename callable, typename... args>
	auto invoke(callable Callable, args... Args)
	{
		class final_act
		{
		public:
			~final_act() noexcept(false)
			{
				raise_exception_if_needed();
			}
		};

		final_act FinalAct;
		return Callable(Args...);
	}

	template<typename T>
	T cast(PyObject* Object)
	{
		return cast<T>(object::from_borrowed(Object));
	}

	template<typename T>
	T cast(const object& Object)
	{
		Object.validate_type_name(T::type_name());
		return T(cast_guard{}, Object);
	}

	void initialize();
	void finalize();

	bool callable_check(const object& Object);
}
