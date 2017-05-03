#include "headers.hpp"

pyobject_ptr Call(const pyobject_ptr& Module, const char* Name, const py_dict& InfoArg)
{
	const auto Func = pyobject_ptr(PyObject_GetAttrString(Module.get(), Name));
	if (Func && PyCallable_Check(Func.get()))
	{
		return PyObject_CallObject(Func.get(), MakeTuple(InfoArg).get());
	}
	return nullptr;
}
