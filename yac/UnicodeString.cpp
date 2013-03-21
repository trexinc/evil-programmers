#pragma hdrstop

#include "UnicodeString.hpp"

size_t UnicodeString::Inflate(size_t nSize)
{
	if ((m_pData->GetRef()==1))
	{
		m_pData->Inflate(nSize);
	}
	else
	{
		UnicodeStringData *pNewData = new UnicodeStringData(nSize);
		size_t nNewLength=Min(m_pData->GetLength(),nSize-1);
		memcpy(pNewData->GetData(),m_pData->GetData(),nNewLength*sizeof(wchar_t));
		pNewData->SetLength(nNewLength);
		m_pData->DecRef();
		m_pData = pNewData;
	}
	return m_pData->GetSize();
}

size_t UnicodeString::GetCharString(LPSTR Dest,size_t DestSize,UINT CodePage)const
{
	size_t nCopyLength=((DestSize-1)<=m_pData->GetLength()?(DestSize-1):m_pData->GetLength());
	WideCharToMultiByte(CodePage,0,m_pData->GetData(),(int)nCopyLength,Dest,(int)nCopyLength+1,NULL,NULL);
	Dest[nCopyLength]=0;
	return nCopyLength;
}

const UnicodeString& UnicodeString::SetData(const UnicodeString& strCopy)
{
	if(strCopy.m_pData!=m_pData)
	{
		if(m_pData)
			m_pData->DecRef();
		m_pData = strCopy.m_pData;
		m_pData->AddRef();
	}
	return *this;
}

const UnicodeString& UnicodeString::SetData(const wchar_t* lpwszData,size_t Length)
{
	/*if(m_pData)
		m_pData->DecRef();
	size_t nLength=Length?Length:StrLength(lpwszData);
	m_pData=new UnicodeStringData(nLength+1);
	wchar_t* pStr=m_pData->GetData();
	if(pStr)
	{
		memcpy(pStr,lpwszData,nLength*sizeof(wchar_t));
		m_pData->SetLength(nLength);
	}
	return *this;
	*/
	size_t nLength = Length?Length:StrLength(lpwszData); 
	if(m_pData&&m_pData->GetRef()==1&&nLength+1<=m_pData->GetSize())
	{
		memmove(m_pData->GetData(),lpwszData,nLength*sizeof(wchar_t));
		m_pData->SetLength(nLength);
	}
	else
	{
		UnicodeStringData *pNewData=new UnicodeStringData(nLength+1);
		memcpy(pNewData->GetData(),lpwszData,nLength*sizeof(wchar_t));
		pNewData->SetLength(nLength);
		if(m_pData)
			m_pData->DecRef(); 
		m_pData=pNewData;
	}
	return *this;
}

const UnicodeString& UnicodeString::SetData(LPCSTR lpszData,UINT CodePage)
{
	if(m_pData)
		m_pData->DecRef();
	size_t nSize=MultiByteToWideChar(CodePage,0,lpszData,-1,NULL,0);
	m_pData=new UnicodeStringData(nSize);
	MultiByteToWideChar(CodePage,0,lpszData,(int)nSize,m_pData->GetData(),(int)m_pData->GetSize());
	m_pData->SetLength(nSize-1);
	return *this;
}

const UnicodeString& UnicodeString::Append(const UnicodeString& strAdd)
{
/*
	UnicodeStringData* pAddData=strAdd.m_pData;
	size_t nNewLength=m_pData->GetLength()+pAddData->GetLength();
	Inflate(nNewLength+1);
	memcpy(m_pData->GetData()+m_pData->GetLength(),pAddData->GetData(),pAddData->GetLength()*sizeof(wchar_t));
	m_pData->SetLength(nNewLength);
	return *this;
*/
	UnicodeStringData *pAddData=strAdd.m_pData;
	size_t nNewLength=m_pData->GetLength()+pAddData->GetLength();
	if (m_pData->GetRef()==1&&nNewLength+1<=m_pData->GetSize())
	{
		memcpy(m_pData->GetData()+m_pData->GetLength(),pAddData->GetData(),pAddData->GetLength()*sizeof(wchar_t));
		m_pData->SetLength(nNewLength);
	}
	else
	{
		UnicodeStringData *pNewData = new UnicodeStringData(nNewLength + 1);
		memcpy(pNewData->GetData(),m_pData->GetData(),m_pData->GetLength()*sizeof(wchar_t));
		memcpy(pNewData->GetData()+m_pData->GetLength(),pAddData->GetData(),pAddData->GetLength()*sizeof(wchar_t));
		pNewData->SetLength(nNewLength);
		m_pData->DecRef();
		m_pData=pNewData;
	}
	return *this;
}

const UnicodeString& UnicodeString::Append(const wchar_t c)
{
	const wchar_t str[]={c,0};
	Append(str);
	return *this;
}

const UnicodeString& UnicodeString::Append(const wchar_t* lpwszAdd)
{
/*
	size_t nAddLength=StrLength(lpwszAdd);
	size_t nNewLength=m_pData->GetLength()+nAddLength;
	Inflate(nNewLength+1);
	memcpy(m_pData->GetData()+m_pData->GetLength(),lpwszAdd,nAddLength*sizeof(wchar_t));
	m_pData->SetLength(nNewLength);
	return *this;
*/
	size_t nAddLength=StrLength(lpwszAdd);
	size_t nNewLength=m_pData->GetLength()+nAddLength;
	if (m_pData->GetRef()==1&&nNewLength+1<=m_pData->GetSize())
	{
		memcpy(m_pData->GetData()+m_pData->GetLength(),lpwszAdd,nAddLength*sizeof(wchar_t));
		m_pData->SetLength(nNewLength);
	}
	else
	{
		UnicodeStringData *pNewData=new UnicodeStringData(nNewLength+1);
		memcpy(pNewData->GetData(),m_pData->GetData(),m_pData->GetLength()*sizeof(wchar_t));
		memcpy(pNewData->GetData()+m_pData->GetLength(),lpwszAdd,nAddLength*sizeof(wchar_t));
		pNewData->SetLength(nNewLength);
		m_pData->DecRef();
		m_pData = pNewData;
	}
	return *this;
}

bool UnicodeString::operator==(const UnicodeString& str2)const
{
	return !(CompareStringW(LOCALE_SYSTEM_DEFAULT,NORM_IGNORECASE,m_pData->GetData(),-1,str2,-1)-2);
}

bool UnicodeString::operator==(const wchar_t* str2) const
{
	return !(CompareStringW(LOCALE_SYSTEM_DEFAULT,NORM_IGNORECASE,m_pData->GetData(),-1,str2,-1)-2);
}

bool UnicodeString::operator!=(const UnicodeString& str2)const
{
	return !operator ==(str2);
}

bool UnicodeString::operator!=(const wchar_t* str2) const
{
	return !operator ==(str2);
}


const UnicodeString& UnicodeString::operator=(const UnicodeString& strCopy)
{
	return SetData(strCopy);
}

const UnicodeString& UnicodeString::operator=(LPCSTR lpszData)
{
	return SetData(lpszData);
}

const UnicodeString& UnicodeString::operator=(const wchar_t* lpwszData)
{
	return SetData(lpwszData);
}

const UnicodeString& UnicodeString::operator+=(const UnicodeString &strAdd)
{
	return Append(strAdd);
}

const UnicodeString& UnicodeString::operator+=(LPCSTR lpszAdd)
{
	return Append(lpszAdd);
}

const UnicodeString& UnicodeString::operator+=(const wchar_t* lpwszAdd)
{
	return Append(lpwszAdd);
}

const UnicodeString operator+(const UnicodeString& strSrc1, const UnicodeString& strSrc2)
{
	UnicodeString Result(strSrc1);
	Result+=strSrc2;
	return Result;
}

const UnicodeString operator+(const UnicodeString& strSrc1, LPCSTR lpszSrc2)
{
	UnicodeString Result(strSrc1);
	Result+=lpszSrc2;
	return Result;
}

const UnicodeString operator+(const UnicodeString& strSrc1, const wchar_t* lpwszSrc2)
{
	UnicodeString Result(strSrc1);
	Result+=lpwszSrc2;
	return Result;
}

const UnicodeString UnicodeString::SubString(size_t from,size_t count)
{
	if(from>m_pData->GetLength())
		return L"";
	UnicodeString Result(&m_pData->GetData()[from],count);
	if(count)
		Result.SetLength(count);
	return Result;
}

const UnicodeString UnicodeString::SubString(size_t from)
{
	return SubString(from,0);
}

wchar_t* UnicodeString::GetBuffer(size_t nLength)
{
	Inflate(nLength==-1?m_pData->GetSize():nLength);
	return m_pData->GetData ();
}

void UnicodeString::ReleaseBuffer (size_t nLength)
{
	if(nLength!=(size_t)-1)
		m_pData->SetLength (nLength);
	else
		m_pData->SetLength (StrLength(m_pData->GetData()));
}

bool UnicodeString::Equal(const wchar_t* lpwszCmp,size_t length,bool IgnoreCase)
{
	return !(CompareStringW(LOCALE_SYSTEM_DEFAULT,IgnoreCase?NORM_IGNORECASE:0,m_pData->GetData(),static_cast<int>(length),lpwszCmp,static_cast<int>(length))-2);
}