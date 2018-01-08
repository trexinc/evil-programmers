#include "headers.hpp"
#pragma hdrstop

#include <initguid.h>

#include "prop_variant.hpp"
#include "seven_zip_module_manager.hpp"

#include "7z.headers/7zip/Archive/IArchive.h"
#include "7z.headers/7zip/Common/FileStreams.h"

#include "system.hpp"

class seven_zip_module
{
public:
	NONCOPYABLE(seven_zip_module);

	explicit seven_zip_module(const wchar_t* ModuleName)
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
				unsigned int NumberOfFormats;
				if (m_GetNumberOfFormats(&NumberOfFormats) == S_OK)
				{
					m_Uids.reserve(NumberOfFormats);

					prop_variant value;

					Result = true;

					for (unsigned int i = 0; i != NumberOfFormats; ++i)
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

	auto begin() const
	{
		return m_Uids.cbegin();
	}

	auto end() const
	{
		return m_Uids.cend();
	}

	using CREATEOBJECT = unsigned int (WINAPI*)(const GUID*, const GUID*, void**);
	using GETHANDLERPROPERTY = HRESULT(WINAPI*)(PROPID propID, PROPVARIANT *value);
	using GETHANDLERPROPERTY2 = HRESULT(WINAPI*)(unsigned int, PROPID, PROPVARIANT*);
	using GETNUMBEROFFORMATS = HRESULT(WINAPI*)(unsigned int*);

	CREATEOBJECT m_CreateObject;
	GETHANDLERPROPERTY m_GetHandlerProperty;
	GETHANDLERPROPERTY2 m_GetHandlerProperty2;
	GETNUMBEROFFORMATS m_GetNumberOfFormats;

private:
	std::vector<UUID> m_Uids;
	module_ptr m_Module;
};

namespace com
{
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
	using ptr = std::unique_ptr<T, detail::releaser<T>>;

	template<typename T, typename... args>
	auto make_ptr(args... Args)
	{
		return ptr<T>(new T(std::forward<args>(Args)...));
	}
}

template<typename T>
class reference_counted: public T
{
public:
	reference_counted() = default;
	NONCOPYABLE(reference_counted);

	HRESULT WINAPI QueryInterfaceImpl(const IID& ThisId, const IID& Id, void** Object)
	{
		if (ThisId != Id)
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

protected:
	virtual ~reference_counted() = default;

private:
	int m_RefCount = 1;
};

class in_file final: public reference_counted<IInStream>
{
public:
	in_file() = default;

	NONCOPYABLE(in_file);

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
		return QueryInterfaceImpl(IID_IInStream, Id, Object);
	}

	HRESULT WINAPI Read(void* Data, unsigned int Size, unsigned int* ProcessedSize) override
	{
		DWORD Read;
		if (!ReadFile(m_File.get(), Data, Size, &Read, nullptr))
			return E_FAIL;

		if (ProcessedSize)
			*ProcessedSize = Read;
			
		return S_OK;
	}

	HRESULT WINAPI Seek(long long Offset, unsigned int SeekOrigin, unsigned long long* NewPosition) override
	{
		LARGE_INTEGER Distance, NewPointer;
		Distance.QuadPart = Offset;
		if (!SetFilePointerEx(m_File.get(), Distance, &NewPointer, SeekOrigin))
			return E_FAIL;

		if (NewPosition)
			*NewPosition = NewPointer.QuadPart;

		return S_OK;
	}

private:
	~in_file() = default;
	handle m_File;
};

class out_file final: public reference_counted<ISequentialOutStream>
{
public:
	out_file() = default;

	NONCOPYABLE(out_file);

	bool open(const wchar_t* FileName)
	{
		auto Attr = GetFileAttributes(FileName);
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

		const auto File = CreateFile(FileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, Attr | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
		if (File == INVALID_HANDLE_VALUE)
			return false;

		m_File.reset(File);
		return true;
	}

	HRESULT WINAPI QueryInterface(const IID& Id, void** Object) override
	{
		return QueryInterfaceImpl(IID_ISequentialOutStream, Id, Object);
	}

	HRESULT WINAPI Write(const void* Data, unsigned int Size, unsigned int* ProcessedSize) override
	{
		DWORD Written;
		if (!WriteFile(m_File.get(), Data, Size, &Written, nullptr))
			return E_FAIL;

		if (ProcessedSize)
			*ProcessedSize = Written;

		return S_OK;
	}

private:
	~out_file() = default;
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
	const auto LastSlashPos = NameCopy.find_last_of(L'\\');
	if (LastSlashPos != NameCopy.npos)
		NameCopy.resize(LastSlashPos);

	CreateDirectory(NameCopy.data());
}

class archive_extract_callback final: public reference_counted<IArchiveExtractCallback>
{
public:
	NONCOPYABLE(archive_extract_callback);

	archive_extract_callback(IInArchive* Archive, callback Callback):
		m_Archive(Archive),
		m_Callback(std::move(Callback))
	{
	}

	void set_extract_folder(const wchar_t* Folder)
	{
		m_FolderToExtract = Folder;
	}

	HRESULT WINAPI QueryInterface(const IID& Id, void** Object) override
	{
		return QueryInterfaceImpl(IID_IArchiveExtractCallback, Id, Object);
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
					const auto File = com::make_ptr<out_file>();
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
	~archive_extract_callback() = default;
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

	const auto File = com::make_ptr<in_file>();
	
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

	for (const auto& i: *m_Module)
	{
		File->Seek(0, FILE_BEGIN, nullptr);

		com::ptr<IInArchive> Archive;

		if (m_Module->m_CreateObject(&i, &IID_IInArchive, reinterpret_cast<void**>(&ptr_setter(Archive))) == S_OK)
		{
			if (Archive->Open(File.get(), nullptr, nullptr) == S_OK)
			{
				const auto Cb = com::make_ptr<archive_extract_callback>(Archive.get(), Callback);
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
