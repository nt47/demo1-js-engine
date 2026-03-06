#include "timer.h"

Timer::Timer(HWND hwnd, int intervalMs, std::function<void()> callback, bool isOneShot)
	: hwnd(hwnd), interval(intervalMs), callback(callback)
{
	running = true;

	// 分配回调数据
	payload = std::make_unique<TimerPayload>();

	payload->timer = this;

	if (isOneShot) {
		CreateTimerQueueTimer(
			&hTimer,
			NULL,
			TimerThreadProc,
			payload.get(),
			interval,
			0,
			WT_EXECUTEDEFAULT
		);

	}
	else
	{
		// 创建系统级计时器（不会被 Win10/11 延迟）
		CreateTimerQueueTimer(
			&hTimer,
			NULL,
			TimerThreadProc,
			payload.get(),
			interval,
			interval,
			WT_EXECUTEDEFAULT
		);
	}
}

Timer::~Timer()
{
	Stop();
}

void Timer::Stop()
{
	if (running)
	{
		running = false;
		DeleteTimerQueueTimer(NULL, hTimer, NULL);
		hTimer = NULL;
	}
}

VOID CALLBACK Timer::TimerThreadProc(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	TimerPayload* payload = (TimerPayload*)lpParam;
	Timer* timer = payload->timer;

	// 回到主线程
	PostMessage(timer->hwnd, WM_NEW_TIMER, (WPARAM)timer, 0);

}

void Timer::TimerCallBack(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	Timer* timer = (Timer*)wParam;
	if (timer->IsRunning())
		timer->callback();   // 主线程执行
}
