#pragma once

/*
far_api.hpp

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

#include "py.err.hpp"
#include "py.method_definitions.hpp"
#include "py.module.hpp"

namespace py
{
	class type;
}

class far_api
{
public:
	NONCOPYABLE(far_api);

	explicit far_api(const PluginStartupInfo* Psi);

	[[nodiscard]] const PluginStartupInfo& psi() const;
	[[nodiscard]] const FarStandardFunctions& fsf() const;
	[[nodiscard]] const py::object& exception() const;

	[[nodiscard]] const py::object& get_module() const;

	static void initialise(const PluginStartupInfo* Psi);
	[[nodiscard]] static const far_api& get();
	[[nodiscard]] static const py::object& module();
	[[nodiscard]] static py::type type(std::string_view TypeName);
	static void uninitialise();

private:
	py::method_definitions m_PyMethods;
	py::module m_Module;
	py::err::exception m_Exception;
	PluginStartupInfo m_Psi;
	FarStandardFunctions m_Fsf;
};
