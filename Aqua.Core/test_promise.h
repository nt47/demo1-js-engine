#pragma once
#include <v8pp/class.hpp>
#include "js_types.h"
#include <iostream>
#include <future>
#include <thread>
#include <queue>
#include <mutex>
#include <functional>
#include"js_engine.h"
#include"js_guard.h"

// ===============================
// 主线程任务队列（你需要在主线程每帧调用 PumpTasks）
// ===============================
class MainThreadTaskQueue {
public:
	static MainThreadTaskQueue& Instance() {
		static MainThreadTaskQueue inst;
		return inst;
	}

	void Enqueue(std::function<void()> fn) {
		std::lock_guard<std::mutex> lock(mutex_);
		tasks_.push(std::move(fn));
	}

	void PumpTasks() {
		std::queue<std::function<void()>> local;

		{
			std::lock_guard<std::mutex> lock(mutex_);
			std::swap(local, tasks_);
		}

		while (!local.empty()) {
			local.front()();
			local.pop();
		}
	}
	void Clear() {
		std::lock_guard<std::mutex> lock(mutex_);
		while (!tasks_.empty()) {
			tasks_.pop();
		}
	}

private:
	std::mutex mutex_;
	std::queue<std::function<void()>> tasks_;
};


// ===============================
// 耗时任务
// ===============================
inline int calculate_sum() {
	std::this_thread::sleep_for(std::chrono::seconds(3));
	return 5 + 10;
}

// ===============================
// V8 Promise 异步函数
// ===============================
inline v8::Local<v8::Promise> promise_test_func(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();

	auto resolver = v8::Promise::Resolver::New(isolate->GetCurrentContext()).ToLocalChecked();

	// 用 shared_ptr 包住 Global，避免复制问题
	auto shared_resolver = std::make_shared<v8::Global<v8::Promise::Resolver>>(isolate, resolver);

	// 不要捕获 isolate（危险）
	// 只捕获 shared_resolver
	std::thread([shared_resolver, isolate]() {
		auto js_guard = std::make_shared<JsGuard>();
		int result = calculate_sum();

		// 回到主线程执行
		MainThreadTaskQueue::Instance().Enqueue([shared_resolver, result, isolate, js_guard]() {
			// 在主线程获取 isolate
			//v8::Isolate* isolate = v8::Isolate::GetCurrent();
			if (!isolate) return; // isolate 已经退出，安全退出
			v8::Locker locker(isolate);
			v8::Isolate::Scope isolate_scope(isolate);
			v8::HandleScope handle_scope(isolate);
			v8::Local<v8::Context> context = JsEngine::GetInstance().
				GetIsolateState(isolate)->context;
			v8::Context::Scope context_scope(context);

			auto local_resolver = shared_resolver->Get(isolate);
			if (local_resolver.IsEmpty()) return; // resolver 已被 GC 回收

			local_resolver->Resolve(
				isolate->GetCurrentContext(),
				v8::Integer::New(isolate, result)
			).Check();
			});

		}).detach();

	return resolver->GetPromise();
}


// ===============================
// 注册到 JS
// ===============================
class TestPromise {
public:
	static void Register(js_State* J) {
		v8::Isolate* isolate = J->isolate();
		J->function("promise_test_func", promise_test_func);
	}
};
