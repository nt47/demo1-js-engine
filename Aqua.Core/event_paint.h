#pragma once
#include"event_base.h"
#include"animal.h"
#include"v8.h"
#include"v8pp/class.hpp"
#include"js_state.h"
#include"js_helper.h"


using PaintFn = const char* (int, const char*);

template<typename Fn>
class PaintHookManager;   // 这是前置声明（必须有）

// 然后紧接着写偏特化版本（真正的类定义）
template<typename Ret, typename... Args>
class PaintHookManager<Ret(Args...)> : public HookManager {
public:

	static PaintHookManager& GetInstance() { //已测试，管理器不适合动态创建
		static PaintHookManager instance;//初始化一次，但可以反复赋值
		return instance;
	}


	void Create() {
		if (!hook_->Hooked())
			hook_->Rehook();

		hook_->HookMethod(CHudHealthBars_Paint, 3);
	}

	void Destroy() {
		if (hook_->Hooked())
			hook_->Unhook();
	}

private:

	PaintHookManager()
	{
	}

	static Ret __fastcall CHudHealthBars_Paint(
		void* thisptr, Args... args)
	{
		//using OriginalFn = Ret(__fastcall*)(void*, Args...);
		using OriginalFn = Ret(__fastcall*)(void*, int, const char*);
		auto& mgr = PaintHookManager::GetInstance();

		EventData data;
		data.args = { args... };

		// 调用原函数
		//const char* name = mgr.hook_->GetMethod<OriginalFn>(3)(thisptr, args...);
		const char* name = mgr.hook_->GetMethod<OriginalFn>(3)(thisptr, 3, std::any_cast<const char*>(data.args[1]));

		data.args.push_back(name);//最后一个是返回值
		data.args[0] = 6;
		data.args[1] = "Cat";
		mgr.CallListener(data);

		return name;


	}
};

class PaintHookListener : public HookListener {
public:
	PaintHookListener(const PaintHookListener&) = delete;
	PaintHookListener& operator=(const PaintHookListener&) = delete;

	explicit PaintHookListener(const v8::FunctionCallbackInfo<v8::Value>& args) :HookListener(args)
	{
		//std::lock_guard<std::mutex> lock(mtx);
		Create();
	}

	~PaintHookListener() {
		std::cout << "~PaintHookListener()" << std::endl;
		if (!isDispose_) {

			Destroy();
		}
	}


	void Create() {
		auto& mgr = PaintHookManager<PaintFn>::GetInstance();
		if (mgr.GetListenerCount() == 0)
		{
			mgr.Reset(g_animal, 0);
			mgr.Create();
		}
		mgr.AddListener(this);
	}

	void Destroy() {
		auto& mgr = PaintHookManager<PaintFn>::GetInstance();
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

	static void Register(js_State* J) {
		v8::Isolate* isolate = J->isolate();
		v8pp::class_<PaintHookListener> PaintHookListener_class(isolate);
		PaintHookListener_class
			.ctor<const v8::FunctionCallbackInfo<v8::Value>&>()
			.auto_wrap_objects(true)
			.function("Dispose", &Dispose);

		J->class_("PaintHookListener", PaintHookListener_class);
		J->function("AddPaintHook", [](const v8::FunctionCallbackInfo<v8::Value>& args) {
			//v8::Isolate* isolate = args.GetIsolate();
			//v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
			//v8::Local<v8::Object> global = ctx->Global();
			//v8::Local<v8::Value> ctor_val =
			//	global->Get(ctx, v8pp::to_v8(isolate, "PaintHookListener")).ToLocalChecked();

			//v8::Local<v8::Function> ctor = ctor_val.As<v8::Function>();

			//v8::Local<v8::Value> argv[] = { args[0] };
			//return ctor->NewInstance(ctx, 1, argv).ToLocalChecked();
			return new_js_object(args.GetIsolate(), "PaintHookListener", args[0]);
			});


	}

};



