#pragma once
#include<v8.h>
#include"js_types.h"
#include "module_loader.h"
using namespace v8;
class JsState
{
public:
	Isolate* isolate = nullptr;
	v8::Local<v8::Context> context = v8::Local<v8::Context>();;
	std::unique_ptr<js_State> state = nullptr;
	std::unique_ptr<ModuleLoader> loader = nullptr;

	JsState() { Create(); }
	~JsState() { Destroy(); }

	void Create();
	void Destroy();
};

