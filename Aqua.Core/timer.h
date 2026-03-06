#pragma once
#include <windows.h>
#include <functional>
#include<memory>

#define WM_NEW_TIMER (WM_APP + 4)
struct TimerPayload;

class Timer
{
public:
	Timer(HWND hwnd, int intervalMs, std::function<void()> callback, bool isOneShot = false);
	~Timer();

	void Stop();
	bool IsRunning() const { return running; }
	static void TimerCallBack(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	static VOID CALLBACK TimerThreadProc(PVOID lpParam, BOOLEAN TimerOrWaitFired);

private:
	HWND hwnd;
	int interval;
	std::function<void()> callback;

	HANDLE hTimer = NULL;
	bool running = false;
	std::unique_ptr<TimerPayload> payload;
};

struct TimerPayload
{
	Timer* timer;
};
