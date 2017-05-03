#include "headers.hpp"
#include "module.hpp"

static UUID UuidFromString(const std::wstring& Str)
{
	UUID Result;
	UuidFromString(reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(Str.data())), &Result);
	return Result;
}


module::module(PyObject* Object):
	m_Object(Object)
{
}

module::~module()
{
}

bool module::CheckFunction(const wchar_t* FunctionName)
{
	const pyobject_ptr Func = PyObject_GetAttr(m_Object.get(), py_str(FunctionName).get());
	PyErr_Clear();
	return (Func && PyCallable_Check(Func.get()))
		// SetStartupInfoW is useless for Python script itself, but crucial for this wrapper
		|| !wcscmp(FunctionName, L"SetStartupInfoW");
}

HANDLE module::AnalyseW(const struct AnalyseInfo *Info)
{
	return nullptr;
}

void module::CloseAnalyseW(const struct CloseAnalyseInfo *Info)
{

}

void module::ClosePanelW(const struct ClosePanelInfo *Info)
{

}

intptr_t module::CompareW(const struct CompareInfo *Info)
{
	return 0;
}

intptr_t module::ConfigureW(const struct ConfigureInfo *Info)
{
	return 0;
}

intptr_t module::DeleteFilesW(const struct DeleteFilesInfo *Info)
{
	return 0;
}

void module::ExitFARW(const struct ExitInfo *Info)
{

}

void module::FreeFindDataW(const struct FreeFindDataInfo *Info)
{

}

intptr_t module::GetFilesW(struct GetFilesInfo *Info)
{
	return 0;
}

intptr_t module::GetFindDataW(struct GetFindDataInfo *Info)
{
	return 0;
}

void module::GetGlobalInfoW(struct GlobalInfo *Info)
{
	py_dict pyInfo;
	pyInfo["Title"] = "";
	pyInfo["Author"] = "";
	pyInfo["Description"] = "";
	pyInfo["Guid"] = "";

	Call(m_Object, "GetGlobalInfoW", pyInfo);

	Info->Title = (m_Title = to_wstring(pyInfo["Title"])).data();
	Info->Author = (m_Author = to_wstring(pyInfo["Author"])).data();
	Info->Description = (m_Description = to_wstring(pyInfo["Description"])).data();
	Info->Guid = m_Uuid = UuidFromString(to_wstring(pyInfo["Guid"]));
}

void module::GetOpenPanelInfoW(struct OpenPanelInfo *Info)
{

}

void module::GetPluginInfoW(struct PluginInfo *Info)
{
	py_dict pyInfo;
	pyInfo["MenuString"] = "";
	pyInfo["Guid"] = "";

	Call(m_Object, "GetPluginInfoW", pyInfo);

	const size_t Size = 1;
	m_MenuStringsData.resize(Size);
	m_MenuStrings.resize(Size);
	m_MEnuUuids.resize(Size);

	m_MenuStringsData[0] = to_wstring(pyInfo["MenuString"]);
	m_MenuStrings[0] = m_MenuStringsData[0].data();
	m_MEnuUuids[0] = UuidFromString(to_wstring(pyInfo["Guid"]));

	Info->PluginMenu.Strings = m_MenuStrings.data();
	Info->PluginMenu.Guids = m_MEnuUuids.data();
	Info->PluginMenu.Count = Size;

	Info->Flags |= PF_PRELOAD;
}

intptr_t module::MakeDirectoryW(struct MakeDirectoryInfo *Info)
{
	return 0;
}

HANDLE module::OpenW(const struct OpenInfo *Info)
{
	// BUGBUG
	m_Psi.PanelControl(PANEL_NONE, FCTL_GETUSERSCREEN, 0, nullptr);

	py_dict pyInfo;
	Call(m_Object, "OpenW", pyInfo);

	// BUGBUG
	m_Psi.PanelControl(PANEL_NONE, FCTL_SETUSERSCREEN, 0, nullptr);

	return nullptr;
}

intptr_t module::ProcessDialogEventW(const struct ProcessDialogEventInfo *Info)
{
	return 0;
}

intptr_t module::ProcessEditorEventW(const struct ProcessEditorEventInfo *Info)
{
	return 0;
}

intptr_t module::ProcessEditorInputW(const struct ProcessEditorInputInfo *Info)
{
	return 0;
}

intptr_t module::ProcessPanelEventW(const struct ProcessPanelEventInfo *Info)
{
	return 0;
}

intptr_t module::ProcessHostFileW(const struct ProcessHostFileInfo *Info)
{
	return 0;
}

intptr_t module::ProcessPanelInputW(const struct ProcessPanelInputInfo *Info)
{
	return 0;
}

intptr_t module::ProcessConsoleInputW(struct ProcessConsoleInputInfo *Info)
{
	return 0;
}

intptr_t module::ProcessSynchroEventW(const struct ProcessSynchroEventInfo *Info)
{
	return 0;
}

intptr_t module::ProcessViewerEventW(const struct ProcessViewerEventInfo *Info)
{
	return 0;
}

intptr_t module::PutFilesW(const struct PutFilesInfo *Info)
{
	return 0;
}

intptr_t module::SetDirectoryW(const struct SetDirectoryInfo *Info)
{
	return 0;
}

intptr_t module::SetFindListW(const struct SetFindListInfo *Info)
{
	return 0;
}

void module::SetStartupInfoW(const struct PluginStartupInfo *Info)
{
	m_Psi = *Info;
	m_Fsf = *Info->FSF;
	m_Psi.FSF = &m_Fsf;
}

intptr_t module::GetContentFieldsW(const struct GetContentFieldsInfo *Info)
{
	return 0;
}

intptr_t module::GetContentDataW(struct GetContentDataInfo *Info)
{
	return 0;
}

void module::FreeContentDataW(const struct GetContentDataInfo *Info)
{

}

//-----------------------------------------------------------------------------

HANDLE WINAPI AnalyseW(const struct AnalyseInfo *Info)
{
	return static_cast<module*>(Info->Instance)->AnalyseW(Info);
}

void WINAPI CloseAnalyseW(const struct CloseAnalyseInfo *Info)
{
	return static_cast<module*>(Info->Instance)->CloseAnalyseW(Info);
}

void WINAPI ClosePanelW(const struct ClosePanelInfo *Info)
{
	return static_cast<module*>(Info->Instance)->ClosePanelW(Info);
}

intptr_t WINAPI CompareW(const struct CompareInfo *Info)
{
	return static_cast<module*>(Info->Instance)->CompareW(Info);
}

intptr_t WINAPI ConfigureW(const struct ConfigureInfo *Info)
{
	return static_cast<module*>(Info->Instance)->ConfigureW(Info);
}

intptr_t WINAPI DeleteFilesW(const struct DeleteFilesInfo *Info)
{
	return static_cast<module*>(Info->Instance)->DeleteFilesW(Info);
}

void WINAPI ExitFARW(const struct ExitInfo *Info)
{
	return static_cast<module*>(Info->Instance)->ExitFARW(Info);
}

void WINAPI FreeFindDataW(const struct FreeFindDataInfo *Info)
{
	return static_cast<module*>(Info->Instance)->FreeFindDataW(Info);
}

intptr_t WINAPI GetFilesW(struct GetFilesInfo *Info)
{
	return static_cast<module*>(Info->Instance)->GetFilesW(Info);
}

intptr_t WINAPI GetFindDataW(struct GetFindDataInfo *Info)
{
	return static_cast<module*>(Info->Instance)->GetFindDataW(Info);
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	return static_cast<module*>(Info->Instance)->GetGlobalInfoW(Info);
}

void WINAPI GetOpenPanelInfoW(struct OpenPanelInfo *Info)
{
	return static_cast<module*>(Info->Instance)->GetOpenPanelInfoW(Info);
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	return static_cast<module*>(Info->Instance)->GetPluginInfoW(Info);
}

intptr_t WINAPI MakeDirectoryW(struct MakeDirectoryInfo *Info)
{
	return static_cast<module*>(Info->Instance)->MakeDirectoryW(Info);
}

HANDLE WINAPI OpenW(const struct OpenInfo *Info)
{
	return static_cast<module*>(Info->Instance)->OpenW(Info);
}

intptr_t WINAPI ProcessDialogEventW(const struct ProcessDialogEventInfo *Info)
{
	return static_cast<module*>(Info->Instance)->ProcessDialogEventW(Info);
}

intptr_t WINAPI ProcessEditorEventW(const struct ProcessEditorEventInfo *Info)
{
	return static_cast<module*>(Info->Instance)->ProcessEditorEventW(Info);
}

intptr_t WINAPI ProcessEditorInputW(const struct ProcessEditorInputInfo *Info)
{
	return static_cast<module*>(Info->Instance)->ProcessEditorInputW(Info);
}

intptr_t WINAPI ProcessPanelEventW(const struct ProcessPanelEventInfo *Info)
{
	return static_cast<module*>(Info->Instance)->ProcessPanelEventW(Info);
}

intptr_t WINAPI ProcessHostFileW(const struct ProcessHostFileInfo *Info)
{
	return static_cast<module*>(Info->Instance)->ProcessHostFileW(Info);
}

intptr_t WINAPI ProcessPanelInputW(const struct ProcessPanelInputInfo *Info)
{
	return static_cast<module*>(Info->Instance)->ProcessPanelInputW(Info);
}

intptr_t WINAPI ProcessConsoleInputW(struct ProcessConsoleInputInfo *Info)
{
	return static_cast<module*>(Info->Instance)->ProcessConsoleInputW(Info);
}

intptr_t WINAPI ProcessSynchroEventW(const struct ProcessSynchroEventInfo *Info)
{
	return static_cast<module*>(Info->Instance)->ProcessSynchroEventW(Info);
}

intptr_t WINAPI ProcessViewerEventW(const struct ProcessViewerEventInfo *Info)
{
	return static_cast<module*>(Info->Instance)->ProcessViewerEventW(Info);
}

intptr_t WINAPI PutFilesW(const struct PutFilesInfo *Info)
{
	return static_cast<module*>(Info->Instance)->PutFilesW(Info);
}

intptr_t WINAPI SetDirectoryW(const struct SetDirectoryInfo *Info)
{
	return static_cast<module*>(Info->Instance)->SetDirectoryW(Info);
}

intptr_t WINAPI SetFindListW(const struct SetFindListInfo *Info)
{
	return static_cast<module*>(Info->Instance)->SetFindListW(Info);
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
	return static_cast<module*>(Info->Instance)->SetStartupInfoW(Info);
}

intptr_t WINAPI GetContentFieldsW(const struct GetContentFieldsInfo *Info)
{
	return static_cast<module*>(Info->Instance)->GetContentFieldsW(Info);
}

intptr_t WINAPI GetContentDataW(struct GetContentDataInfo *Info)
{
	return static_cast<module*>(Info->Instance)->GetContentDataW(Info);
}

void WINAPI FreeContentDataW(const struct GetContentDataInfo *Info)
{
	return static_cast<module*>(Info->Instance)->FreeContentDataW(Info);
}
