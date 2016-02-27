#include "CmdTokens.hpp"
#include "functions.hpp"

CmdTokens::CmdTokens()
{
}

CmdTokens::~CmdTokens()
{
}

UINT CmdTokens::NextToken(const wchar_t* &Str)
{
	const wchar_t* Tokens[]=
	{
		UnknownTokenData,
		L"cfg",
		L"unload",
		L"~",
		L"help",
		L"info",
		L"load",
		L"save",
		//L"",
	};
	bool InQuotes=false;
	while(IsSpace(*Str))
	{
		Str++;
	}
	int Len=0;
	while((InQuotes?InQuotes:!IsSpace(Str[Len]))&&Str[Len])
	{
		if(Str[Len]==L'"')
		{
			InQuotes=!InQuotes;
		}
		Len++;
	}
	UINT Token=0;
	if(!Len)
	{
		return TK_EMPTY;
	}
	for(UINT i=1;i<ARRAYSIZE(Tokens);i++)
	{
		if(!strcmpin(Str,Tokens[i],Len))
		{
			Token=i;
			break;
		}
	}
	if(!Token)
	{
		lstrcpynW(UnknownTokenData,Str,Len+1);
	}
	if(Str[Len])
	{
		Len++;
	}
	Str+=Len;
	return Token;
}
