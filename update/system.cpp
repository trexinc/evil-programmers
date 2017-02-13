#include "headers.hpp"
#pragma hdrstop

#include "system.hpp"

std::wstring GetLastErrorMessage(DWORD LastError)
{
	auto Message = L"[" + std::to_wstring(LastError) + L"] ";
	local_ptr<wchar_t> MessagePtr;
	const auto Size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, GetModuleHandle(L"wininet.dll"), LastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<wchar_t*>(&ptr_setter(MessagePtr)), 0, nullptr);
	if (Size)
		Message.append(MessagePtr.get(), Size);

	const auto IsEol = [](wchar_t c){ return c == L'\r' || c == L'\n'; };

	while (!Message.empty() && IsEol(Message.back()))
		Message.pop_back();

	return Message;
}