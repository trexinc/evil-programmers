#include "Tokenizer.hpp"

Tokenizer::Tokenizer(const wchar_t* str,const wchar_t* delim)
{
	Delim=delim;
	size_t start=0;
	Start=true;
	End=true;
	Item=NULL;
	count=0;
	while(str[start])
	{
		while(IsDelim(str[start])&&str[start])
			start++;
		size_t end=start;
		while(!IsDelim(str[end])&&str[end])
			end++;
		string element(&str[start],end-start);
		start=end;

		if(!Item)
		{
			count++;
			Item=new ITEM;
			Item->data=element;
			Item->prev=NULL;
			Item->next=NULL;
		}
		else
		{
			count++;
			ITEM* NewItem=new ITEM;
			NewItem->data=element;
			NewItem->prev=Item;
			NewItem->next=0;
			if(Item)
			{
				Item->next=NewItem;
				Item=NewItem;
			}
		}
	}
	ToStart();
}

Tokenizer::~Tokenizer()
{
	ToEnd();
	while(!Start)
	{
		Prev();
		if(!Start && Item && Item->next)
			delete Item->next;
	}
	if(Item)
		delete Item;
}

bool Tokenizer::IsDelim(wchar_t c)
{
	size_t pos=0;
	return Delim.Pos(pos,c);
}

void Tokenizer::ToStart()
{
	if(Item)
	{
		while(Item && Item->prev)
			Item=Item->prev;
		Start=true;
		End=!Start;
	}
}

void Tokenizer::ToEnd()
{
	if(Item)
	{
		while(Item && Item->next)
			Item=Item->next;
		End=true;
		Start=!End;
	}
}
/*
string Tokenizer::This()
{
	return Item->data;
}
*/
string Tokenizer::Next()
{
	string res;
	if(Item)
	{
		res=Item->data;
		if(Item->next)
			Item=Item->next;
		else
			End=true;
	}
	else
		End=true;
	return res;
}

string Tokenizer::Prev()
{
	string res;
	if(Item)
	{
		res=Item->data;
		if(Item->prev)
			Item=Item->prev;
		else
			Start=true;
	}
	else
		Start=true;
	return res;
}

bool Tokenizer::IsEnd()
{
	return End;
}

bool Tokenizer::IsStart()
{
	return Start;
}

UINT Tokenizer::Count()
{
	return count;
}