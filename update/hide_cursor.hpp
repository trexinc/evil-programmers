#pragma once

class hide_cursor
{
public:
	hide_cursor();
	~hide_cursor();

private:
	CONSOLE_CURSOR_INFO m_Cci;
};
