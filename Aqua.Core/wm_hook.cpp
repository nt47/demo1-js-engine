//#define NOMINMAX // 防止 Windows.h 定义 min 和 max 宏，影响 std::min 和 std::max
#include "wm_hook.h"
#include<Windows.h>
#include<iostream>
#include"js_engine.h"
#include"utils.h"
#include"system.h"
#include"timer.h"
#include"animal.h"
#include"timer_wrapper.h"
#define WM_LOAD_SCRIPTS   (WM_APP + 1)
#define WM_UNLOAD_SCRIPTS   (WM_APP + 2)
#define WM_SET_HOTKEYS	(WM_APP + 3)

WNDPROC oWndProc = NULL;
HWND hMainWindow = NULL;

void OnClose() {

	UnregisterHotKey(hMainWindow, 1001);
	UnregisterHotKey(hMainWindow, 1002);
	timeOut.Clear();
	timeInterval.Clear();
	while (JsEngine::GetInstance().IsRunning())
		Sleep(1000);
	JsEngine::GetInstance().UnloadAllScripts();
	System::PrintLn(LEVEL::INFO, "Scripts unloaded.");
}

void OnLoadScripts() {

	if (g_animal)
	{
		delete g_animal;
		g_animal = nullptr;
	}

	if (!g_animal) {
		g_animal = CreateAnimal();
	}
	timeOut.Clear();
	timeInterval.Clear();
	if (JsEngine::GetInstance().IsRunning())
	{
		System::PrintLn(LEVEL::ERR, "Scripts is running.");
		return;
	}

	JsEngine::GetInstance().LoadScript(L"脚本1.js");
	System::PrintLn(LEVEL::INFO, "Scripts reloaded.");

	g_animal->Speak();
	g_animal->GetAge();
	g_animal->GetType(9, "Dog");

}

void OnUnloadScripts() {

	timeOut.Clear();
	timeInterval.Clear();
	if (JsEngine::GetInstance().IsRunning())
	{
		System::PrintLn(LEVEL::ERR, "Scripts is running.");
		return;
	}
	JsEngine::GetInstance().UnloadAllScripts();
	System::PrintLn(LEVEL::INFO, "Scripts unloaded.");

}

void OnSetHotkeys() {
	if (RegisterHotKey(hMainWindow, 1001, NULL, VK_HOME) &&
		RegisterHotKey(hMainWindow, 1002, NULL, VK_END)
		) {//无法跨线程使用窗口句柄注册热键
		std::cout << "热键注册成功！" << std::endl;
	}
	else {
		std::cout << "热键注册失败！" << std::endl;
	}
}


LRESULT CALLBACK NewWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {

	case WM_CLOSE:
		//MessageBox(hWnd, L"Window is closing!", L"Notification", MB_OK);
		//DestroyWindow(hWnd);//会导致DLL_PROCESS_DETACH失败
		OnClose();
		break;
	case WM_LOAD_SCRIPTS:
		OnLoadScripts();//必须封装成函数加载脚本，否则js异常处理不了
		break;
	case WM_UNLOAD_SCRIPTS:
		OnUnloadScripts();
		break;
	case WM_SET_HOTKEYS:
		OnSetHotkeys();
		break;
	case WM_HOTKEY:
	{
		switch (wParam) {
		case 1001:
			PostMessage(hMainWindow, WM_LOAD_SCRIPTS, 0, 0);
			break;
		case 1002:
			PostMessage(hMainWindow, WM_UNLOAD_SCRIPTS, 0, 0);
			break;
		default:
			break;
		}
	}
	break;
	case WM_NEW_TIMER:
		Timer::TimerCallBack(hWnd, uMsg, wParam, lParam);
		break;
	case WM_DESTROY:
		//PostQuitMessage(0);//会导致DLL_PROCESS_DETACH失败
		break;

	default:
		break;
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

void HookWindowMessage() {
	hMainWindow = FindWindow(NULL, L"EmptyProject");
	if (!hMainWindow) {
		MessageBox(NULL, L"Failed to find target window!", L"Error", MB_ICONERROR);
		return;
	}

	oWndProc = (WNDPROC)SetWindowLongPtr(hMainWindow, GWLP_WNDPROC, (LONG_PTR)NewWindowProc);

	if (oWndProc == 0) {
		MessageBox(NULL, L"Failed to set new window procedure!", L"Error", MB_ICONERROR);
		return;
	}

	SendMessage(hMainWindow, WM_SET_HOTKEYS, 0, 0);
}