#pragma once

enum class color
{
	black,
	dark_blue,
	dark_green,
	dark_cyan,
	dark_red,
	dark_magenta,
	dark_yellow,
	grey,
	dark_grey,
	blue,
	green,
	cyan,
	red,
	magenta,
	yellow,
	white
};

class text_color
{
public:
	text_color(color NewColor);
	~text_color();

private:
	WORD m_InitialColor;
};
