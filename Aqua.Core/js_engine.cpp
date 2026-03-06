#include "js_engine.h"
#include"js_global.h"

JsEngine::JsEngine() {}


JsEngine& JsEngine::GetInstance() {
	static JsEngine  instance;//全局变量，只定义一次
	return instance;
}

void JsEngine::UnloadScript(std::wstring name) {
	std::map<std::wstring, JsState>::const_iterator it = scripts_.find(name);
	if (it != scripts_.end()) {
		states_.erase(scripts_[name].isolate);
		scripts_.erase(name);
	}
}

void JsEngine::LoadScript(std::wstring name) {
	UnloadScript(name);
	bool ok = false;
	{
		v8::HandleScope scope(scripts_[name].isolate);
		RegisterGlobalFunctions(scripts_[name].state.get());//注册自定义全局函数
		ok = scripts_[name].loader->LoadDomain(name);
		states_[scripts_[name].isolate] = &scripts_[name];

		//顺序不能乱，退出是为了用锁
		scripts_[name].context->Exit();
		scripts_[name].isolate->Exit();
	}
	if (!ok) {
		UnloadScript(name); // 现在安全了
	}
}

void JsEngine::UnloadAllScripts() {
	states_.clear();
	scripts_.clear();
}