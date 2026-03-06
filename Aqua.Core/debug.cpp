#include "debug.h"
#include<Windows.h>
#include<iostream>
void EnableANSI()
{
	// 获取控制台输出句柄并设置虚拟终端处理
	DWORD dwMode = 0;
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	// 获取当前控制台模式
	if (GetConsoleMode(hOut, &dwMode))
	{
		// 启用虚拟终端处理，支持 ANSI 转义序列
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hOut, dwMode);
	}
}
void Debug::Console()
{
	wchar_t title[56];
	swprintf_s(title, L"Debug Window - %d", GetCurrentProcessId());

	setlocale(LC_ALL, "chs");
	AllocConsole();
	SetConsoleTitle(title);
	//freopen_s("CON", "w", stdout);
	FILE* consoleOutput;
	freopen_s(&consoleOutput, "CONOUT$", "w", stdout);

	SetConsoleOutputCP(CP_UTF8); // 设置控制台输出 UTF-8
	EnableANSI();
}