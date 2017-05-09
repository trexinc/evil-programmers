#pragma once

#include "py_object.hpp"

class far_api
{
public:
	NONCOPYABLE(far_api);

	explicit far_api(const PluginStartupInfo* Psi);
	~far_api();

	const PluginStartupInfo& psi() const;
	const FarStandardFunctions& fsf() const;
	const py::object& get() const;

private:
	py::object m_Module;
	py::object m_Exception;
	PluginStartupInfo m_Psi;
	FarStandardFunctions m_Fsf;
};
