#pragma once
#include "unicodestring.hpp"

class Tokenizer
{
	struct ITEM
	{
		string data;
		ITEM *next,*prev;
	}
	*Item/*,Start,End*/;
	bool Start,End;
	string Delim;
	bool IsDelim(wchar_t c);
	int count;
public:
	Tokenizer(const wchar_t* Str,const wchar_t* Delim);
	~Tokenizer();
	void ToStart();
	void ToEnd();
	string Next();
//	string This();
	string Prev();
	bool IsStart();
	bool IsEnd();
	UINT Count();
};
