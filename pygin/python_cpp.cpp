#include "stdafx.h"

pyobject_ptr Call(const pyobject_ptr& Module, const char* Name, const py_dict& InfoArg)
{
	const auto Func = pyobject_ptr(PyObject_GetAttrString(Module.get(), Name));
	if (Func && PyCallable_Check(Func.get()))
	{
		const auto Args = pyobject_ptr(PyTuple_New(1));
		PyTuple_SetItem(Args.get(), 0, InfoArg.get());
		return PyObject_CallObject(Func.get(), Args.get());
	}
	return nullptr;
}
