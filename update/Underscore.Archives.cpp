#include "headers.hpp"

#include "PropVariant.h"
#include "Underscore.Archives.h"

MY_DEFINE_GUID(IID_IInArchive, 0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00, 0x00);
MY_DEFINE_GUID(IID_ISequentialOutStream, 0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00);
MY_DEFINE_GUID(IID_IInStream, 0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00);
MY_DEFINE_GUID(IID_IArchiveExtractCallback, 0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x20, 0x00, 0x00);

MY_DEFINE_GUID(IID_ICryptoGetTextPassword, 0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x05, 0x00, 0x10, 0x00, 0x00);


bool CInFile::Open (const wchar_t *lpFileName)
{
	HANDLE hFile = CreateFile (lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		m_hFile = hFile;
		return true;
	}

	return false;
}

CInFile::CInFile ()
{
	m_nRefCount = 1;
	m_hFile = INVALID_HANDLE_VALUE;
}

CInFile::~CInFile ()
{
	if ( m_hFile != INVALID_HANDLE_VALUE )
		CloseHandle (m_hFile);
}

HRESULT __stdcall CInFile::QueryInterface (const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_IInStream )
	{
		*ppvObject = (void*)(IInStream*)this;
		AddRef ();

		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG __stdcall CInFile::AddRef ()
{
	return ++m_nRefCount;
}

ULONG __stdcall CInFile::Release ()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}

HRESULT __stdcall CInFile::Read (void *data, unsigned int size, unsigned int *processedSize)
{
	DWORD dwRead;

	if ( ReadFile (m_hFile, data, size, &dwRead, NULL) )
	{
		if ( processedSize )
			*processedSize = dwRead;

		return S_OK;
	}

	return E_FAIL;
}

HRESULT __stdcall CInFile::Seek (__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition)
{
	DWORD lo;

	lo = (DWORD)offset;

	lo = SetFilePointer (m_hFile, lo, NULL, seekOrigin);

	if ( (lo == INVALID_SET_FILE_POINTER) && (GetLastError () != NO_ERROR) )
		return E_FAIL;
	else
	{
		if ( newPosition )
			*newPosition = (unsigned __int64)lo;
		return S_OK;
	}
}

CArchiveExtractCallback::CArchiveExtractCallback (IInArchive *pArchive)
{
	m_pArchive = pArchive;
	m_nRefCount = 1;
	m_lpFolderToExtract = NULL;
}

CArchiveExtractCallback::~CArchiveExtractCallback ()
{
	if (m_lpFolderToExtract)
		LocalFree (m_lpFolderToExtract);
}

ULONG __stdcall CArchiveExtractCallback::AddRef ()
{
	return ++m_nRefCount;
}

ULONG __stdcall CArchiveExtractCallback::Release ()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}

HRESULT __stdcall CArchiveExtractCallback::QueryInterface (const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_IArchiveExtractCallback )
	{
		*ppvObject = (void*)(IArchiveExtractCallback*)this;
		AddRef ();

		return S_OK;
	}

	return E_NOINTERFACE;
}

HRESULT __stdcall CArchiveExtractCallback::SetTotal (unsigned __int64 total)
{
	return S_OK;
} 

HRESULT CArchiveExtractCallback::SetCompleted (const unsigned __int64* completeValue)
{
	return S_OK;
}

void CutToSlash (wchar_t *s)
{
	for (int i = lstrlen(s) - 1; i >= 0; i--)
	{
		if ( s[i] == L'\\')
		{
			s[i+1] = 0;
			break;
		}
	}
}

void CreateDirectoryEx (const wchar_t *lpFullPath, DWORD dwFileAttributes = INVALID_FILE_ATTRIBUTES) //$ 16.05.2002 AA
{
	wchar_t *lpFullPathCopy = StrDup(lpFullPath);

	for (wchar_t *c = lpFullPathCopy; *c; c++)
	{
		if(*c!=' ')
		{
			for(; *c; c++)
			{
				if(*c == L'\\')
				{
					*c=0;
					CreateDirectory (lpFullPathCopy, NULL);
					*c = L'\\';
				}
			}

			CreateDirectory(lpFullPathCopy, NULL);

			if ( dwFileAttributes != INVALID_FILE_ATTRIBUTES )
				SetFileAttributes(lpFullPathCopy, dwFileAttributes);
			break;
		}
	}

	LocalFree(lpFullPathCopy);
}

void CreateDirs (const wchar_t *lpFileName)
{
	wchar_t *lpNameCopy = StrDup (lpFileName);

	CutToSlash (lpNameCopy);

	CreateDirectoryEx (lpNameCopy);

	LocalFree (lpNameCopy);
}

HRESULT __stdcall CArchiveExtractCallback::GetStream (unsigned int index, ISequentialOutStream **outStream, int askExtractMode)
{
	CPropVariant value;

	*outStream = NULL;
	m_pArchive->GetProperty (index, kpidPath, &value);

	if ( value.vt == VT_BSTR )
	{
		wchar_t *lpFileName = new wchar_t[MAX_PATH];

#ifndef _UNICODE
		WideCharToMultiByte (CP_OEMCP, 0, value.bstrVal, -1, lpFileName, 260, NULL, NULL);
#else
		lstrcpy(lpFileName, value.bstrVal);
#endif

		bool bIsFolder = false;

		if ( m_pArchive->GetProperty (index, kpidIsFolder, &value) == S_OK )
		{
			if ( value.vt == VT_BOOL )
				bIsFolder = (value.boolVal == VARIANT_TRUE);
		}

		DWORD dwFileAttributes = 0;

		if ( m_pArchive->GetProperty (index, kpidAttributes, &value) == S_OK )
		{
			if ( value.vt == VT_UI4 )
				dwFileAttributes = value.ulVal;
   	}

   	bIsFolder |= (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;

		wchar_t *lpFullName = new wchar_t[MAX_PATH];

		lstrcpy (lpFullName, m_lpFolderToExtract);
		lstrcat (lpFullName, lpFileName);

		if ( bIsFolder )
		{
			*outStream = NULL;
			CreateDirectoryEx (lpFullName);
		}
		else
		{
			CreateDirs (lpFullName);
			if ( value.boolVal != VARIANT_TRUE ) 
			{
				COutFile *file = new COutFile;
				if (file->Open (lpFullName))
					*outStream = file;
				else
				{
					delete [] lpFullName;
					delete [] lpFileName;
					*outStream = NULL;
					return S_FALSE;
				}
			}
		}

		delete [] lpFullName;
		delete [] lpFileName;
	}

	return S_OK;

}

void CArchiveExtractCallback::SetExtractFolder (const wchar_t *lpFolder)
{
	if (m_lpFolderToExtract)
		LocalFree (m_lpFolderToExtract);

	m_lpFolderToExtract = StrDup (lpFolder);
}

HRESULT __stdcall CArchiveExtractCallback::PrepareOperation (int askExtractMode)
{
	return S_OK;
}

HRESULT __stdcall CArchiveExtractCallback::SetOperationResult (int resultEOperationResult)
{
	return S_OK;
}

bool COutFile::Open (const wchar_t *lpFileName)
{
	DWORD Attr=GetFileAttributes(lpFileName);
	if(Attr!=INVALID_FILE_ATTRIBUTES)
	{
		if(Attr&FILE_ATTRIBUTE_READONLY)
		{
			Attr^=FILE_ATTRIBUTE_READONLY;
			SetFileAttributes(lpFileName,Attr);
		}
	}
	else
	{
		Attr=FILE_ATTRIBUTE_NORMAL;
	}
	HANDLE hFile = CreateFile (lpFileName, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,Attr|FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		m_hFile = hFile;
		return true;
	}

	return false;
}

COutFile::COutFile ()
{
	m_nRefCount = 1;
	m_hFile = INVALID_HANDLE_VALUE;
}

COutFile::~COutFile ()
{
	if ( m_hFile != INVALID_HANDLE_VALUE )
		CloseHandle (m_hFile);
}

HRESULT __stdcall COutFile::QueryInterface (const IID &iid, void ** ppvObject)
{
	*ppvObject = NULL;

	if ( iid == IID_ISequentialOutStream )
	{
		*ppvObject = this;
		AddRef ();

		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG __stdcall COutFile::AddRef ()
{
	return ++m_nRefCount;
}

ULONG __stdcall COutFile::Release ()
{
	if ( --m_nRefCount == 0 )
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}

HRESULT __stdcall COutFile::Write (const void *data, unsigned int size, unsigned int *processedSize)
{
	DWORD dwWritten;

	if ( WriteFile (m_hFile, data, size, &dwWritten, NULL) )
	{
		if ( processedSize )
			*processedSize = dwWritten;

		return S_OK;
	}

	return E_FAIL;
}

bool SevenZipModule::Initialize (const wchar_t *lpModuleName)
{
	bool bResult = false;

	m_hModule = NULL;
	m_lpModuleName = NULL;

	if ( lpModuleName )
	{
		m_lpModuleName = StrDup (lpModuleName);

		m_hModule = LoadLibraryEx (lpModuleName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH); 

		m_pfnCreateObject = (CREATEOBJECT)GetProcAddress (m_hModule, "CreateObject");
		m_pfnGetHandlerProperty = (GETHANDLERPROPERTY)GetProcAddress (m_hModule, "GetHandlerProperty");
		m_pfnGetHandlerProperty2 = (GETHANDLERPROPERTY2)GetProcAddress (m_hModule, "GetHandlerProperty2");
		m_pfnGetNumberOfFormats = (GETNUMBEROFFORMATS)GetProcAddress (m_hModule, "GetNumberOfFormats");

		if ( m_pfnCreateObject &&
			 (m_pfnGetHandlerProperty || (m_pfnGetHandlerProperty2 && m_pfnGetNumberOfFormats)) )
		{
			if ( m_pfnGetHandlerProperty2 && m_pfnGetNumberOfFormats )
			{
				if ( m_pfnGetNumberOfFormats (&m_nNumberOfFormats) == S_OK )
				{
					m_puids = (GUID*)malloc (m_nNumberOfFormats*sizeof (GUID));

					CPropVariant value;

					bResult = true;

					for (unsigned int i = 0; i < m_nNumberOfFormats; i++)
					{
						if ( (m_pfnGetHandlerProperty2 (i, NArchive::kClassID, &value) != S_OK) ||
							 (value.vt != VT_BSTR) )
						{
							bResult = false;
							break;
						}

						memcpy (&m_puids[i], value.bstrVal, sizeof (GUID));
					}
				}
			}
			else
			{
				m_nNumberOfFormats= 1;

				m_puids = (GUID*)malloc (m_nNumberOfFormats*sizeof (GUID));

				CPropVariant value;

				if ( (m_pfnGetHandlerProperty (NArchive::kClassID, &value) == S_OK) &&
					 (value.vt == VT_BSTR) )
				{
					memcpy (&m_puids[0], value.bstrVal, sizeof (GUID));
					bResult = true;
				}
			}
		}
	} 

	return bResult;
}

void SevenZipModule::Finalize ()
{
	if (m_lpModuleName)
		LocalFree (m_lpModuleName);

	if ( m_hModule )
		FreeLibrary(m_hModule);
}

SevenZipModuleManager::SevenZipModuleManager(LPCWSTR s7zPath)
{
	m_module = NULL;
	SevenZipModule *pModule = new SevenZipModule;
	if ( pModule )
	{
		if ( !pModule->Initialize (s7zPath) )
		{
			pModule->Finalize();
			delete pModule;
		}
		m_module = pModule;
	}
}

bool SevenZipModuleManager::Extract (const wchar_t *lpFileName, const wchar_t *lpDestDir)
{
	bool bResult = false;

	CInFile *file = new CInFile;

	if ( file && file->Open (lpFileName) )
	{
		SevenZipModule *pModule = m_module;

		for (unsigned int j = 0; j < pModule->m_nNumberOfFormats; j++)
		{
			file->Seek (0, FILE_BEGIN, NULL);

			IInArchive *pArchive = NULL;

			if ( pModule->m_pfnCreateObject (&pModule->m_puids[j], &IID_IInArchive, (void**)&pArchive) == S_OK )
			{
				if ( pArchive->Open (file, 0, 0) == S_OK )
				{
					CArchiveExtractCallback *pcb = new CArchiveExtractCallback (pArchive);

					pcb->SetExtractFolder(lpDestDir);

					if ( pArchive->Extract (NULL, 0xFFFFFFFF, 0, pcb) == S_OK )
					{
						bResult = true;
						delete pcb;
						pArchive->Close ();
						break;
					}
					pArchive->Close ();
					pArchive->Release ();
				}
			}
		}
	}

	if ( file )
		delete file;

	return bResult;
}

SevenZipModuleManager::~SevenZipModuleManager()
{
	m_module->Finalize();
	delete m_module;
	m_module = NULL;
}

