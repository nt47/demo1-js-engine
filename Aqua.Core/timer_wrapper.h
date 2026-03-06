#pragma once
#include<v8.h>
#include<v8pp/module.hpp>
#include<v8pp/class.hpp>
#include"js_types.h"
#include"timer.h"
#include"js_engine.h"

class TimerWrapper {
public:
	static void Register(js_State* J);
};


class TimeOut {
public:
	std::map<__int64, std::unique_ptr<Timer>> timers;
	std::atomic<__int64> timerId = 0;
	std::map<__int64, v8::Global<v8::Function>> callbacks_;
	std::map<__int64, std::vector<v8::Global<v8::Value>>> extraArgs_;
	void Clear() {
		timers.clear();
		timerId = 0;
		for (auto& [id, callback] : callbacks_) {
			callback.Reset();
		}
		callbacks_.clear();
		extraArgs_.clear();
	}
};

class TimeInterval {
public:
	std::map<__int64, std::unique_ptr<Timer>> timers;
	std::atomic<__int64> timerId = 0;
	std::map<__int64, v8::Global<v8::Function>> callbacks_;
	std::map<__int64, std::vector<v8::Global<v8::Value>>> extraArgs_;
	void Clear() {
		timers.clear();
		timerId = 0;
		for (auto& [id, callback] : callbacks_) {
			callback.Reset();
		}
		callbacks_.clear();
		extraArgs_.clear();
	}
};

extern TimeOut timeOut;
extern TimeInterval timeInterval;