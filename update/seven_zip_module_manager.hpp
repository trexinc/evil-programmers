#pragma once

class seven_zip_module;

enum callback_mode
{
	notification,
	error,
	retry_confirmation,
};

using callback = std::function<bool(callback_mode, const wchar_t*)>;

class seven_zip_module_manager
{
public:
	NONCOPYABLE(seven_zip_module_manager);

	explicit seven_zip_module_manager(const wchar_t* PathTo7z);
	~seven_zip_module_manager();

	bool extract(const wchar_t* FileName, const wchar_t* DestDir, const callback& Callback) const;

private:
	std::unique_ptr<seven_zip_module> m_Module;
};
