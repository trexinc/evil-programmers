#include "functions.hpp"
#include "registry.hpp"

inline bool IsSlash(wchar_t x)
{
	return x==L'\\'||x==L'/';
}

inline bool IsSpace(wchar_t x)
{
	return x==L' '||x==L'\t';
}

inline bool IsEol(wchar_t x)
{
	return x==L'\r'||x==L'\n';
}

const wchar_t* GetFileName(const wchar_t* fullpath)
{
	const wchar_t* Ptr=fullpath;
	while(*Ptr)
		Ptr++;
	while(Ptr>fullpath && !IsSlash(Ptr[-1]))
		Ptr--;
	return Ptr;
}

bool CmdSeparator(const wchar_t c)
{
	return (/*IsSlash(c)||*/IsSpace(c)||c==L'<'||c==L'>'||c==L'|'/*||c==L':'||c==L'*'||c==L'"'*/);
}

BOOL AddEndSlash(wchar_t* Path, wchar_t TypeSlash)
{
	BOOL Result=FALSE;
	if(Path)
	{
		wchar_t* end;
		int Slash=0, BackSlash=0;
		if(!TypeSlash)
		{
			end=Path;
			while(*end)
			{
				Slash+=(*end==L'\\');
				BackSlash+=(*end==L'/');
				end++;
			}
		}
		else
		{
			end=Path+StrLength(Path);
			if(TypeSlash == L'\\')Slash=1;
			else BackSlash=1;
		}
		size_t Length=end-Path;
		wchar_t c=(Slash<BackSlash)?L'/':L'\\';
		Result=TRUE;
		if(!Length)
		{
			end[0]=c;
			end[1]=0;
		}
		else
		{
			end--;
			if (!IsSlash(*end))
			{
				end[1]=c;
				end[2]=0;
			}
			else *end=c;
		}
	}
	return Result;
}

BOOL AddEndSlash(string &strPath,wchar_t TypeSlash)
{
	wchar_t* lpwszPath=strPath.GetBuffer(strPath.GetLength()+2);
	BOOL Result=AddEndSlash(lpwszPath, TypeSlash);
	strPath.ReleaseBuffer();
	return Result;
}

BOOL WINAPI AddEndSlash(wchar_t* Path)
{
	return AddEndSlash(Path,L'\\');
}

BOOL AddEndSlash(string &strPath)
{
	return AddEndSlash(strPath,L'\\');
}

wchar_t* RemoveLeadingSpaces(wchar_t* Str)
{
	if(Str)
	{
		wchar_t* ChPtr=Str;
		for(;IsSpace(*ChPtr);ChPtr++);
		if(ChPtr!=Str)
		{
			memmove(Str,ChPtr,(StrLength(ChPtr)+1)*sizeof(wchar_t));
		}
	}
	return Str;
}

string& RemoveLeadingSpaces(string &strStr)
{
	wchar_t* ChPtr=strStr.GetBuffer();
	wchar_t* Str=ChPtr;
	for(;IsSpace(*ChPtr);ChPtr++);
	if(ChPtr!=Str)
	{
		memmove(Str,ChPtr,(StrLength(ChPtr)+1)*sizeof(wchar_t));
	}
	strStr.ReleaseBuffer();
	return strStr;
}

wchar_t* RemoveTrailingSpaces(wchar_t* Str)
{
	if(Str && *Str)
	{
		size_t I;
		for(wchar_t* ChPtr=Str+(I=StrLength(Str))-1;I>0;I--,ChPtr--)
		{
			if(IsSpace(*ChPtr)||IsEol(*ChPtr))
			{
				*ChPtr=0;
			}
			else
			{
				break;
			}
		}
	}
	return Str;
}

string& RemoveTrailingSpaces(string &strStr)
{
	if(!strStr.Empty())
	{
		RemoveTrailingSpaces(strStr.GetBuffer());
		strStr.ReleaseBuffer ();
	}
	return strStr;
}

wchar_t* WINAPI RemoveExternalSpaces(wchar_t* Str)
{
	return RemoveTrailingSpaces(RemoveLeadingSpaces(Str));
}

string& WINAPI RemoveExternalSpaces(string &strStr)
{
	return RemoveTrailingSpaces(RemoveLeadingSpaces(strStr));
}

bool CheckQuotedSymbols(const wchar_t* Str)
{
	string symbols;
	string ExpStr;
	apiExpandEnvironmentStrings(Str,ExpStr);
	DWORD type;
	string root=GetRegRootString();
	string newroot=root;
	newroot.SetLength(newroot.GetLength()-wcslen(L"\\plugins"));
	SetRegRootString(newroot);
	GetRegKey(L"System",L"QuotedSymbols",symbols,L" &()[]{}^=;!'+,`",&type);
	SetRegRootString(root);
	symbols+=L"\t";
	for(int i=0;symbols.At(i);i++)
	{
		if(wcschr(ExpStr,symbols.At(i)))
		{
			return true;
		}
	}
	return false;
}

bool IsQuotedSymbol(wchar_t c)
{
	if(c==L'"')
		return false;
	wchar_t Str[]={c,0};
	return CheckQuotedSymbols(Str);
}

DWORD apiExpandEnvironmentStrings(const wchar_t* src,string &strDest)
{
	string strSrc=src;
	DWORD length = ExpandEnvironmentStringsW(strSrc,NULL,0);
	if(length)
	{
		wchar_t* lpwszDest=strDest.GetBuffer(length);
		ExpandEnvironmentStringsW(strSrc,lpwszDest,length);
		strDest.ReleaseBuffer();
		length=(DWORD)strDest.GetLength();
	}
	return length;
}
/*
// Заменить в строке Str Count вхождений подстроки FindStr на подстроку ReplStr
// Если Count < 0 - заменять "до полной победы"
// Return - количество замен
int ReplaceStrings(string &strStr,const wchar_t* FindStr,const wchar_t* ReplStr,UINT Count,bool IgnoreCase)
{
	int I=0, J=0, Res;
	int LenReplStr=StrLength(ReplStr);
	int LenFindStr=StrLength(FindStr);
	int L=(int)strStr.GetLength();

	wchar_t* Str=strStr.GetBuffer(10240); //BUGBUG!!!

	while(I<=L-LenFindStr)
	{
		Res=IgnoreCase?strcmpin(Str+I,FindStr,LenFindStr):strcmpn(Str+I,FindStr,LenFindStr);
		if(!Res)
		{
			if(LenReplStr>LenFindStr)
				memmove(Str+I+(LenReplStr-LenFindStr),Str+I,(StrLength(Str+I)+1)*sizeof(wchar_t)); // >>
			else if(LenReplStr<LenFindStr)
				memmove(Str+I,Str+I+(LenFindStr-LenReplStr),(StrLength(Str+I+(LenFindStr-LenReplStr))+1)*sizeof(wchar_t)); //??
			memcpy(Str+I,ReplStr,LenReplStr*sizeof(wchar_t));
			I+=LenReplStr;
			if(++J==Count&&Count>0)
				break;
		}
		else
			I++;
		L=StrLength(Str);
	}
	strStr.ReleaseBuffer();
	return J;
}
*/
void Unquote(wchar_t* Str)
{
	wchar_t *pStr;
	int i=0;
	for(pStr=Str;*pStr;pStr++)
		if(*pStr!=L'"')
		{
			Str[i]=*pStr;
			i++;
		}
	Str[i]=0;
}

void Unquote(string &Str)
{
	wchar_t* lpwszStr=Str.GetBuffer();
	Unquote(lpwszStr);
	Str.ReleaseBuffer();
}

bool TokenVirtualRoot(const wchar_t* str,string& out)
{
	const wchar_t* ptr=str;
	while(*ptr)
		ptr++;
	while((ptr > str) && !IsSlash(*ptr) && (*ptr!=L':'))
		ptr--;
	size_t len=ptr-str;
	if(!len&&!IsSlash(*str))
		return false;
	len++;
	out=str;
	out.SetLength(len);
	return true;
}

bool apiGetModuleFileName(__in_opt HMODULE hModule,__out string &strFilename)
{
	wchar_t* buffer=NULL;
	DWORD Size = MAX_PATH;
	buffer=(wchar_t*)malloc(Size*sizeof(wchar_t));
	SetLastError(ERROR_SUCCESS);
	GetModuleFileNameW(hModule,buffer,Size);
	while(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
	{
		Size<<=1;
		buffer=(wchar_t*)realloc(buffer,Size*sizeof(wchar_t));
		GetModuleFileNameW(hModule,buffer,Size);
	}
	strFilename=buffer;
	free(buffer);
	return true;
}
