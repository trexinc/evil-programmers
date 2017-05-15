#pragma once

#include "py_object.hpp"

class module
{
public:
	NONCOPYABLE(module);

	module(const py::object& Object);
	~module();

	bool check_function(const wchar_t* FunctionName) const;

	template<typename... args>
	py::object call(const char* FunctionName, const args&... Args) const;

	HANDLE AnalyseW(const AnalyseInfo *Info);
	void CloseAnalyseW(const CloseAnalyseInfo *Info);
	void ClosePanelW(const ClosePanelInfo *Info);
	intptr_t CompareW(const CompareInfo *Info);
	intptr_t ConfigureW(const ConfigureInfo *Info);
	intptr_t DeleteFilesW(const DeleteFilesInfo *Info);
	void ExitFARW(const ExitInfo *Info);
	void FreeFindDataW(const FreeFindDataInfo *Info);
	intptr_t GetFilesW(GetFilesInfo *Info);
	intptr_t GetFindDataW(GetFindDataInfo *Info);
	void GetGlobalInfoW(GlobalInfo *Info);
	void GetOpenPanelInfoW(OpenPanelInfo *Info);
	void GetPluginInfoW(PluginInfo *Info);
	intptr_t MakeDirectoryW(MakeDirectoryInfo *Info);
	HANDLE OpenW(const OpenInfo *Info);
	intptr_t ProcessDialogEventW(const ProcessDialogEventInfo *Info);
	intptr_t ProcessEditorEventW(const ProcessEditorEventInfo *Info);
	intptr_t ProcessEditorInputW(const ProcessEditorInputInfo *Info);
	intptr_t ProcessPanelEventW(const ProcessPanelEventInfo *Info);
	intptr_t ProcessHostFileW(const ProcessHostFileInfo *Info);
	intptr_t ProcessPanelInputW(const ProcessPanelInputInfo *Info);
	intptr_t ProcessConsoleInputW(ProcessConsoleInputInfo *Info);
	intptr_t ProcessSynchroEventW(const ProcessSynchroEventInfo *Info);
	intptr_t ProcessViewerEventW(const ProcessViewerEventInfo *Info);
	intptr_t PutFilesW(const PutFilesInfo *Info);
	intptr_t SetDirectoryW(const SetDirectoryInfo *Info);
	intptr_t SetFindListW(const SetFindListInfo *Info);
	void SetStartupInfoW(const PluginStartupInfo *Info);
	intptr_t GetContentFieldsW(const GetContentFieldsInfo *Info);
	intptr_t GetContentDataW(GetContentDataInfo *Info);
	void FreeContentDataW(const GetContentDataInfo *Info);

private:
	py::object m_PluginModule;
	py::object m_PluginModuleClass;
	mutable std::unordered_map<std::string, py::object> m_PluginModuleClassFunctions;
	py::object m_PluginModuleInstance;

	std::wstring m_Title;
	std::wstring m_Author;
	std::wstring m_Description;
	UUID m_Uuid;

	struct menu_items
	{
		std::vector<std::wstring> StringsData;
		std::vector<const wchar_t*> Strings;
		std::vector<UUID> Uuids;
	};

	menu_items m_PluginMenuItems;
	menu_items m_DiskMenuItems;
	menu_items m_PluginConfigItems;
};
