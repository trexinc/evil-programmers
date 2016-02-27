#pragma once

#include "headers.hpp"

class TextColor
{
public:
	TextColor(WORD NewColor);
	~TextColor();

private:
	WORD InitialColor;
};
