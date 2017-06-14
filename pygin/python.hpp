#pragma once

//#define USE_RELEASE_PYTHON_DLL

#ifdef USE_RELEASE_PYTHON_DLL
#pragma push_macro("_DEBUG")
#undef _DEBUG
#endif

#include <Python.h>

#ifdef USE_RELEASE_PYTHON_DLL
#pragma pop_macro("_DEBUG")
#endif


#define DONT_STEAL_REFERENCE(x) Py_XINCREF(x)
