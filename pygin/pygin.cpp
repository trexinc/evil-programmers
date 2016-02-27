#include "stdafx.h"
#include "module.h"

BOOL WINAPI adapter_Initialize(GlobalInfo* Info)
{
	Info->StructSize = sizeof(GlobalInfo);

	Info->MinFarVersion = MAKEFARVERSION(3, 0, 0, 3000, VS_RELEASE);
	Info->Version = MAKEFARVERSION(0, 0, 0, 1, VS_ALPHA);

	static const UUID PyginID = { 0x66714600, 0xd5fa, 0x4fae, { 0xac, 0x4b, 0x77, 0x25, 0xe5, 0x7a, 0x87, 0x39 } };
	Info->Guid = PyginID;

	Info->Title = L"Pygin";
	Info->Author = L"Far Group";
	Info->Description = L"Python language support for Far Manager";

	Py_Initialize();
	return TRUE;
}

BOOL WINAPI adapter_IsPlugin(const wchar_t* filename)
{
	return !wcscmp(wcsrchr(filename, L'\\') + 1, L"__init__.py");
}

HANDLE WINAPI adapter_CreateInstance(const wchar_t* filename)
{
	std::wstring Dir(filename);
	const auto SlashPos = Dir.rfind(L'\\');
	auto Name = Dir.substr(SlashPos + 1);
	Name.resize(Name.size() - 3);
	Dir.resize(SlashPos);

	PyList_Append(PySys_GetObject("path"), py_str(Dir).get());

	const auto Object = PyImport_Import(py_str(Name).get());
	PyErr_Print();
	return Object? new module(Object) : nullptr;
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