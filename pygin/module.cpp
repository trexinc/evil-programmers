#include "headers.hpp"

#include "module.hpp"

#include "py_boolean.hpp"
#include "py_common.hpp"
#include "py_floating.hpp"
#include "py_list.hpp"
#include "py_string.hpp"
#include "py_tuple.hpp"
#include "py_uuid.hpp"

#include "far_api.hpp"
#include "py_integer.hpp"
#include "py_bytes.hpp"

using namespace py::literals;

module::module(const py::object& Object):
	m_PluginModule(Object),
	m_PluginModuleClass(m_PluginModule["FarPluginClass"])
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

	// Not perfect, but that should always be pure ASCII anyway, so why not.
	std::string NarrowName(FunctionName, FunctionName + wcslen(FunctionName));

	if (!m_PluginModuleClass.has_attribute(NarrowName.data()))
		return false;

	const auto Function = m_PluginModuleClass[NarrowName.data()];
	if (!py::callable_check(Function))
		return false;

	m_PluginModuleClassFunctions.emplace(std::move(NarrowName), Function);
	return true;
}

template<typename ... args>
py::object module::call(const char* FunctionName, const args&... Args) const
{
	return m_PluginModuleClassFunctions.at(FunctionName)(m_PluginModuleInstance, Args...);
}

#define STR(x) #x

HANDLE module::AnalyseW(const AnalyseInfo* Info)
{
	return nullptr;
}

void module::CloseAnalyseW(const CloseAnalyseInfo* Info)
{
}

void module::ClosePanelW(const ClosePanelInfo* Info)
{
}

intptr_t module::CompareW(const CompareInfo* Info)
{
	return 0;
}

intptr_t module::ConfigureW(const ConfigureInfo* Info)
{
	auto ConfigureInstance = far_api::type("ConfigureInfo"s)();

	ConfigureInstance["Guid"] = py::uuid(*Info->Guid);

	return py::cast<bool>(call(STR(ConfigureW), ConfigureInstance));
}

intptr_t module::DeleteFilesW(const DeleteFilesInfo* Info)
{
	return 0;
}

void module::ExitFARW(const ExitInfo* Info)
{
	const auto ExitInfoInstance = far_api::type("ExitInfo"s)();
	call(STR(ExitFARW), ExitInfoInstance);

	// Point of no return
	m_PluginModuleInstance = {};
	m_PluginModuleClassFunctions.clear();
	m_PluginModuleClass = {};
	m_PluginModule = {};
}

void module::FreeFindDataW(const FreeFindDataInfo* Info)
{
}

intptr_t module::GetFilesW(GetFilesInfo* Info)
{
	return 0;
}

intptr_t module::GetFindDataW(GetFindDataInfo* Info)
{
	return 0;
}

void module::GetGlobalInfoW(GlobalInfo* Info)
{
	Info->Title = (m_Title = py::cast<std::wstring>(m_PluginModuleClass["Title"])).data();
	Info->Author = (m_Author = py::cast<std::wstring>(m_PluginModuleClass["Author"])).data();
	Info->Description = (m_Description = py::cast<std::wstring>(m_PluginModuleClass["Description"])).data();
	Info->Guid = py::cast<UUID>(m_PluginModuleClass["Guid"]);
}

void module::GetOpenPanelInfoW(OpenPanelInfo* Info)
{
}

void module::GetPluginInfoW(PluginInfo* Info)
{
	auto PluginInfoType = far_api::type("PluginInfo"s);
	const auto PyInfo = call(STR(GetPluginInfoW));
	PyInfo.ensure_type(PluginInfoType);

	Info->Flags = py::cast<unsigned long long>(PyInfo["Flags"]);

	const auto& ConvertPluginMenuItem = [&](const char* Kind, menu_items& MenuItems, PluginMenuItem& Destination)
	{
		const auto ItemsList = py::cast<py::list>(PyInfo[Kind]);

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
			MenuItems.StringsData.emplace_back(py::cast<std::wstring>(Tuple[0]));
			MenuItems.Strings.emplace_back(MenuItems.StringsData.back().data());
			MenuItems.Uuids.emplace_back(py::cast<UUID>(Tuple[1]));
		}

		Destination.Strings = MenuItems.Strings.data();
		Destination.Guids = MenuItems.Uuids.data();
		Destination.Count = ListSize;
	};

	ConvertPluginMenuItem("PluginMenuItems", m_PluginMenuItems, Info->PluginMenu);
	ConvertPluginMenuItem("DiskMenuItems", m_DiskMenuItems, Info->DiskMenu);
	ConvertPluginMenuItem("PluginConfigItems", m_PluginConfigItems, Info->PluginConfig);

	m_CommandPrefix = py::cast<std::wstring>(PyInfo["CommandPrefix"]);
	Info->CommandPrefix = m_CommandPrefix.data();
}

intptr_t module::MakeDirectoryW(MakeDirectoryInfo* Info)
{
	return 0;
}

static py::list ConvertValues(const FarMacroValue* Value, size_t Size);

static py::object ConvertValue(const FarMacroValue* Value)
{
	auto FarMacroValueInstance = far_api::type("FarMacroValue")();

	const auto& Convert = [&]() -> py::object
	{
		switch (Value->Type)
		{
		case FMVT_UNKNOWN:
			return 0_py;

		case FMVT_INTEGER:
			return py::integer(Value->Integer);

		case FMVT_STRING:
			return py::string(Value->String);

		case FMVT_DOUBLE:
			return py::floating(Value->Double);

		case FMVT_BOOLEAN:
			return py::boolean(Value->Boolean != 0);

		case FMVT_BINARY:
			return py::bytes(Value->Binary.Data, Value->Binary.Size);

		case FMVT_POINTER:
			return py::integer(reinterpret_cast<uintptr_t>(Value->Pointer));

		case FMVT_NIL:
			return {};

		case FMVT_ARRAY:
			return ConvertValues(Value->Array.Values, Value->Array.Count);

		case FMVT_PANEL:
			return py::integer(reinterpret_cast<uintptr_t>(Value->Pointer));

		default:
			return {};
		}
	};

	FarMacroValueInstance["Type"] = far_api::type("FarMacroVarType")(py::integer(Value->Type));
	FarMacroValueInstance["Value"] = Convert();
	return FarMacroValueInstance;
}

static py::list ConvertValues(const FarMacroValue* Value, size_t Size)
{
	py::list List(0);
	for (auto Iterator = Value, End = Value + Size; Iterator != End; ++Iterator)
	{
		List.push_back(ConvertValue(Iterator));
	}
	return List;
}

HANDLE module::OpenW(const OpenInfo* Info)
{
	auto OpenInfoInstance = far_api::type("OpenInfo"s)();

	OpenInfoInstance["OpenFrom"] = far_api::type("OpenFrom"s)(py::integer(Info->OpenFrom));
	OpenInfoInstance["Guid"] = py::uuid(*Info->Guid);

	switch(Info->OpenFrom)
	{
	case OPEN_SHORTCUT:
		{
			const auto Data = reinterpret_cast<const OpenShortcutInfo*>(Info->Data);
			auto OpenShortcutInfoInstance = far_api::type("OpenShortcutInfo")();
			OpenShortcutInfoInstance["HostFile"] = py::string(Data->HostFile);
			OpenShortcutInfoInstance["ShortcutData"] = py::string(Data->ShortcutData);
			OpenShortcutInfoInstance["Flags"] = far_api::type("OpenShortcutFlags")(py::integer(Data->Flags));
			OpenInfoInstance["Data"] = OpenShortcutInfoInstance;
		}
		break;

	case OPEN_COMMANDLINE:
		{
			const auto Data = reinterpret_cast<const OpenCommandLineInfo*>(Info->Data);
			auto OpenCommandLineInfoInstance = far_api::type("OpenCommandLineInfo")();
			OpenCommandLineInfoInstance["CommandLine"] = py::string(Data->CommandLine);
			OpenInfoInstance["Data"] = OpenCommandLineInfoInstance;
		}
		break;

	case OPEN_DIALOG:
		// OpenDlgPluginData
		break;

	case OPEN_ANALYSE:
		{
			const auto Data = reinterpret_cast<const AnalyseInfo*>(Info->Data);
			auto AnalyseInfoInstance = far_api::type("AnalyseInfo")();
			AnalyseInfoInstance["FileName"] = py::string(Data->FileName);
			AnalyseInfoInstance["Buffer"] = py::bytes(Data->Buffer, Data->BufferSize);
			AnalyseInfoInstance["OpMode"] = far_api::type("OperationModes")(py::integer(Data->OpMode));
			OpenInfoInstance["Data"] = AnalyseInfoInstance;
		}
		break;

	case OPEN_FROMMACRO:
		{
			const auto Data = reinterpret_cast<const OpenMacroInfo*>(Info->Data);
			auto OpenMacroInfoInstance = far_api::type("OpenMacroInfo")();
			OpenMacroInfoInstance["Values"] = ConvertValues(Data->Values, Data->Count);
			OpenInfoInstance["Data"] = OpenMacroInfoInstance;
		}
		break;

	default:
		break;
	}

	const auto Result = call(STR(OpenW), OpenInfoInstance);

	// BUGBUG
	return nullptr;
}

intptr_t module::ProcessDialogEventW(const ProcessDialogEventInfo* Info)
{
	return 0;
}

intptr_t module::ProcessEditorEventW(const ProcessEditorEventInfo* Info)
{
	return 0;
}

intptr_t module::ProcessEditorInputW(const ProcessEditorInputInfo* Info)
{
	return 0;
}

intptr_t module::ProcessPanelEventW(const ProcessPanelEventInfo* Info)
{
	return 0;
}

intptr_t module::ProcessHostFileW(const ProcessHostFileInfo* Info)
{
	return 0;
}

intptr_t module::ProcessPanelInputW(const ProcessPanelInputInfo* Info)
{
	return 0;
}

intptr_t module::ProcessConsoleInputW(ProcessConsoleInputInfo* Info)
{
	return 0;
}

intptr_t module::ProcessSynchroEventW(const ProcessSynchroEventInfo* Info)
{
	return 0;
}

intptr_t module::ProcessViewerEventW(const ProcessViewerEventInfo* Info)
{
	return 0;
}

intptr_t module::PutFilesW(const PutFilesInfo* Info)
{
	return 0;
}

intptr_t module::SetDirectoryW(const SetDirectoryInfo* Info)
{
	return 0;
}

intptr_t module::SetFindListW(const SetFindListInfo* Info)
{
	return 0;
}

void module::SetStartupInfoW(const PluginStartupInfo* Info)
{
	far_api::initialise(Info);
	m_PluginModuleInstance = m_PluginModuleClass(far_api::module());
}

intptr_t module::GetContentFieldsW(const GetContentFieldsInfo* Info)
{
	return 0;
}

intptr_t module::GetContentDataW(GetContentDataInfo* Info)
{
	return 0;
}

void module::FreeContentDataW(const GetContentDataInfo* Info)
{
}
