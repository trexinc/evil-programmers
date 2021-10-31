#pragma once

#include "helpers.hpp"

struct open_panel_info_storage
{
	helpers::py_string_storage strings;
};

class get_find_data
{
public:
	static PluginPanelItem* get_items(py::object const& find_data, size_t& item_count, OPERATION_MODES&);

	static void free(PluginPanelItem*);
private:
	get_find_data(py::list const& py_items, PluginPanelItem*);
private:
	std::vector<py::object> m_PyItems;
	helpers::py_string_storage m_Strings;
};

