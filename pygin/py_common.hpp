#pragma once

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

	void initialize();
	void finalize();

	bool callable_check(const object& Object);
}
