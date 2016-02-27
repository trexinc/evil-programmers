#include "registry.hpp"
#include "functions.hpp"

long CloseRegKey(HKEY hKey);

int CopyKeyTree(const wchar_t* Src,const wchar_t* Dest,const wchar_t* Skip=NULL);

void DeleteKeyTreePart(const wchar_t* KeyName);

int DeleteCount;
string *pStrRegRoot;

HKEY hRegRootKey=HKEY_CURRENT_USER;
HKEY hRegCurrentKey=NULL;
int RequestSameKey=FALSE;

string GetRegRootString()
{
	return *pStrRegRoot;
}

void FreeRegRootString() //BUGBUG
{
	delete pStrRegRoot;
}

void SetRegRootString(const wchar_t* Key)
{
	if(!pStrRegRoot)
		pStrRegRoot=new string;
	*pStrRegRoot=Key;
}

void SetRegRootKey(HKEY hRootKey)
{
	hRegRootKey=hRootKey;
}

void CloseSameRegKey()
{
	if(hRegCurrentKey!=NULL)
	{
		RegCloseKey(hRegCurrentKey);
		hRegCurrentKey=NULL;
	}
	RequestSameKey=FALSE;
}

void UseSameRegKey()
{
	CloseSameRegKey();
	RequestSameKey=TRUE;
}

string &MkKeyName(const wchar_t* Key,string &strDest)
{
	strDest=*pStrRegRoot;
	if(*Key)
	{
		strDest += L"\\";
		strDest += Key;
	}
	return strDest;
}

HKEY CreateRegKey(const wchar_t* Key)
{
	if(hRegCurrentKey)
		return(hRegCurrentKey);
	HKEY hKey;
	DWORD Disposition;

	string strFullKeyName;
	MkKeyName(Key,strFullKeyName);
	if(RegCreateKeyExW(hRegRootKey,strFullKeyName,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition)!=ERROR_SUCCESS)
		hKey=NULL;
	if (RequestSameKey)
	{
		RequestSameKey=FALSE;
		hRegCurrentKey=hKey;
	}
	return hKey;
}

long SetRegKey(const wchar_t* Key,const wchar_t* ValueName,const wchar_t* ValueData, int SizeData, DWORD Type)
{
	
	long Ret=ERROR_SUCCESS;
	HKEY hKey=CreateRegKey(Key);
	if(hKey)
	{
		Ret=RegSetValueExW(hKey,ValueName,0,Type,(LPBYTE)ValueData,SizeData);
		CloseRegKey(hKey);
	}
	return Ret;
}

long SetRegKey(const wchar_t* Key,const wchar_t* ValueName,const wchar_t* const ValueData)
{
	long Ret=ERROR_SUCCESS;
	HKEY hKey=CreateRegKey(Key);
	if(hKey)
	{
		Ret=RegSetValueExW(hKey,ValueName,0,REG_SZ,(LPBYTE)ValueData,static_cast<DWORD>((StrLength(ValueData)+1)*sizeof(wchar_t)));
		CloseRegKey(hKey);
	}
	return Ret;
}

long SetRegKey(const wchar_t* Key,const wchar_t* ValueName,DWORD ValueData)
{
	long Ret=ERROR_SUCCESS;
	HKEY hKey=CreateRegKey(Key);
	if(hKey)
	{
		Ret=RegSetValueExW(hKey,ValueName,0,REG_DWORD,(LPBYTE)&ValueData,sizeof(ValueData));
		CloseRegKey(hKey);
	}
	return Ret;
}

/*
long SetRegKey64(const wchar_t* Key,const wchar_t* ValueName,UINT64 ValueData)
{
	long Ret=ERROR_SUCCESS;
	HKEY hKey=CreateRegKey(Key);
	if(hKey)
	{
		Ret=RegSetValueExW(hKey,ValueName,0,REG_QWORD,(LPBYTE)&ValueData,sizeof(ValueData));
		CloseRegKey(hKey);
	}
	return Ret;
}
*/

long SetRegKey(const wchar_t* Key,const wchar_t* ValueName,const BYTE *ValueData,DWORD ValueSize)
{
	long Ret=ERROR_SUCCESS;
	HKEY hKey=CreateRegKey(Key);
	if(hKey)
	{
		Ret=RegSetValueExW(hKey,ValueName,0,REG_BINARY,ValueData,ValueSize);
		CloseRegKey(hKey);
	}
	return Ret;
}


HKEY OpenRegKey(const wchar_t* Key)
{
	if(hRegCurrentKey)
		return(hRegCurrentKey);
	HKEY hKey;
	string strFullKeyName;
	MkKeyName(Key,strFullKeyName);
	if(RegOpenKeyExW(hRegRootKey,strFullKeyName,0,KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS,&hKey)!=ERROR_SUCCESS)
	{
		CloseSameRegKey();
		return NULL;
	}
	if(RequestSameKey)
	{
		RequestSameKey=FALSE;
		hRegCurrentKey=hKey;
	}
	return hKey;
}

int GetRegKeySize(HKEY hKey,const wchar_t* ValueName)
{
	if(hKey)
	{
		BYTE Buffer;
		DWORD Type,QueryDataSize=sizeof(Buffer);
		int ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,(unsigned char*)&Buffer,&QueryDataSize);
		if(ExitCode==ERROR_SUCCESS||ExitCode==ERROR_MORE_DATA)
		return QueryDataSize;
	}
	return 0;
}

int GetRegKeySize(const wchar_t* Key,const wchar_t* ValueName)
{
	HKEY hKey=OpenRegKey(Key);
	DWORD QueryDataSize=GetRegKeySize(hKey,ValueName);
	CloseRegKey(hKey);
	return QueryDataSize;
}

int GetRegKey(const wchar_t* Key,const wchar_t* ValueName,string &strValueData,const wchar_t* Default,DWORD *pType)
{
  int ExitCode=0;
  HKEY hKey=OpenRegKey(Key);
  if(hKey) // надобно проверить!
  {
    DWORD Type,QueryDataSize=0;

    if ( (ExitCode = RegQueryValueExW (
            hKey,
            ValueName,
            0,
            &Type,
            NULL,
            &QueryDataSize
            )) == ERROR_SUCCESS )
    {
      wchar_t* TempBuffer = strValueData.GetBuffer (QueryDataSize+1); // ...то выделим сколько надо

      ExitCode = RegQueryValueExW(hKey,ValueName,0,&Type,(unsigned char *)TempBuffer,&QueryDataSize);

      strValueData.ReleaseBuffer();
    }
    if(pType)
      *pType=Type;
    CloseRegKey(hKey);
  }
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    strValueData = Default;
    return(FALSE);
  }
  return(TRUE);
}

int GetRegKey(const wchar_t* Key,const wchar_t* ValueName,DWORD &ValueData,DWORD Default)
{
  int ExitCode=0;
  HKEY hKey=OpenRegKey(Key);
  if(hKey)
  {
    DWORD Type,Size=sizeof(ValueData);
    ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,(BYTE *)&ValueData,&Size);
    CloseRegKey(hKey);
  }
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    ValueData=Default;
    return(FALSE);
  }
  return(TRUE);
}

int GetRegKey(const wchar_t* Key,const wchar_t* ValueName,DWORD Default)
{
  DWORD ValueData;
  GetRegKey(Key,ValueName,ValueData,Default);
  return(ValueData);
}
/*
int GetRegKey64(const wchar_t* Key,const wchar_t* ValueName,__int64 &ValueData,unsigned __int64 Default)
{
  int ExitCode=0;
  HKEY hKey=OpenRegKey(Key);
  if(hKey)
  {
    DWORD Type,Size=sizeof(ValueData);
    ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,(BYTE *)&ValueData,&Size);
    CloseRegKey(hKey);
  }
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    ValueData=Default;
    return(FALSE);
  }
  return(TRUE);
}
*/
/*
__int64 GetRegKey64(const wchar_t* Key,const wchar_t* ValueName,unsigned __int64 Default)
{
  __int64 ValueData;
  GetRegKey64(Key,ValueName,ValueData,Default);
  return(ValueData);
}
*/

int GetRegKey(const wchar_t* Key,const wchar_t* ValueName,BYTE *ValueData,const BYTE *Default,DWORD DataSize,DWORD *pType)
{
  int ExitCode=0;
  HKEY hKey=OpenRegKey(Key);
  DWORD Required=DataSize;
  if(hKey)
  {
    DWORD Type;
    ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,ValueData,&Required);
    if(ExitCode == ERROR_MORE_DATA) // если размер не подходящие...
    {
      char *TempBuffer=new char[Required+1]; // ...то выделим сколько надо
      if(TempBuffer) // Если с памятью все нормально...
      {
        if((ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,(unsigned char *)TempBuffer,&Required)) == ERROR_SUCCESS)
          memcpy(ValueData,TempBuffer,DataSize);  // скопируем сколько надо.
        delete[] TempBuffer;
      }
    }
    if(pType)
      *pType=Type;
    CloseRegKey(hKey);
  }
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    if (Default!=NULL)
      memcpy(ValueData,Default,DataSize);
    else
      memset(ValueData,0,DataSize);
    return(0);
  }
  return(Required);
}

/*
void DeleteRegKey(const wchar_t* Key)
{
  string strFullKeyName;
  MkKeyName(Key,strFullKeyName);
  RegDeleteKeyW(hRegRootKey,strFullKeyName);
}

/*
void DeleteRegValue(const wchar_t* Key,const wchar_t* Value)
{
  HKEY hKey;
  string strFullKeyName;
  MkKeyName(Key,strFullKeyName);
  if (RegOpenKeyExW(hRegRootKey,strFullKeyName,0,KEY_WRITE,&hKey)==ERROR_SUCCESS)
  {
    RegDeleteValueW(hKey,Value);
    CloseRegKey(hKey);
  }
}
*/
int CheckRegKey(const wchar_t* Key)
{
  HKEY hKey;
  string strFullKeyName;
  MkKeyName(Key,strFullKeyName);
  int Exist=RegOpenKeyExW(hRegRootKey,strFullKeyName,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS;
  CloseRegKey(hKey);
  return(Exist);
}

int CheckRegValue(const wchar_t* Key,const wchar_t* ValueName)
{
  int ExitCode=0;
  DWORD DataSize=0;
  HKEY hKey=OpenRegKey(Key);
  if(hKey)
  {
    DWORD Type;
    ExitCode=RegQueryValueExW(hKey,ValueName,0,&Type,NULL,&DataSize);
    CloseRegKey(hKey);
  }
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS || !DataSize)
    return(FALSE);
  return(TRUE);
}

int EnumRegKey(const wchar_t* Key,DWORD Index,string &strDestName,bool full)
{
	HKEY hKey=OpenRegKey(Key);
	if(hKey)
	{
		FILETIME LastWriteTime;
		wchar_t SubName[512]; //BUGBUG, dynamic
		DWORD SubSize=sizeof(SubName);
		int ExitCode=RegEnumKeyExW(hKey,Index,SubName,&SubSize,NULL,NULL,NULL,&LastWriteTime);
		CloseRegKey(hKey);
		if(ExitCode==ERROR_SUCCESS)
		{
			if(full)
			{
				string strTempName;
				strTempName=Key;
				if(!strTempName.Empty())
				AddEndSlash(strTempName);
				strTempName+=SubName;
				strDestName=strTempName; //???
			}
			else
			{
				strDestName=SubName;
			}
			return TRUE;
		}
	}
	return FALSE;
}

/*
int EnumRegValue(const wchar_t* Key,DWORD Index, string &strDestName,LPBYTE SData,DWORD SDataSize,LPDWORD IData,__int64* IData64)
{
  HKEY hKey=OpenRegKey(Key);
  int RetCode=REG_NONE;

  if(hKey)
  {
    wchar_t ValueName[512]; //BUGBUG, dynamic

    for(;;)
    {
      DWORD ValSize=sizeof(ValueName);
      DWORD Type=(DWORD)-1;

      if (RegEnumValueW(hKey,Index,ValueName,&ValSize,NULL,&Type,SData,&SDataSize) != ERROR_SUCCESS)
        break;

      RetCode=Type;

      strDestName = ValueName;

      if(Type == REG_SZ)
        break;
      else if(Type == REG_DWORD)
      {
        if(IData)
          *IData=*(DWORD*)SData;
        break;
      }
      else if(Type == REG_QWORD)
      {
        if(IData64)
          *IData64=*(__int64*)SData;
        break;
      }
    }

    CloseRegKey(hKey);
  }
  return RetCode;
}
*/
/*
int EnumRegValueEx(const wchar_t* Key,DWORD Index, string &strDestName, string &strSData, LPDWORD IData,__int64* IData64)
{
	HKEY hKey=OpenRegKey(Key);
	int RetCode=REG_NONE;

	if(hKey)
	{
		wchar_t ValueName[512]; //BUGBUG, dynamic
		for(;;)
		{
			DWORD ValSize=sizeof(ValueName),Type=(DWORD)-1,Size=0;
			if(RegEnumValueW(hKey,Index,ValueName,&ValSize, NULL, &Type, NULL, &Size)!=ERROR_SUCCESS)
				break;
			wchar_t* Data=strSData.GetBuffer(Size/sizeof(wchar_t)+1);
			ValSize=sizeof(ValueName); // НАДА, иначе получаем ERROR_MORE_DATA
			int Ret=RegEnumValueW(hKey,Index,ValueName,&ValSize,NULL,&Type,(LPBYTE)Data,&Size);
			strSData.ReleaseBuffer();

			if(Ret!=ERROR_SUCCESS)
				break;
			RetCode=Type;
			strDestName=ValueName;

			if(Type==REG_SZ)
				break;
			else if(Type==REG_DWORD)
			{
				if(IData)
					*IData=*(LPDWORD)(const wchar_t*)strSData;
				break;
			}
			else if(Type==REG_QWORD)
			{
				if(IData64)
					*IData64=*(__int64*)(const wchar_t*)strSData;
				break;
			}
		}
		CloseRegKey(hKey);
	}
	return RetCode;
}
*/
long CloseRegKey(HKEY hKey)
{
	return (hRegCurrentKey||!hKey)?ERROR_SUCCESS:RegCloseKey(hKey);
}


int RegQueryStringValueEx(HKEY hKey,const wchar_t* lpwszValueName,string &strData,const wchar_t* lpwszDefault)
{
    DWORD cbSize = 0;

    int nResult = RegQueryValueExW (
            hKey,
            lpwszValueName,
            NULL,
            NULL,
            NULL,
            &cbSize
            );

    if ( nResult == ERROR_SUCCESS )
    {
        wchar_t* lpwszData = strData.GetBuffer (cbSize+1);

        nResult = RegQueryValueExW (
            hKey,
            lpwszValueName,
            NULL,
            NULL,
            (LPBYTE)lpwszData,
            &cbSize
            );

        strData.ReleaseBuffer ();
    }

    if ( nResult != ERROR_SUCCESS )
        strData = lpwszDefault;

    return nResult;
}

int RegQueryStringValue (HKEY hKey,const wchar_t* lpwszSubKey,string &strData,const wchar_t* lpwszDefault)
{
    long cbSize = 0;

    int nResult = RegQueryValueW (
            hKey,
            lpwszSubKey,
            NULL,
            &cbSize
            );

    if ( nResult == ERROR_SUCCESS )
    {
        wchar_t* lpwszData = strData.GetBuffer (cbSize+1);

        nResult = RegQueryValueW (
            hKey,
            lpwszSubKey,
            (wchar_t*)lpwszData,
            &cbSize
            );

        strData.ReleaseBuffer ();
    }

    if ( nResult != ERROR_SUCCESS )
        strData = lpwszDefault;

    return nResult;
}

bool IsRegKeyExist(const wchar_t* Key)
{
	HKEY hKey=OpenRegKey(Key);
	if(!hKey)
		return false;
	CloseRegKey(hKey);
	return true;
}
