#include "headers.hpp"

#include "far_api.hpp"

#include "py_import.hpp"
#include "py_integer.hpp"
#include "py_list.hpp"
#include "py_string.hpp"
#include "py_tuple.hpp"
#include "py_uuid.hpp"

#include "py_common.hpp"

#include "error_handling.hpp"

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

	static auto string_list_to_vector(const py::list& List)
	{
		const auto Size = List.size();
		std::vector<std::wstring> Result;
		Result.reserve(Size);
		for (size_t i = 0; i != Size; ++i)
		{
			Result.push_back(py::cast<std::wstring>(List[i]));
		}
		return Result;
	}

	static auto get_args(PyObject* RawArgs, size_t Count)
	{
		const auto Args = py::cast<py::tuple>(py::object::from_borrowed(RawArgs));
		if (Args.size() != Count)
			throw MAKE_PYGIN_EXCEPTION("Message: wrong number of arguments");
		return Args;
	}

	static PyObject* Message(PyObject* Self, PyObject* RawArgs)
	{
		const auto Args = get_args(RawArgs, 7);

		const auto PluginId = py::cast<UUID>(Args[0]);
		const auto Id = py::cast<UUID>(Args[1]);
		const auto Flags = py::cast<FARMESSAGEFLAGS>(Args[2]);
		const auto HelpTopic = py::cast<std::wstring>(Args[3]);
		const auto Title = py::cast<std::wstring>(Args[4]);
		const auto Items = string_list_to_vector(py::cast<py::list>(Args[5]));
		const auto Buttons = string_list_to_vector(py::cast<py::list>(Args[6]));

		std::vector<const wchar_t*> AllItems;
		AllItems.reserve(1 + Items.size() + Buttons.size());
		AllItems.emplace_back(Title.data());
		std::transform(Items.cbegin(), Items.cend(), std::back_inserter(AllItems), [](const auto& i) { return i.data(); });
		std::transform(Buttons.cbegin(), Buttons.cend(), std::back_inserter(AllItems), [](const auto& i) { return i.data(); });

		const auto Result = psi().Message(&PluginId, &Id, Flags & ~FMSG_ALLINONE, HelpTopic.data(), AllItems.data(), AllItems.size(), Buttons.size());
		return PyLong_FromSsize_t(Result);
	}

	static PyObject* GetUserScreen(PyObject* Self, PyObject* Args)
	{
		psi().PanelControl(PANEL_NONE, FCTL_GETUSERSCREEN, 0, nullptr);

		Py_RETURN_NONE;
	}

	static PyObject* SetUserScreen(PyObject* Self, PyObject* Args)
	{
		psi().PanelControl(PANEL_NONE, FCTL_SETUSERSCREEN, 0, nullptr);

		Py_RETURN_NONE;
	}


	static PyMethodDef Methods[] =
	{
#define FUNC_NAME_VALUE(x) "__" #x, x

		{ FUNC_NAME_VALUE(Message), METH_VARARGS, "Show message" },
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

py::type far_api::type(const std::string& TypeName)
{
	return py::cast<py::type>(s_FarApi->m_Module[TypeName.data()]);
}

void far_api::uninitialise()
{
	s_FarApi.reset();
}
