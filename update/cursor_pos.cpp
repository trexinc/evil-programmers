#include "headers.hpp"
#pragma hdrstop

#include "cursor_pos.hpp"

cursor_pos::cursor_pos()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	Pos = csbi.dwCursorPosition;
}

cursor_pos::~cursor_pos()
{
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}
