#pragma once
#include "7z.headers/7zip/Archive/IArchive.h"
#include "7z.headers/7zip/IPassword.h"
#include "7z.headers/7zip/Common/FileStreams.h" 

#ifdef __cplusplus
  #define MY_EXTERN_C extern "C"
#else
  #define MY_EXTERN_C extern
#endif

#define MY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
  MY_EXTERN_C const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

typedef unsigned int (__stdcall *CREATEOBJECT) (const GUID *, const GUID *, void **);
typedef HRESULT (__stdcall *GETHANDLERPROPERTY) (PROPID propID, PROPVARIANT *value);
typedef HRESULT (__stdcall *GETHANDLERPROPERTY2) (unsigned int formatIndex, PROPID propID, PROPVARIANT *value);
typedef HRESULT (__stdcall *GETNUMBEROFFORMATS) (unsigned int *numFormats);

class SevenZipModule {

public:

	HMODULE m_hModule;

	CREATEOBJECT m_pfnCreateObject;
	GETHANDLERPROPERTY m_pfnGetHandlerProperty;
	GETHANDLERPROPERTY2 m_pfnGetHandlerProperty2;
	GETNUMBEROFFORMATS m_pfnGetNumberOfFormats;

	unsigned int m_nNumberOfFormats;
	GUID *m_puids;

	wchar_t *m_lpModuleName;

public:

	bool Initialize (const wchar_t *lpModuleName);
	void Finalize ();
};


class SevenZipModuleManager {

public:
	SevenZipModule *m_module;
public:
	SevenZipModuleManager(LPCTSTR s7zPath);
	~SevenZipModuleManager();

	bool Extract (const wchar_t *lpFileName, const wchar_t *lpDestDir);
};


class CInFile : public IInStream {

private:

	HANDLE m_hFile;
	int m_nRefCount;

public:

	CInFile ();
	~CInFile ();

	bool Open (const wchar_t *lpFileName);

	virtual HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject);
	virtual ULONG __stdcall AddRef ();
	virtual ULONG __stdcall Release ();
	
	virtual HRESULT __stdcall Read (void *data, unsigned int size, unsigned int *processedSize);
	virtual HRESULT __stdcall Seek (__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition);
};



class CArchiveExtractCallback : public IArchiveExtractCallback {

private:

	int m_nRefCount;

	IInArchive *m_pArchive;

	wchar_t *m_lpFolderToExtract;

public:

	CArchiveExtractCallback (IInArchive *pArchive);
	~CArchiveExtractCallback ();

	void SetExtractFolder (const wchar_t *lpFolder);

	virtual HRESULT __stdcall QueryInterface (const IID &iid, void ** ppvObject);
	virtual ULONG __stdcall AddRef ();
	virtual ULONG __stdcall Release ();

	virtual HRESULT __stdcall SetTotal (unsigned __int64 total);
	virtual HRESULT __stdcall SetCompleted (const unsigned __int64* completeValue);

	virtual HRESULT __stdcall GetStream (unsigned int index, ISequentialOutStream **outStream, int askExtractMode);
  // GetStream OUT: S_OK - OK, S_FALSE - skeep this file
	virtual HRESULT __stdcall PrepareOperation (int askExtractMode);
	virtual HRESULT __stdcall SetOperationResult (int resultEOperationResult);
};

class COutFile : public ISequentialOutStream {

private:

	HANDLE m_hFile;
	int m_nRefCount;

public:

	COutFile ();
	~COutFile ();

	bool Open (const wchar_t *lpFileName);

	virtual HRESULT __stdcall QueryInterface (const IID &iid, void ** ppvObject);
	virtual ULONG __stdcall AddRef ();
	virtual ULONG __stdcall Release ();

	virtual HRESULT __stdcall Write (const void *data, unsigned int size, unsigned int* processedSize);
};


