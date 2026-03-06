#include "timer_wrapper.h"

void TimerWrapperCallback(v8::Isolate* isolate,
	v8::Global<v8::Function>& callback,
	std::vector<v8::Global<v8::Value>>& extraArgs) {


	v8::Locker locker(isolate);
	v8::Isolate::Scope isolate_scope(isolate);//多线程必须开启，相当于isolate->Enter()
	v8::HandleScope handle_scope(isolate);//管理v8变量
	//v8::Local<v8::Context> context = isolate_->GetCurrentContext();//空的，因为被我Exit()了
	v8::Local<v8::Context> context = JsEngine::GetInstance().
		GetIsolateState(isolate)->context;
	v8::Context::Scope context_scope(context);


	v8::Local<v8::Function> func = callback.Get(isolate);

	std::vector<v8::Local<v8::Value>> argv;
	argv.reserve(extraArgs.size());

	for (auto& g : extraArgs) {
		argv.push_back(g.Get(isolate));
	}

	func->Call(context, context->Global(), (int)argv.size(), argv.data()).ToLocalChecked();

}


TimeOut timeOut;
TimeInterval timeInterval;

__int64 SetTimeOut(const v8::FunctionCallbackInfo<v8::Value>& args) {
	__int64 id = ++timeOut.timerId;
	if (!args[0]->IsFunction()) return 0;
	if (!args[1]->IsNumber()) return 0;

	v8::Isolate* isolate = args.GetIsolate();
	timeOut.callbacks_[id].Reset(isolate, args[0].As<v8::Function>());

	for (int i = 2; i < args.Length(); ++i) {
		timeOut.extraArgs_[id].emplace_back(args.GetIsolate(), args[i]);
	}

	timeOut.timers[id] = std::make_unique<Timer>(FindWindow(NULL, L"EmptyProject"), v8pp::from_v8<int>(args.GetIsolate(), args[1]), [=]() {
		TimerWrapperCallback(isolate, timeOut.callbacks_[id], timeOut.extraArgs_[id]);
		}, true);
	return id;
}

__int64 SetInterval(const v8::FunctionCallbackInfo<v8::Value>& args) {//参数必须写const，否则无法识别参数个数
	__int64 id = ++timeInterval.timerId;
	if (!args[0]->IsFunction()) return 0;
	if (!args[1]->IsNumber()) return 0;

	v8::Isolate* isolate = args.GetIsolate();
	timeInterval.callbacks_[id].Reset(isolate, args[0].As<v8::Function>());

	for (int i = 2; i < args.Length(); ++i) {
		timeInterval.extraArgs_[id].emplace_back(args.GetIsolate(), args[i]);
	}

	timeInterval.timers[id] = std::make_unique<Timer>(FindWindow(NULL, L"EmptyProject"), v8pp::from_v8<int>(args.GetIsolate(), args[1]), [=]() {
		TimerWrapperCallback(isolate, timeInterval.callbacks_[id], timeInterval.extraArgs_[id]);
		});
	return id;
}

void ClearTimeOut(__int64 id) {
	timeOut.timers.erase(id); // unique_ptr 自动 delete
}

void ClearInterval(__int64 id) {
	timeInterval.timers.erase(id); // unique_ptr 自动 delete
}


void TimerWrapper::Register(js_State* J) {
	J->function("setTimeout", SetTimeOut);
	J->function("setInterval", SetInterval);
	J->function("clearTimeout", ClearTimeOut);
	J->function("clearInterval", ClearInterval);
}
