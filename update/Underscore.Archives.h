#pragma once
#include "7z.headers/7zip/Archive/IArchive.h"
#include "7z.headers/7zip/IPassword.h"
#include "7z.headers/7zip/Common/FileStreams.h" 

#define MY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
  extern "C" const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

typedef unsigned int (WINAPI *CREATEOBJECT) (const GUID *, const GUID *, void **);
typedef HRESULT(WINAPI *GETHANDLERPROPERTY) (PROPID propID, PROPVARIANT *value);
typedef HRESULT(WINAPI *GETHANDLERPROPERTY2) (unsigned int formatIndex, PROPID propID, PROPVARIANT *value);
typedef HRESULT(WINAPI *GETNUMBEROFFORMATS) (unsigned int *numFormats);

class SevenZipModule
{
public:
	bool Initialize (const wchar_t *lpModuleName);
	void Finalize ();

	HMODULE m_hModule;

	CREATEOBJECT m_pfnCreateObject;
	GETHANDLERPROPERTY m_pfnGetHandlerProperty;
	GETHANDLERPROPERTY2 m_pfnGetHandlerProperty2;
	GETNUMBEROFFORMATS m_pfnGetNumberOfFormats;

	unsigned int m_nNumberOfFormats;
	GUID *m_puids;
	wchar_t* m_lpModuleName;
};


class SevenZipModuleManager
{
public:
	SevenZipModuleManager(const wchar_t* s7zPath);
	~SevenZipModuleManager();

	bool Extract(const wchar_t* FileName, const wchar_t* DestDir);

	SevenZipModule *m_module;
};


class CInFile : public IInStream
{
public:
	CInFile();
	~CInFile();

	bool Open (const wchar_t *lpFileName);

	virtual HRESULT WINAPI QueryInterface(REFIID iid, void** ppvObject);
	virtual ULONG WINAPI AddRef();
	virtual ULONG WINAPI Release();
	
	virtual HRESULT WINAPI Read(void *data, unsigned int size, unsigned int *processedSize);
	virtual HRESULT WINAPI Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition);

private:
	HANDLE m_hFile;
	int m_nRefCount;
};



class CArchiveExtractCallback : public IArchiveExtractCallback
{
public:
	CArchiveExtractCallback(IInArchive *pArchive);
	~CArchiveExtractCallback();

	void SetExtractFolder (const wchar_t *lpFolder);

	virtual HRESULT WINAPI QueryInterface(const IID &iid, void ** ppvObject);
	virtual ULONG WINAPI AddRef();
	virtual ULONG WINAPI Release();

	virtual HRESULT WINAPI SetTotal(unsigned __int64 total);
	virtual HRESULT WINAPI SetCompleted(const unsigned __int64* completeValue);

	virtual HRESULT WINAPI GetStream(unsigned int index, ISequentialOutStream** outStream, int askExtractMode);
	// GetStream OUT: S_OK - OK, S_FALSE - skip this file
	virtual HRESULT WINAPI PrepareOperation(int askExtractMode);
	virtual HRESULT WINAPI SetOperationResult(int resultEOperationResult);

private:
	int m_nRefCount;
	IInArchive *m_pArchive;
	wchar_t *m_lpFolderToExtract;
};

class COutFile : public ISequentialOutStream
{
public:
	COutFile();
	~COutFile();

	bool Open(const wchar_t *lpFileName);

	virtual HRESULT WINAPI QueryInterface(const IID &iid, void ** ppvObject);
	virtual ULONG WINAPI AddRef();
	virtual ULONG WINAPI Release();
	virtual HRESULT WINAPI Write(const void *data, unsigned int size, unsigned int* processedSize);

private:
	HANDLE m_hFile;
	int m_nRefCount;
};


