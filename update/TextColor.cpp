#include "headers.hpp"

#include "TextColor.hpp"

TextColor::TextColor(WORD NewColor)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&csbi);
	InitialColor=csbi.wAttributes;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),NewColor);
}

TextColor::~TextColor()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),InitialColor);
}
