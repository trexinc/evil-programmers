/**
    bci.h
    Copyright (C) 2009 GrAnD

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
**/
#ifndef BCI_HPP
#define BCI_HPP

#include <windows.h>

struct IconItem {
	size_t id;
	size_t start;
	BYTE pr;

	IconItem(): id(0), start(0), pr(111) {}
	IconItem(size_t id_, size_t st_): id(id_), start(st_), pr(111) {}
};

void	inline		XchgByte(WORD &inout) {
	inout = inout >> 8 | inout << 8;
}
void	inline		XchgWord(DWORD &inout) {
	inout = inout >> 16 | inout << 16;
}

template <typename Type>
inline	const Type&	Min(const Type &a, const Type &b) {
	return (a < b) ? a : b;
}
template <typename Type>
inline	const Type&	Max(const Type &a, const Type &b) {
	return (a < b) ? b : a;
}
template <typename Type>
inline	void		Swp(Type &x, Type &y) {
	Type tmp(x);
	x = y;
	y = tmp;
}

///========================================================================================== WinMem
PVOID	inline		MemAlloc(size_t size) {
	return ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}
PVOID	inline		MemRealloc(PCVOID in, size_t size) {
	return ::HeapReAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, (PVOID)in, size);
}
void	inline		MemFree(PCVOID in) {
	::HeapFree(::GetProcessHeap(), 0, (PVOID)in);
}
void	inline		MemCopy(PVOID dest, PCVOID sour, size_t size) {
	::CopyMemory(dest, sour, size);
}
void	inline		MemFill(PVOID in, size_t size, char fill) {
	::FillMemory(in, size, (BYTE)fill);
}
void	inline		MemZero(PVOID in, size_t size) {
	::ZeroMemory(in, size);
}
template<typename Type>
void	inline		MemFill(Type &in, char fill) {
	::FillMemory(&in, sizeof(in), (BYTE)fill);
}
template<typename Type>
void	inline		MemZero(Type &in) {
	::ZeroMemory(&in, sizeof(in));
}

#endif // BCI_HPP
