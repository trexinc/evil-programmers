#include "headers.hpp"
#include "module.hpp"

static void AddToPythonPath(const std::wstring& Path)
{
	py_list PathList(PySys_GetObject("path"));
	auto NewItem = py_str(Path);
	bool Found = false;

	for (size_t i = 0, size = PathList.size();  i != size; ++i)
	{
		if (!PyUnicode_Compare(PathList[i], NewItem.get()))
		{
			Found = true;
		}
	}
	if (!Found)
	{
		PathList.push_back(NewItem);
	}
}

static PyObject* AddOrReloadModule(const std::wstring& Name)
{
	py_dict ModulesDict(PySys_GetObject("modules"));
	auto NewModuleName = py_str(Name);
	auto ExistingModule = ModulesDict.get_at(NewModuleName);
	auto NewModule = PyImport_Import(NewModuleName.get());

	if (ExistingModule)
	{
		NewModule = PyImport_ReloadModule(ExistingModule.get());
	}
	return NewModule;
}

static void DebugPrintTracebackIfAny()
{
	if (!PyErr_Occurred())
		return;

	PyErr_Print();
}

HANDLE WINAPI adapter_CreateInstance(const wchar_t* filename)
{
	std::wstring Dir(filename);
	const auto SlashPos = Dir.rfind(L'\\');
	Dir.resize(SlashPos);
	const auto PrevSlashPos = Dir.rfind(L'\\');
	const std::wstring Path(Dir, 0, PrevSlashPos);
	const std::wstring ModuleName(Dir, PrevSlashPos + 1);
	AddToPythonPath(Path);
	const auto Object = AddOrReloadModule(ModuleName);
	DebugPrintTracebackIfAny();
	return Object ? new module(Object) : nullptr;
}

extern "C" IMAGE_DOS_HEADER __ImageBase;

BOOL WINAPI adapter_Initialize(GlobalInfo* Info)
{
	Info->StructSize = sizeof(GlobalInfo);

	Info->MinFarVersion = MAKEFARVERSION(3, 0, 0, 3000, VS_RELEASE);
	Info->Version = MAKEFARVERSION(0, 0, 0, 3, VS_ALPHA);

	static const UUID PyginID = { 0x66714600, 0xd5fa, 0x4fae, { 0xac, 0x4b, 0x77, 0x25, 0xe5, 0x7a, 0x87, 0x39 } };
	Info->Guid = PyginID;

	Info->Title = L"Pygin";
	Info->Author = L"Far Group";
	Info->Description = L"Python language support for Far Manager";

	Py_Initialize();

	wchar_t AdaptherPath[MAX_PATH];
	GetModuleFileNameW(reinterpret_cast<HINSTANCE>(&__ImageBase), AdaptherPath, static_cast<DWORD>(std::size(AdaptherPath)));
	*(wcsrchr(AdaptherPath, L'\\') + 1) = 0;
	auto PythonModule(reinterpret_cast<PyObject*>(adapter_CreateInstance((AdaptherPath + L"__init__.py"s).data())));

	return TRUE;
}

BOOL WINAPI adapter_IsPlugin(const wchar_t* filename)
{
	return !wcscmp(wcsrchr(filename, L'\\') + 1, L"__init__.py");
}

FARPROC WINAPI adapter_GetFunctionAddress(HANDLE Instance, const wchar_t* functionname)
{
	static std::unordered_map<std::wstring, void*> FunctionsMap;
	if (FunctionsMap.empty())
	{
		FunctionsMap[L"GetGlobalInfoW"] = GetGlobalInfoW;
		FunctionsMap[L"SetStartupInfoW"] = SetStartupInfoW;
		FunctionsMap[L"OpenW"] = OpenW;
		FunctionsMap[L"ClosePanelW"] = ClosePanelW;
		FunctionsMap[L"GetPluginInfoW"] = GetPluginInfoW;
		FunctionsMap[L"GetOpenPanelInfoW"] = GetOpenPanelInfoW;
		FunctionsMap[L"GetFindDataW"] = GetFindDataW;
		FunctionsMap[L"FreeFindDataW"] = FreeFindDataW;
		FunctionsMap[L"SetDirectoryW"] = SetDirectoryW;
		FunctionsMap[L"GetFilesW"] = GetFilesW;
		FunctionsMap[L"PutFilesW"] = PutFilesW;
		FunctionsMap[L"DeleteFilesW"] = DeleteFilesW;
		FunctionsMap[L"MakeDirectoryW"] = MakeDirectoryW;
		FunctionsMap[L"ProcessHostFileW"] = ProcessHostFileW;
		FunctionsMap[L"SetFindListW"] = SetFindListW;
		FunctionsMap[L"ConfigureW"] = ConfigureW;
		FunctionsMap[L"ExitFARW"] = ExitFARW;
		FunctionsMap[L"ProcessPanelInputW"] = ProcessPanelInputW;
		FunctionsMap[L"ProcessPanelEventW"] = ProcessPanelEventW;
		FunctionsMap[L"ProcessEditorEventW"] = ProcessEditorEventW;
		FunctionsMap[L"CompareW"] = CompareW;
		FunctionsMap[L"ProcessEditorInputW"] = ProcessEditorInputW;
		FunctionsMap[L"ProcessViewerEventW"] = ProcessViewerEventW;
		FunctionsMap[L"ProcessDialogEventW"] = ProcessDialogEventW;
		FunctionsMap[L"ProcessSynchroEventW"] = ProcessSynchroEventW;
		FunctionsMap[L"ProcessConsoleInputW"] = ProcessConsoleInputW;
		FunctionsMap[L"AnalyseW"] = AnalyseW;
		FunctionsMap[L"CloseAnalyseW"] = CloseAnalyseW;
		FunctionsMap[L"GetContentFieldsW"] = GetContentFieldsW;
		FunctionsMap[L"GetContentDataW"] = GetContentDataW;
		FunctionsMap[L"FreeContentDataW"] = FreeContentDataW;
	}


	const auto Module = static_cast<module*>(Instance);
	return Module->CheckFunction(functionname) && FunctionsMap.count(functionname)? reinterpret_cast<FARPROC>(FunctionsMap[functionname]) : nullptr;
}

BOOL WINAPI adapter_DestroyInstance(HANDLE Instance)
{
	auto Module = static_cast<module*>(Instance);
	delete Module;
	return TRUE;
}

void WINAPI adapter_Free(const ExitInfo* info)
{
	Py_Finalize();
}