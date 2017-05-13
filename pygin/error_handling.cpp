#include "headers.hpp"

#include "error_handling.hpp"

void notify_user(const std::string& ExceptionText)
{
	// Very bad, but betta than nothing
	MessageBoxA(nullptr, ExceptionText.data(), "Pygin Exception", MB_ICONERROR);
}
