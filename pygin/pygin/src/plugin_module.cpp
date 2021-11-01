/*
plugin_module.cpp

*/
/*
Copyright 2017 Alex Alabuzhev
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"

#include "plugin_module.hpp"

#include "py.boolean.hpp"
#include "py.bytes.hpp"
#include "py.common.hpp"
#include "py.floating.hpp"
#include "py.integer.hpp"
#include "py.list.hpp"
#include "py.string.hpp"
#include "py.tuple.hpp"
#include "py.uuid.hpp"

#include "far_api.hpp"
#include "error_handling.hpp"
#include "helpers.hpp"
#include "panel.hpp"

using namespace py::literals;

auto EmptyToNull(const wchar_t* Str)
{
	return Str && *Str? Str : nullptr;
}

class context
{
public:
	context(py::object&& context)
		: m_Context(std::move(context))
	{
	}
public:
	py::object const& as_py() const { return this->m_Context; }

	open_panel_info_storage open_panel_info_storage;
private:
	py::object m_Context;
};

plugin_module::plugin_module(py::object Object):
	m_PluginModule(std::move(Object)),
	m_PluginModuleClass(m_PluginModule["FarPluginClass"sv])
{
}

bool plugin_module::check_function(const wchar_t* FunctionName) const
{
	// Mapped to class static fields
	if (FunctionName == L"GetGlobalInfoW"sv)
		return true;

	// Mapped to class ctor
	if (FunctionName == L"SetStartupInfoW"sv)
		return true;

	// always handle resources deallocation
	if (FunctionName == L"FreeFindDataW"sv)
		return true;

	// The rest is as is

	if (!m_PluginModuleClass.has_attribute(FunctionName))
		return false;

	const auto Function = m_PluginModuleClass[FunctionName];
	if (!py::callable_check(Function))
		return false;

	m_PluginModuleClassFunctions.emplace(FunctionName, Function);
	return true;
}

template<typename ... args>
py::object plugin_module::call(const wchar_t* FunctionName, const args&... Args) const
{
	if (!m_PluginModuleInstance)
		throw silent_exception{};

	return m_PluginModuleClassFunctions.at(FunctionName)(m_PluginModuleInstance, Args...);
}

HANDLE plugin_module::AnalyseW(const AnalyseInfo* Info)
{
	return nullptr;
}

void plugin_module::CloseAnalyseW(const CloseAnalyseInfo* Info)
{
}

void plugin_module::ClosePanelW(const ClosePanelInfo* Info)
{
	if (Info->StructSize >= sizeof(*Info))
		delete reinterpret_cast<context*>(Info->hPanel);
}

intptr_t plugin_module::CompareW(const CompareInfo* Info)
{
	return 0;
}

intptr_t plugin_module::ConfigureW(const ConfigureInfo* Info)
{
	auto ConfigureInstance = far_api::type("ConfigureInfo"sv)();

	ConfigureInstance["Guid"sv] = *Info->Guid;

	return py::cast<bool>(call(L"ConfigureW", ConfigureInstance));
}

intptr_t plugin_module::DeleteFilesW(const DeleteFilesInfo* Info)
{
	return 0;
}

void plugin_module::ExitFARW(const ExitInfo* Info)
{
	const auto ExitInfoInstance = far_api::type("ExitInfo"sv)();
	(void)call(L"ExitFARW", ExitInfoInstance);

	// Point of no return
	m_PluginModuleInstance = {};
	m_PluginModuleClassFunctions.clear();
	m_PluginModuleClass = {};
	m_PluginModule = {};
}

void plugin_module::FreeFindDataW(const FreeFindDataInfo* Info)
{
	if (Info->StructSize >= sizeof(*Info))
		get_find_data::free(Info->PanelItem);
}

intptr_t plugin_module::GetFilesW(GetFilesInfo* Info)
{
	if (Info->StructSize < sizeof(*Info))
		return 0;

	auto& Context = *reinterpret_cast<context*>(Info->hPanel);
	auto PanelItems = py::list(Info->ItemsNumber);
	for (int i = 0; i < Info->ItemsNumber; ++i)
	{
		auto const& Item = Info->PanelItem[i];
		auto* PyItem = static_cast<PyObject*>(Item.UserData.Data);
		PanelItems.set_at(i, py::object::from_borrowed(PyItem));
	}
	auto GetFileInfoCtor = far_api::type("GetFilesInfo"sv);
	auto PyOpMode = far_api::type("OperationModes"sv)(Info->OpMode);
	auto py_info = GetFileInfoCtor(
		Context.as_py(),
		PanelItems,
		Info->Move,
		Info->DestPath,
		PyOpMode
		);
	return py::cast<intptr_t>(call(L"GetFilesW", py_info));
}

intptr_t plugin_module::GetFindDataW(GetFindDataInfo* Info)
{
	if (Info->StructSize < sizeof(*Info))
		return 0;

	auto& Context = *reinterpret_cast<context*>(Info->hPanel);
	Info->PanelItem = get_find_data::get_items(
		call(L"GetFindDataW", Context.as_py()),
		Info->ItemsNumber,
		Info->OpMode);
	return 1;
}

void plugin_module::GetGlobalInfoW(GlobalInfo* Info)
{
	Info->Title = (m_Title = py::cast<std::wstring>(m_PluginModuleClass["Title"sv])).c_str();
	Info->Author = (m_Author = py::cast<std::wstring>(m_PluginModuleClass["Author"sv])).c_str();
	Info->Description = (m_Description = py::cast<std::wstring>(m_PluginModuleClass["Description"sv])).c_str();
	Info->Guid = py::cast<UUID>(m_PluginModuleClass["Guid"sv]);

	const auto Version = m_PluginModuleClass.get_attribute("Version"sv);

	// This won't work as far_api is not initialised yet
	// Consider 2-stage initialisation
	//Version.ensure_type(far_api::type("VersionInfo"sv));

	Info->Version.Major = py::cast<DWORD>(Version["Major"sv]);
	Info->Version.Minor = py::cast<DWORD>(Version["Minor"sv]);
	Info->Version.Revision = py::cast<DWORD>(Version["Revision"sv]);
	Info->Version.Build = py::cast<DWORD>(Version["Build"sv]);
	Info->Version.Stage = py::cast<VERSION_STAGE>(Version["Stage"sv]);
}

void plugin_module::GetOpenPanelInfoW(OpenPanelInfo* Info)
{
	auto& Context = *reinterpret_cast<context*>(Info->hPanel);

	auto OpenPanelInfoType = far_api::type("OpenPanelInfo"sv);
	const auto PyInfo = call(L"GetOpenPanelInfoW", Context.as_py());
	PyInfo.ensure_type(OpenPanelInfoType);

	Info->StructSize = sizeof(*Info);
	Info->Flags = py::cast<OPENPANELINFO_FLAGS>(PyInfo["Flags"sv]);
	auto& strings = Context.open_panel_info_storage.strings;
	Info->HostFile = push_back_if_not_none(PyInfo["HostFile"sv], strings);
	Info->CurDir = push_back_if_not_none(PyInfo["CurDir"sv], strings);
	Info->Format = push_back_if_not_none(PyInfo["Format"sv], strings);
	Info->PanelTitle = push_back_if_not_none(PyInfo["PanelTitle"sv], strings);
}

void plugin_module::GetPluginInfoW(PluginInfo* Info)
{
	auto PluginInfoType = far_api::type("PluginInfo"sv);
	const auto PyInfo = call(L"GetPluginInfoW");
	PyInfo.ensure_type(PluginInfoType);

	Info->Flags = py::cast<unsigned long long>(PyInfo["Flags"sv]);

	const auto& ConvertPluginMenuItem = [&](const char* Kind, menu_items& MenuItems, PluginMenuItem& Destination)
	{
		const auto Items = py::cast<py::list>(PyInfo[Kind]);

		const auto ItemsSize = Items.size();

		const auto prepare = [ItemsSize](auto& Container)
		{
			Container.clear();
			Container.reserve(ItemsSize);
		};

		prepare(MenuItems.Strings);
		prepare(MenuItems.Uuids);

		for (const auto& Item: Items)
		{
			const auto Tuple = py::cast<py::tuple>(Item);
			MenuItems.Strings.push_back_allow_none(py::cast<py::string>(Tuple[0]));
			MenuItems.Uuids.emplace_back(py::cast<UUID>(Tuple[1]));
		}

		Destination.Strings = MenuItems.Strings.data();
		Destination.Guids = MenuItems.Uuids.data();
		Destination.Count = ItemsSize;
	};

	ConvertPluginMenuItem("PluginMenuItems", m_PluginMenuItems, Info->PluginMenu);
	ConvertPluginMenuItem("DiskMenuItems", m_DiskMenuItems, Info->DiskMenu);
	ConvertPluginMenuItem("PluginConfigItems", m_PluginConfigItems, Info->PluginConfig);

	m_CommandPrefix = py::cast<std::wstring>(PyInfo["CommandPrefix"sv]);
	Info->CommandPrefix = m_CommandPrefix.c_str();
}

intptr_t plugin_module::MakeDirectoryW(MakeDirectoryInfo* Info)
{
	return 0;
}

static py::object ConvertValue(const FarMacroValue& Value)
{
	auto FarMacroValueInstance = far_api::type("FarMacroValue"sv)();

	const auto& Convert = [&]() -> py::object
	{
		switch (Value.Type)
		{
		case FMVT_UNKNOWN:
			return 0_py;

		case FMVT_INTEGER:
			return py::integer(Value.Integer);

		case FMVT_STRING:
			return py::string(Value.String);

		case FMVT_DOUBLE:
			return py::floating(Value.Double);

		case FMVT_BOOLEAN:
			return py::boolean(Value.Boolean != 0);

		case FMVT_BINARY:
			return py::bytes(Value.Binary.Data, Value.Binary.Size);

		case FMVT_POINTER:
			return py::integer(Value.Pointer);

		case FMVT_NIL:
			return {};

		case FMVT_ARRAY:
			return helpers::list::from_array(Value.Array.Values, Value.Array.Count, ConvertValue);

		case FMVT_PANEL:
			return py::integer(Value.Pointer);

		default:
			return {};
		}
	};

	FarMacroValueInstance["Type"sv] = far_api::type("FarMacroVarType"sv)(Value.Type);
	FarMacroValueInstance["Value"sv] = Convert();
	return FarMacroValueInstance;
}

HANDLE plugin_module::OpenW(const OpenInfo* Info)
{
	auto OpenInfoInstance = far_api::type("OpenInfo"sv)();

	OpenInfoInstance["OpenFrom"sv] = far_api::type("OpenFrom"sv)(Info->OpenFrom);
	OpenInfoInstance["Guid"sv] = *Info->Guid;

	switch(Info->OpenFrom)
	{
	case OPEN_SHORTCUT:
		{
			const auto Data = reinterpret_cast<const OpenShortcutInfo*>(Info->Data);
			auto OpenShortcutInfoInstance = far_api::type("OpenShortcutInfo"sv)();
			OpenShortcutInfoInstance["HostFile"sv] = Data->HostFile;
			OpenShortcutInfoInstance["ShortcutData"sv] = Data->ShortcutData;
			OpenShortcutInfoInstance["Flags"sv] = far_api::type("OpenShortcutFlags"sv)(Data->Flags);
			OpenInfoInstance["Data"sv] = OpenShortcutInfoInstance;
		}
		break;

	case OPEN_COMMANDLINE:
		{
			const auto Data = reinterpret_cast<const OpenCommandLineInfo*>(Info->Data);
			auto OpenCommandLineInfoInstance = far_api::type("OpenCommandLineInfo"sv)();
			OpenCommandLineInfoInstance["CommandLine"sv] = Data->CommandLine;
			OpenInfoInstance["Data"sv] = OpenCommandLineInfoInstance;
		}
		break;

	case OPEN_DIALOG:
		{
			const auto Data = reinterpret_cast<const OpenDlgPluginData*>(Info->Data);
			auto OpenDlgPluginDataInstance = far_api::type("OpenDlgPluginData"sv)();
			OpenDlgPluginDataInstance["Dialog"sv] = py::integer(Data->hDlg);
			OpenInfoInstance["Data"sv] = OpenDlgPluginDataInstance;
		}
		break;

	case OPEN_ANALYSE:
		{
			const auto Data = reinterpret_cast<const AnalyseInfo*>(Info->Data);
			auto AnalyseInfoInstance = far_api::type("AnalyseInfo"sv)();
			AnalyseInfoInstance["FileName"sv] = Data->FileName;
			AnalyseInfoInstance["Buffer"sv] = py::bytes(Data->Buffer, Data->BufferSize);
			AnalyseInfoInstance["OpMode"sv] = far_api::type("OperationModes"sv)(Data->OpMode);
			OpenInfoInstance["Data"sv] = AnalyseInfoInstance;
		}
		break;

	case OPEN_FROMMACRO:
		{
			const auto Data = reinterpret_cast<const OpenMacroInfo*>(Info->Data);
			auto OpenMacroInfoInstance = far_api::type("OpenMacroInfo"sv)();
			OpenMacroInfoInstance["Values"sv] = helpers::list::from_array(Data->Values, Data->Count, ConvertValue);
			OpenInfoInstance["Data"sv] = OpenMacroInfoInstance;
		}
		break;

	default:
		break;
	}

	auto Result = call(L"OpenW", OpenInfoInstance);
	return Result ? new context(std::move(Result)) : nullptr;
}

intptr_t plugin_module::ProcessDialogEventW(const ProcessDialogEventInfo* Info)
{
	return 0;
}

intptr_t plugin_module::ProcessEditorEventW(const ProcessEditorEventInfo* Info)
{
	return 0;
}

intptr_t plugin_module::ProcessEditorInputW(const ProcessEditorInputInfo* Info)
{
	return 0;
}

intptr_t plugin_module::ProcessPanelEventW(const ProcessPanelEventInfo* Info)
{
	return 0;
}

intptr_t plugin_module::ProcessHostFileW(const ProcessHostFileInfo* Info)
{
	return 0;
}

intptr_t plugin_module::ProcessPanelInputW(const ProcessPanelInputInfo* Info)
{
	return 0;
}

intptr_t plugin_module::ProcessConsoleInputW(ProcessConsoleInputInfo* Info)
{
	return 0;
}

intptr_t plugin_module::ProcessSynchroEventW(const ProcessSynchroEventInfo* Info)
{
	return 0;
}

intptr_t plugin_module::ProcessViewerEventW(const ProcessViewerEventInfo* Info)
{
	return 0;
}

intptr_t plugin_module::PutFilesW(const PutFilesInfo* Info)
{
	return 0;
}

intptr_t plugin_module::SetDirectoryW(const SetDirectoryInfo* Info)
{
	if (Info->StructSize < sizeof(*Info))
		return 0;

	auto& Context = *reinterpret_cast<context*>(Info->hPanel);

	auto py_info = far_api::type("SetDirectoryInfo"sv)();
	py_info["Panel"sv] = Context.as_py();
	py_info["Dir"sv] = Info->Dir;

	return py::cast<bool>(call(L"SetDirectoryW", py_info));
}

intptr_t plugin_module::SetFindListW(const SetFindListInfo* Info)
{
	return 0;
}

void plugin_module::SetStartupInfoW(const PluginStartupInfo* Info)
{
	far_api::initialise(Info);
	m_PluginModuleInstance = m_PluginModuleClass();
}

intptr_t plugin_module::GetContentFieldsW(const GetContentFieldsInfo* Info)
{
	return 0;
}

intptr_t plugin_module::GetContentDataW(GetContentDataInfo* Info)
{
	return 0;
}

void plugin_module::FreeContentDataW(const GetContentDataInfo* Info)
{
}
