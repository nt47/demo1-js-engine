//#define NOMINMAX // 防止 Windows.h 定义 min 和 max 宏，影响 std::min 和 std::max
#include "framework.h"
#include<iostream>
#include<thread>
#include<v8.h>
#include <libplatform/libplatform.h>
#include"system.h"
#include"debug.h"
#include"timer.h"
#include"wm_hook.h"
#include"animal.h"
#include"test_promise.h"
using namespace v8;
using namespace std;

bool g_unload = false;
bool g_reload = false;
bool g_exit = false;



void MainThread() {
	Debug::Console();
	System::PrintLn(LEVEL::SUCCESS, "程序开始运行！");

	std::unique_ptr<Platform> platform = platform::NewDefaultPlatform();//不需要手动删除，unique_ptr 会自动释放
	V8::InitializePlatform(platform.get());
	//V8::SetFlagsFromString("--expose-gc", strlen("--expose-gc"));
	V8::Initialize();

	HookWindowMessage();

	//std::unique_ptr<Timer> timer = make_unique<Timer>(FindWindow(NULL, L"EmptyProject"), 1000, []() {
	//	System::PrintLn(LEVEL::DEBUG, "新时钟每1秒执行一次！");
	//	});



	while (true) {

		if (g_exit) {
			break;
		}
		//System::PrintLn(LEVEL::DEBUG, "主线程每3秒执行一次！");
		//if (g_animal)
		//{
		//	g_animal->Speak();
		//	g_animal->GetAge();
		//	g_animal->GetType(9, "Dog");
		//}
		MainThreadTaskQueue::Instance().PumpTasks();
		Sleep(1000);
	}

}

void Cleanup() {
	MessageBox(NULL, L"程序即将退出！", L"提示", MB_OK | MB_ICONINFORMATION);

	//已经不需要手动卸载了
	V8::Dispose();
	V8::DisposePlatform();
	System::PrintLn(LEVEL::INFO, "资源已经释放！");
}

void Initialize() {
	thread mainThread(MainThread);
	mainThread.detach();

}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);// 禁用线程库调用以优化性能,并且防止死锁
		Initialize();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		Cleanup();
		break;
	}
	return TRUE;
}

