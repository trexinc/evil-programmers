#pragma once
#include "headers.hpp"

#define StrLength(x) (x?wcslen(x):0)
#define UnicodeToOEM(src,dst,lendst) {WideCharToMultiByte(CP_OEMCP,0,(src),-1,(dst),(lendst),NULL,FALSE);dst[lendst-1]=0;}
#define OEMToUnicode(src,dst,lendst) {MultiByteToWideChar(CP_OEMCP,0,(src),-1,(dst),(lendst));dst[lendst-1]=0;}

#define string UnicodeString

#define __US_DELTA 20

class UnicodeStringData
{
private:
	size_t m_nLength,m_nSize,m_nDelta;
	int m_nRefCount;
	wchar_t* m_pData;

	wchar_t* AllocData(size_t nSize,size_t *nNewSize)
	{
		if(nSize<=m_nDelta) 
			*nNewSize=m_nDelta; 
		else if(nSize%m_nDelta>0) 
			*nNewSize=(nSize/m_nDelta+1)*m_nDelta; 
		else 
			*nNewSize=nSize;
		return new wchar_t[*nNewSize];
	}

	void FreeData(wchar_t* pData)
	{
		if (pData)
			delete[] pData;
	}

public:
	UnicodeStringData(size_t nSize=0,size_t nDelta=0)
	{
		m_nDelta=(!nDelta?__US_DELTA:nDelta);
		m_nLength=0;
		m_nRefCount=1;
		m_pData=AllocData(nSize,&m_nSize);
		if(m_pData)
			m_pData[0]=0;
		else
			m_nSize=0;
	}

	size_t SetLength(size_t nLength)
	{
		if (nLength<m_nSize)
		{
			m_nLength=nLength;
			m_pData[m_nLength]=0;
		}
		return m_nLength;
	}

	size_t Inflate(size_t nSize)
	{
		if(nSize<=m_nSize)
			return m_nSize;
		wchar_t* pOldData=m_pData;
		size_t nOldSize=m_nSize;
		if(nSize>=m_nDelta<<3) 
			nSize=nSize<<1; 
		else 
			nSize=(nSize/m_nDelta+1)*m_nDelta;
		m_pData=AllocData(nSize,&m_nSize);
		if(!m_pData)
		{
			m_pData=pOldData;
			m_nSize=nOldSize;
		}
		else
		{
			if (pOldData)
				memcpy(m_pData,pOldData,m_nLength*sizeof(wchar_t));
			m_pData[m_nLength] = 0;
			FreeData(pOldData);
		}
		return m_nSize;
	}

	wchar_t* GetData()
	{
		return m_pData;
	}

	size_t GetLength() const
	{
		return m_nLength;
	}

	size_t GetSize() const
	{
		return m_nSize;
	}

	int GetRef()
	{
		return m_nRefCount;
	}

	void AddRef()
	{
		m_nRefCount++;
	}

	void DecRef()
	{
		m_nRefCount--;
		if(!m_nRefCount)
			delete this;
	}

	~UnicodeStringData()
	{
		FreeData(m_pData);
	}
};


class UnicodeString
{
private:
	UnicodeStringData *m_pData;
	void DeleteData()
	{
		if(m_pData)
			m_pData->DecRef();
	}

public:
	UnicodeString(size_t nSize=0,size_t nDelta=0)
	{
		m_pData=new UnicodeStringData(nSize,nDelta);
	}

	const UnicodeString& SetData(const wchar_t* lpwszData,size_t Length=0);
	const UnicodeString& SetData(const UnicodeString &strCopy);
	const UnicodeString& SetData(LPCSTR lpszData,UINT CodePage=CP_OEMCP);

	UnicodeString(const UnicodeString &strCopy)
	{
		m_pData = NULL;
		SetData(strCopy);
	}

	UnicodeString(const wchar_t* lpwszData,size_t Length=0)
	{
		m_pData=NULL;
		SetData(lpwszData,Length);
	}

	UnicodeString(LPCSTR lpszData,UINT CodePage=CP_OEMCP)
	{
		m_pData=NULL;
		SetData(lpszData,CodePage);
	}

	~UnicodeString()
	{
		DeleteData();
	}

	size_t Inflate(size_t nSize);

	size_t GetLength()const
	{
		return m_pData->GetLength();
	}

	size_t GetCharString(LPSTR Dest,size_t DestSize,UINT CodePage=CP_OEMCP)const;

	const UnicodeString& Append(const wchar_t c);
	const UnicodeString& Append(const wchar_t* lpwszAdd);
	const UnicodeString& Append(const UnicodeString &strAdd);
	bool Equal(const wchar_t* lpwszCmp,size_t length=-1,bool IgnoreCase=true);

	operator const wchar_t* ()const
	{
		return m_pData->GetData();
	}

	const wchar_t* CPtr()
	{
		return m_pData->GetData();
	}

	bool operator==(const wchar_t* str2)const;
	bool operator==(const UnicodeString &str2)const;

	bool operator!=(const wchar_t* str2)const;
	bool operator!=(const UnicodeString &str2)const;

	const UnicodeString& operator=(LPCSTR lpszData);
	const UnicodeString& operator=(const wchar_t* lpwszData);
	const UnicodeString& operator=(const UnicodeString &strCopy);

	const UnicodeString& operator+=(LPCSTR lpszAdd);
	const UnicodeString& operator+=(const wchar_t* lpwszAdd);
	const UnicodeString& operator+=(const UnicodeString &strAdd);

	friend const UnicodeString operator+(const UnicodeString &strSrc1,LPCSTR lpszSrc2);
	friend const UnicodeString operator+(const UnicodeString &strSrc1,const wchar_t* lpwszSrc2);
	friend const UnicodeString operator+(const UnicodeString &strSrc1,const UnicodeString &strSrc2);

	const UnicodeString SubString(size_t from,size_t count);
	const UnicodeString SubString(size_t from);

	wchar_t* GetBuffer(size_t nLength=-1);
	void ReleaseBuffer(size_t nLength=-1);

	size_t SetLength(size_t nLength)
	{
		if(nLength<m_pData->GetLength())
		{
			Inflate(nLength+1);
			m_pData->SetLength(nLength);
		}
		return m_pData->GetLength();
	}

	wchar_t At(size_t nIndex) const
	{
		if(nIndex>GetLength())
			return 0;
		const wchar_t* lpwszData=m_pData->GetData();
		return lpwszData[nIndex];
	}

	bool Empty() const
	{
		return !(m_pData->GetLength()&&*m_pData->GetData());
	}

	void Lower ()
	{
		Inflate(m_pData->GetSize());
		CharLowerW(m_pData->GetData());
	}

	void Upper()
	{
		Inflate(m_pData->GetSize());
		CharUpperW(m_pData->GetData());
	}

	void RShift(size_t nNewPos)
	{
		if(nNewPos>m_pData->GetLength())
			nNewPos=m_pData->GetLength();
		Inflate(m_pData->GetSize());
		memmove(m_pData->GetData()+nNewPos,m_pData->GetData(),(m_pData->GetLength()-nNewPos+1)*sizeof(wchar_t));
	}

	void LShift (size_t nNewPos)
	{
		if(nNewPos>m_pData->GetLength())
			nNewPos = m_pData->GetLength ();
		Inflate(m_pData->GetSize());
		memmove(m_pData->GetData(),m_pData->GetData()+nNewPos,(m_pData->GetLength()-nNewPos+1)*sizeof(wchar_t));
		m_pData->SetLength(m_pData->GetLength()-nNewPos);
	}

	int __cdecl vFormat(const wchar_t* format,va_list argptr)
	{
		wchar_t* buffer=NULL;
		size_t Size = MAX_PATH;
		int retValue=-1;
		do
		{
			Size <<= 1;
			wchar_t* tmpbuffer=(wchar_t*)realloc(buffer,Size*sizeof(wchar_t));
			if (!tmpbuffer)
			{
				va_end( argptr );
				free (buffer);
				return retValue;
			}
			buffer = tmpbuffer;
			//_vsnwprintf не всегда ставит '\0' вконце.
			//Поэтому надо обнулить и передать в _vsnwprintf размер-1.
			buffer[Size-1]=0;
			retValue=_vsnwprintf(buffer,Size-1,format,argptr);
		}
		while(retValue==-1);
		SetData (buffer);
		free(buffer);
		return retValue;
	}
	
	int __cdecl Format(const wchar_t* format,...)
	{
		va_list argptr;
		va_start(argptr,format);
		int retValue=vFormat(format,argptr);
		va_end(argptr);
		return retValue;
	}

	bool Pos(size_t& nPos,const wchar_t Ch,size_t nStartPos=0) const
	{
		const wchar_t* lpwszStr=wcschr(m_pData->GetData()+nStartPos,Ch);
		if(lpwszStr)
		{
			nPos = lpwszStr - m_pData->GetData();
			return true;
		}
		return false;
	}

	bool Pos(size_t &nPos,const wchar_t* lpwszFind, size_t nStartPos=0) const
	{
		const wchar_t* lpwszStr=wcsstr(m_pData->GetData()+nStartPos,lpwszFind);
		if(lpwszStr) 
		{
			nPos=lpwszStr-m_pData->GetData();
			return true; 
		}
		return false;
	}

	bool RPos(size_t &nPos, wchar_t Ch, size_t nStartPos=0) const
	{
		const wchar_t* lpwszStr=wcsrchr(m_pData->GetData()+nStartPos,Ch);
		if(lpwszStr)
		{
			nPos=lpwszStr-m_pData->GetData(); 
			return true;
		}
		return false;
	}
};
