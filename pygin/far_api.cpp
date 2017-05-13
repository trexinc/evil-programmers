#include "headers.hpp"

#include "far_api.hpp"

#include "py_integer.hpp"

#include "python.hpp"

namespace far_api_implementation
{
	static const char ApiPointerAttributeName[] = "api_pointer";

	static const auto& api(PyObject* Object)
	{
		const auto Ptr = py::cast<py::integer>(py::object::from_borrowed(Object).get_attribute(ApiPointerAttributeName));
		return *reinterpret_cast<const far_api*>(Ptr.to_size_t());
	}

	static const auto& fsf(PyObject* Object)
	{
		return api(Object).fsf();
	}

	static const auto& psi(PyObject* Object)
	{
		return api(Object).psi();
	}

	static PyObject* GetUserScreen(PyObject* self, PyObject* args)
	{
		psi(self).PanelControl(PANEL_NONE, FCTL_GETUSERSCREEN, 0, nullptr);

		Py_INCREF(Py_None);
		return Py_None;
	}

	static PyObject* SetUserScreen(PyObject* self, PyObject* args)
	{
		psi(self).PanelControl(PANEL_NONE, FCTL_SETUSERSCREEN, 0, nullptr);

		Py_INCREF(Py_None);
		return Py_None;
	}

	static PyMethodDef methods[] =
	{
#define FUNC_NAME_VALUE(x) #x, x
		{ FUNC_NAME_VALUE(GetUserScreen), METH_NOARGS, "Get user screen" },
		{ FUNC_NAME_VALUE(SetUserScreen), METH_NOARGS, "Set user screen" },

#undef FUNC_NAME_VALUE
		{}
	};

	static PyModuleDef ModuleDefinition =
	{
		PyModuleDef_HEAD_INIT,
		"farapi",
		"Far API module",
		-1,
		methods
	};
}

far_api::far_api(const PluginStartupInfo* Psi):
	m_Module(py::invoke(PyModule_Create2, &far_api_implementation::ModuleDefinition, PYTHON_API_VERSION)),
	m_Exception(py::invoke(PyErr_NewException, "farapi.error", nullptr, nullptr)),
	m_Psi(*Psi),
	m_Fsf(*Psi->FSF)
{
	m_Psi.FSF = &m_Fsf;


	DONT_STEAL_REFERENCE(m_Exception.get());
	py::invoke(PyModule_AddObject, m_Module.get(), "error", m_Exception.get());

	m_Module.set_attribute(far_api_implementation::ApiPointerAttributeName, py::integer(reinterpret_cast<uintptr_t>(this)));
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

const py::object& far_api::get() const
{
	return m_Module;
}
