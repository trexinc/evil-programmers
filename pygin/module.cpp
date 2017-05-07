#include "headers.hpp"

#include "module.hpp"

#include "py_dictionary.hpp"
#include "py_err.hpp"
#include "py_string.hpp"
#include "py_tuple.hpp"
#include "py_tools.hpp"

using namespace py::literals;

static UUID UuidFromString(const std::wstring& Str)
{
	UUID Result;
	UuidFromString(reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(Str.data())), &Result);
	return Result;
}

module::module(const py::object& Object):
	m_PluginModule(Object)
{
}

module::~module()
{
}

bool module::check_function(const wchar_t* FunctionName) const
{
	const auto Func = m_PluginModule.get_attribute(py::string(FunctionName));
	py::err::clear();
	return Func && py::callable_check(Func);
}

py::object module::call_function(const char* FunctionName, const py::object& InfoArg) const
{
	const auto Func = m_PluginModule.get_attribute(py::string(FunctionName));
	if (!Func)
		throw std::runtime_error(FunctionName + " is absent"s);

	if (!py::callable_check(Func))
		throw std::runtime_error(FunctionName + " is not callable"s);

	const auto Result = Func.call(InfoArg? py::tuple::make(InfoArg) : InfoArg);
	if (!Result)
	{
		py::err::print_if_any();
		throw std::runtime_error(FunctionName + " call failed"s);
	}

	return Result;
}


#define STR(x) #x

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
	py::dictionary pyInfo;
	call_function(STR(ExitFARW), pyInfo);
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

	call_function(STR(GetGlobalInfoW), pyInfo);

	Info->Title = (m_Title = py::as_string(pyInfo["Title"]).to_wstring()).data();
	Info->Author = (m_Author = py::as_string(pyInfo["Author"]).to_wstring()).data();
	Info->Description = (m_Description = py::as_string(pyInfo["Description"]).to_wstring()).data();
	Info->Guid = m_Uuid = UuidFromString(py::as_string(pyInfo["Guid"]).to_wstring());
}

void module::GetOpenPanelInfoW(OpenPanelInfo *Info)
{
}

void module::GetPluginInfoW(PluginInfo *Info)
{
	py::dictionary pyInfo;
	pyInfo["MenuString"] = "";
	pyInfo["Guid"] = "";

	call_function(STR(GetPluginInfoW), pyInfo);

	const size_t Size = 1;
	m_MenuStringsData.resize(Size);
	m_MenuStrings.resize(Size);
	m_MenuUuids.resize(Size);

	m_MenuStringsData[0] = py::as_string(pyInfo["MenuString"]).to_wstring();
	m_MenuStrings[0] = m_MenuStringsData[0].data();
	m_MenuUuids[0] = UuidFromString(py::as_string(pyInfo["Guid"]).to_wstring());

	Info->PluginMenu.Strings = m_MenuStrings.data();
	Info->PluginMenu.Guids = m_MenuUuids.data();
	Info->PluginMenu.Count = Size;

	Info->Flags |= PF_PRELOAD;
}

intptr_t module::MakeDirectoryW(MakeDirectoryInfo *Info)
{
	return 0;
}

HANDLE module::OpenW(const OpenInfo *Info)
{
	py::dictionary pyInfo;
	const auto Result = call_function(STR(OpenW), pyInfo);
	// BUGBUG
	return nullptr;
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
	m_FarApi = std::make_unique<far_api>(Info);
	call_function(STR(SetStartupInfoW), m_FarApi->get());
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
