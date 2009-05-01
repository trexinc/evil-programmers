#include "headers.hpp"

#include "HideCursor.hpp"

HideCursor::HideCursor()
{
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&cci);
	CONSOLE_CURSOR_INFO cciNew={cci.dwSize,FALSE};
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&cciNew);
}

HideCursor::~HideCursor()
{
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&cci);
}
