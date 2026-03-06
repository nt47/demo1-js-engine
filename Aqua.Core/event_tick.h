#pragma once
#include"event_base.h"
#include"v8.h"
#include"v8pp/class.hpp"
#include"js_state.h"
#include"timer.h"
#include"js_helper.h"

class TickEventManager :public EventManager {
public:

	static TickEventManager& GetInstance() {
		static TickEventManager instance;
		return instance;
	}

	void Create() {
		if (!timer_)
			timer_ = new Timer(FindWindow(NULL, L"EmptyProject"), 100, OnTick);
	}
	void Destroy() {
		if (timer_) {
			delete timer_;
			timer_ = nullptr;
		}
	}


private:
	TickEventManager() {//只能针对初始化一次的Timer，否则没意义

	}

	static void OnTick() {
		auto& mgr = TickEventManager::GetInstance();
		EventData data;
		//data.args = { args... };
		std::cout << "TickEventManager::OnTick called" << std::endl;
		mgr.CallListener(data);
	}
	Timer* timer_ = nullptr;
};


class TickEventListener : public HookListener {
public:
	TickEventListener(const TickEventListener&) = delete;
	TickEventListener& operator=(const TickEventListener&) = delete;

	explicit TickEventListener(const v8::FunctionCallbackInfo<v8::Value>& args) :HookListener(args)
	{

		Create();
	}

	void Create() {
		auto& mgr = TickEventManager::GetInstance();
		if (mgr.GetListenerCount() == 0)
		{
			mgr.Create();
		}
		mgr.AddListener(this);
	}

	void Destroy() {
		auto& mgr = TickEventManager::GetInstance();
		callback_.Reset();
		mgr.RemoveListener(this);
		if (mgr.GetListenerCount() == 0)
			mgr.Destroy();
	}

	void Dispose() {
		if (!isDispose_) {
			isDispose_ = true;
			Destroy();
		}
	}

	~TickEventListener() {
		if (!isDispose_) {
			Destroy();
		}
	}


	static void Register(js_State* J) {
		v8::Isolate* isolate = J->isolate();
		v8pp::class_<TickEventListener> TickEventListener_class(isolate);
		TickEventListener_class
			.ctor<const v8::FunctionCallbackInfo<v8::Value>&>()
			.auto_wrap_objects(true)
			.function("Dispose", &Dispose);

		J->class_("TickEventListener", TickEventListener_class);
		J->function("AddTickEvent", [](const v8::FunctionCallbackInfo<v8::Value>& args) {
			return new_js_object(args.GetIsolate(), "TickEventListener", args[0]);
			});
	}

};