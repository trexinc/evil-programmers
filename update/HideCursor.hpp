#pragma once

#include "headers.hpp"

class HideCursor
{
public:
	HideCursor();
	~HideCursor();

private:
	CONSOLE_CURSOR_INFO cci;
};
