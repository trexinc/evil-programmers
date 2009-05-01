#pragma once

#include "headers.hpp"

class HideCursor
{
	CONSOLE_CURSOR_INFO cci;
public:
	HideCursor();
	~HideCursor();
};
