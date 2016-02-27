#include "Container.hpp"
#include "functions.hpp"


Container::Container()
{
	_arraysize=128;
	Item=(ITEM**)malloc(_arraysize*sizeof(ITEM*));
	Item[0]=0;
	_count=0;
	_maxlength=0;
	SelItems=0;
}

Container::~Container()
{
	for(int i=0;i<_count;i++)
		delete Item[i];
	if(Item)
		free(Item);
	if(SelItems)
		free(SelItems);
}

int Container::Count()
{
	return _count;
}

size_t Container::MaxFarLen()
{
	return _maxlength;
}

int Container::Add(const wchar_t* Name,const DWORD Type,const wchar_t* Data1)
{
	int n=GetIndex(Name);
	if((n!=-1)&&Item[n])
	{
		if(!(Item[n]->Type&Type))
			Item[n]->Type|=(Type|0x10000000);
		return 0;          //^^^^^^^^^^ - сей бит есть признак нескольких вхождений
	}
	size_t l=wcslen(Name);
	if(l>_maxlength)
		_maxlength=l;
	Item[_count]=new ITEM;
	Item[_count]->Name=Name;
	Item[_count]->Data1=Data1;
	Item[_count]->Type=Type;
	_count++;
	if(_count>=_arraysize)
	{
		_arraysize<<=1;
		Item=(ITEM**)realloc(Item,_arraysize*sizeof(ITEM*));
	}
	Item[_count]=0;
	return 1;
}

int Container::Sort(bool reverse)
{
	ITEM *tmp;
	for(int i=1;i<_count;i++)
	{
		tmp=Item[i];
		int j=i-1;
		int type=reverse?-1:1;
		while((type*lstrcmpiW(tmp->Name,Item[j]->Name))<0) //CompareStringW(LOCALE_USER_DEFAULT,NORM_IGNORECASE|SORT_STRINGSORT,tmp,-1,Item[j].Data,-1)-2<0)
		{
			Item[j+1]=Item[j];
			j--;
			if(j<0)
				break;
		}
		Item[j+1]=tmp;
	}
	return 1;
}

/*
int Container::Delete(const wchar_t* Data)
{
	int i=GetIndex(Data);
	if(i==-1)return 0;
	delete Item[i];
	for(;i<_count;i++)
		Item[i]=Item[i+1];
	_count--;
	return 1;
}
*/

int Container::GetIndex(const wchar_t* Data)
{
	if(!Data||!*Data)
		return -1;
	for(int i=0;i<_count;i++)
		if(Item[i]&&(Item[i]->Name==Data))
			return i;
	return -1;
}

int Container::Select(const wchar_t* Pattern)
{
	FreeSelect();
	int _selarraysize=128;
	SelItems=(ITEM**)malloc(_selarraysize*sizeof(ITEM*));
	int SelCount=0;
	for(int i=0;i<_count;i++)
		if(Item[i])
		{
			string UnquotedName=Item[i]->Name;
			Unquote(UnquotedName);
			if(!UnquotedName.Equal(Pattern,wcslen(Pattern)))
				continue;
			SelItems[SelCount]=Item[i];
			SelItems[SelCount]->Number=i;
			SelCount++;
			if(SelCount>=_selarraysize)
			{
				_selarraysize<<=1;
				SelItems=(ITEM**)realloc(SelItems,_selarraysize*sizeof(ITEM*));
			}
		}
	return SelCount;
}

void Container::FreeSelect()
{
	if(SelItems)
		free(SelItems);
	SelItems=NULL;
}