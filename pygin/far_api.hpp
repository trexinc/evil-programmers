#pragma once

#include "py_err.hpp"
#include "py_module.hpp"

namespace py
{
	class type;
}

class far_api
{
public:
	NONCOPYABLE(far_api);

	explicit far_api(const PluginStartupInfo* Psi);

	const PluginStartupInfo& psi() const;
	const FarStandardFunctions& fsf() const;

	const py::object& get_module() const;

	static void initialise(const PluginStartupInfo* Psi);
	static const far_api& get();
	static const py::object& module();
	static py::type type(const std::string& TypeName);
	static void uninitialise();

private:
	py::module m_Module;
	py::err::exception m_Exception;
	PluginStartupInfo m_Psi;
	FarStandardFunctions m_Fsf;
};
