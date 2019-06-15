/*
error_handling.cpp

*/
/*
Copyright 2019 Alex Alabuzhev
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
		MultiByteToWideChar(CP_UTF8, 0, Description.data(), static_cast<int>(Description.size()), &ErrorContext.Description[0], Size);
	}
}

bool get_error_context(ErrorInfo* Info) noexcept
{
	if (!ErrorContext.Occurred)
		return false;

	Info->StructSize = sizeof(*Info);
	Info->Summary = ErrorContext.Summary.c_str();
	Info->Description = ErrorContext.Description.c_str();
	return true;
}

void reset_error_context() noexcept
{
	ErrorContext.Occurred = false;
}
