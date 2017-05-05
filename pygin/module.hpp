#pragma once

#include "py_object.hpp"

class module
{
public:
	module(const py::object& Object);
	~module();

	bool CheckFunction(const wchar_t* FunctionName) const;
	py::object CallFunction(const char* FunctionName, const py::object& InfoArg) const;

	template<typename callable, typename fallback>
	auto try_call(const callable& Callable, const fallback& Fallback) noexcept
	{
		try
		{
			return Callable();
		}
		catch (...)
		{
			try
			{
				return Fallback();
			}
			catch (...)
			{
			}
		}
	}

	HANDLE AnalyseW(const AnalyseInfo *Info) noexcept;
	void CloseAnalyseW(const CloseAnalyseInfo *Info) noexcept;
	void ClosePanelW(const ClosePanelInfo *Info) noexcept;
	intptr_t CompareW(const CompareInfo *Info) noexcept;
	intptr_t ConfigureW(const ConfigureInfo *Info) noexcept;
	intptr_t DeleteFilesW(const DeleteFilesInfo *Info) noexcept;
	void ExitFARW(const ExitInfo *Info) noexcept;
	void FreeFindDataW(const FreeFindDataInfo *Info) noexcept;
	intptr_t GetFilesW(GetFilesInfo *Info) noexcept;
	intptr_t GetFindDataW(GetFindDataInfo *Info) noexcept;
	void GetGlobalInfoW(GlobalInfo *Info) noexcept;
	void GetOpenPanelInfoW(OpenPanelInfo *Info) noexcept;
	void GetPluginInfoW(PluginInfo *Info) noexcept;
	intptr_t MakeDirectoryW(MakeDirectoryInfo *Info) noexcept;
	HANDLE OpenW(const OpenInfo *Info) noexcept;
	intptr_t ProcessDialogEventW(const ProcessDialogEventInfo *Info) noexcept;
	intptr_t ProcessEditorEventW(const ProcessEditorEventInfo *Info) noexcept;
	intptr_t ProcessEditorInputW(const ProcessEditorInputInfo *Info) noexcept;
	intptr_t ProcessPanelEventW(const ProcessPanelEventInfo *Info) noexcept;
	intptr_t ProcessHostFileW(const ProcessHostFileInfo *Info) noexcept;
	intptr_t ProcessPanelInputW(const ProcessPanelInputInfo *Info) noexcept;
	intptr_t ProcessConsoleInputW(ProcessConsoleInputInfo *Info) noexcept;
	intptr_t ProcessSynchroEventW(const ProcessSynchroEventInfo *Info) noexcept;
	intptr_t ProcessViewerEventW(const ProcessViewerEventInfo *Info) noexcept;
	intptr_t PutFilesW(const PutFilesInfo *Info) noexcept;
	intptr_t SetDirectoryW(const SetDirectoryInfo *Info) noexcept;
	intptr_t SetFindListW(const SetFindListInfo *Info) noexcept;
	void SetStartupInfoW(const PluginStartupInfo *Info) noexcept;
	intptr_t GetContentFieldsW(const GetContentFieldsInfo *Info) noexcept;
	intptr_t GetContentDataW(GetContentDataInfo *Info) noexcept;
	void FreeContentDataW(const GetContentDataInfo *Info) noexcept;

private:
	py::object m_Object;

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

