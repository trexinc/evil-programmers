#pragma once

namespace detail
{
	struct handle_closer
	{
		void operator()(HANDLE Handle) const
		{
			CloseHandle(Handle);
		}
	};
}

using handle = std::unique_ptr<std::remove_pointer_t<HANDLE>, detail::handle_closer>;

inline auto normalize_handle(HANDLE Handle)
{
	return Handle == INVALID_HANDLE_VALUE? nullptr : Handle;
}

namespace detail
{
	struct module_closer
	{
		void operator()(HMODULE Module) const
		{
			FreeLibrary(Module);
		}
	};
}

using module_ptr = std::unique_ptr<std::remove_pointer_t<HMODULE>, detail::module_closer>;

namespace detail
{
	struct local_deleter
	{
		void operator()(HLOCAL Memory) const
		{
			LocalFree(Memory);
		}
	};
}

template<typename T>
using local_ptr = std::unique_ptr<T, detail::local_deleter>;

namespace detail
{
	struct nop_deleter
	{
		void operator()(void*) const
		{
		}
	};
}

template<class T>
using movalbe_ptr = std::unique_ptr<T, detail::nop_deleter>;

template<typename T>
class ptr_setter_t
{
public:
	ptr_setter_t(const ptr_setter_t&) = delete;
	ptr_setter_t& operator=(const ptr_setter_t&) = delete;
	ptr_setter_t(ptr_setter_t&&) = default;
	ptr_setter_t& operator=(ptr_setter_t&&) = default;

	ptr_setter_t(T& Ptr): m_Ptr(&Ptr)
	{
	}

	~ptr_setter_t()
	{
		if (m_Ptr)
			m_Ptr->reset(m_RawPtr);
	}

	auto operator&()
	{
		return &m_RawPtr;
	}

private:
	movalbe_ptr<T> m_Ptr;
	typename T::pointer m_RawPtr{};
};

template<typename T>
auto ptr_setter(T& Ptr)
{
	return ptr_setter_t<T>(Ptr);
}

std::wstring GetLastErrorMessage(DWORD LastError);
