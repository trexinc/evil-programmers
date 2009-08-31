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
	UINT id;
	UINT start;
	BYTE pr;

	IconItem(): id(0), start(0), pr(111) {}
	IconItem(UINT id_, UINT st_): id(id_), start(st_), pr(111) {}
};

#endif // BCI_HPP
