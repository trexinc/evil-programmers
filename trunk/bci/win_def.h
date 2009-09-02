/**
	win_def.h

	main windows application include, always include first
	Copyright (C) 2009 GrAnD
	@link ()
**/

#ifndef WIN_DEF_HPP
#define WIN_DEF_HPP

#if (_WIN32_WINNT < 0x0501)
#undef _WIN32_WINNT
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501		// Windows 2000 or later
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

///===================================================================================== definitions
#define sizeofa(array) (sizeof(array)/sizeof(array[0]))

typedef const TCHAR *PCTSTR;
typedef const void	*PCVOID;

///=================================================================================================
inline void						XchgByte(WORD &inout) {
	inout = inout >> 8 | inout << 8;
}
inline void						XchgWord(DWORD &inout) {
	inout = inout >> 16 | inout << 16;
}

template <typename Type>
inline	const Type&				Min(const Type &a, const Type &b) {
	return (a < b) ? a : b;
}
template <typename Type>
inline	const Type&				Max(const Type &a, const Type &b) {
	return (a < b) ? b : a;
}
template <typename Type>
inline	void					Swp(Type &x, Type &y) {
	Type tmp(x);
	x = y;
	y = tmp;
}

///========================================================================================== WinStr
namespace	WinStr {
	inline bool					Eq(PCSTR in1, PCSTR in2) {
		return	::strcmp(in1, in2) == 0;
	}
	inline bool					Eq(PCWSTR in1, PCWSTR in2) {
		return	::wcscmp(in1, in2) == 0;
	}
	inline bool					Eqi(PCSTR in1, PCSTR in2) {
		return	::_stricmp(in1, in2) == 0;
	}
	inline bool					Eqi(PCWSTR in1, PCWSTR in2) {
		return	::_wcsicmp(in1, in2) == 0;
	}
	inline int					Cmp(PCSTR in1, PCSTR in2) {
		return	::strcmp(in1, in2);
	}
	inline int					Cmp(PCWSTR in1, PCWSTR in2) {
		return	::wcscmp(in1, in2);
	}
	inline int					Cmpi(PCSTR in1, PCSTR in2) {
		return	::_stricmp(in1, in2);
	}
	inline int					Cmpi(PCWSTR in1, PCWSTR in2) {
		return	::_wcsicmp(in1, in2);
	}
	inline size_t				Len(PCSTR in) {
		return	::strlen(in);
	}
	inline size_t				Len(PCWSTR in) {
		return	::wcslen(in);
	}
	inline PSTR					Copy(PSTR dest, PCSTR src) {
		return	::strcpy(dest, src);
	}
	inline PWSTR				Copy(PWSTR dest, PCWSTR src) {
		return	::wcscpy(dest, src);
	}
	inline PSTR					Copy(PSTR dest, PCSTR src, size_t size) {
		return	::strncpy(dest, src, size);
	}
	inline PWSTR				Copy(PWSTR dest, PCWSTR src, size_t size) {
		return	::wcsncpy(dest, src, size);
	}

	inline long long			AsLongLong(PCSTR in) {
		return	_atoi64(in);
	}
	inline unsigned long		AsULong(PCSTR in) {
		return	(unsigned long)_atoi64(in);
	}
	inline long					AsLong(PCSTR in) {
		return	(long)_atoi64(in);
	}
	inline unsigned int			AsUInt(PCSTR in) {
		return	(unsigned int)_atoi64(in);
	}
	inline int					AsInt(PCSTR in) {
		return	(int)_atoi64(in);
	}

	inline long long			AsLongLong(PCWSTR in) {
		return	_wtoi64(in);
	}
	inline unsigned long		AsULong(PCWSTR in) {
		return	(unsigned long)_wtoi64(in);
	}
	inline long					AsLong(PCWSTR in) {
		return	_wtol(in);
	}
	inline unsigned int			AsUInt(PCWSTR in) {
		return	(unsigned int)_wtol(in);
	}
	inline int					AsInt(PCWSTR in) {
		return	_wtoi(in);
	}
}

///========================================================================================== WinMem
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

	inline void					Copy(PVOID dest, PCVOID sour, size_t size) {
		::CopyMemory(dest, sour, size);
	}
	inline void					Fill(PVOID in, size_t size, char fill) {
		::FillMemory(in, size, (BYTE)fill);
	}
	inline void					Zero(PVOID in, size_t size) {
		::ZeroMemory(in, size);
	}
	template<typename Type>
	inline void					Fill(Type &in, char fill) {
		::FillMemory(&in, sizeof(in), (BYTE)fill);
	}
	template<typename Type>
	inline void					Zero(Type &in) {
		::ZeroMemory(&in, sizeof(in));
	}
}

///============================================================================================ CStr
class CStrA {
	typedef		char		cType;
	typedef		char*		pType;
	typedef		const char*	pcType;
	pType	mutable	st_;
	size_t	mutable	sz_;

	void			Assign(pcType in, size_t size = 0) {
		Free();
		size = Max(size, Len(in));
		Alloc(size);
		Cpy(st_, in, sz_);
	}
	bool			CheckSize(size_t size) const {
		if (size >= sz_) {
			sz_ = size;
			Realloc(sz_);
			return false;
		}
		return true;
	}

	void			Alloc(size_t size) {
		sz_ = size + 1;
		st_ = (pType)WinMem::Alloc((sz_) * sizeof(cType));
	}
	void			Realloc(size_t size) const {
		sz_ = size + 1;
		st_ = (pType)WinMem::Realloc(st_, (sz_) * sizeof(cType));
	}
	void			Free() {
		if (st_)
			WinMem::Free(st_);
		st_ = NULL;
		sz_ = 0;
	}
	size_t			Len(pcType in) const {
		return	WinStr::Len(in);
	}
	int				Cmp(pcType in1, pcType in2) const {
		return	WinStr::Cmp(in1, in2);
	}
	int				Cmpi(pcType in1, pcType in2) const {
		return ::lstrcmpiA(in1, in2);
	}
	pType			Cpy(pType dest, pcType in, size_t size) const {
		return WinStr::Copy(dest, in, size);
	}
public:
	~CStrA() {
		Free();
	}
	CStrA(size_t size = MAX_PATH): st_(NULL) {
		Alloc(size);
	}
	CStrA(const CStrA &in): st_(NULL) {
		Assign(in.st_);
	}
	CStrA(const CStrA &in, size_t size): st_(NULL) {
		Assign(in.st_, size);
	}
	CStrA(pcType in): st_(NULL) {
		Assign(in, Len(in));
	}
	const CStrA&	operator=(const CStrA &in) {
		Assign(in.st_, in.sz_);
		return	*this;
	}
	const CStrA&	operator=(pcType in) {
		Assign(in, Len(in));
		return	*this;
	}

	const CStrA&	operator+=(pcType in) {
		if (st_ != in) {
			size_t len1 = Len(st_), len2 = Len(in);
			CheckSize(len1 + len2);
			Cpy(st_ + len1, in, sz_ - len1);
		}
		return *this;
	}
	CStrA			operator+(const CStrA &in) const {
		CStrA Result(*this);
		return Result += in;
	}

	bool			operator==(const CStrA &in) const {
		return Cmp(st_, in.st_) == 0;
	}
	bool			operator!=(const CStrA &in) const {
		return !operator==(in);
	}
	bool			operator<(const CStrA &in) const {
		return Cmp(st_, in.st_) < 0;
	}
	bool			operator>(const CStrA &in) const {
		return Cmp(st_, in.st_) > 0;
	}
	bool			operator<=(const CStrA &in) const {
		return operator==(in) || operator<(in);
	}
	bool			operator>=(const CStrA &in) const {
		return operator==(in) || operator>(in);
	}

	cType&			operator[](int in) {
		return st_[in];
	}

	operator		pType() const {
		return	(pType)st_;
	}
	operator		pcType() const {
		return	(pcType)st_;
	}
	size_t			Len() const {
		return	Len(st_);
	}
	size_t			Size() const {
		return	sz_;
	}
	pcType			c_str() const {
		return	(pcType)st_;
	}
	pType			Data() const {
		return	(pType)st_;
	}
};
class CStrW {
	typedef		wchar_t			cType;
	typedef		wchar_t*		pType;
	typedef		const wchar_t*	pcType;
	pType	mutable	st_;
	size_t	mutable	sz_;

	void			Assign(pcType in, size_t size = 0) {
		Free();
		size = Max(size, Len(in));
		Alloc(size);
		Cpy(st_, in, sz_);
	}
	bool			CheckSize(size_t size) const {
		if (size >= sz_) {
			sz_ = size;
			Realloc(sz_);
			return false;
		}
		return true;
	}

	void			Alloc(size_t size) {
		sz_ = size + 1;
		st_ = (pType)WinMem::Alloc((sz_) * sizeof(cType));
	}
	void			Realloc(size_t size) const {
		sz_ = size + 1;
		st_ = (pType)WinMem::Realloc(st_, (sz_) * sizeof(cType));
	}
	void			Free() {
		if (st_)
			WinMem::Free(st_);
		st_ = NULL;
		sz_ = 0;
	}
	size_t			Len(pcType in) const {
		return	WinStr::Len(in);
	}
	int				Cmp(pcType in1, pcType in2) const {
		return	WinStr::Cmp(in1, in2);
	}
	int				Cmpi(pcType in1, pcType in2) const {
		return ::lstrcmpiW(in1, in2);
	}
	pType			Cpy(pType dest, pcType in, size_t size) const {
		return	WinStr::Copy(dest, in, size);
	}
public:
	~CStrW() {
		Free();
	}
	CStrW(size_t size = MAX_PATH): st_(NULL) {
		Alloc(size);
	}
	CStrW(const CStrW &in): st_(NULL) {
		Assign(in.st_);
	}
	CStrW(const CStrW &in, size_t size): st_(NULL) {
		Assign(in.st_, size);
	}
	CStrW(pcType in): st_(NULL) {
		Assign(in, Len(in));
	}
	const CStrW&	operator=(const CStrW &in) {
		Assign(in.st_, in.sz_);
		return	*this;
	}
	const CStrW&	operator=(pcType in) {
		Assign(in, Len(in));
		return	*this;
	}

	const CStrW&	operator+=(pcType in) {
		if (st_ != in) {
			size_t len1 = Len(st_), len2 = Len(in);
			CheckSize(len1 + len2);
			Cpy(st_ + len1, in, sz_ - len1);
		}
		return *this;
	}
	CStrW			operator+(const CStrW &in) const {
		CStrW Result(*this);
		return Result += in;
	}

	bool			operator==(const CStrW &in) const {
		return Cmp(st_, in.st_) == 0;
	}
	bool			operator!=(const CStrW &in) const {
		return !operator==(in);
	}
	bool			operator<(const CStrW &in) const {
		return Cmp(st_, in.st_) < 0;
	}
	bool			operator>(const CStrW &in) const {
		return Cmp(st_, in.st_) > 0;
	}
	bool			operator<=(const CStrW &in) const {
		return operator==(in) || operator<(in);
	}
	bool			operator>=(const CStrW &in) const {
		return operator==(in) || operator>(in);
	}

	cType&			operator[](int in) {
		return st_[in];
	}

	operator		pType() const {
		return	(pType)st_;
	}
	operator		pcType() const {
		return	(pcType)st_;
	}
	size_t			Len() const {
		return	Len(st_);
	}
	size_t			Size() const {
		return	sz_;
	}
	pcType			c_str() const {
		return	(pcType)st_;
	}
	pType			Data() const {
		return	(pType)st_;
	}
};

#ifdef UNICODE
#define CStr CStrW
#else
#define CStr CStrA
#endif

///====================================================================================== Uncopyable
class Uncopyable {
	Uncopyable(const Uncopyable&);
	Uncopyable& operator=(const Uncopyable&);
protected:
	~Uncopyable() {
	}
	Uncopyable() {
	}
};

///=================================================================================== WinErrorCheck
class WinErrorCheck {
	HRESULT	mutable	err_;
protected:
	~WinErrorCheck() {
	}
	WinErrorCheck(): err_(NO_ERROR) {
	}
public:
	HRESULT			GetErr() const {
		return err_;
	}
	HRESULT			SetErr() const {
		return err_ =::GetLastError();
	}
	HRESULT			SetErr(HRESULT err) const {
		return err_ = err;
	}
	bool			IsOK() const {
		return err_ == NO_ERROR;
	}
	bool			ChkSucc(bool in) const {
		if (!in)
			SetErr();
		else
			SetErr(NO_ERROR);
		return in;
	}
//	bool			ChkFail(bool in) const {
//		return ChkSucc(!in);
//	}
	template<typename Type>
	void			SetIfFail(Type &in, const Type &value) {
		if (!IsOK())
			in = value;
	}
};

#endif // WIN_DEF_HPP
