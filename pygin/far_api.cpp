#include "headers.hpp"

#include "far_api.hpp"
#include "python.hpp"

#include "py_integer.hpp"

namespace far_api_implementation
{
	static const char ApiPointerAttribute[] = "api_pointer";

	static const far_api* Api(PyObject* Object)
	{
		const auto Ptr = py::object::from_borrowed(Object).get_attribute(ApiPointerAttribute);
		return reinterpret_cast<const far_api*>(PyLong_AsSize_t(Ptr.get()));
	}

	static PyObject* GetUserScreen(PyObject* self, PyObject* args)
	{
		Api(self)->m_Psi.PanelControl(PANEL_NONE, FCTL_GETUSERSCREEN, 0, nullptr);

		Py_INCREF(Py_None);
		return Py_None;
	}

	static PyObject* SetUserScreen(PyObject* self, PyObject* args)
	{
		Api(self)->m_Psi.PanelControl(PANEL_NONE, FCTL_SETUSERSCREEN, 0, nullptr);

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
}

far_api::far_api(const PluginStartupInfo* Psi):
	m_Module(nullptr),
	m_Exception(nullptr),
	m_Psi(*Psi),
	m_Fsf(*Psi->FSF)
{
	m_Psi.FSF = &m_Fsf;

	static PyModuleDef ModuleDefinition =
	{
		PyModuleDef_HEAD_INIT,
		"farapi",
		"Far API module",
		-1,
		far_api_implementation::methods
	};

	m_Module = py::object(PyModule_Create(&ModuleDefinition));
	if (!m_Module)
		return;

	m_Exception = py::object(PyErr_NewException("spam.error", nullptr, nullptr));
	DONT_STEAL_REFERENCE(m_Exception.get());
	PyModule_AddObject(m_Module.get(), "error", m_Exception.get());

	m_Module.set_attribute(far_api_implementation::ApiPointerAttribute, py::integer(reinterpret_cast<uintptr_t>(this)));
}

far_api::~far_api()
{
}

const py::object& far_api::get() const
{
	return m_Module;
}
