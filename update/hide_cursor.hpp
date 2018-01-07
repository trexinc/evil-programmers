#pragma once

class hide_cursor
{
public:
	NONCOPYABLE(hide_cursor);

	hide_cursor();
	~hide_cursor();

private:
	CONSOLE_CURSOR_INFO m_Cci;
};
