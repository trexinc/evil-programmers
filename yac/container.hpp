#pragma once
#include "headers.hpp"

#include "unicodestring.hpp"

class Container
{
	int _count,_arraysize;
	size_t _maxlength;
public:
	struct ITEM
	{
		string Name;
		string Data1;
		DWORD Type;
		int Number;
	};

	Container();
	~Container();
	ITEM **Item,**SelItems;
	int Count();
	int GetIndex(const wchar_t* Data);
	size_t MaxFarLen();
	int Add(const wchar_t* Name,const DWORD Type=0,const wchar_t* Data1=NULL);
	int Sort(bool reverse=false);
	int Select(const wchar_t* Pattern);
	void FreeSelect();
};

enum
{
	T_FIL,
	T_PTH,
	T_SRV,
	T_SHR,
	T_ENV,
	T_DRV,
	T_ADS,
	T_REG,
	T_PIP,
	T_VOL,
	T_PRC,
	T_SVC,
	T_CMD,
	T_ALS,
	T_SAL,
	T_PRF,
	T_KEY,
	T_USR,
	T_PNL,
	T_HST,
	T_NMS,
};
