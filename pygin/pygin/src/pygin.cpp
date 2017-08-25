/*
pygin.cpp

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

#include "pygin.hpp"

#include "py.dictionary.hpp"
#include "py.import.hpp"
#include "py.list.hpp"
#include "py.string.hpp"
#include "py.sys.hpp"

#include "error_handling.hpp"
#include "far_api.hpp"
#include "module.hpp"
#include "types_cache.hpp"

using namespace py::literals;

static void add_to_python_path(const std::wstring& Path)
{
	auto PathList = py::cast<py::list>(py::sys::get_object("path"));
	const py::string NewItem(Path);

	for (size_t i = 0, size = PathList.size();  i != size; ++i)
	{
		if (const auto Str = py::try_cast<py::string>(PathList[i]))
			if (Str == NewItem)
				return;
	}

	PathList.push_back(NewItem);
}

static py::module add_or_reload_module(const std::wstring& Name)
{
	const auto ModulesDict = py::cast<py::dictionary>(py::sys::get_object("modules"));
	const py::string ModuleName(Name);
	if (const auto ExistingModule = py::try_cast<py::module>(ModulesDict[ModuleName]))
		return py::import::reload(ExistingModule);

	return py::import::import(ModuleName);
}

extern "C" IMAGE_DOS_HEADER __ImageBase;

static auto create_pygin_module()
{
	wchar_t AdaptherPath[MAX_PATH];
	GetModuleFileName(reinterpret_cast<HINSTANCE>(&__ImageBase), AdaptherPath, static_cast<DWORD>(std::size(AdaptherPath)));
	*wcsrchr(AdaptherPath, L'\\') = 0;

	add_to_python_path(AdaptherPath);
	return add_or_reload_module(L"pygin");
}

pygin::pygin(GlobalInfo* Info):
	m_PyginModule(create_pygin_module()),
	m_PyginLoadPlugin(py::cast<py::function>(m_PyginModule["_loader"]["_load_plugin"]))
{
	Info->StructSize = sizeof(GlobalInfo);

	Info->MinFarVersion = MAKEFARVERSION(3, 0, 0, 3000, VS_RELEASE);
	Info->Version = MAKEFARVERSION(1, 0, 0, 0, VS_ALPHA);

	static const UUID PyginID{ 0x66714600, 0xd5fa, 0x4fae, { 0xac, 0x4b, 0x77, 0x25, 0xe5, 0x7a, 0x87, 0x39 } };
	Info->Guid = PyginID;

	Info->Title = L"Pygin";
	Info->Author = L"Alex Alabuzhev";
	Info->Description = L"Python language support for Far Manager";
}

pygin::~pygin()
{
	far_api::uninitialise();
	types_cache::clear();
}

static const auto PluginFileNameSuffix = L".far.py";

bool pygin::is_module(const wchar_t* FileName) const
{
	// BUGBUG, ends_with
	static const auto SuffixSize = wcslen(PluginFileNameSuffix);

	const auto FileNameLength = wcslen(FileName);
	return FileNameLength >= SuffixSize && !_wcsnicmp(FileName + FileNameLength - SuffixSize, PluginFileNameSuffix, SuffixSize);
}

std::unique_ptr<module> pygin::create_module(const wchar_t* FileName) const
{
	const auto NamePtr = std::wcsrchr(FileName, L'\\') + 1;
	const auto NameSize = wcslen(NamePtr) - wcslen(PluginFileNameSuffix);

	const auto Module = m_PyginLoadPlugin(py::string(NamePtr, NameSize), FileName);

	if (!Module)
		return nullptr;

	return std::make_unique<module>(Module);
}

FARPROC WINAPI pygin::get_function(HANDLE Instance, const wchar_t* FunctionName) const
{
	static const std::unordered_map<std::wstring, void*> FunctionsMap =
	{
#define KEY_VALUE(x) { L ## #x, x }

		KEY_VALUE(GetGlobalInfoW),
		KEY_VALUE(SetStartupInfoW),
		KEY_VALUE(OpenW),
		KEY_VALUE(ClosePanelW),
		KEY_VALUE(GetPluginInfoW),
		KEY_VALUE(GetOpenPanelInfoW),
		KEY_VALUE(GetFindDataW),
		KEY_VALUE(FreeFindDataW),
		KEY_VALUE(SetDirectoryW),
		KEY_VALUE(GetFilesW),
		KEY_VALUE(PutFilesW),
		KEY_VALUE(DeleteFilesW),
		KEY_VALUE(MakeDirectoryW),
		KEY_VALUE(ProcessHostFileW),
		KEY_VALUE(SetFindListW),
		KEY_VALUE(ConfigureW),
		KEY_VALUE(ExitFARW),
		KEY_VALUE(ProcessPanelInputW),
		KEY_VALUE(ProcessPanelEventW),
		KEY_VALUE(ProcessEditorEventW),
		KEY_VALUE(CompareW),
		KEY_VALUE(ProcessEditorInputW),
		KEY_VALUE(ProcessViewerEventW),
		KEY_VALUE(ProcessDialogEventW),
		KEY_VALUE(ProcessSynchroEventW),
		KEY_VALUE(ProcessConsoleInputW),
		KEY_VALUE(AnalyseW),
		KEY_VALUE(CloseAnalyseW),
		KEY_VALUE(GetContentFieldsW),
		KEY_VALUE(GetContentDataW),
		KEY_VALUE(FreeContentDataW),

#undef KEY_VALUE
	};

	const auto Module = static_cast<module*>(Instance);
	return Module->check_function(FunctionName) && FunctionsMap.count(FunctionName)? reinterpret_cast<FARPROC>(FunctionsMap.at(FunctionName)) : nullptr;
}

bool pygin::get_error(ErrorInfo* Info) const
{
	return get_error_context(Info);
}

pygin::python::python()
{
	py::initialize();
}

pygin::python::~python()
{
	py::finalize();
}
