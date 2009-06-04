/**
    bcsvc_interface.h
    Copyright (C) 2000-2009 zg

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

#ifndef __BCSVC_INTERFACE_H__
#define __BCSVC_INTERFACE_H__

#define BCSVC_START      0UL
#define BCSVC_STOP       1UL
#define BCSVC_START_JOB  2UL
#define BCSVC_STOP_JOB   3UL

#include <windows.h>

#define DLL_EXPORT __declspec(dllexport)

struct BCSvcJob {
	unsigned long StructSize;
	wchar_t JobName[2*MAX_PATH];
};

#ifdef __cplusplus
extern "C" {
#endif
//	int DLL_EXPORT WINAPI Notify(unsigned long Msg, void *Data);
	int WINAPI Notify(unsigned long Msg, void *Data);
#ifdef __cplusplus
}
#endif

#endif
