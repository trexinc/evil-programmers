#pragma once

class cursor_pos
{
public:
	NONCOPYABLE(cursor_pos);

	cursor_pos();
	~cursor_pos();

private:
	COORD Pos;
};
