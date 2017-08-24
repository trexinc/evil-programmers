#pragma once

/*
py.object.hpp

*/
/*
Copyright 2017 Alex Alabuzhev
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

using PyObject = struct _object;

namespace py
{
	class object;
	class tuple;
	class type;

	class cast_guard {};

	template<typename owner_type, typename key_type>
	class proxy_owner
	{
	public:
		auto operator[](key_type Key);

		auto operator[](const key_type& Key) const
		{
			return static_cast<const owner_type*>(this)->get_at(Key);
		}
	};

	template<typename owner_type, typename... keys>
	class proxies_owner: public proxy_owner<owner_type, keys>...
	{
	};

	template<typename owner_type, typename key_type>
	class value_proxy: public proxies_owner<value_proxy<owner_type, key_type>, const char*, const wchar_t*, std::string, std::wstring>
	{
	public:
		MOVABLE(value_proxy);
		NONCOPYABLE(value_proxy);

		value_proxy(owner_type* Owner, key_type&& Key):
			m_Owner(Owner),
			m_Key(std::forward<key_type>(Key))
		{
		}

		template<typename type>
		value_proxy& operator=(const type& Value)
		{
			m_Owner->set_at(m_Key, from(Value));
			return *this;
		}

		template<typename type>
		object get_at(const type& Name) const;

		bool set_at(const char* Name, const object& Value) const;

		operator object() const;

	private:
		owner_type* m_Owner;
		key_type m_Key;
	};

	template<typename owner_type, typename key_type>
	auto proxy_owner<owner_type, key_type>::operator[](key_type Key)
	{
		return value_proxy<owner_type, key_type>{ static_cast<owner_type*>(this), std::move(Key) };
	}

	class object: public proxies_owner<object, const char*, const wchar_t*, std::string, std::wstring>
	{
	public:
		using proxy_owner<object, const char*>::operator[];
		using proxy_owner<object, const wchar_t*>::operator[];
		using proxy_owner<object, std::string>::operator[];
		using proxy_owner<object, std::wstring>::operator[];

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
		PyObject* release();

		bool has_attribute(const char* Name) const;
		bool has_attribute(const wchar_t* Name) const;
		bool has_attribute(const std::string& Name) const;
		bool has_attribute(const std::wstring& Name) const;
		bool has_attribute(const object& Name) const;

		object get_attribute(const char* Name) const;
		object get_attribute(const wchar_t* Name) const;
		object get_attribute(const std::string& Name) const;
		object get_attribute(const std::wstring& Name) const;
		object get_attribute(const object& Name) const;

		template<typename type>
		object get_at(const type& Type) const
		{
			return get_attribute(Type);
		}

		bool set_attribute(const char* Name, const object& Value);
		bool set_attribute(const wchar_t* Name, const object& Value);
		bool set_attribute(const std::string& Name, const object& Value);
		bool set_attribute(const std::wstring& Name, const object& Value);
		bool set_attribute(const object& Name, const object& Value);
		bool set_at(const char* Name, const object& Value);

		template<typename... args>
		object operator()(const args&... Args) const
		{
			return operator()({ from(Args)... });
		}

		static object from_borrowed(PyObject* Object);
		bool check_type(const type& Type) const;
		void ensure_type(const type& Type) const;

	private:
		object operator()(const std::initializer_list<object>& Args) const;

		PyObject* m_Object;
	};

	template<typename owner_type, typename key_type>
	template<typename type>
	object value_proxy<owner_type, key_type>::get_at(const type& Name) const
	{
		return operator object().get_at(Name);
	}

	template<typename owner_type, typename key_type>
	bool value_proxy<owner_type, key_type>::set_at(const char* Name, const object& Value) const
	{
		return operator object().set_at(Name, Value);
	}

	template<typename owner_type, typename key_type>
	value_proxy<owner_type, key_type>::operator object() const
	{
		return m_Owner->get_at(m_Key);
	}

	template<typename T, bool IsClass>
	struct cast_impl;

	template<typename T>
	struct cast_impl<T, true>
	{
		static T impl(const object& Object)
		{
			Object.ensure_type(T::get_type());
			return T(cast_guard{}, Object);
		}
	};

	template<typename T>
	T cast(const object& Object)
	{
		return cast_impl<T, std::is_class<T>::value>::impl(Object);
	}

	template<typename T>
	T try_cast(const object& Object)
	{
		return T(cast_guard{}, Object && Object.check_type(T::get_type())? Object : object(nullptr));
	}

	inline object from(const object& Value)
	{
		return Value;
	}
}
