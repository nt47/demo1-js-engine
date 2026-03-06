#include "utils.h"

namespace utils {
	std::filesystem::path GetModulePath(HMODULE module) {
		wchar_t buffer[MAX_PATH];
		DWORD len = GetModuleFileNameW(module, buffer, MAX_PATH);
		return std::filesystem::path(buffer, buffer + len);
	}

	extern "C" IMAGE_DOS_HEADER __ImageBase;

	HMODULE GetCurrentModule() {
		return reinterpret_cast<HMODULE>(&__ImageBase);
	}

	std::filesystem::path GetModuleDirectoryWithoutSlash() {
		auto module_path = GetModulePath(GetCurrentModule());
		return module_path.parent_path();  // 自动取目录
	}

	std::string GetModuleDirectoryUtf8()
	{
		std::filesystem::path dir = GetModuleDirectoryWithoutSlash();
		dir /= "";  // Windows 下自动加反斜杠

		std::u8string u8 = dir.u8string();
		return std::string(reinterpret_cast<const char*>(u8.data()), u8.size());
	}

	std::wstring GetModuleDirectoryW()
	{
		std::filesystem::path dir = GetModuleDirectoryWithoutSlash();
		dir /= "";  // Windows 下自动加反斜杠

		return dir.wstring();
	}

	// 获取最后一次错误的信息
	std::wstring GetLastErrorAsString() {
		DWORD errorCode = GetLastError();
		LPVOID errorMsgBuffer = nullptr;

		DWORD result = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			errorCode,
			0,
			(LPWSTR)&errorMsgBuffer,
			0,
			NULL
		);

		std::wstring errorMessage;

		if (result != 0) {
			// 构建错误信息字符串
			errorMessage = L"Error Code: " + std::to_wstring(errorCode) + L"\n";
			errorMessage += L"Error Message: " + std::wstring(static_cast<LPCWSTR>(errorMsgBuffer)) + L"\n";

			// 释放分配的缓冲区
			LocalFree(errorMsgBuffer);
		}
		else {
			errorMessage = L"Failed to get error message.\n";
		}

		return errorMessage;
	}

}



