#include<Windows.h>
#include<TlHelp32.h>
#include<vector>
// 挂起当前进程中除当前线程外的所有线程（RAII）
class Suspender {
public:
	Suspender() {
		const DWORD currentTid = GetCurrentThreadId();
		const DWORD currentPid = GetCurrentProcessId();

		HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (snap == INVALID_HANDLE_VALUE)
			return;

		THREADENTRY32 te{};
		te.dwSize = sizeof(te);
		if (!Thread32First(snap, &te)) {
			CloseHandle(snap);
			return;
		}

		do {
			if (te.th32OwnerProcessID != currentPid)
				continue;
			if (te.th32ThreadID == currentTid)
				continue;

			HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION,
				FALSE, te.th32ThreadID);
			if (!hThread)
				continue;

			if (SuspendThread(hThread) != (DWORD)-1) {
				threads_.push_back(hThread);
			}
			else {
				CloseHandle(hThread);
			}
		} while (Thread32Next(snap, &te));

		CloseHandle(snap);
	}

	~Suspender() {
		for (HANDLE hThread : threads_) {
			ResumeThread(hThread);
			CloseHandle(hThread);
		}
	}

	Suspender(const Suspender&) = delete;
	Suspender& operator=(const Suspender&) = delete;

private:
	std::vector<HANDLE> threads_;
};