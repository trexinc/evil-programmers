#pragma once

#include "headers.hpp"

class TextColor
{
	WORD InitialColor;
public:
	TextColor(WORD NewColor);
	~TextColor();
};
