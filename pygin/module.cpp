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

bool module::CheckFunction(const wchar_t* FunctionName) const
{
	const auto Func = m_Object.get_attribute(py::string(FunctionName));
	py::err::clear();
	return (Func && py::callable_check(Func))
		// SetStartupInfoW is useless for Python script itself, but crucial for this wrapper
		|| !wcscmp(FunctionName, L"SetStartupInfoW");
}

py::object module::CallFunction(const char* FunctionName, const py::object& InfoArg) const
{
	const auto Func = m_Object.get_attribute(py::string(FunctionName));
	if (Func && py::callable_check(Func))
	{
		return py::object(Func.call(py::tuple::make(InfoArg)));
	}
	return nullptr;
}


HANDLE module::AnalyseW(const AnalyseInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return nullptr;
	},
	[]
	{
		return nullptr;
	});
}

void module::CloseAnalyseW(const CloseAnalyseInfo *Info) noexcept
{
	return try_call(
	[]
	{
	},
	[]
	{
	});
}

void module::ClosePanelW(const ClosePanelInfo *Info) noexcept
{
	return try_call(
	[]
	{
	},
	[]
	{
	});
}

intptr_t module::CompareW(const CompareInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

intptr_t module::ConfigureW(const ConfigureInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

intptr_t module::DeleteFilesW(const DeleteFilesInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

void module::ExitFARW(const ExitInfo *Info) noexcept
{
	return try_call(
	[]
	{
	},
	[]
	{
	});
}

void module::FreeFindDataW(const FreeFindDataInfo *Info) noexcept
{
	return try_call(
	[]
	{
	},
	[]
	{
	});
}

intptr_t module::GetFilesW(GetFilesInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

intptr_t module::GetFindDataW(GetFindDataInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

void module::GetGlobalInfoW(GlobalInfo *Info) noexcept
{
	return try_call(
	[&]
	{
		py::dictionary pyInfo;
		pyInfo["Title"] = "";
		pyInfo["Author"] = "";
		pyInfo["Description"] = "";
		pyInfo["Guid"] = "";

		CallFunction("GetGlobalInfoW", pyInfo);

		Info->Title = (m_Title = py::string::to_wstring(pyInfo["Title"])).data();
		Info->Author = (m_Author = py::string::to_wstring(pyInfo["Author"])).data();
		Info->Description = (m_Description = py::string::to_wstring(pyInfo["Description"])).data();
		Info->Guid = m_Uuid = UuidFromString(py::string::to_wstring(pyInfo["Guid"]));
	},
	[]
	{
	});
}

void module::GetOpenPanelInfoW(OpenPanelInfo *Info) noexcept
{
	return try_call(
	[]
	{
	},
	[]
	{
	});
}

void module::GetPluginInfoW(PluginInfo *Info) noexcept
{
	return try_call(
	[&]
	{
		py::dictionary pyInfo;
		pyInfo["MenuString"] = "";
		pyInfo["Guid"] = "";

		CallFunction("GetPluginInfoW", pyInfo);

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
	},
	[]
	{
	});
}

intptr_t module::MakeDirectoryW(MakeDirectoryInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

HANDLE module::OpenW(const OpenInfo *Info) noexcept
{
	return try_call(
	[&]
	{
		// BUGBUG
		m_Psi.PanelControl(PANEL_NONE, FCTL_GETUSERSCREEN, 0, nullptr);

		py::dictionary pyInfo;
		CallFunction("OpenW", pyInfo);

		// BUGBUG
		m_Psi.PanelControl(PANEL_NONE, FCTL_SETUSERSCREEN, 0, nullptr);

		return nullptr;
	},
	[]
	{
		return nullptr;
	});
}

intptr_t module::ProcessDialogEventW(const ProcessDialogEventInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

intptr_t module::ProcessEditorEventW(const ProcessEditorEventInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

intptr_t module::ProcessEditorInputW(const ProcessEditorInputInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

intptr_t module::ProcessPanelEventW(const ProcessPanelEventInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

intptr_t module::ProcessHostFileW(const ProcessHostFileInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

intptr_t module::ProcessPanelInputW(const ProcessPanelInputInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

intptr_t module::ProcessConsoleInputW(ProcessConsoleInputInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

intptr_t module::ProcessSynchroEventW(const ProcessSynchroEventInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

intptr_t module::ProcessViewerEventW(const ProcessViewerEventInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

intptr_t module::PutFilesW(const PutFilesInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

intptr_t module::SetDirectoryW(const SetDirectoryInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

intptr_t module::SetFindListW(const SetFindListInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

void module::SetStartupInfoW(const PluginStartupInfo *Info) noexcept
{
	return try_call(
	[&]
	{
		m_Psi = *Info;
		m_Fsf = *Info->FSF;
		m_Psi.FSF = &m_Fsf;
	},
	[]
	{
	});
}

intptr_t module::GetContentFieldsW(const GetContentFieldsInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

intptr_t module::GetContentDataW(GetContentDataInfo *Info) noexcept
{
	return try_call(
	[]
	{
		return 0;
	},
	[]
	{
		return 0;
	});
}

void module::FreeContentDataW(const GetContentDataInfo *Info) noexcept
{
	return try_call(
	[]
	{
	},
	[]
	{
	});
}

//-----------------------------------------------------------------------------

HANDLE WINAPI AnalyseW(const AnalyseInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->AnalyseW(Info);
}

void WINAPI CloseAnalyseW(const CloseAnalyseInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->CloseAnalyseW(Info);
}

void WINAPI ClosePanelW(const ClosePanelInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->ClosePanelW(Info);
}

intptr_t WINAPI CompareW(const CompareInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->CompareW(Info);
}

intptr_t WINAPI ConfigureW(const ConfigureInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->ConfigureW(Info);
}

intptr_t WINAPI DeleteFilesW(const DeleteFilesInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->DeleteFilesW(Info);
}

void WINAPI ExitFARW(const ExitInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->ExitFARW(Info);
}

void WINAPI FreeFindDataW(const FreeFindDataInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->FreeFindDataW(Info);
}

intptr_t WINAPI GetFilesW(GetFilesInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->GetFilesW(Info);
}

intptr_t WINAPI GetFindDataW(GetFindDataInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->GetFindDataW(Info);
}

void WINAPI GetGlobalInfoW(GlobalInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->GetGlobalInfoW(Info);
}

void WINAPI GetOpenPanelInfoW(OpenPanelInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->GetOpenPanelInfoW(Info);
}

void WINAPI GetPluginInfoW(PluginInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->GetPluginInfoW(Info);
}

intptr_t WINAPI MakeDirectoryW(MakeDirectoryInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->MakeDirectoryW(Info);
}

HANDLE WINAPI OpenW(const OpenInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->OpenW(Info);
}

intptr_t WINAPI ProcessDialogEventW(const ProcessDialogEventInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->ProcessDialogEventW(Info);
}

intptr_t WINAPI ProcessEditorEventW(const ProcessEditorEventInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->ProcessEditorEventW(Info);
}

intptr_t WINAPI ProcessEditorInputW(const ProcessEditorInputInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->ProcessEditorInputW(Info);
}

intptr_t WINAPI ProcessPanelEventW(const ProcessPanelEventInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->ProcessPanelEventW(Info);
}

intptr_t WINAPI ProcessHostFileW(const ProcessHostFileInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->ProcessHostFileW(Info);
}

intptr_t WINAPI ProcessPanelInputW(const ProcessPanelInputInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->ProcessPanelInputW(Info);
}

intptr_t WINAPI ProcessConsoleInputW(ProcessConsoleInputInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->ProcessConsoleInputW(Info);
}

intptr_t WINAPI ProcessSynchroEventW(const ProcessSynchroEventInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->ProcessSynchroEventW(Info);
}

intptr_t WINAPI ProcessViewerEventW(const ProcessViewerEventInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->ProcessViewerEventW(Info);
}

intptr_t WINAPI PutFilesW(const PutFilesInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->PutFilesW(Info);
}

intptr_t WINAPI SetDirectoryW(const SetDirectoryInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->SetDirectoryW(Info);
}

intptr_t WINAPI SetFindListW(const SetFindListInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->SetFindListW(Info);
}

void WINAPI SetStartupInfoW(const PluginStartupInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->SetStartupInfoW(Info);
}

intptr_t WINAPI GetContentFieldsW(const GetContentFieldsInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->GetContentFieldsW(Info);
}

intptr_t WINAPI GetContentDataW(GetContentDataInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->GetContentDataW(Info);
}

void WINAPI FreeContentDataW(const GetContentDataInfo *Info) noexcept
{
	return static_cast<module*>(Info->Instance)->FreeContentDataW(Info);
}
