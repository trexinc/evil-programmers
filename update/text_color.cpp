#include "headers.hpp"
#pragma hdrstop

#include "text_color.hpp"

text_color::text_color(color NewColor)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	m_InitialColor = csbi.wAttributes;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(NewColor));
}

text_color::~text_color()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), m_InitialColor);
}
