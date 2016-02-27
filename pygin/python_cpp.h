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

class pyobject_ptr
{
public:
	pyobject_ptr(PyObject* Object): m_Object(Object) {}
	pyobject_ptr(const pyobject_ptr& rhs): m_Object(rhs.m_Object) { Py_XINCREF(m_Object); }

	pyobject_ptr& operator=(const pyobject_ptr& rhs)
	{
		Py_XINCREF(rhs.m_Object);
		Py_XDECREF(m_Object);
		m_Object = rhs.m_Object;
	}
	~pyobject_ptr()
	{
		Py_XDECREF(m_Object);
	}

	PyObject* get() const { return m_Object; }
	operator const void*() const { return m_Object; }

private:
	PyObject* m_Object;

};

inline pyobject_ptr py_str(const char* Str) { return PyUnicode_FromString(Str); }
inline pyobject_ptr py_str(const std::string& Str) { return PyUnicode_FromString(Str.data()); }
inline pyobject_ptr py_str(const wchar_t* Str) { return PyUnicode_FromUnicode(Str, wcslen(Str)); }
inline pyobject_ptr py_str(const std::wstring& Str) { return PyUnicode_FromUnicode(Str.data(), Str.size()); }
inline pyobject_ptr py_int(int Value) { return PyLong_FromLong(Value); }

inline pyobject_ptr pyobject_ptr_borrowed(PyObject* Object) { Py_XINCREF(Object); return pyobject_ptr(Object); }

inline std::string to_string(const pyobject_ptr& Object) { return std::string(PyBytes_AsString(pyobject_ptr(PyUnicode_AsUTF8String(Object.get())).get())); }
inline std::wstring to_wstring(const pyobject_ptr& Object) { return std::wstring(PyUnicode_AS_UNICODE(Object.get())); }

class py_dict
{
public:
	py_dict(): m_Object(PyDict_New()){}
	py_dict(PyObject* Object): m_Object(pyobject_ptr_borrowed(Object)){}
	friend class value_proxy;
	class value_proxy
	{
	public:
		value_proxy(py_dict* Owner, const pyobject_ptr& Key): m_Owner(Owner), m_Key(Key) {}
		value_proxy(const value_proxy& rhs): m_Owner(rhs.m_Owner), m_Key(rhs.m_Key)
		{}
		value_proxy& operator=(const char* value)
		{
			m_Owner->set_at(m_Key, py_str(value));
			return *this;
		}
		value_proxy& operator=(const int value)
		{
			m_Owner->set_at(m_Key, py_int(value));
			return *this;
		}
		value_proxy& operator=(const pyobject_ptr& value)
		{
			m_Owner->set_at(m_Key, value);
			return *this;
		}
		value_proxy& operator=(const value_proxy& rhs)
		{
			m_Owner->set_at(m_Key, rhs);
			return *this;
		}
		operator pyobject_ptr() const { return m_Owner->get_at(m_Key); }
	private:
		pyobject_ptr m_Key;
		py_dict* m_Owner;
	};
	value_proxy operator[](const char* Key)
	{
		return value_proxy(this, py_str(Key));
	}
	void set_at(const pyobject_ptr& Key, const pyobject_ptr& Value)
	{
		PyDict_SetItem(m_Object.get(), Key.get(), Value.get());
	}

	pyobject_ptr get_at(const pyobject_ptr& Key)
	{
		return pyobject_ptr_borrowed(PyDict_GetItem(m_Object.get(), Key.get()));
	}

	PyObject* get() const { return m_Object.get(); }
private:
	pyobject_ptr m_Object;
};

class py_list
{
public:
	py_list(PyObject* Object): m_List(pyobject_ptr_borrowed(Object)) {}
	size_t size() const { return PyList_Size(m_List.get()); }
	PyObject* operator[](size_t index) { return PyList_GetItem(m_List.get(), index); }
	void push_back(pyobject_ptr Object) { PyList_Append(m_List.get(), Object.get()); }
	void insert(pyobject_ptr Object, size_t index) { PyList_Insert(m_List.get(), index, Object.get()); }
private:
	pyobject_ptr m_List;
};

pyobject_ptr Call(const pyobject_ptr& Module, const char* Name, const py_dict& InfoArg);

