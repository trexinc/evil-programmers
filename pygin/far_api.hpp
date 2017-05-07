#pragma once

#include "py_object.hpp"

struct PyModuleDef;

class far_api
{
public:
	NONCOPYABLE(far_api);

	far_api(const PluginStartupInfo* Psi);
	~far_api();

	const py::object& get() const;

private:
	py::object m_Module;
	py::object m_Exception;

public: // BUGBUG
	PluginStartupInfo m_Psi;
	FarStandardFunctions m_Fsf;
};
