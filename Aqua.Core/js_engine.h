#pragma once
#include"js_state.h"
#include <map>
#include <string>
class JsEngine
{
public:
	void LoadScript(std::wstring name);
	void UnloadScript(std::wstring name);
	void UnloadAllScripts();
	//void ExecuteScript(std::wstring name);
	JsState& GetScriptState(std::wstring name) {
		return scripts_[name];
	}
	JsState*& GetIsolateState(v8::Isolate* isolate) {
		return states_[isolate];
	}
	static JsEngine& GetInstance();

	std::atomic<int> pending_tasks{ 0 };

	bool IsRunning() {
		return pending_tasks > 0;
	}

private:
	std::map<std::wstring, JsState> scripts_;
	std::map<v8::Isolate*, JsState*> states_;

	JsEngine();
	//explicit JsEngine(JsEngine const&);//禁止隐式转换比如A a=10;必须A a(10)
	//禁止复制和赋值
	JsEngine& operator=(JsEngine const&) = delete;
	JsEngine(JsEngine const&) = delete;
};

