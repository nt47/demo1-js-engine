#include"js_state.h"
#include"bug.h"
#include<iostream>
void JsState::Create() {
	state = std::make_unique<js_State>();
	isolate = state->isolate();
	v8::HandleScope scope(isolate);
	isolate->SetPromiseRejectCallback(Bug::PromiseRejectCallback);
	isolate->AddMessageListener(Bug::MessageCallback);
	// 设置动态 import 回调
	isolate->SetHostImportModuleDynamicallyCallback(
		ModuleLoader::ImportModuleDynamicallyCallback);

	context = state->impl();
	//context = isolate->GetCurrentContext();

	loader = std::make_unique<ModuleLoader>(isolate);

}

void JsState::Destroy() {

	//顺序不能乱
	isolate->Enter();
	context->Enter();
}