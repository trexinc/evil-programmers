#include "headers.hpp"

#include "module.hpp"

#include "py_dictionary.hpp"
#include "py_err.hpp"
#include "py_string.hpp"
#include "py_tuple.hpp"
#include "py_tools.hpp"

static UUID UuidFromString(const std::wstring& Str)
{
	UUID Result;
	UuidFromString(reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(Str.data())), &Result);
	return Result;
}


module::module(const py::object& Object):
	m_Object(Object)
{
}

module::~module()
{
}

bool module::check_function(const wchar_t* FunctionName) const
{
	const auto Func = m_Object.get_attribute(py::string(FunctionName));
	py::err::clear();
	return (Func && py::callable_check(Func))
		// SetStartupInfoW is useless for Python script itself, but crucial for this wrapper
		|| !wcscmp(FunctionName, L"SetStartupInfoW");
}

py::object module::call_function(const char* FunctionName, const py::object& InfoArg) const
{
	const auto Func = m_Object.get_attribute(py::string(FunctionName));
	if (!Func)
		throw std::runtime_error(FunctionName + " is absent"s);

	if (!py::callable_check(Func))
		throw std::runtime_error(FunctionName + " is not callable"s);

	const auto Result = Func.call(py::tuple::make(InfoArg));
	if (!Result)
	{
		py::err::print_if_any();
		throw std::runtime_error(FunctionName + " call failed"s);
	}

	return Result;
}


HANDLE module::AnalyseW(const AnalyseInfo *Info)
{
	return nullptr;
}

void module::CloseAnalyseW(const CloseAnalyseInfo *Info)
{
}

void module::ClosePanelW(const ClosePanelInfo *Info)
{
}

intptr_t module::CompareW(const CompareInfo *Info)
{
	return 0;
}

intptr_t module::ConfigureW(const ConfigureInfo *Info)
{
	return 0;
}

intptr_t module::DeleteFilesW(const DeleteFilesInfo *Info)
{
	return 0;
}

void module::ExitFARW(const ExitInfo *Info)
{
}

void module::FreeFindDataW(const FreeFindDataInfo *Info)
{
}

intptr_t module::GetFilesW(GetFilesInfo *Info)
{
	return 0;
}

intptr_t module::GetFindDataW(GetFindDataInfo *Info)
{
	return 0;
}

void module::GetGlobalInfoW(GlobalInfo *Info)
{
	py::dictionary pyInfo;
	pyInfo["Title"] = "";
	pyInfo["Author"] = "";
	pyInfo["Description"] = "";
	pyInfo["Guid"] = "";

	call_function("GetGlobalInfoW", pyInfo);

	Info->Title = (m_Title = py::string::to_wstring(pyInfo["Title"])).data();
	Info->Author = (m_Author = py::string::to_wstring(pyInfo["Author"])).data();
	Info->Description = (m_Description = py::string::to_wstring(pyInfo["Description"])).data();
	Info->Guid = m_Uuid = UuidFromString(py::string::to_wstring(pyInfo["Guid"]));
}

void module::GetOpenPanelInfoW(OpenPanelInfo *Info)
{
}

void module::GetPluginInfoW(PluginInfo *Info)
{
	py::dictionary pyInfo;
	pyInfo["MenuString"] = "";
	pyInfo["Guid"] = "";

	call_function("GetPluginInfoW", pyInfo);

	const size_t Size = 1;
	m_MenuStringsData.resize(Size);
	m_MenuStrings.resize(Size);
	m_MEnuUuids.resize(Size);

	m_MenuStringsData[0] = py::string::to_wstring(pyInfo["MenuString"]);
	m_MenuStrings[0] = m_MenuStringsData[0].data();
	m_MEnuUuids[0] = UuidFromString(py::string::to_wstring(pyInfo["Guid"]));

	Info->PluginMenu.Strings = m_MenuStrings.data();
	Info->PluginMenu.Guids = m_MEnuUuids.data();
	Info->PluginMenu.Count = Size;

	Info->Flags |= PF_PRELOAD;
}

intptr_t module::MakeDirectoryW(MakeDirectoryInfo *Info)
{
	return 0;
}

HANDLE module::OpenW(const OpenInfo *Info)
{
	try
	{
		// BUGBUG
		m_Psi.PanelControl(PANEL_NONE, FCTL_GETUSERSCREEN, 0, nullptr);

		py::dictionary pyInfo;
		call_function("OpenW", pyInfo);

		// BUGBUG
		m_Psi.PanelControl(PANEL_NONE, FCTL_SETUSERSCREEN, 0, nullptr);
		return nullptr;
	}
	catch(...)
	{
		// BUGBUG
		m_Psi.PanelControl(PANEL_NONE, FCTL_SETUSERSCREEN, 0, nullptr);
		throw;
	}
}

intptr_t module::ProcessDialogEventW(const ProcessDialogEventInfo *Info)
{
	return 0;
}

intptr_t module::ProcessEditorEventW(const ProcessEditorEventInfo *Info)
{
	return 0;
}

intptr_t module::ProcessEditorInputW(const ProcessEditorInputInfo *Info)
{
	return 0;
}

intptr_t module::ProcessPanelEventW(const ProcessPanelEventInfo *Info)
{
	return 0;
}

intptr_t module::ProcessHostFileW(const ProcessHostFileInfo *Info)
{
	return 0;
}

intptr_t module::ProcessPanelInputW(const ProcessPanelInputInfo *Info)
{
	return 0;
}

intptr_t module::ProcessConsoleInputW(ProcessConsoleInputInfo *Info)
{
	return 0;
}

intptr_t module::ProcessSynchroEventW(const ProcessSynchroEventInfo *Info)
{
	return 0;
}

intptr_t module::ProcessViewerEventW(const ProcessViewerEventInfo *Info)
{
	return 0;
}

intptr_t module::PutFilesW(const PutFilesInfo *Info)
{
	return 0;
}

intptr_t module::SetDirectoryW(const SetDirectoryInfo *Info)
{
	return 0;
}

intptr_t module::SetFindListW(const SetFindListInfo *Info)
{
	return 0;
}

void module::SetStartupInfoW(const PluginStartupInfo *Info)
{
	m_Psi = *Info;
	m_Fsf = *Info->FSF;
	m_Psi.FSF = &m_Fsf;
}

intptr_t module::GetContentFieldsW(const GetContentFieldsInfo *Info)
{
	return 0;
}

intptr_t module::GetContentDataW(GetContentDataInfo *Info)
{
	return 0;
}

void module::FreeContentDataW(const GetContentDataInfo *Info)
{
}
