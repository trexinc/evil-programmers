#include "stringstack.hpp"

StringStack::StringStack()
{
	Data=NULL;
}

StringStack::~StringStack()
{
	flush();
}

void StringStack::flush()
{
	while(Data && Data->prev)
	{
		data* tmp=Data;
		Data=Data->prev;
		delete tmp;
	}
	if(Data)
	{
		delete Data;
		Data=NULL;
	}
}

void StringStack::push(string& str)
{
	if(!Data)
	{
		Data=new data;
		Data->item=str;
		Data->prev=NULL;
	}
	else
	{
		data* NewData=new data;
		NewData->item=str;
		NewData->prev=Data;
		Data=NewData;
	}
}

string StringStack::pop()
{
	string res;
	if(Data)
	{
		res=Data->item;
		data* tmp=Data;
		Data=Data->prev;
		delete tmp;
	}
	return res;
}
