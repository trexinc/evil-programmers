#pragma once
class module
{
public:
	module(PyObject* Object);
	~module();

	bool CheckFunction(const wchar_t* FunctionName);

	HANDLE AnalyseW(const struct AnalyseInfo *Info);
	void CloseAnalyseW(const struct CloseAnalyseInfo *Info);
	void ClosePanelW(const struct ClosePanelInfo *Info);
	intptr_t CompareW(const struct CompareInfo *Info);
	intptr_t ConfigureW(const struct ConfigureInfo *Info);;
	intptr_t DeleteFilesW(const struct DeleteFilesInfo *Info);
	void ExitFARW(const struct ExitInfo *Info);
	void FreeFindDataW(const struct FreeFindDataInfo *Info);
	intptr_t GetFilesW(struct GetFilesInfo *Info);
	intptr_t GetFindDataW(struct GetFindDataInfo *Info);
	void GetGlobalInfoW(struct GlobalInfo *Info);
	void GetOpenPanelInfoW(struct OpenPanelInfo *Info);
	void GetPluginInfoW(struct PluginInfo *Info);
	intptr_t MakeDirectoryW(struct MakeDirectoryInfo *Info);
	HANDLE OpenW(const struct OpenInfo *Info);
	intptr_t ProcessDialogEventW(const struct ProcessDialogEventInfo *Info);
	intptr_t ProcessEditorEventW(const struct ProcessEditorEventInfo *Info);
	intptr_t ProcessEditorInputW(const struct ProcessEditorInputInfo *Info);
	intptr_t ProcessPanelEventW(const struct ProcessPanelEventInfo *Info);
	intptr_t ProcessHostFileW(const struct ProcessHostFileInfo *Info);
	intptr_t ProcessPanelInputW(const struct ProcessPanelInputInfo *Info);
	intptr_t ProcessConsoleInputW(struct ProcessConsoleInputInfo *Info);
	intptr_t ProcessSynchroEventW(const struct ProcessSynchroEventInfo *Info);
	intptr_t ProcessViewerEventW(const struct ProcessViewerEventInfo *Info);
	intptr_t PutFilesW(const struct PutFilesInfo *Info);
	intptr_t SetDirectoryW(const struct SetDirectoryInfo *Info);
	intptr_t SetFindListW(const struct SetFindListInfo *Info);
	void SetStartupInfoW(const struct PluginStartupInfo *Info);
	intptr_t GetContentFieldsW(const struct GetContentFieldsInfo *Info);
	intptr_t GetContentDataW(struct GetContentDataInfo *Info);
	void FreeContentDataW(const struct GetContentDataInfo *Info);

private:
	pyobject_ptr m_Object;

	std::wstring m_Title;
	std::wstring m_Author;
	std::wstring m_Description;
	UUID m_Uuid;

	std::vector<std::wstring> m_MenuStringsData;
	std::vector<const wchar_t*> m_MenuStrings;
	std::vector<UUID> m_MEnuUuids;

	PluginStartupInfo m_Psi;
	FarStandardFunctions m_Fsf;
};

