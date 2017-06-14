#pragma once

using PyObject = struct _object;

namespace py
{
	class tuple;

	class cast_guard {};

	class object
	{
	public:
		object();
		explicit object(PyObject* Object);
		object(const object& rhs);
		object(object&& Object) noexcept;
		object(std::nullptr_t);

		~object();

		object& operator=(PyObject* rhs) &;
		object& operator=(const object& rhs) &;
		object& operator=(object&& Object) & noexcept;

		explicit operator bool() const;
		bool operator!() const;

		PyObject* get() const;

		bool has_attribute(const char* Name) const;
		bool has_attribute(const object& Name) const;

		object get_attribute(const char* Name) const;
		object get_attribute(const object& Name) const;

		bool set_attribute(const char* Name, const object& Value) const;
		bool set_attribute(const object& Name, const object& Value) const;

		class value_proxy
		{
		public:
			MOVABLE(value_proxy);

			value_proxy(object* Owner, const char* Key);
			value_proxy(const value_proxy& rhs);

			value_proxy& operator=(const object& value);
			value_proxy& operator=(const value_proxy& value);

			operator object() const;

		private:
			object* m_Owner;
			const char* m_Key;
		};

		value_proxy operator[](const char* Key);
		object operator[](const char* Key) const;

		template<typename... args>
		object operator()(const args&... Args) const
		{
			return operator()({ Args... });
		}

		static object from_borrowed(PyObject* Object);
		const char* type_name() const;
		bool check_type_name(const char* TypeName) const;
		void ensure_type_name(const char* TypeName) const;

	private:
		object operator()(const std::initializer_list<object>& Args) const;

		PyObject* m_Object;
	};

	template<typename T>
	T cast(PyObject* Object)
	{
		return cast<T>(object::from_borrowed(Object));
	}

	template<typename T>
	T cast(const object& Object)
	{
		Object.ensure_type_name(T::type_name());
		return T(cast_guard{}, Object);
	}

	template<typename T>
	T try_cast(PyObject* Object)
	{
		return try_cast<T>(object::from_borrowed(Object));
	}

	template<typename T>
	T try_cast(const object& Object)
	{
		return T(cast_guard{}, Object.check_type_name(T::type_name())? Object : object(nullptr));
	}

}
