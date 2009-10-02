/**
	bcn.cpp
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
#include "bcn.h"

PCWSTR		WindowName	= L"BCI2";
PCWSTR		EventName	= L"Global\\BCI2";

int WINAPI		Notify(unsigned long Msg, void*) {
	HANDLE	hEvent = ::OpenEvent(EVENT_MODIFY_STATE, false, EventName);

	if (hEvent) {
		if (Msg == BCSVC_START_JOB)
			::SetEvent(hEvent);
		::CloseHandle(hEvent);
	}
	return	0;
}

BOOL WINAPI		DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
	}
	return	TRUE;
}
