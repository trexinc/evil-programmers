#pragma once

#include <Python.h>

#define DONT_STEAL_REFERENCE(x) Py_XINCREF(x)
