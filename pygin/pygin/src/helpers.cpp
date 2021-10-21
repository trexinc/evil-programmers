#include "headers.hpp"

#include "helpers.hpp"
#include "python.hpp"
#include "py.common.hpp"

namespace helpers
{

namespace {
struct py_deleter
{
	void operator ()(void* p) const { PyMem_Free(p); }
};
} // anonymous

void py_string_storage::clear()
{
	while (!m_Strings.empty())
	{
		py_deleter()(m_Strings.back());
		m_Strings.pop_back();
	}
}

wchar_t const* py_string_storage::push_back(py::string const& s)
{
	Py_ssize_t Size;
	std::unique_ptr<wchar_t [], py_deleter> Data(
		py::invoke(PyUnicode_AsWideCharString, s.get(), &Size));
	m_Strings.push_back(Data.get());
	return Data.release();
}

wchar_t const* py_string_storage::push_back_allow_none(py::object const& o)
{
	if (o)
		return this->push_back(cast<py::string>(o));

	m_Strings.push_back(nullptr);
	return nullptr;
}


wchar_t const* push_back_if_not_none(py::object const& o, py_string_storage& storage)
{
	if (!o)
		return nullptr;
	return storage.push_back(cast<py::string>(o));
}

} // helpers