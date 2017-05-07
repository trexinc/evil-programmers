#pragma once

#ifdef _DEBUG
#undef _DEBUG
#define _OLDDEBUG 1
#endif
#include <Python.h>
#ifdef _OLDDEBUG
#undef _OLDDDEBUG
#define _DEBUG 1
#endif

#define DONT_STEAL_REFERENCE(x) Py_XINCREF(x)
