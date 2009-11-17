/**
	win_def
	Main windows application include, always include first
	@classes	(WinMem, WinStr, CStr, WinFlag, WinBit, WinTimer, WinErrorCheck, Uncopyable, WinCOM)
	@author		Copyright © 2009 Andrew Grechkin
	@link		()
	@link		(ole32) for WinCom
**/

#ifndef WIN_DEF_HPP
#define WIN_DEF_HPP

#if (_WIN32_WINNT < 0x0501)
#undef _WIN32_WINNT
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#if (WINVER < 0x0501)
#undef WINVER
#endif
#ifndef WINVER
#define WINVER 0x0501
#endif

#if (_WIN32_IE < 0x0600)
#undef _WIN32_IE
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#include <windows.h>
//#include <wchar.h>

class		CStrA;
class		CStrW;

#ifdef UNICODE
typedef	CStrW	CStr;
#else
typedef	CStrA	CStr;
#endif

#ifdef DEBUG
#include <iostream>
#include <ostream>
using	std::cout;
using	std::ostream;
using	std::endl;
ostream			&operator<<(ostream &s, PCWSTR rhs);
#endif

#ifdef _DEBUG
#define	DLOG(out) out;
#else
#define	DLOG(out)
#endif

EXTERN_C {
	WINBASEAPI VOID WINAPI GetNativeSystemInfo(LPSYSTEM_INFO);
	WINBASEAPI ULONGLONG WINAPI GetTickCount64();
	WINBASEAPI DWORD WINAPI GetProcessId(HANDLE Process);
	_CRTIMP int __cdecl __MINGW_NOTHROW	_snwprintf(wchar_t*, size_t, const wchar_t*, ...);
}

///===================================================================================== definitions
#define MAX_PATH_LENGTH			32768
#define SPACE					L' '
#define PATH_SEPARATOR			L"\\" // Path separator in the file system
#define PATH_SEPARATOR_C		L'\\' // Path separator in the file system
#define PATH_PREFIX				L"\\\\?\\" // Prefix to put ahead of a long path for Windows API
#define NORM_M_PREFIX(m)        (*(LPDWORD)m==0x5c005c)
#define REV_M_PREFIX(m)         (*(LPDWORD)m==0x2f002f)

#define sizeofa(array)			(sizeof(array)/sizeof(array[0]))
#define sizeofe(array)			(sizeof(array[0]))

typedef const TCHAR			*PCTSTR;
typedef const void			*PCVOID;

#ifdef DynamicLink
typedef	void*(__cdecl *dl_memset)(void*, int, size_t);
typedef	void*(__cdecl *dl_memcpy)(void*, const void*, size_t);
typedef	long long(__cdecl *dl__atoi64)(const char *);
typedef	long long(__cdecl *dl__wtoi64)(const wchar_t *);
typedef	int(__cdecl *dl__wcsicmp)(const wchar_t *, const wchar_t *);
typedef	int(__cdecl *dl_wcscmp)(const wchar_t *, const wchar_t *);
typedef	size_t(__cdecl *dl_wcslen)(const wchar_t *);
typedef	wchar_t*(__cdecl *dl_wcscpy)(wchar_t *, const wchar_t *);
typedef	wchar_t*(__cdecl *dl_wcsncpy)(wchar_t *, const wchar_t *, size_t);
typedef	int(__cdecl *dl__snwprintf)(wchar_t *, size_t, const wchar_t *, ...);

namespace	DynLink {
	extern HINSTANCE		hNtDll;
	extern dl__snwprintf	my_snprintf;
	bool					InitLib();
	void					FreeLib();
}
#define my_snwprintf DynLink::my_snprintf
#else
namespace	DynLink {
	bool	inline			InitLib() {
		return	true;
	}
	void	inline			FreeLib() {}
}
#define my_snwprintf _snwprintf
#endif

///=================================================================================================
inline void						XchgByte(WORD &inout) {
	inout = inout >> 8 | inout << 8;
}
inline void						XchgWord(DWORD &inout) {
	inout = inout >> 16 | inout << 16;
}

template <typename Type>
inline	const Type				&Min(const Type &a, const Type &b) {
	return	(a < b) ? a : b;
}
template <typename Type>
inline	const Type				&Max(const Type &a, const Type &b) {
	return	(a < b) ? b : a;
}
template <typename Type>
inline	void					Swp(Type &x, Type &y) {
	Type tmp(x);
	x = y;
	y = tmp;
}

///====================================================================================== Uncopyable
/// Базовый класс для наследования классами, объекты которых не должны копироваться
class		Uncopyable {
	Uncopyable(const Uncopyable&);
	Uncopyable &operator=(const Uncopyable&);
protected:
	~Uncopyable() {
	}
	Uncopyable() {
	}
};

///=================================================================================== WinErrorCheck
/// Базовый класс для проверки и хранения кода ошибки
class		WinErrorCheck {
	HRESULT	mutable	m_err;
protected:
	~WinErrorCheck() {
	}
	WinErrorCheck(): m_err(NO_ERROR) {
	}
public:
	HRESULT			err() const {
		return	m_err;
	}
	HRESULT			err(HRESULT err) const {
		return	m_err = err;
	}
	bool			IsOK() const {
		return	m_err == NO_ERROR;
	}
	bool			ChkSucc(bool in) const {
		if (!in)
			err(::GetLastError());
		else
			err(NO_ERROR);
		return	in;
	}
	template<typename Type>
	void			SetIfFail(Type &in, const Type &value) {
		if (m_err != NO_ERROR)
			in = value;
	}
};

///========================================================================================== WinMem
/// Функции работы с кучей
namespace	WinMem {
	inline PVOID				Alloc(size_t size) {
		return	::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size);
	}
	inline PVOID				Realloc(PCVOID in, size_t size) {
		return	::HeapReAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, (PVOID)in, size);
	}
	inline void					Free(PCVOID in) {
		::HeapFree(::GetProcessHeap(), 0, (PVOID)in);
	}
	inline size_t				Size(PCVOID in) {
		return	::HeapSize(::GetProcessHeap(), 0, in);
	}

	inline PVOID				Copy(PVOID dest, PCVOID sour, size_t size) {
#ifdef DynamicLink
		dl_memcpy ProcAdd = (dl_memcpy) ::GetProcAddress(DynLink::hNtDll, "memcpy");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load memcpy", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(dest, sour, size) : NULL;
#else
		return	::CopyMemory(dest, sour, size);
#endif
	}
	inline PVOID				Fill(PVOID in, size_t size, char fill) {
#ifdef DynamicLink
		dl_memset ProcAdd = (dl_memset) ::GetProcAddress(DynLink::hNtDll, "memset");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load _memset", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(in, fill, size) : NULL;
#else
		return	::FillMemory(in, size, (BYTE)fill);
#endif
	}
	inline void					Zero(PVOID in, size_t size) {
		Fill(in, size, 0);
	}
	template<typename Type>
	inline void					Fill(Type &in, char fill) {
		Fill(&in, sizeof(in), fill);
	}
	template<typename Type>
	inline void					Zero(Type &in) {
		Fill(&in, sizeof(in), 0);
	}
}

///========================================================================================== WinStr
/// Функции работы символьными строками
namespace	WinStr {
	inline size_t				Len(PCSTR in) {
		return	::strlen(in);
	}
	inline size_t				Len(PCWSTR in) {
#ifdef DynamicLink
		dl_wcslen ProcAdd = (dl_wcslen) ::GetProcAddress(DynLink::hNtDll, "wcslen");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load wcslen", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(in) : 0;
#else
		return	::wcslen(in);
#endif
	}
	inline int					Cmp(PCSTR in1, PCSTR in2) {
		return	::strcmp(in1, in2);
	}
	inline int					Cmp(PCWSTR in1, PCWSTR in2) {
#ifdef DynamicLink
		dl_wcscmp ProcAdd = (dl_wcscmp) ::GetProcAddress(DynLink::hNtDll, "wcscmp");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load _wcscmp", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(in1, in2) : 0;
#else
		return	::wcscmp(in1, in2);
#endif
	}
	inline int					Cmpi(PCSTR in1, PCSTR in2) {
		return	::_stricmp(in1, in2);
	}
	inline int					Cmpi(PCWSTR in1, PCWSTR in2) {
#ifdef DynamicLink
		dl__wcsicmp ProcAdd = (dl__wcsicmp) ::GetProcAddress(DynLink::hNtDll, "_wcsicmp");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load _wcsicmp", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(in1, in2) : 0;
#else
		return	::_wcsicmp(in1, in2);
#endif
	}
	inline bool					Eq(PCSTR in1, PCSTR in2) {
		return	Cmp(in1, in2) == 0;
	}
	inline bool					Eq(PCWSTR in1, PCWSTR in2) {
		return	Cmp(in1, in2) == 0;
	}
	inline bool					Eqi(PCSTR in1, PCSTR in2) {
		return	Cmpi(in1, in2) == 0;
	}
	inline bool					Eqi(PCWSTR in1, PCWSTR in2) {
		return	Cmpi(in1, in2) == 0;
	}
	inline PSTR					Copy(PSTR dest, PCSTR src) {
		return	::strcpy(dest, src);
	}
	inline PWSTR				Copy(PWSTR dest, PCWSTR src) {
#ifdef DynamicLink
		dl_wcscpy ProcAdd = (dl_wcscpy) ::GetProcAddress(DynLink::hNtDll, "wcscpy");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load wcscpy", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(dest, src) : (PWSTR)L"";
#else
		return	::wcscpy(dest, src);
#endif
	}
	inline PSTR					Copy(PSTR dest, PCSTR src, size_t size) {
		return	::strncpy(dest, src, size);
	}
	inline PWSTR				Copy(PWSTR dest, PCWSTR src, size_t size) {
#ifdef DynamicLink
		dl_wcsncpy ProcAdd = (dl_wcsncpy) ::GetProcAddress(DynLink::hNtDll, "wcsncpy");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load wcsncpy", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(dest, src, size) : (PWSTR)L"";
#else
		return	::wcsncpy(dest, src, size);
#endif
	}

	inline PSTR					Cat(PSTR dest, PCSTR src) {
		return	::strcat(dest, src);
	}
	inline PWSTR				Cat(PWSTR dest, PCWSTR src) {
		return	::wcscat(dest, src);
	}

	inline PCWSTR				Assign(PCWSTR &dest, PCWSTR src) {
		size_t	len = WinStr::Len(src) + 1;
		dest = (PCWSTR)WinMem::Alloc(len * sizeof(WCHAR));
		WinStr::Copy((PWSTR)dest, src, len);
		return	dest;
	}
	inline void					Free(PCWSTR in) {
		return	WinMem::Free(in);
	}

	inline PCSTR				CharFirst(PCSTR in, CHAR ch) {
		return	::strchr(in, ch);
	}
	inline PCWSTR				CharFirst(PCWSTR in, WCHAR ch) {
		return	::wcschr(in, ch);
	}
	inline PSTR					CharFirst(PSTR in, CHAR ch) {
		return	::strchr(in, ch);
	}
	inline PWSTR				CharFirst(PWSTR in, WCHAR ch) {
		return	::wcschr(in, ch);
	}

	inline PCSTR				CharLast(PCSTR in, CHAR ch) {
		return	::strrchr(in, ch);
	}
	inline PCWSTR				CharLast(PCWSTR in, WCHAR ch) {
		return	::wcsrchr(in, ch);
	}
	inline PSTR					CharLast(PSTR in, CHAR ch) {
		return	::strrchr(in, ch);
	}
	inline PWSTR				CharLast(PWSTR in, WCHAR ch) {
		return	::wcsrchr(in, ch);
	}

	inline PWSTR				CharFirstOf(PCWSTR in, PCWSTR mask) {
		size_t	lin = Len(in);
		size_t	len = Len(mask);
		for (size_t i = 0; i < lin; ++i) {
			for (size_t j = 0; j < len; ++j) {
				if (in[i] == mask[j])
					return	(PWSTR)&in[i];
			}
		}
		return	NULL;
	}
	inline PWSTR				CharFirstNotOf(PCWSTR in, PCWSTR mask) {
		size_t	lin = Len(in);
		size_t	len = Len(mask);
		for (size_t i = 0; i < lin; ++i) {
			for (size_t j = 0; j < len; ++j) {
				if (in[i] == mask[j])
					break;
				if (j == len - 1)
					return	(PWSTR)&in[i];
			}
		}
		return	NULL;
	}
	inline PWSTR				CharLastOf(PCWSTR in, PCWSTR mask) {
		size_t	len = Len(mask);
		for (size_t i = Len(in); i > 0; --i) {
			for (size_t j = 0; j < len; ++j) {
				if (in[i-1] == mask[j])
					return	(PWSTR)&in[i-1];
			}
		}
		return	NULL;
	}
	inline PWSTR				CharLastNotOf(PCWSTR in, PCWSTR mask) {
		size_t	len = Len(mask);
		for (size_t i = Len(in); i > 0; --i) {
			for (size_t j = 0; j < len; ++j) {
				if (in[i-1] == mask[j])
					break;
				if (j == len - 1)
					return	(PWSTR)&in[i-1];
			}
		}
		return	NULL;
	}

	inline PSTR					Fill(PSTR in, CHAR ch) {
		return	::_strset(in, ch);
	}
	inline PWSTR				Fill(PWSTR in, WCHAR ch) {
		return	::_wcsset(in, ch);
	}

	inline PSTR					Reverse(PSTR in) {
		return	::_strrev(in);
	}
	inline PWSTR				Reverse(PWSTR in) {
		return	::_wcsrev(in);
	}

	inline long long			AsLongLong(PCSTR in) {
#ifdef DynamicLink
		dl__atoi64 ProcAdd = (dl__atoi64) ::GetProcAddress(DynLink::hNtDll, "_atoi64");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load _atoi64", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(in) : 0LL;
#else
		return	_atoi64(in);
#endif
	}
	inline unsigned long		AsULong(PCSTR in) {
		return	(unsigned long)AsLongLong(in);
	}
	inline long					AsLong(PCSTR in) {
		return	(long)AsLongLong(in);
	}
	inline unsigned int			AsUInt(PCSTR in) {
		return	(unsigned int)AsLongLong(in);
	}
	inline int					AsInt(PCSTR in) {
		return	(int)AsLongLong(in);
	}

	inline long long			AsLongLong(PCWSTR in) {
#ifdef DynamicLink
		dl__wtoi64 ProcAdd = (dl__wtoi64) ::GetProcAddress(DynLink::hNtDll, "_wtoi64");
		if (!ProcAdd)
			::MessageBoxW(NULL, L"Can`t load _wtoi64", L"Critical error", MB_ICONERROR);
		return	(ProcAdd) ? ProcAdd(in) : 0LL;
#else
		return	_wtoi64(in);
#endif
	}
	inline unsigned long		AsULong(PCWSTR in) {
		return	(unsigned long)AsLongLong(in);
	}
	inline long					AsLong(PCWSTR in) {
		return	(long)AsLongLong(in);
	}
	inline unsigned int			AsUInt(PCWSTR in) {
		return	(unsigned int)AsLongLong(in);
	}
	inline int					AsInt(PCWSTR in) {
		return	(int)AsLongLong(in);
	}
}

///======================================================================================= WinBuffer
/// Обертка буфера
template<typename Type>
class		WinBuffer {
	Type		m_buf;
	size_t		m_load;

	bool				Free() {
		WinMem::Free(m_buf);
		m_buf = NULL;
		return	true;
	}
public:
	~WinBuffer() {
		Free();
	}
	WinBuffer(size_t size): m_buf(NULL),  m_load(0) {
		m_buf = (Type)WinMem::Alloc(size);
	}
	WinBuffer(const WinBuffer &in): m_buf(NULL),  m_load(0) {
		size_t	size = in.Size();
		m_load = in.m_load;
		m_buf = (Type)WinMem::Alloc(size);
		if (m_buf) {
			WinMem::Copy(m_buf, in.m_buf, size);
		}
	}
	operator			Type() {
		return	m_buf;
	}
	Type				Buf() const {
		return	m_buf;
	}
	Type				operator->() const {
		return	m_buf;
	}
	const WinBuffer	&operator=(const WinBuffer &in) {
		if (this != &in) {
			Free();
			size_t	size = in.Size();
			m_load = in.m_load;
			m_buf = (Type)WinMem::Alloc(size);
			if (m_buf) {
				WinMem::Copy(m_buf, in.m_buf, size);
			}
		}
		return	*this;
	}

	size_t				capacity() {
		return	WinMem::Size(m_buf);
	}
	bool				reserve(size_t size) {
		if (capacity() < size) {
			m_buf = (Type)WinMem::Realloc(m_buf, size);
			return	false;
		}
		return	true;
	}

	size_t				size() {
		return	WinMem::Size(m_buf);
	}
};

/// Обертка буфера с подсчетом ссылок
template<typename Type>
class		WinBufferCtr: public Uncopyable {
	Type		*m_buf;
	size_t		m_ctr;

public:
	~WinBufferCtr() {
		Dec();
	}
	WinBufferCtr(): m_buf(NULL),  m_ctr(0) {
	}
	bool				Init(size_t size) {
		m_ctr = 1;
		m_buf = (Type*)WinMem::Alloc(size * sizeof(Type));
		return	true;
	}
	operator			Type*() const {
		return	m_buf;
	}
	Type				*Buf() const {
		return	m_buf;
	}

	size_t				capacity() const {
		return	WinMem::Size(m_buf) / sizeof(Type);
	}
	bool				reserve(size_t size) {
		if (capacity() < size) {
			m_buf = (Type*)WinMem::Realloc(m_buf, size * sizeof(Type));
			return	false;
		}
		return	true;
	}

	size_t				links() const {
		return	m_ctr;
	}
	bool				IsSingle() const {
		return	m_ctr == 1;
	}
	void				Inc() {
		++m_ctr;
	}
	bool				Dec() {
		if (--m_ctr == 0) {
			WinMem::Free(m_buf);
			m_buf = NULL;
			return	true;
		}
		return	false;
	}
};

///============================================================================================ CStr
/// Строки с счетчиком ссылок, передача по значению обходится очень дешево
class		CStrA {
	WinBufferCtr<CHAR>	*m_data;

	void				Alloc(WinBufferCtr<CHAR>* &data, size_t size) {
		DLOG((std::cout << "CStrA::Alloc\tSize: " << size << std::endl))
		data = (WinBufferCtr<CHAR>*) WinMem::Alloc(sizeof(*data));
		data->Init(size);
	}
	void				Assign(WinBufferCtr<CHAR>* &data, PCSTR in, size_t size) {
		DLOG((std::cout << "CStrA::Assign\tSize: '" << size << "' \tData: '" << in << "'" << std::endl))
		data = (WinBufferCtr<CHAR>*) WinMem::Alloc(sizeof(*data));
		data->Init(size);
		WinStr::Copy(data->Buf(), in, data->capacity());
	}
	void				Release(WinBufferCtr<CHAR>* &data) {
		DLOG((std::cout << "CStrA::Release(\"" << data->Buf() << "\")"))
		if (data && data->Dec()) {
			DLOG((std::cout << "\tERASED"))
			WinMem::Free(data);
			data = NULL;
		}
		DLOG((std::cout << std::endl))
	}

public:
	~CStrA() {
		DLOG((std::cout << "CStrA::~CStrA(\"" << m_data->Buf() << "\")" << std::endl))
		Release(m_data);
	}
	explicit CStrA(): m_data(NULL) {
		DLOG((std::cout << "CStrA::CStrA()" << std::endl))
		Assign(m_data, "", 1);
	}
	CStrA(const CStrA &in) {
		DLOG((std::cout << "CStrA::CStrA(\"" << in << "\")" << std::endl))
		in.m_data->Inc();
		m_data = in.m_data;
	}
	CStrA(PCSTR in) {
		DLOG((std::cout << "CStrA::CStrA(\"" << in << "\")" << std::endl))
		Assign(m_data, in, WinStr::Len(in) + 1);
	}
	CStrA(PCWSTR in, UINT cp) {
//		DLOG((std::cout << "CStrA::CStrA(\"" << in << "\", " << cp << ")" << std::endl))
		Alloc(m_data, ::WideCharToMultiByte(cp, 0, in, -1, NULL, 0, NULL, NULL));
		::WideCharToMultiByte(cp, 0, in, -1, m_data->Buf(), m_data->capacity(), NULL, NULL);
	}
	CStrA(size_t in) {
		DLOG((std::cout << "CStrA::CStrA(\"" << in << "\")" << std::endl))
		Alloc(m_data, in);
	}

	const CStrA			&operator=(const CStrA &in) {
		DLOG((std::cout << "CStrA::operator=(\"" << in << "\")" << std::endl))
		if (m_data != in.m_data) {
			Release(m_data);
			m_data = in.m_data;
			m_data->Inc();
		}
		return	*this;
	}
	const CStrA			&operator=(PCSTR in) {
		DLOG((std::cout << "CStrA::operator=(\"" << in << "\")" << std::endl))
		WinBufferCtr<CHAR>	*tmp;
		Assign(tmp, in, WinStr::Len(in) + 1);
		Swp(m_data, tmp);
		Release(tmp);
		return	*this;
	}
	const CStrA			&operator=(CHAR in) {
		DLOG((std::cout << "CStrA::operator=(\"" << in << "\")" << std::endl))
		CHAR tmp[] = {in, '\0'};
		operator+=(tmp);
		return	*this;
	}
	CStrA				&operator+=(const CStrA &in) {
		DLOG((std::cout << "CStrA::operator+=(\"" << in << "\")" << std::endl))
		if (m_data != in.m_data) {
			operator+=(in.m_data->Buf());
		}
		return	*this;
	}
	CStrA				&operator+=(PCSTR in) {
		DLOG((std::cout << "CStrA::operator+=(\"" << in << "\")" << std::endl))
		size_t	len = m_data->capacity();
		if (m_data->IsSingle()) {
			m_data->reserve(len + WinStr::Len(in));
			WinStr::Cat(m_data->Buf(), in);
		} else {
			WinBufferCtr<CHAR>	*tmp;
			Assign(tmp, m_data->Buf(), len + WinStr::Len(in));
			WinStr::Cat(tmp->Buf(), in);
			Swp(m_data, tmp);
			Release(tmp);
		}
		return	*this;
	}
	CStrA				&operator+=(CHAR in) {
		DLOG((std::cout << "CStrA::operator+=(\"" << in << "\")" << std::endl))
		CHAR tmp[] = {in, '\0'};
		operator+=(tmp);
		return	*this;
	}

	CStrA				operator+(const CStrA &in) {
		DLOG((std::cout << "CStrA::operator+(\"" << in << "\")" << std::endl))
		CStrA	tmp(*this);
		tmp += in;
		return	tmp;
	}
	CStrA				operator+(PCSTR in) {
		DLOG((std::cout << "CStrA::operator+(\"" << in << "\")" << std::endl))
		CStrA	tmp(*this);
		tmp += in;
		return	tmp;
	}
	CStrA				operator+(CHAR in) {
		DLOG((std::cout << "CStrA::operator+(\"" << in << "\")" << std::endl))
		CStrA	tmp(*this);
		tmp += in;
		return	tmp;
	}

	bool				operator==(const CStrA &in) const {
		DLOG((std::cout << "CStrA::operator==(\"" << in << "\")" << std::endl))
		return	WinStr::Eq(m_data->Buf(), in.m_data->Buf());
	}
	bool				operator==(PCSTR in) const {
		DLOG((std::cout << "CStrA::operator==(\"" << in << "\")" << std::endl))
		return	WinStr::Eq(m_data->Buf(), in);
	}

	bool				operator!=(const CStrA &in) const {
		DLOG((std::cout << "CStrA::operator!=(\"" << in << "\")" << std::endl))
		return	!operator==(in);
	}
	bool				operator!=(PCSTR in) const {
		DLOG((std::cout << "CStrA::operator!=(\"" << in << "\")" << std::endl))
		return	!operator==(in);
	}

	bool				operator<(const CStrA &in) const {
		DLOG((std::cout << "CStrA::operator<(\"" << in << "\")" << std::endl))
		return	WinStr::Cmp(m_data->Buf(), in.m_data->Buf()) < 0;
	}
	bool				operator<(PCSTR in) const {
		DLOG((std::cout << "CStrA::operator<(\"" << in << "\")" << std::endl))
		return	WinStr::Cmp(m_data->Buf(), in) < 0;
	}

	bool				operator>(const CStrA &in) const {
		DLOG((std::cout << "CStrA::operator>(\"" << in << "\")" << std::endl))
		return	WinStr::Cmp(m_data->Buf(), in.m_data->Buf()) > 0;
	}
	bool				operator>(PCSTR in) const {
		DLOG((std::cout << "CStrA::operator>(\"" << in << "\")" << std::endl))
		return	WinStr::Cmp(m_data->Buf(), in) > 0;
	}

	bool				operator<=(const CStrA &in) const {
		DLOG((std::cout << "CStrA::operator<=(\"" << in << "\")" << std::endl))
		return	operator==(in) || operator<(in);
	}
	bool				operator<=(PCSTR in) const {
		DLOG((std::cout << "CStrA::operator<=(\"" << in << "\")" << std::endl))
		return	operator==(in) || operator<(in);
	}

	bool				operator>=(const CStrA &in) const {
		DLOG((std::cout << "CStrA::operator>=(\"" << in << "\")" << std::endl))
		return	operator==(in) || operator>(in);
	}
	bool				operator>=(PCSTR in) const {
		DLOG((std::cout << "CStrA::operator>=(\"" << in << "\")" << std::endl))
		return	operator==(in) || operator>(in);
	}

	CHAR				&operator[](int in) {
		DLOG((std::cout << "CStrA::operator[" << in << "]" << std::endl))
		if (!m_data->IsSingle()) {
			WinBufferCtr<CHAR>	*tmp;
			Assign(tmp, m_data->Buf(), m_data->capacity());
			Swp(m_data, tmp);
			Release(tmp);
		}
		return	m_data->Buf()[in];
	}
	const CHAR			&operator[](int in) const {
		DLOG((std::cout << "CStrA::operator[" << in << "] const" << std::endl))
		return	m_data->Buf()[in];
	}

	size_t				links() const {
		return	m_data->links();
	}
	size_t				capacity() const {
		return	m_data->capacity();
	}
	size_t				size() const {
		return	WinStr::Len(m_data->Buf());
	}
	bool				empty() const {
		return	(m_data && m_data->Buf() && (m_data->Buf()[0] == '\0'));
	}
	void				reserve(size_t size) {
		if (!m_data->IsSingle()) {
			WinBufferCtr<CHAR>	*tmp;
			Assign(tmp, m_data->Buf(), Max(m_data->capacity(), size));
			Swp(m_data, tmp);
			Release(tmp);
		}
		m_data->reserve(size);
	}

	operator			PCSTR() const {
		return	m_data->Buf();
	}
	PCSTR				c_str() const {
		return	m_data->Buf();
	}

	CStrA				&cp(PCWSTR in, UINT cp) {
		DLOG((std::cout << "CStrA::cp(\"PCWSTR in\", " << cp << ")" << std::endl))
		size_t len = ::WideCharToMultiByte(cp, 0, in, -1, NULL, 0, NULL, NULL);
		if (m_data->IsSingle()) {
			m_data->reserve(len);
		} else {
			WinBufferCtr<CHAR>	*tmp;
			tmp->Init(len);
			Swp(m_data, tmp);
			Release(tmp);
		}
		::WideCharToMultiByte(cp, 0, in, -1, m_data->Buf(), m_data->capacity(), NULL, NULL);
		return	*this;
	}
};
class		CStrW {
	WinBufferCtr<WCHAR>	*m_data;

	void				Alloc(WinBufferCtr<WCHAR>* &data, size_t size) {
		DLOG(std::cout << "CStrW::Alloc\tSize: " << size << std::endl;)
		data = (WinBufferCtr<WCHAR>*) WinMem::Alloc(sizeof(*data));
		data->Init(size);
	}
	void				Assign(WinBufferCtr<WCHAR>* &data, PCWSTR in, size_t size) {
		DLOG((std::cout << "CStrW::Assign\tSize: '" << size << "' \tData: '" << in << "'" << std::endl))
		data = (WinBufferCtr<WCHAR>*) WinMem::Alloc(sizeof(*data));
		data->Init(size);
		WinStr::Copy(data->Buf(), in, data->capacity());
	}
	void				Release(WinBufferCtr<WCHAR>* &data) {
		DLOG((std::cout << "CStrW::Release(\"" << data->Buf() << "\")"))
		if (data && data->Dec()) {
			DLOG((std::cout << "\tERASED"))
			WinMem::Free(data);
			data = NULL;
		}
		DLOG((std::cout << std::endl))
	}

public:
	~CStrW() {
		DLOG((std::cout << "CStrW::~CStrW(\"" << m_data->Buf() << "\")" << std::endl))
		Release(m_data);
	}
	explicit CStrW(): m_data(NULL) {
		DLOG((std::cout << "CStrW::CStrW()" << std::endl))
		Assign(m_data, L"", 1);
	}
	CStrW(const CStrW &in) {
		DLOG((std::cout << "CStrW::CStrW(\"" << in << "\")" << std::endl))
		in.m_data->Inc();
		m_data = in.m_data;
	}
	CStrW(PCWSTR in, size_t num = 0) {
		DLOG((std::cout << "CStrW::CStrW(\"" << in << "\")" << std::endl))
		if (num == 0)
			num = WinStr::Len(in);
		Assign(m_data, in, num + 1);
	}
	CStrW(PCSTR in, UINT cp) {
//		DLOG((cout << "CStrW::CStrW(\"" << in << "\", " << cp << ")" << std::endl))
		Alloc(m_data, ::MultiByteToWideChar(cp, 0, in, -1, NULL, 0));
		::MultiByteToWideChar(cp, 0, in, -1, m_data->Buf(), m_data->capacity());
	}
	CStrW(size_t in) {
		DLOG((std::cout << "CStrW::CStrW(\"" << in << "\")" << std::endl))
		Alloc(m_data, in);
	}

	const CStrW			&operator=(const CStrW &in) {
		DLOG((std::cout << "CStrW::operator=(\"" << in << "\")" << std::endl))
		if (m_data != in.m_data) {
			Release(m_data);
			m_data = in.m_data;
			m_data->Inc();
		}
		return	*this;
	}
	const CStrW			&operator=(PCWSTR in) {
		DLOG((std::cout << "CStrW::operator=(\"" << in << "\")" << std::endl))
		WinBufferCtr<WCHAR>	*tmp;
		Assign(tmp, in, WinStr::Len(in) + 1);
		Swp(m_data, tmp);
		Release(tmp);
		return	*this;
	}
	const CStrW			&operator=(WCHAR in) {
		DLOG((std::cout << "CStrW::operator=(\"" << in << "\")" << std::endl))
		WCHAR tmp[] = {in, L'\0'};
		operator+=(tmp);
		return	*this;
	}
	CStrW				&operator+=(const CStrW &in) {
		DLOG((std::cout << "CStrW::operator+=(\"" << in << "\")" << std::endl))
		if (m_data != in.m_data) {
			operator+=(in.m_data->Buf());
		}
		return	*this;
	}
	CStrW				&operator+=(PCWSTR in) {
		DLOG((std::cout << "CStrW::operator+=(\"" << in << "\")" << std::endl))
		size_t	len = m_data->capacity();
		if (m_data->IsSingle()) {
			m_data->reserve(len + WinStr::Len(in));
			WinStr::Cat(m_data->Buf(), in);
		} else {
			WinBufferCtr<WCHAR>	*tmp;
			Assign(tmp, m_data->Buf(), len + WinStr::Len(in));
			WinStr::Cat(tmp->Buf(), in);
			Swp(m_data, tmp);
			Release(tmp);
		}
		return	*this;
	}
	CStrW				&operator+=(WCHAR in) {
		DLOG((std::cout << "CStrW::operator+=(\"" << in << "\")" << std::endl))
		WCHAR tmp[] = {in, L'\0'};
		operator+=(tmp);
		return	*this;
	}
	CStrW				&operator+=(int in) {
		DLOG((std::cout << "CStrW::operator+=(\"" << in << "\")" << std::endl))
		CStrW	tmp(12);
		::_itow(in, (PWSTR)tmp.c_str(), 10);
		operator+=(tmp);
		return	*this;
	}
	CStrW				&operator+=(size_t in) {
		DLOG((std::cout << "CStrW::operator+=(\"" << in << "\")" << std::endl))
		CStrW	tmp(12);
		::_itow(in, (PWSTR)tmp.c_str(), 10);
		operator+=(tmp);
		return	*this;
	}

	CStrW				operator+(const CStrW &in) {
		DLOG((std::cout << "CStrW::operator+(\"" << in << "\")" << std::endl))
		CStrW	tmp(*this);
		tmp += in;
		return	tmp;
	}
	CStrW				operator+(PCWSTR in) {
		DLOG((std::cout << "CStrW::operator+(\"" << in << "\")" << std::endl))
		CStrW	tmp(*this);
		tmp += in;
		return	tmp;
	}
	CStrW				operator+(WCHAR in) {
		DLOG((std::cout << "CStrW::operator+(\"" << in << "\")" << std::endl))
		CStrW	tmp(*this);
		tmp += in;
		return	tmp;
	}

	bool				operator==(const CStrW &in) const {
		DLOG((std::cout << "CStrW::operator==(\"" << in << "\")" << std::endl))
		return	WinStr::Eq(m_data->Buf(), in.m_data->Buf());
	}
	bool				operator==(PCWSTR in) const {
		DLOG((std::cout << "CStrW::operator==(\"" << in << "\")" << std::endl))
		return	WinStr::Eq(m_data->Buf(), in);
	}

	bool				operator!=(const CStrW &in) const {
		DLOG((std::cout << "CStrW::operator!=(\"" << in << "\")" << std::endl))
		return	!operator==(in);
	}
	bool				operator!=(PCWSTR in) const {
		DLOG((std::cout << "CStrW::operator!=(\"" << in << "\")" << std::endl))
		return	!operator==(in);
	}

	bool				operator<(const CStrW &in) const {
		DLOG((std::cout << "CStrW::operator<(\"" << in << "\")" << std::endl))
		return	WinStr::Cmp(m_data->Buf(), in.m_data->Buf()) < 0;
	}
	bool				operator<(PCWSTR in) const {
		DLOG((std::cout << "CStrW::operator<(\"" << in << "\")" << std::endl))
		return	WinStr::Cmp(m_data->Buf(), in) < 0;
	}

	bool				operator>(const CStrW &in) const {
		DLOG((std::cout << "CStrW::operator>(\"" << in << "\")" << std::endl))
		return	WinStr::Cmp(m_data->Buf(), in.m_data->Buf()) > 0;
	}
	bool				operator>(PCWSTR in) const {
		DLOG((std::cout << "CStrW::operator>(\"" << in << "\")" << std::endl))
		return	WinStr::Cmp(m_data->Buf(), in) > 0;
	}

	bool				operator<=(const CStrW &in) const {
		DLOG((std::cout << "CStrW::operator<=(\"" << in << "\")" << std::endl))
		return	operator==(in) || operator<(in);
	}
	bool				operator<=(PCWSTR in) const {
		DLOG((std::cout << "CStrW::operator<=(\"" << in << "\")" << std::endl))
		return	operator==(in) || operator<(in);
	}

	bool				operator>=(const CStrW &in) const {
		DLOG((std::cout << "CStrW::operator>=(\"" << in << "\")" << std::endl))
		return	operator==(in) || operator>(in);
	}
	bool				operator>=(PCWSTR in) const {
		DLOG((std::cout << "CStrW::operator>=(\"" << in << "\")" << std::endl))
		return	operator==(in) || operator>(in);
	}

	WCHAR				&operator[](int in) {
		DLOG((std::cout << "CStrW::operator[" << in << "]" << std::endl))
		if (!m_data->IsSingle()) {
			WinBufferCtr<WCHAR>	*tmp;
			Assign(tmp, m_data->Buf(), m_data->capacity());
			Swp(m_data, tmp);
			Release(tmp);
		}
		return	m_data->Buf()[in];
	}
	const WCHAR			&operator[](int in) const {
		DLOG((std::cout << "CStrW::operator[" << in << "] const" << std::endl))
		return	m_data->Buf()[in];
	}

	size_t				links() const {
		return	m_data->links();
	}
	size_t				capacity() const {
		return	m_data->capacity();
	}
	size_t				size() const {
		return	WinStr::Len(m_data->Buf());
	}
	bool				empty() const {
		return	(m_data && m_data->Buf() && (m_data->Buf()[0] == '\0'));
	}
	void				reserve(size_t size) {
		if (!m_data->IsSingle()) {
			WinBufferCtr<WCHAR>	*tmp;
			Assign(tmp, m_data->Buf(), Max(m_data->capacity(), size));
			Swp(m_data, tmp);
			Release(tmp);
		}
		m_data->reserve(size);
	}

	operator			PCWSTR() const {
		return	m_data->Buf();
	}
	PCWSTR				c_str() const {
		return	m_data->Buf();
	}

	CStrW				&cp(PCSTR in, UINT cp) {
		DLOG((std::cout << "CStrW::cp(\"PCSTR in\", " << cp << ")" << std::endl))
		size_t len = ::MultiByteToWideChar(cp, 0, in, -1, NULL, 0);
		if (m_data->IsSingle()) {
			m_data->reserve(len);
		} else {
			WinBufferCtr<WCHAR>	*tmp;
			tmp->Init(len);
			Swp(m_data, tmp);
			Release(tmp);
		}
		::MultiByteToWideChar(cp, 0, in, -1, m_data->Buf(), m_data->capacity());
		return	*this;
	}
	CStrA				utf8() {
		CStrA	tmp(c_str(), CP_UTF8);
		return	tmp;
	}

	bool				cout() const {
		return	consoleout(*this, STD_OUTPUT_HANDLE);
	}
	bool				cerr() const {
		return	consoleout(*this, STD_ERROR_HANDLE);
	}

// static
	static CStrW		err(HRESULT err) {
		CStrW	Result(16 * 1024);
//		std::cout << "sixe: " << Result.capacity() << std::endl;
		PWSTR	buf = NULL;
		::FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(PWSTR)&buf, 0, NULL);
		Result = (buf) ? buf : L"Unknown error\r\n";
		::LocalFree(buf);
		Result[Result.size()-2] = L'\0';
		return	Result;
	}
	static CStrW		wmierr(HRESULT err) {
		if (err == 0) {
			return	CStrW::err(err);
		}
		PWSTR	buf = NULL;
		static HMODULE	mod = ::LoadLibraryW(L"wmiutils.dll");
		::FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
			mod,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(PWSTR)&buf, 0, NULL);
		CStrW	Result = (buf) ? buf : L"Unknown error\r\n";
		::LocalFree(buf);
		Result[Result.size()-2] = L'\0';
		return	Result;
	}
	static CStrW		time(const SYSTEMTIME &in, bool tolocal = true) {
		CStrW		Result(MAX_PATH);
		SYSTEMTIME	stTime;
		if (tolocal) {
			::SystemTimeToTzSpecificLocalTime(NULL, (SYSTEMTIME*)&in, &stTime);
		} else {
			stTime = in;
		}
		_snwprintf((PWSTR)Result.c_str(), Result.capacity(), L"%04d-%02d-%02d %02d:%02d",
				   stTime.wYear, stTime.wMonth, stTime.wDay,
				   stTime.wHour, stTime.wMinute);
		return	Result;
	}
	static CStrW		time(const FILETIME &in) {
		SYSTEMTIME	stUTC;
		::FileTimeToSystemTime(&in, &stUTC);
		return	time(stUTC);
	}
	static bool			consoleout(PCWSTR in, DWORD h) {
		HANDLE hStdOut = ::GetStdHandle(h);
		if (hStdOut != INVALID_HANDLE_VALUE) {
			DWORD	lpNumberOfCharsWritten;
			return	::WriteConsoleW(hStdOut, in, WinStr::Len(in), &lpNumberOfCharsWritten, NULL);
		}
		return	false;
	}
	static bool			consoleout(const CStrW &in, DWORD h) {
		HANDLE hStdOut = ::GetStdHandle(h);
		if (hStdOut != INVALID_HANDLE_VALUE) {
			DWORD	lpNumberOfCharsWritten;
			return	::WriteConsoleW(hStdOut, in.c_str(), in.size(), &lpNumberOfCharsWritten, NULL);
		}
		return	false;
	}
	static bool			cout(const CStrW &in) {
		return	consoleout(in, STD_OUTPUT_HANDLE);
	}
	bool				cerr(const CStrW &in) {
		return	consoleout(in, STD_ERROR_HANDLE);
	}
};

inline CStrW		Num2Str(int in, int base = 10) {
	CStrW	buf(12);
	::_itow(in, (PWSTR)buf.c_str(), base);
	return	buf.c_str();
}

#ifdef DEBUG
inline ostream		&operator<<(ostream &s, PCWSTR rhs) {
	CStrA	oem(rhs, CP_OEMCP);
	return	(s << oem.c_str());
}
inline ostream		&operator<<(ostream &s, const CStrW &rhs) {
	CStrA	oem(rhs, CP_OEMCP);
	return	(s << oem.c_str());
}
#endif

/*
class		BOM {
public:
	static	PCSTR	utf8() {
		static	const char bom_utf8[] = {'\xEF', '\xBB', '\xBF', '\xAA'};
		return	bom_utf8;
	}
	static	PCSTR	utf16le() {
		static	const char bom_utf16le[] = {'\xFF', '\xFE', '\xAA'};
		return	bom_utf16le;
	}
	static	PCSTR	utf16be() {
		static	const char bom_utf16be[] = {'\xFE', '\xFF', '\xAA'};
		return	bom_utf16be;
	}
	static	PCSTR	utf32le() {
		static	const char bom_utf32le[] = {'\xFF', '\xFE', '\0', '\0', '\xAA'};
		return	bom_utf32le;
	}
	static	PCSTR	utf32be() {
		static	const char bom_utf32be[] = {'\0', '\0', '\xFE', '\xFF', '\xAA'};
		return	bom_utf32be;
	}
//	00 00 FE FF	UTF-32, big-endian
//	FF FE 00 00	UTF-32, little-endian
//	FE FF	UTF-16, big-endian
//	FF FE	UTF-16, little-endian
//	EF BB BF	UTF-8
};
*/

///========================================================================================= WinFlag
/// Класс проверки и установки флагов
template<typename Type>
class		WinFlag {
public:
	static bool			Check(Type in, Type flag) {
		return	flag == (in & flag);
	}
	static Type		&Set(Type &in, Type flag) {
		return	in |= flag;
	}
	static Type		&UnSet(Type &in, Type flag) {
		return	in &= ~flag;
	}
};

///========================================================================================= WinFlag
/// Класс проверки и установки битов
template<typename Type>
class		WinBit {
	static size_t		BIT_LIMIT() {
		return	sizeof(Type) * 8;
	}
	static bool			BadBit(size_t in) {
		return	(in < 0) || (in >= BIT_LIMIT());
	}
public:
	static bool			Check(Type in, size_t bit) {
		if (BadBit(bit))
			return	false;
		Type tmp = 1;
		tmp <<= bit;
		return	(in & tmp);
	}
	static Type		&Set(Type &in, size_t bit) {
		if (BadBit(bit))
			return	in;
		Type	tmp = 1;
		tmp <<= bit;
		in |= tmp;
		return	in;
	}
	static Type		&UnSet(Type &in, size_t bit) {
		if (BadBit(bit))
			return	in;
		Type	tmp = 1;
		tmp <<= bit;
		in &= ~tmp;
		return	in;
	}
};

///======================================================================================== WinTimer
/// Оконный таймер
class		WinTimer {
	HANDLE			hTimer;
	LARGE_INTEGER	liUTC;
	long			lPeriod;

	void			Open() {
		Close();
		hTimer = ::CreateWaitableTimer(NULL, false, NULL);
	}
	void			Close() {
		if (hTimer) {
			::CloseHandle(hTimer);
			hTimer = NULL;
		}
	}
public:
	~WinTimer() {
		Stop();
		Close();
	}
	WinTimer(): hTimer(NULL) {
		Open();
		liUTC.QuadPart = 0LL;
		lPeriod = 0L;
	}
	WinTimer(LONGLONG time, long period = 0): hTimer(NULL) {
		Open();
		Set(time, period);
	}
	void			Set(LONGLONG time, long period = 0) {
		if (time == 0LL) {
			liUTC.QuadPart = -period * 10000LL;
		} else {
			liUTC.QuadPart = time;
		}
		lPeriod = period;
	}
	void			Start() {
		if (hTimer)
			::SetWaitableTimer(hTimer, &liUTC, lPeriod, NULL, NULL, false);
	}
	void			Stop() {
		if (hTimer)
			::CancelWaitableTimer(hTimer);
	}
	operator		HANDLE() const {
		return	hTimer;
	}
	void			StartTimer() {
		// объявляем свои локальные переменные
		FILETIME ftLocal, ftUTC;
		LARGE_INTEGER liUTC;

// таймер должен сработать в первый раз 1 января 2002 года в 1:00 PM но местному времени
		SYSTEMTIME st;
		::GetSystemTime(&st);
//		st.wOayOfWeek = 0;			// игнорируется
//		st.wHour = 0;				// 0 PM
//		st.wMinute = 0;				// 0 минут
		st.wSecond = 0;				// 0 секунд
		st.wMilliseconds = 0;		// 0 миллисекунд
		::SystemTimeToFileTime(&st, &ftLocal);
//		::LocalFileTimeToFilelime(&ttLocal, &ftUTC);

// преобразуем FILETIME в LARGE_INTEGER из-за различий в выравнивании данных
		liUTC.LowPart = ftUTC.dwLowDateTime;
		liUTC.HighPart = ftUTC.dwHighDateTime;
	}
};

///========================================================================================== WinEnv
/// Функции работы с
namespace	WinEnv {
	inline CStrW	Get(PCWSTR name) {
		CStrW	buf(::GetEnvironmentVariable(name, NULL, 0));
		::GetEnvironmentVariable(name, (PWSTR)buf.c_str(), buf.capacity());
		return	buf;
	}
	inline bool		Set(PCWSTR name, PCWSTR val) {
		return	::SetEnvironmentVariable(name, val);
	}
	inline bool		Add(PCWSTR name, PCWSTR val) {
		CStrW	buf(::GetEnvironmentVariable(name, NULL, 0) + WinStr::Len(val));
		::GetEnvironmentVariable(name, (PWSTR)buf.c_str(), buf.capacity());
		buf += val;
		return	::SetEnvironmentVariable(name, buf.c_str());
	}
	inline bool		Del(PCWSTR name) {
		return	::SetEnvironmentVariable(name, NULL);
	}
}

///=========================================================================================== WinFS
/// Работа с файловой системой (неокончено)
class		WinFilePos {
	LARGE_INTEGER	m_pos;
public:
	WinFilePos(LARGE_INTEGER pos): m_pos(pos) {
	}
	WinFilePos(LONGLONG pos = 0LL) {
		m_pos.QuadPart = pos;
	}
	operator		LONGLONG() const {
		return	m_pos.QuadPart;
	}
	operator		LARGE_INTEGER() const {
		return	m_pos;
	}
	operator		PLARGE_INTEGER() const {
		return	(PLARGE_INTEGER)&m_pos;
	}
};

namespace	WinFS {
	typedef		WIN32_FILE_ATTRIBUTE_DATA	FileInfo;
	inline	CStrW	ExtractPath(PCWSTR path) {
		size_t	len = WinStr::Len(path);
		PWSTR	ch = WinStr::CharLast((PWSTR)path, PATH_SEPARATOR_C);
		if (ch && ch < (path + len)) {
			return	CStrW(path, ch - path);
		}
		return	L"";
	}
	inline	CStrW	ExtractFile(PCWSTR path) {
		size_t	len = WinStr::Len(path);
		PWSTR	ch = WinStr::CharLast((PWSTR)path, PATH_SEPARATOR_C);
		if (ch && ++ch < (path + len)) {
			return	CStrW(ch);
		}
		return	L"";
	}

	inline bool		ValidName(PCWSTR path) {
		return	!(WinStr::Eq(path, L".") || WinStr::Eq(path, L"..") || WinStr::Eq(path, L"..."));
	}
	inline bool		IsExist(PCWSTR	path) {
		::GetFileAttributesW(path);
		return	::GetLastError() != ERROR_FILE_NOT_FOUND;
	}
	inline bool		IsDir(PCWSTR	path) {
		return	::GetFileAttributesW(path) & FILE_ATTRIBUTE_DIRECTORY;
	}
	inline bool		IsJunc(PCWSTR	path) {
		return	::GetFileAttributesW(path) & FILE_ATTRIBUTE_REPARSE_POINT;
	}

	inline CStrW	Expand(PCWSTR path) {
		CStrW	tmp(::ExpandEnvironmentStringsW(path, NULL, 0));
		::ExpandEnvironmentStringsW(path, (PWSTR)tmp.c_str(), tmp.capacity());
		return	tmp;
	}
	inline DWORD	GetAttr(PCWSTR	path) {
		return	::GetFileAttributesW(path);
	}
	inline FileInfo	GetInfo(PCWSTR	path) {
		FileInfo	info;
		::GetFileAttributesEx(path, GetFileExInfoStandard, &info);
		return	info;
	}
	inline LONGLONG	GetSize(PCWSTR	path) {
		FileInfo	info;
		::GetFileAttributesEx(path, GetFileExInfoStandard, &info);
		return	((LONGLONG)info.nFileSizeHigh) << 32 | info.nFileSizeLow;
	}
	inline LONGLONG	GetSize(HANDLE	hFile) {
		LARGE_INTEGER	size;
		::GetFileSizeEx(hFile, &size);
		return	size.QuadPart;
	}
	inline FILETIME	GetTimeCreate(PCWSTR	path) {
		FileInfo	info;
		::GetFileAttributesEx(path, GetFileExInfoStandard, &info);
		return	info.ftCreationTime;
	}
	inline FILETIME	GetTimeAccess(PCWSTR	path) {
		FileInfo	info;
		::GetFileAttributesEx(path, GetFileExInfoStandard, &info);
		return	info.ftLastAccessTime;
	}
	inline FILETIME	GetTimeWrite(PCWSTR	path) {
		FileInfo	info;
		::GetFileAttributesEx(path, GetFileExInfoStandard, &info);
		return	info.ftLastWriteTime;
	}

	inline bool		SetAttr(PCWSTR	path, DWORD attr) {
		return	::SetFileAttributes(path, attr);
	}

	inline bool		HardLink(PCWSTR path, PCWSTR newfile) {
		return	::CreateHardLinkW(newfile, path, NULL);
	}
	inline bool		Copy(PCWSTR path, PCWSTR dest) {
		return	::CopyFileW(path, dest, true);
	}
	inline bool		Move(PCWSTR path, PCWSTR dest) {
		return	::MoveFileExW(path, dest, MOVEFILE_COPY_ALLOWED);
	}
	inline bool		DelDir(PCWSTR	path) {
		::SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL);
		return	::RemoveDirectoryW(path);
	}
	inline bool		DelFile(PCWSTR	path) {
		::SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL);
		return	::DeleteFileW(path);
	}

	inline bool		FileOpenRead(PCWSTR	path, HANDLE &hFile) {
		hFile = ::CreateFileW(path, FILE_READ_DATA, FILE_SHARE_DELETE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
							  FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		return	hFile && hFile != INVALID_HANDLE_VALUE;
	}
	inline bool		FileClose(HANDLE hFile) {
		return	::CloseHandle(hFile);
	}
	inline bool		FilePos(HANDLE hFile, const WinFilePos &pos, DWORD m = FILE_BEGIN) {
		return	::SetFilePointerEx(hFile, pos, NULL, m);
	}
	inline LONGLONG	FilePos(HANDLE hFile) {
		WinFilePos	pos;
		return	::SetFilePointerEx(hFile, WinFilePos(0LL), pos, FILE_CURRENT);
	}
	inline LONGLONG	FileSize(HANDLE hFile) {
		DWORD		dwFileSizeHigh;
		LONGLONG	Result = ::GetFileSize(hFile, &dwFileSizeHigh);
		return	Result += (((LONGLONG)dwFileSizeHigh) << 32);
	}
	inline bool		FileRead(HANDLE hFile, PBYTE buf, DWORD &size) {
		return	::ReadFile(hFile, buf, size, &size, NULL);
	}
	inline bool		FileRead(PCWSTR	path, CStrA &buf) {
		bool	Result = false;
		HANDLE	hFile = ::CreateFileW(path, GENERIC_READ, 0, NULL, OPEN_EXISTING,
									 FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			DWORD	size = GetSize(hFile);
			buf.reserve(size);
			Result = ::ReadFile(hFile, (PSTR)buf.c_str(), buf.size(), &size, NULL);
			::CloseHandle(hFile);
		}
		return	Result;
	}
	inline bool		FileWrite(PCWSTR	path, const CStrA &buf) {
		bool	Result = false;
		HANDLE	hFile = ::CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
									 FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			DWORD	written;
			Result = ::WriteFile(hFile, buf.c_str(), buf.size(), &written, NULL);
			::CloseHandle(hFile);
		}
		return	Result;
	}
}

///========================================================================================= FileMap
/// Отображение файла в память блоками
class		FileMap : public Uncopyable, public WinErrorCheck {
	HANDLE		m_hFile;
	HANDLE		m_hSect;
	PVOID		m_data;
	LONGLONG	m_sizefull;
	LONGLONG	m_offset;
	size_t		m_binb;
	bool		m_write;
public:
	~FileMap() {
		Close();
	}
	FileMap(): m_hFile(NULL), m_hSect(NULL), m_data(NULL), m_sizefull(0), m_offset(0) {
		err(ERROR_FILE_NOT_FOUND);
	}
	FileMap(PCWSTR in, bool write = false): m_hFile(NULL), m_hSect(NULL), m_data(NULL), m_sizefull(0), m_offset(0) {
		Open(in, write);
	}

	bool			Close() {
		if (m_data) {
			::UnmapViewOfFile(m_data);
			m_data = NULL;
		}
		if (m_hSect) {
			::CloseHandle(m_hSect);
			m_hSect = NULL;
		}
		if (m_hFile) {
			::CloseHandle(m_hFile);
			m_hFile = NULL;
			return	true;
		}
		return	false;
	}
	bool			Open(PCWSTR in, bool write) {
		m_write	= write;
		ACCESS_MASK	amask = (m_write) ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ;
		DWORD		share = (m_write) ? 0 : FILE_SHARE_DELETE | FILE_SHARE_READ;
		DWORD		creat = (m_write) ? OPEN_EXISTING : OPEN_EXISTING;
		DWORD		flags = (m_write) ? FILE_ATTRIBUTE_NORMAL : FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS;
		Close();
		Home();
		m_hFile = ::CreateFileW(in, amask, share, NULL, creat, flags, NULL);
		if (m_hFile && m_hFile != INVALID_HANDLE_VALUE) {
			m_sizefull = WinFS::FileSize(m_hFile);
			amask = (m_write) ? PAGE_READWRITE : PAGE_READONLY;
			m_hSect = ::CreateFileMapping(m_hFile, NULL, amask, 0, 0, NULL);
		}
		return	ChkSucc(m_hSect);
	}
	bool			Create(PCWSTR in, LONGLONG size) {
		m_write	= true;
		ACCESS_MASK	amask = GENERIC_READ | GENERIC_WRITE;
		DWORD		share = 0;
		DWORD		creat = CREATE_ALWAYS;
		DWORD		flags = FILE_ATTRIBUTE_NORMAL;
		Close();
		Home();
		m_hFile = ::CreateFileW(in, amask, share, NULL, creat, flags, NULL);
		if (m_hFile && m_hFile != INVALID_HANDLE_VALUE) {
			m_sizefull = size;
			LONG dwHigh = (m_sizefull >> 32);
			::SetFilePointer(m_hFile, (LONG)(m_sizefull & 0xFFFFFFFF), &dwHigh, FILE_BEGIN);
			::SetEndOfFile(m_hFile);
			amask = (m_write) ? PAGE_READWRITE : PAGE_READONLY;
			m_hSect = ::CreateFileMapping(m_hFile, NULL, amask, 0, 0, NULL);
		}
		return	ChkSucc(m_hSect);
	}

	bool			Next() {
		if (m_data) {
			::UnmapViewOfFile(m_data);
			m_data = NULL;
		}
		if ((m_sizefull - m_offset) > 0) {
			if ((m_sizefull - m_offset) < m_binb)
				m_binb = m_sizefull - m_offset;
			if (m_hSect) {
				ACCESS_MASK	amask = (m_write) ? FILE_MAP_WRITE : FILE_MAP_READ;
				m_data = ::MapViewOfFile(m_hSect, amask, (DWORD)(m_offset >> 32), (DWORD)(m_offset & 0xFFFFFFFF), m_binb);
				m_offset += m_binb;
				return	ChkSucc(m_data);
			}
		}
		return	false;
	}
	bool			Home() {
		SYSTEM_INFO	info;
		::GetSystemInfo(&info);
		m_binb = info.dwAllocationGranularity * 1024;
		m_offset = 0LL;
		return	true;
	}

	LONGLONG		sizefile() const {
		return	m_sizefull;
	}
	size_t			size() const {
		return	m_binb;
	}
	PVOID			data() const {
		return	m_data;
	}
};

///========================================================================================= WinPriv
/// Функции работы с привилегиями
namespace	WinPriv {
	bool inline		IsExist(HANDLE hToken, LUID priv) {
		bool	Result = false;

		DWORD	dwSize = 0;
		// определяем размер буфера, необходимый для получения всех привилегий
		if (!::GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &dwSize) && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			// выделяем память для выходного буфера
			WinBuffer<PTOKEN_PRIVILEGES>	ptp(dwSize);
			if (::GetTokenInformation(hToken, TokenPrivileges, ptp, ptp.capacity(), &dwSize)) {
				// проходим по списку привилегий и проверяем, есть ли в нем указанная привилегия
				for (DWORD i = 0; i < ptp->PrivilegeCount; ++i) {
					if (ptp->Privileges[i].Luid.LowPart == priv.LowPart && ptp->Privileges[i].Luid.HighPart == priv.HighPart) {
						Result = true;
						break;
					}
				}
			}
		}
		return	Result;
	}
	bool inline		IsExist(HANDLE hToken, PCWSTR sPriv) {
		LUID	luid;
		if (::LookupPrivilegeValueW(NULL, sPriv, &luid))
			return	IsExist(hToken, luid);
		return	false;
	}
	bool inline		IsExist(HANDLE hToken, PCSTR sPriv) {
		LUID	luid;
		if (::LookupPrivilegeValueA(NULL, sPriv, &luid))
			return	IsExist(hToken, luid);
		return	false;
	}

	bool inline		IsEnabled(HANDLE hToken, LUID priv) {
		BOOL	Result = false;

		PRIVILEGE_SET		ps;
		WinMem::Zero(ps);
		ps.PrivilegeCount = 1;
		ps.Privilege[0].Luid = priv;
		::PrivilegeCheck(hToken, &ps, &Result);
		return	Result;
	}
	bool inline		IsEnabled(HANDLE hToken, PCWSTR sPriv) {
		LUID	luid;
		// получаем идентификатор привилегии
		if (::LookupPrivilegeValueW(NULL, sPriv, &luid))
			return	IsEnabled(hToken, luid);
		return	false;
	}
	bool inline		IsEnabled(HANDLE hToken, PCSTR sPriv) {
		LUID	luid;
		// получаем идентификатор привилегии
		if (::LookupPrivilegeValueA(NULL, sPriv, &luid))
			return	IsEnabled(hToken, luid);
		return	false;
	}

	bool inline		Modify(HANDLE hToken, LUID priv, bool bEnable) {
		bool	Result = false;
		TOKEN_PRIVILEGES	tp;
		WinMem::Zero(tp);
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = priv;
		tp.Privileges[0].Attributes = (bEnable) ? SE_PRIVILEGE_ENABLED : 0;

		if (::AdjustTokenPrivileges(hToken, false, &tp, sizeof(tp), NULL, NULL))
			Result = true;
		return	Result;
	}
	bool inline		Modify(HANDLE hToken, PCWSTR sPriv, bool bEnable) {
		LUID	luid;
		if (::LookupPrivilegeValueW(NULL, sPriv, &luid))
			return	Modify(hToken, luid, bEnable);
		return	false;
	}
	bool inline		Modify(HANDLE hToken, PCSTR	sPriv, bool bEnable) {
		LUID	luid;
		if (::LookupPrivilegeValueA(NULL, sPriv, &luid))
			return	Modify(hToken, luid, bEnable);
		return	false;
	}
}

///======================================================================================== WinToken
/// Обертка хэндла процесса
class		WinProc: public Uncopyable, public WinErrorCheck {
	HANDLE	m_handle;
public:
	~WinProc() {
		::CloseHandle(m_handle);
	}
	WinProc() {
		m_handle = ::GetCurrentProcess();
	}
	WinProc(ACCESS_MASK mask, DWORD pid): m_handle(NULL) {
		m_handle = ::OpenProcess(mask, false, pid);
		ChkSucc(m_handle);
	}
	operator		HANDLE() const {
		return	m_handle;
	}
	DWORD			GetId() const {
		return	::GetProcessId(m_handle);
	}

// static
	static	DWORD	Id() {
		return	::GetCurrentProcessId();
	}
	static	DWORD	Id(HANDLE hProc) {
		return	::GetProcessId(hProc);
	}
	static	CStrW	User() {
		DWORD	size = 0;
		::GetUserNameW(NULL, &size);
		CStrW	buf(size);
		::GetUserNameW((PWSTR)buf.c_str(), &size);
		return	buf;
	}
	static	CStrW	Path() {
		CStrW	Result(MAX_PATH);
		DWORD	size = ::GetModuleFileNameW(NULL, (PWSTR)Result.c_str(), Result.capacity());
		if (size > Result.capacity()) {
			Result.reserve(size);
			::GetModuleFileNameW(NULL, (PWSTR)Result.c_str(), Result.capacity());
		}
		return	Result;
	}
};

///======================================================================================== WinToken
/// Обертка токена
class		WinToken: public Uncopyable, public WinErrorCheck {
	HANDLE	m_handle;
public:
	~WinToken() {
		DLOG((std::cout << "WinToken::~WinToken()" << std::endl))
		::CloseHandle(m_handle);
	}
	WinToken(ACCESS_MASK mask = TOKEN_ALL_ACCESS): m_handle(NULL) {
		DLOG((std::cout << "WinToken::WinToken(\"" << mask << "\")" << std::endl))
		ChkSucc(::OpenProcessToken(WinProc(), mask, &m_handle));
	}
	WinToken(ACCESS_MASK mask, HANDLE hProcess): m_handle(NULL) {
		DLOG((std::cout << "WinToken::WinToken(\"" << mask << ", " << hProcess << "\")" << std::endl))
		ChkSucc(::OpenProcessToken(hProcess, mask, &m_handle));
	}
	operator		HANDLE() const {
		return	m_handle;
	}
};

///=========================================================================================== Win64
/// Функции работы с WOW64
namespace	Win64 {
	inline bool					DisableWOW() {
		bool Result = false;
		HINSTANCE hDLL = ::LoadLibraryA("Kernel32.dll");
		if (hDLL != NULL) {
			typedef bool (__cdecl * MYPROC)(PVOID*);
			MYPROC func = (MYPROC)::GetProcAddress(hDLL, "Wow64DisableWow64FsRedirection");
			if (func != NULL) {
				PVOID OldValue;
				(func)(&OldValue);
				Result = true;
			}
			::FreeLibrary(hDLL);
		}
		return	Result;
	}
	inline bool					IsWOW64() {
		typedef BOOL (WINAPI * PFN_ISWOW64PROCESS)(HANDLE, PBOOL);
		BOOL	Result = false;
		PFN_ISWOW64PROCESS fnIsWow64Process = (PFN_ISWOW64PROCESS) ::GetProcAddress(::GetModuleHandleW(L"kernel32"), "IsWow64Process");
		if (fnIsWow64Process) {
			if (!fnIsWow64Process(::GetCurrentProcess(), &Result)) {
				return	false;
			}
		}
		return	Result;
	}
}

///========================================================================================== WinCom
namespace	WinNet {
	bool inline	GetCompName(CStrW &buf, COMPUTER_NAME_FORMAT cnf) {
		DWORD	size = 0;
		::GetComputerNameExW(cnf, NULL, &size);
		buf.reserve(size);
		return	::GetComputerNameExW(cnf, (PWSTR)buf.c_str(), &size);
	}
	bool inline	SetCompName(const CStrW &in, COMPUTER_NAME_FORMAT cnf) {
		return	::SetComputerNameExW(cnf, in.c_str());
	}
}

///========================================================================================== WinCom
/// Класс инициализации COM (объекты создавать запрещено, нужно использовать фукцию IsOK)
class		WinCOM {
	bool		m_err;

	WinCOM(const WinCOM&);
	WinCOM(): m_err(true) {
		if (m_err) {
			m_err = !SUCCEEDED(::CoInitializeEx(NULL, COINIT_MULTITHREADED));
		}
	}
public:
	~WinCOM() {
		if (IsOK()) {
			::CoUninitialize();
			the().m_err = true;
		}
	}
	static WinCOM	&the() {
		static WinCOM com;
		return	com;
	}
	static bool		IsOK() {
		return	!the().m_err;
	}
};

#endif // WIN_DEF_HPP
