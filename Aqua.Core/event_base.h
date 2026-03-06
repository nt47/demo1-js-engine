#pragma once
#include"vmthooks.h"
#include<list>
#include<any>
#include<mutex>
#include"v8.h"
#include"js_engine.h"
#include<iostream>
//inline std::mutex mtx;

struct EventData {
	std::vector<std::any> args; // 任意参数
};

class HookListener {
public:
	HookListener(const HookListener&) = delete;
	HookListener& operator=(const HookListener&) = delete;

	HookListener(const v8::FunctionCallbackInfo<v8::Value>& args)
		: isolate_(args.GetIsolate()) {
		if (!args[0]->IsFunction()) return;

		callback_.Reset(isolate_, args[0].As<v8::Function>());
	}
	void NewEvent(const EventData& data) {
		// 调用 JS 监听器

		//std::cout << v8::Isolate::GetCurrent() << std::endl;
		//if (!v8::Isolate::GetCurrent())
		v8::Locker locker(isolate_);
		v8::Isolate::Scope isolate_scope(isolate_);//多线程必须开启，相当于isolate->Enter()
		v8::HandleScope handle_scope(isolate_);//管理v8变量
		//v8::Local<v8::Context> context = isolate_->GetCurrentContext();//空的，因为被我Exit()了
		v8::Local<v8::Context> context = JsEngine::GetInstance().
			GetIsolateState(isolate_)->context;
		v8::Context::Scope context_scope(context);


		v8::Local<v8::Function> func = callback_.Get(isolate_);

		std::vector<Local<Value>> argv;
		argv.reserve(data.args.size());

		for (auto& a : data.args) {
			if (a.type() == typeid(int)) {
				argv.push_back(Integer::New(isolate_, std::any_cast<int>(a)));
			}
			else if (a.type() == typeid(const char*)) {
				argv.push_back(String::NewFromUtf8(isolate_, std::any_cast<const char*>(a)).ToLocalChecked());
			}
			else {
				argv.push_back(Undefined(isolate_));
			}
		}


		func->Call(context, context->Global(), (int)argv.size(), argv.data()).ToLocalChecked();

	}
	virtual ~HookListener() = default;

protected:
	v8::Isolate* isolate_;
	v8::Global<v8::Function> callback_;
	bool isDispose_ = false;
};

class EventManager {
public:
	EventManager() {}

	void AddListener(HookListener* listener) {
		callbacks_.push_back(listener);
	}

	void RemoveListener(HookListener* listener) {
		std::list<HookListener*>::iterator it = std::find(
			callbacks_.begin(), callbacks_.end(), listener);
		if (it != callbacks_.end()) {
			callbacks_.erase(it);
		}
	}

	void CallListener(const EventData& data) {
		std::list<HookListener*> copy;
		{
			copy = callbacks_;
		}
		for (auto listener : copy) {
			listener->NewEvent(data);
		}
	}
	int GetListenerCount() {
		return (int)callbacks_.size();
	}
protected:
	std::list<HookListener*> callbacks_;
};

class HookManager {
public:
	//HookManager(void* object, int offset) : hook_(new utils::VtableHook(object, offset)) {}
	HookManager() {}
	~HookManager() {
		delete hook_;
	}
	void AddListener(HookListener* listener) {
		//std::lock_guard<std::mutex> lock(mtx);
		callbacks_.push_back(listener);
	}

	void RemoveListener(HookListener* listener) {
		//std::lock_guard<std::mutex> lock(mtx);
		std::list<HookListener*>::iterator it = std::find(
			callbacks_.begin(), callbacks_.end(), listener);
		if (it != callbacks_.end()) {
			callbacks_.erase(it);
		}
	}

	void CallListener(const EventData& data) {
		std::list<HookListener*> copy;
		{
			//std::lock_guard<std::mutex> lock(mtx);
			copy = callbacks_;
		}
		for (auto listener : copy) {
			listener->NewEvent(data);
		}
	}

	int GetListenerCount() {
		//std::lock_guard<std::mutex> lock(mtx);
		return (int)callbacks_.size();
	}
	void Reset(void* newObj, int newOffset) {
		if (hook_)
		{
			hook_->Poof();//应对目标对象虚表销毁
			delete hook_;
		}
		hook_ = new utils::VtableHook(newObj, newOffset);
	}



protected:
	utils::VtableHook* hook_ = nullptr;
	std::list<HookListener*> callbacks_;
};





