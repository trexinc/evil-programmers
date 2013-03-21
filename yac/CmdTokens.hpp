#pragma once
#include "headers.hpp"

class CmdTokens
{
	wchar_t UnknownTokenData[1024];
	const wchar_t* *_Tokens;
	size_t TokensCount;
public:
	enum TOKENS
	{
		TK_EMPTY=-1,
		TK_UNKNOWN,
		TK_CFG,
		TK_UNLOAD,
		TK_HOME,
		TK_HELP,
		TK_INFO,
		TK_LOAD,
		TK_SAVE,
	};
	CmdTokens();
	~CmdTokens();
	UINT NextToken(const wchar_t* &Str);
};
