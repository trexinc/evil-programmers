#pragma once
#include "unicodestring.hpp"

class StringStack
{
	struct data
	{
		string item;
		data* prev;
	}
	*Data;
public:
	StringStack();
	~StringStack();
	void flush();
	void push(string& str);
	string pop();
};