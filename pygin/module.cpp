#include "headers.hpp"

#include "module.hpp"
#include "far_api.hpp"

#include "py_string.hpp"
#include "py_tuple.hpp"
#include "py_uuid.hpp"
#include "py_list.hpp"
#include "py_boolean.hpp"
#include "py_common.hpp"

using namespace py::literals;

module::module(const py::object& Object):
	m_PluginModule(Object),
	m_PluginModuleClass(m_PluginModule.get_attribute("FarPluginClass"))
{
}

module::~module()
{
}

bool module::check_function(const wchar_t* FunctionName) const
{
	// Mapped to class static fields
	if (!wcscmp(FunctionName, L"GetGlobalInfoW"))
		return true;

	// Mapped to class ctor
	if (!wcscmp(FunctionName, L"SetStartupInfoW"))
		return true;

	// The rest is as is
	const auto Name = py::string(FunctionName);
	if (!m_PluginModuleClass.has_attribute(Name))
		return false;

	const auto Function = m_PluginModuleClass.get_attribute(Name);
	if (!py::callable_check(Function))
		return false;

	// Not perfect, but that should always be pure ASCII anyway, so why not.
	std::string NarrowName(FunctionName, FunctionName + wcslen(FunctionName));
	m_PluginModuleClassFunctions.emplace(std::move(NarrowName), Function);
	return true;
}

template<typename ... args>
py::object module::call(const char* FunctionName, const args&... Args) const
{
	return m_PluginModuleClassFunctions.at(FunctionName)(m_PluginModuleInstance, Args...);
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
	const auto ConfigureInstance = far_api::type("ConfigureInfo"s)();

	ConfigureInstance.set_attribute("Guid", py::uuid(*Info->Guid));

	const auto Result = py::cast<py::boolean>(call(STR(ConfigureW), ConfigureInstance));
	return Result.to_bool();
}

intptr_t module::DeleteFilesW(const DeleteFilesInfo *Info)
{
	return 0;
}

void module::ExitFARW(const ExitInfo *Info)
{
	const auto ExitInfoInstance = far_api::type("ExitInfo"s)();
	call(STR(ExitFARW), ExitInfoInstance);

	// Point of no return
	m_PluginModuleInstance = {};
	m_PluginModuleClassFunctions.clear();
	m_PluginModuleClass = {};
	m_PluginModule = {};
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
	Info->Title = (m_Title = py::cast<py::string>(m_PluginModuleClass.get_attribute("Title")).to_wstring()).data();
	Info->Author = (m_Author = py::cast<py::string>(m_PluginModuleClass.get_attribute("Author")).to_wstring()).data();
	Info->Description = (m_Description = py::cast<py::string>(m_PluginModuleClass.get_attribute("Description")).to_wstring()).data();
	Info->Guid = py::cast<py::uuid>(m_PluginModuleClass.get_attribute("Guid")).to_uuid();
}

void module::GetOpenPanelInfoW(OpenPanelInfo *Info)
{
}

void module::GetPluginInfoW(PluginInfo *Info)
{
	const auto PyInfo = call(STR(GetPluginInfoW));
	PyInfo.ensure_type_name("PluginInfo");

	const auto& ConvertPluginMenuItem = [&](const char* Kind, menu_items& MenuItems, PluginMenuItem PluginInfo::*Destination)
	{
		const auto ItemsList = py::cast<py::list>(PyInfo.get_attribute(Kind));

		const auto ListSize = ItemsList.size();

		const auto prepare = [ListSize](auto& Container)
		{
			Container.clear();
			Container.reserve(ListSize);
		};

		prepare(MenuItems.StringsData);
		prepare(MenuItems.Strings);
		prepare(MenuItems.Uuids);

		// TODO: enumerator
		for (size_t i = 0; i != ListSize; ++i)
		{
			const auto Tuple = py::cast<py::tuple>(ItemsList[i]);
			MenuItems.StringsData.emplace_back(py::cast<py::string>(Tuple[0]).to_wstring());
			MenuItems.Strings.emplace_back(MenuItems.StringsData.back().data());
			MenuItems.Uuids.emplace_back(py::cast<py::uuid>(Tuple[1]).to_uuid());
		}

		(Info->*Destination).Strings = MenuItems.Strings.data();
		(Info->*Destination).Guids = MenuItems.Uuids.data();
		(Info->*Destination).Count = ListSize;
	};

	ConvertPluginMenuItem("PluginMenuItems", m_PluginMenuItems, &PluginInfo::PluginMenu);
	ConvertPluginMenuItem("DiskMenuItems", m_DiskMenuItems, &PluginInfo::DiskMenu);
	ConvertPluginMenuItem("PluginConfigItems", m_PluginConfigItems, &PluginInfo::PluginConfig);

	Info->Flags |= PF_PRELOAD;
}

intptr_t module::MakeDirectoryW(MakeDirectoryInfo *Info)
{
	return 0;
}

HANDLE module::OpenW(const OpenInfo *Info)
{
	const auto OpenInfoInstance = far_api::type("OpenInfo"s)();

	OpenInfoInstance.set_attribute("Guid", py::uuid(*Info->Guid));

	const auto Result = call(STR(OpenW), OpenInfoInstance);

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
	far_api::initialise(Info);
	m_PluginModuleInstance = m_PluginModuleClass(far_api::module());
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
