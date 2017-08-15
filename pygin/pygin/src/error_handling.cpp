#include "headers.hpp"

#include "error_handling.hpp"

static thread_local struct
{
	bool Occurred;
	std::wstring Summary;
	std::wstring Description;
}
ErrorContext;

void set_error_context(const wchar_t* Summary, const std::string& Description)
{
	ErrorContext.Occurred = true;
	ErrorContext.Summary = Summary;

	const auto Size = MultiByteToWideChar(CP_UTF8, 0, Description.data(), static_cast<int>(Description.size()), nullptr, 0);
	if (Size)
	{
		ErrorContext.Description.resize(Size);
		MultiByteToWideChar(CP_UTF8, 0, Description.data(), -1, &ErrorContext.Description[0], Size);
	}
}

bool get_error_context(ErrorInfo* Info)
{
	if (!ErrorContext.Occurred)
		return false;

	Info->Summary = ErrorContext.Summary.data();
	Info->Description = ErrorContext.Description.data();
	return true;
}

void reset_error_context()
{
	ErrorContext.Occurred = false;
}

detail::exception_impl::exception_impl(const std::string& Message, const char* Function, const char* File, int Line):
	m_Message(Message),
	m_FullMessage(Message + "\n (at " + Function + ", " + File + ":" + std::to_string(Line) + ")")
{
}

const std::string& detail::exception_impl::get_message() const noexcept
{
	return m_Message;
}

const std::string& detail::exception_impl::get_full_message() const noexcept
{
	return m_FullMessage;
}

pygin_exception::pygin_exception(const std::string& Message, const char* Function, const char* File, int Line):
	exception_impl(Message, Function, File, Line),
	std::runtime_error(get_full_message())
{
}
