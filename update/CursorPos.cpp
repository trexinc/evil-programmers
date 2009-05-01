#include "headers.hpp"

#include "CursorPos.hpp"

CursorPos::CursorPos()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&csbi);
	Pos=csbi.dwCursorPosition;
};

CursorPos::~CursorPos()
{
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),Pos);
}
