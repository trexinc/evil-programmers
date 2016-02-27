#include "headers.hpp"

LPVOID WINAPIV malloc(size_t size)
{
	return size?HeapAlloc(GetProcessHeap(),0,size):NULL;
}

LPVOID WINAPIV realloc(LPVOID block, size_t size)
{
	if(!size)
	{
		if(block)
		{
			HeapFree(GetProcessHeap(),0,block);
		}
		return NULL;
	}
	else
	{
		return block?HeapReAlloc(GetProcessHeap(),0,block,size):HeapAlloc(GetProcessHeap(),0,size);
	}
}

VOID WINAPIV free(LPVOID block)
{
	if(block)
	{
		HeapFree(GetProcessHeap(),0,block);
	}
}

LPVOID WINAPIV operator new(size_t size)
{
	return malloc(size);
}

LPVOID WINAPIV operator new[](size_t size)
{
	return ::operator new(size);
}

VOID WINAPIV operator delete(LPVOID block)
{
	free(block);
}

VOID WINAPIV operator delete[](LPVOID ptr)
{
	::operator delete(ptr);
}

LPVOID WINAPIV memcpy(LPVOID dst,LPCVOID src, size_t count)
{
	LPVOID ret=dst;
	while(count--)
	{
		*static_cast<BYTE*>(dst)=*static_cast<const BYTE*>(src);
		dst=static_cast<BYTE*>(dst)+1;
		src=static_cast<const BYTE*>(src)+1;
	}
	return ret;
}

LPVOID WINAPIV memset(LPVOID dst,INT val, size_t count)
{
	LPVOID start=dst;
	while(count--)
	{
		*static_cast<PBYTE>(dst)=static_cast<BYTE>(val);
		dst=static_cast<PBYTE>(dst)+1;
	}
	return start;
}

LPVOID WINAPIV memmove(LPVOID dst,LPCVOID src, size_t count)
{
	LPVOID ret=dst;
	if(dst<=src||static_cast<PBYTE>(dst)>=(static_cast<const BYTE*>(src)+count))
	{
		while(count--)
		{
			*static_cast<PBYTE>(dst)=*static_cast<const BYTE*>(src);
			dst=static_cast<PBYTE>(dst)+1;
			src=static_cast<const BYTE*>(src)+1;
		}
	}
	else
	{
		dst=static_cast<PBYTE>(dst)+count-1;
		src=static_cast<const BYTE*>(src)+count-1;
		while (count--)
		{
			*static_cast<PBYTE>(dst)=*static_cast<const BYTE*>(src);
			dst=static_cast<PBYTE>(dst)-1;
			src=static_cast<const BYTE*>(src)-1;
		}
	}
	return ret;
}

INT WINAPIV memcmp(LPCVOID src,LPCVOID dst,size_t count)
{
	for(size_t i=0;i<count;i++)
	{
		if(static_cast<const BYTE*>(src)[i]!=static_cast<const BYTE*>(dst)[i])
		{
			return (static_cast<const BYTE*>(src)[i]>static_cast<const BYTE*>(dst)[i])?1:-1;
		}
	}
	return 0;
}

int WINAPIV _purecall()
{
	return 0;
}
