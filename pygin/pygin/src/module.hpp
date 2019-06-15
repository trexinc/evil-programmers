#pragma once

/*
module.hpp

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

#include "py.object.hpp"

class module
{
public:
	NONCOPYABLE(module);

	explicit module(py::object Object);

	[[nodiscard]] bool check_function(const wchar_t* FunctionName) const;

	template<typename... args>
	[[nodiscard]] py::object call(const wchar_t* FunctionName, const args&... Args) const;

	HANDLE AnalyseW(const AnalyseInfo* Info);
	void CloseAnalyseW(const CloseAnalyseInfo* Info);
	void ClosePanelW(const ClosePanelInfo* Info);
	intptr_t CompareW(const CompareInfo* Info);
	intptr_t ConfigureW(const ConfigureInfo* Info);
	intptr_t DeleteFilesW(const DeleteFilesInfo* Info);
	void ExitFARW(const ExitInfo* Info);
	void FreeFindDataW(const FreeFindDataInfo* Info);
	intptr_t GetFilesW(GetFilesInfo* Info);
	intptr_t GetFindDataW(GetFindDataInfo* Info);
	void GetGlobalInfoW(GlobalInfo* Info);
	void GetOpenPanelInfoW(OpenPanelInfo* Info);
	void GetPluginInfoW(PluginInfo* Info);
	intptr_t MakeDirectoryW(MakeDirectoryInfo* Info);
	HANDLE OpenW(const OpenInfo* Info);
	intptr_t ProcessDialogEventW(const ProcessDialogEventInfo* Info);
	intptr_t ProcessEditorEventW(const ProcessEditorEventInfo* Info);
	intptr_t ProcessEditorInputW(const ProcessEditorInputInfo* Info);
	intptr_t ProcessPanelEventW(const ProcessPanelEventInfo* Info);
	intptr_t ProcessHostFileW(const ProcessHostFileInfo* Info);
	intptr_t ProcessPanelInputW(const ProcessPanelInputInfo* Info);
	intptr_t ProcessConsoleInputW(ProcessConsoleInputInfo* Info);
	intptr_t ProcessSynchroEventW(const ProcessSynchroEventInfo* Info);
	intptr_t ProcessViewerEventW(const ProcessViewerEventInfo* Info);
	intptr_t PutFilesW(const PutFilesInfo* Info);
	intptr_t SetDirectoryW(const SetDirectoryInfo* Info);
	intptr_t SetFindListW(const SetFindListInfo* Info);
	void SetStartupInfoW(const PluginStartupInfo* Info);
	intptr_t GetContentFieldsW(const GetContentFieldsInfo* Info);
	intptr_t GetContentDataW(GetContentDataInfo* Info);
	void FreeContentDataW(const GetContentDataInfo* Info);

private:
	py::object m_PluginModule;
	py::object m_PluginModuleClass;
	mutable std::unordered_map<std::wstring, py::object> m_PluginModuleClassFunctions;
	py::object m_PluginModuleInstance;

	std::wstring m_Title;
	std::wstring m_Author;
	std::wstring m_Description;

	struct menu_items
	{
		std::vector<std::wstring> StringsData;
		std::vector<const wchar_t*> Strings;
		std::vector<UUID> Uuids;
	};

	menu_items m_PluginMenuItems;
	menu_items m_DiskMenuItems;
	menu_items m_PluginConfigItems;

	std::wstring m_CommandPrefix;
};
