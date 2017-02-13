#include "headers.hpp"
#pragma hdrstop

#include "hide_cursor.hpp"

hide_cursor::hide_cursor()
{
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &m_Cci);
	CONSOLE_CURSOR_INFO cciNew { m_Cci.dwSize, FALSE };
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cciNew);
}

hide_cursor::~hide_cursor()
{
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &m_Cci);
}
