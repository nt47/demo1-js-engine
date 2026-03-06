#pragma once
#include"js_engine.h"
class JsGuard {
public:
	JsGuard() {
		JsEngine::GetInstance().pending_tasks++;
	}

	~JsGuard() {
		JsEngine::GetInstance().pending_tasks--;
	}
};