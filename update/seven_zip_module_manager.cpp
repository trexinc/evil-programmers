#include "headers.hpp"
#pragma hdrstop

#include "prop_variant.hpp"
#include "seven_zip_module_manager.hpp"

#include "7z.headers/7zip/Archive/IArchive.h"
#include "7z.headers/7zip/Common/FileStreams.h" 

#include <initguid.h>
#include "system.hpp"

DEFINE_GUID(IID_IInArchive, 0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00, 0x00);
DEFINE_GUID(IID_ISequentialOutStream, 0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00);
DEFINE_GUID(IID_IInStream, 0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00);
DEFINE_GUID(IID_IArchiveExtractCallback, 0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x20, 0x00, 0x00);

class seven_zip_module
{
public:
	seven_zip_module(const wchar_t* ModuleName)
	{
		m_Module.reset(LoadLibraryEx(ModuleName, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH));
		if (!m_Module)
			return;

		m_CreateObject = reinterpret_cast<CREATEOBJECT>(GetProcAddress(m_Module.get(), "CreateObject"));
		m_GetHandlerProperty = reinterpret_cast<GETHANDLERPROPERTY>(GetProcAddress(m_Module.get(), "GetHandlerProperty"));
		m_GetHandlerProperty2 = reinterpret_cast<GETHANDLERPROPERTY2>(GetProcAddress(m_Module.get(), "GetHandlerProperty2"));
		m_GetNumberOfFormats = reinterpret_cast<GETNUMBEROFFORMATS>(GetProcAddress(m_Module.get(), "GetNumberOfFormats"));

		auto Result = false;

		if (m_CreateObject && (m_GetHandlerProperty || (m_GetHandlerProperty2 && m_GetNumberOfFormats)))
		{
			if (m_GetHandlerProperty2 && m_GetNumberOfFormats)
			{
				if (m_GetNumberOfFormats(&m_NumberOfFormats) == S_OK)
				{
					m_Uids.reserve(m_NumberOfFormats);

					prop_variant value;

					Result = true;

					for (unsigned int i = 0; i < m_NumberOfFormats; i++)
					{
						if ((m_GetHandlerProperty2(i, NArchive::kClassID, &value) != S_OK) ||
							(value.vt != VT_BSTR))
						{
							Result = false;
							break;
						}

						m_Uids.emplace_back(*reinterpret_cast<GUID*>(value.bstrVal));
					}
				}
			}
			else
			{
				m_NumberOfFormats = 1;

				m_Uids.reserve(m_NumberOfFormats);

				prop_variant value;

				if (m_GetHandlerProperty(NArchive::kClassID, &value) == S_OK && value.vt == VT_BSTR)
				{
					m_Uids.emplace_back(*reinterpret_cast<GUID*>(value.bstrVal));
					Result = true;
				}
			}
		}

		if (!Result)
			m_Module.reset();
	}

	explicit operator bool() const
	{
		return m_Module != nullptr;
	}

	std::vector<GUID> m_Uids;
	unsigned int m_NumberOfFormats;

	using CREATEOBJECT = unsigned int (WINAPI*)(const GUID*, const GUID*, void**);
	using GETHANDLERPROPERTY = HRESULT(WINAPI*)(PROPID propID, PROPVARIANT *value);
	using GETHANDLERPROPERTY2 = HRESULT(WINAPI*)(unsigned int, PROPID, PROPVARIANT*);
	using GETNUMBEROFFORMATS = HRESULT(WINAPI*)(unsigned int*);

	CREATEOBJECT m_CreateObject;
	GETHANDLERPROPERTY m_GetHandlerProperty;
	GETHANDLERPROPERTY2 m_GetHandlerProperty2;
	GETNUMBEROFFORMATS m_GetNumberOfFormats;

private:
	module_ptr m_Module;
};

namespace detail
{
	template<typename T>
	struct releaser
	{
		void operator()(T* Object) const
		{
			Object->Release();
		}
	};
}
template<typename T>
using com_ptr = std::unique_ptr<T, detail::releaser<T>>;

template<typename T, typename... args>
auto make_com_ptr(args... Args)
{
	return com_ptr<T>(new T(std::forward<args>(Args)...));
}

class in_file: public IInStream
{
public:
	bool open(const wchar_t* FileName)
	{
		const auto File = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
		if (File == INVALID_HANDLE_VALUE)
			return false;
		
		m_File.reset(File);
		return true;
	}

	HRESULT WINAPI QueryInterface(const IID& Id, void** Object) override
	{
		if (Id != IID_IInStream)
		{
			*Object = nullptr;
			return E_NOINTERFACE;
		}

		*Object = static_cast<void*>(static_cast<IInStream*>(this));
		AddRef();
		return S_OK;
	}

	ULONG WINAPI AddRef() override
	{
		return ++m_RefCount;
	}

	ULONG WINAPI Release() override
	{
		if (--m_RefCount == 0)
		{
			delete this;
			return 0;
		}

		return m_RefCount;
	}

	HRESULT WINAPI Read(void* Data, unsigned int Size, unsigned int* ProcessedSize) override
	{
		DWORD dwRead;

		if (ReadFile(m_File.get(), Data, Size, &dwRead, nullptr))
		{
			if (ProcessedSize)
				*ProcessedSize = dwRead;
			
			return S_OK;
		}

		return E_FAIL;
	}

	HRESULT WINAPI Seek(long long Offset, unsigned int SeekOrigin, unsigned long long* NewPosition) override
	{
		auto lo = static_cast<DWORD>(Offset);
		lo = SetFilePointer(m_File.get(), lo, nullptr, SeekOrigin);

		if (lo == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
			return E_FAIL;

		if (NewPosition)
			*NewPosition = lo;
		return S_OK;
	}

private:
	int m_RefCount{ 1 };
	handle m_File;
};

class out_file: public ISequentialOutStream
{
public:
	bool open(const wchar_t* FileName)
	{
		DWORD Attr = GetFileAttributes(FileName);
		if (Attr != INVALID_FILE_ATTRIBUTES)
		{
			if (Attr&FILE_ATTRIBUTE_READONLY)
			{
				Attr ^= FILE_ATTRIBUTE_READONLY;
				SetFileAttributes(FileName, Attr);
			}
		}
		else
		{
			Attr = FILE_ATTRIBUTE_NORMAL;
		}

		auto hFile = CreateFile(FileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, Attr | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		m_File.reset(hFile);
		return true;
	}

	HRESULT WINAPI QueryInterface(const IID& Id, void** Object) override
	{
		if (Id != IID_ISequentialOutStream)
		{
			*Object = nullptr;
			return E_NOINTERFACE;
		}

		*Object = this;
		AddRef();
		return S_OK;
	}

	ULONG WINAPI AddRef() override
	{
		return ++m_RefCount;
	}

	ULONG WINAPI Release() override
	{
		if (--m_RefCount == 0)
		{
			delete this;
			return 0;
		}
		return m_RefCount;
	}

	HRESULT WINAPI Write(const void* Data, unsigned int Size, unsigned int* ProcessedSize) override
	{
		DWORD dwWritten;

		if (WriteFile(m_File.get(), Data, Size, &dwWritten, nullptr))
		{
			if (ProcessedSize)
				*ProcessedSize = dwWritten;

			return S_OK;
		}

		return E_FAIL;
	}

private:
	int m_RefCount{ 1 };
	handle m_File;
};

static void CreateDirectory(const wchar_t *FullPath, DWORD dwFileAttributes = INVALID_FILE_ATTRIBUTES)
{
	std::wstring FullPathCopy = FullPath;
	const auto& Advance = [&](auto i) { return std::find(i + 1, FullPathCopy.end(), L'\\'); };

	for (auto i = Advance(FullPathCopy.begin()), end = FullPathCopy.end(); i != end; i = Advance(i))
	{
		*i = 0;
		CreateDirectory(FullPathCopy.data(), nullptr);
		*i = L'\\';
	}

	CreateDirectory(FullPathCopy.data(), nullptr);

	if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
		SetFileAttributes(FullPathCopy.data(), dwFileAttributes);
}

static void CreateDirectories(const wchar_t *FileName)
{
	std::wstring NameCopy = FileName;
	auto LastSlashPos = NameCopy.find_last_of(L'\\');
	if (LastSlashPos != NameCopy.npos)
		NameCopy.resize(LastSlashPos);

	CreateDirectory(NameCopy.data());
}

class archive_extract_callback: public IArchiveExtractCallback
{
public:
	archive_extract_callback(IInArchive* Archive, const callback& Callback):
		m_Archive(Archive),
		m_Callback(Callback)
	{
	}

	void set_extract_folder(const wchar_t* Folder)
	{
		m_FolderToExtract = Folder;
	}

	HRESULT WINAPI QueryInterface(const IID& Id, void** Object) override
	{
		if (Id != IID_IArchiveExtractCallback)
		{
			*Object = nullptr;
			return E_NOINTERFACE;
		}
		
		*Object = this;
		AddRef();
		return S_OK;
	}

	ULONG WINAPI AddRef() override
	{
		return ++m_RefCount;
	}

	ULONG WINAPI Release() override
	{
		if (--m_RefCount == 0)
		{
			delete this;
			return 0;
		}

		return m_RefCount;
	}

	HRESULT WINAPI SetTotal(unsigned long long Total) override
	{
		return S_OK;
	}

	HRESULT WINAPI SetCompleted(const unsigned long long* CompleteValue) override
	{
		return S_OK;
	}

	HRESULT WINAPI GetStream(unsigned int Index, ISequentialOutStream** OutStream, int AskExtractMode) override
	{
		prop_variant value;

		*OutStream = nullptr;
		m_Archive->GetProperty(Index, kpidPath, &value);

		if (value.vt == VT_BSTR)
		{
			const auto FileName = value.bstrVal;

			auto bIsFolder = false;

			if (m_Archive->GetProperty(Index, kpidIsFolder, &value) == S_OK)
			{
				if (value.vt == VT_BOOL)
					bIsFolder = value.boolVal == VARIANT_TRUE;
			}

			DWORD dwFileAttributes = 0;

			if (m_Archive->GetProperty(Index, kpidAttributes, &value) == S_OK)
			{
				if (value.vt == VT_UI4)
					dwFileAttributes = value.ulVal;
			}

			bIsFolder |= (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;

			auto FullName = m_FolderToExtract + FileName;

			if (bIsFolder)
			{
				*OutStream = nullptr;
				CreateDirectory(FullName.data());
			}
			else
			{
				m_Callback(notification, (L" Creating " + FullName).data());
				CreateDirectories(FullName.data());
				if (value.boolVal != VARIANT_TRUE)
				{
					const auto File = make_com_ptr<out_file>();
					bool Opened;
					for (;;)
					{
						Opened = File->open(FullName.data());
						if (Opened)
							break;
						auto Message = L" Error creating "s + FileName + L":\n  "s + GetLastErrorMessage(GetLastError()) + L"\n"s;
						m_Callback(error, Message.data());
						if (!m_Callback(retry_confirmation, L""))
							break;
					}

					if (Opened)
					{
						*OutStream = File.get();
						File->AddRef();
					}
					else
					{
						*OutStream = nullptr;
						return E_ABORT;
					}
				}
			}
		}

		return S_OK;
	}

	HRESULT WINAPI PrepareOperation(int AskExtractMode) override
	{
		return S_OK;
	}

	HRESULT WINAPI SetOperationResult(int ResultEOperationResult) override
	{
		return S_OK;
	}

private:
	int m_RefCount{ 1 };
	IInArchive *m_Archive;
	std::wstring m_FolderToExtract;
	callback m_Callback;
};

seven_zip_module_manager::seven_zip_module_manager(const wchar_t* PathTo7z):
	m_Module(std::make_unique<seven_zip_module>(PathTo7z))
{
	if (!*m_Module)
	{
		m_Module.reset();
	}
}

seven_zip_module_manager::~seven_zip_module_manager() = default;

bool seven_zip_module_manager::extract(const wchar_t* FileName, const wchar_t* DestDir, const callback& Callback) const
{
	if (!m_Module)
	{
		Callback(error, L"7-Zip module not found");
		return false;
	}

	auto Result = false;

	const auto File = make_com_ptr<in_file>();
	
	bool Opened;
	for(;;)
	{
		Opened = File->open(FileName);
		if (Opened)
			break;
		auto Message = L" Error opening "s + FileName + L":\n  "s + GetLastErrorMessage(GetLastError()) + L"\n"s;
		Callback(error, Message.data());
		if (!Callback(retry_confirmation, L""))
			break;
	}

	if (!Opened)
		return false;

	for (unsigned int i = 0; i != m_Module->m_NumberOfFormats; ++i)
	{
		File->Seek(0, FILE_BEGIN, nullptr);

		com_ptr<IInArchive> Archive;

		if (m_Module->m_CreateObject(&m_Module->m_Uids[i], &IID_IInArchive, reinterpret_cast<void**>(&ptr_setter(Archive))) == S_OK)
		{
			if (Archive->Open(File.get(), nullptr, nullptr) == S_OK)
			{
				const auto Cb = make_com_ptr<archive_extract_callback>(Archive.get(), Callback);
				Cb->set_extract_folder(DestDir);

				if (Archive->Extract(nullptr, std::numeric_limits<uint32_t>::max(), 0, Cb.get()) == S_OK)
				{
					Result = true;
					Archive->Close();
					break;
				}
				Archive->Close();
			}
		}
	}
	return Result;
}
