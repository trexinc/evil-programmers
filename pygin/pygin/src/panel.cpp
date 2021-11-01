#include "headers.hpp"

#include "panel.hpp"
#include "far_api.hpp"
#include "py.integer.hpp"

namespace {

void WINAPI free_py(void* UserData, const struct FarPanelItemFreeInfo* Info)
{
	py::object dec_ref(static_cast<PyObject*>(UserData));
}
	
void let_far_manage_py_items_life_time(std::span<PluginPanelItem> const& items) noexcept
{
	for (auto& item : items)
	{
		auto* inc_ref = static_cast<PyObject*>(item.UserData.Data);
		(void)py::object::from_borrowed(inc_ref).release();
		item.UserData.FreeData = &free_py;
	}
}

} // anonymous

PluginPanelItem*
get_find_data::get_items(py::object const& find_data, size_t& item_count, OPERATION_MODES& mode)
{
	find_data.ensure_type(far_api::type("GetFindDataInfo"sv));
	auto py_items = py::cast<py::list>(find_data["PanelItems"sv]);
	auto size = py_items.size();
	auto mem = std::make_unique<char[]>(sizeof(get_find_data) + sizeof(PluginPanelItem) * size);
	auto* result = reinterpret_cast<PluginPanelItem*>(mem.get() + sizeof(get_find_data));
	new (mem.get()) get_find_data(py_items, result);
	item_count = size;
	mode = py::cast<OPERATION_MODES>(find_data["OpMode"sv]);
	let_far_manage_py_items_life_time(std::span(result, size));
	mem.release();
	return result;
}

void get_find_data::free(PluginPanelItem* items)
{
	char* mem = reinterpret_cast<char*>(items) - sizeof(get_find_data);
	reinterpret_cast<get_find_data*>(mem)->~get_find_data();
	delete[] mem;
}

get_find_data::get_find_data(py::list const& py_items, PluginPanelItem* items)
{
	static const std::pair< wchar_t const* (PluginPanelItem::*), char const*> string_fields[] = {
		{&PluginPanelItem::FileName, "FileName"},
		{&PluginPanelItem::AlternateFileName, "AlternateFileName"},
		{&PluginPanelItem::Description, "Description"},
		{&PluginPanelItem::Owner, "Owner"},
	};
	m_Strings.reserve(std::size(string_fields) * py_items.size());
	for (auto&& py_item : py_items)
	{
		items->CreationTime = far_api::file_time(py_item["CreationTime"sv]);
		items->LastAccessTime = far_api::file_time(py_item["LastAccessTime"sv]);
		items->LastWriteTime = far_api::file_time(py_item["LastWriteTime"sv]);
		items->ChangeTime = far_api::file_time(py_item["ChangeTime"sv]);
		items->FileSize = py::cast<uint64_t>(py_item["FileSize"sv]);
		items->AllocationSize = py::cast<uint64_t>(py_item["AllocationSize"sv]);
		items->CustomColumnData = nullptr;
		items->CustomColumnNumber = 0;
		items->Flags = py::cast<PLUGINPANELITEMFLAGS>(py_item["Flags"sv]);
		items->UserData.Data = nullptr;
		items->UserData.FreeData = nullptr;
		items->FileAttributes = py::cast<uintptr_t>(py_item["FileAttributes"sv]);
		items->NumberOfLinks = py::cast<uintptr_t>(py_item["NumberOfLinks"sv]);
		items->CRC32 = py::cast<uintptr_t>(py_item["CRC32"sv]);
		memset(&items->Reserved, 0, sizeof(items->Reserved));
		for (auto&& kv : string_fields)
			items->*kv.first = push_back_if_not_none(py_item[kv.second], m_Strings);
		items->UserData = { py_item.get() };		
		++items;
	}
}
