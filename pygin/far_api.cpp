#include "headers.hpp"

#include "far_api.hpp"

#include "py_import.hpp"
#include "py_string.hpp"

#include "types_cache.hpp"

#include "python.hpp"

using namespace py::literals;

namespace far_api_implementation
{
	static const auto& fsf()
	{
		return far_api::get().fsf();
	}

	static const auto& psi()
	{
		return far_api::get().psi();
	}

	static PyObject* GetUserScreen(PyObject* self, PyObject* args)
	{
		psi().PanelControl(PANEL_NONE, FCTL_GETUSERSCREEN, 0, nullptr);

		Py_INCREF(Py_None);
		return Py_None;
	}

	static PyObject* SetUserScreen(PyObject* self, PyObject* args)
	{
		psi().PanelControl(PANEL_NONE, FCTL_SETUSERSCREEN, 0, nullptr);

		Py_INCREF(Py_None);
		return Py_None;
	}


	static PyMethodDef Methods[] =
	{
#define FUNC_NAME_VALUE(x) #x, x
		{ FUNC_NAME_VALUE(GetUserScreen), METH_NOARGS, "Get user screen" },
		{ FUNC_NAME_VALUE(SetUserScreen), METH_NOARGS, "Set user screen" },

#undef FUNC_NAME_VALUE
		{}
	};
}

far_api::far_api(const PluginStartupInfo* Psi):
	m_Module(py::import::import("pygin.far_api"_py)),
	m_Exception("pygin.far_api.error"),
	m_Psi(*Psi),
	m_Fsf(*Psi->FSF)
{
	m_Psi.FSF = &m_Fsf;

	// We use single instances of PSI and FSF for all plugins.
	// This is perfectly fine for FSF as it is completely static.
	// PSI, however, has some dynamic fields.
	// It is better to reset all of them to avoid any misusing.
	m_Psi.ModuleName = nullptr;
	m_Psi.Private = nullptr;
	m_Psi.Instance = nullptr;

	m_Module.add_object("error", m_Exception);
	m_Module.add_functions(far_api_implementation::Methods);
}

far_api::~far_api()
{
}

const PluginStartupInfo& far_api::psi() const
{
	return m_Psi;
}
const FarStandardFunctions& far_api::fsf() const
{
	return m_Fsf;
}

const py::object& far_api::get_module() const
{
	return m_Module;
}

const py::type& far_api::get_type(const std::string& TypeName) const
{
	return types_cache::get_type([this]{ return m_Module; }, TypeName);
}

static std::unique_ptr<far_api> s_FarApi;

void far_api::initialise(const PluginStartupInfo* Psi)
{
	if (!s_FarApi)
		s_FarApi = std::make_unique<far_api>(Psi);
}

const far_api& far_api::get()
{
	return *s_FarApi;
}

const py::object& far_api::module()
{
	return s_FarApi->get_module();
}

const py::type& far_api::type(const std::string& TypeName)
{
	return s_FarApi->get_type(TypeName);
}

void far_api::uninitialise()
{
	s_FarApi.reset();
}
