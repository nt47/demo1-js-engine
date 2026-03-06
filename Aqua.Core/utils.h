#pragma once
#include <string>
#include <filesystem>
#include <windows.h>

namespace utils {
	std::filesystem::path GetModuleDirectoryWithoutSlash();
	std::string GetModuleDirectoryUtf8();
	std::wstring GetModuleDirectoryW();
	HMODULE GetCurrentModule();
	std::wstring GetLastErrorAsString();
}

