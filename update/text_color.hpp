#pragma once

enum class color
{
	black        = 0,
	dark_blue    = FOREGROUND_BLUE,
	dark_green   = FOREGROUND_GREEN,
	dark_cyan    = dark_blue | dark_green,
	dark_red     = FOREGROUND_RED,
	dark_magenta = dark_red | dark_blue,
	dark_yellow  = dark_red | dark_green,
	grey         = dark_red | dark_green | dark_blue,
	dark_grey    = black | FOREGROUND_INTENSITY,
	blue         = dark_blue | FOREGROUND_INTENSITY,
	green        = dark_green | FOREGROUND_INTENSITY,
	cyan         = dark_cyan | FOREGROUND_INTENSITY,
	red          = dark_red | FOREGROUND_INTENSITY,
	magenta      = dark_magenta | FOREGROUND_INTENSITY,
	yellow       = dark_yellow | FOREGROUND_INTENSITY,
	white        = grey | FOREGROUND_INTENSITY,
};

class text_color
{
public:
	text_color(color NewColor);
	~text_color();

private:
	WORD m_InitialColor;
};
