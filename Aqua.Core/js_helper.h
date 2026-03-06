#pragma once
#include"v8.h"
#include"v8pp/convert.hpp"
template<typename... Args>
inline v8::Local<v8::Value> new_js_object(v8::Isolate* isolate,
	const char* ctor_name,
	Args&&... args)
{
	v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
	v8::Local<v8::Object> global = ctx->Global();

	v8::Local<v8::Value> ctor_val =
		global->Get(ctx, v8pp::to_v8(isolate, ctor_name)).ToLocalChecked();

	v8::Local<v8::Function> ctor = ctor_val.As<v8::Function>();

	v8::Local<v8::Value> argv[] = { v8pp::to_v8(isolate, std::forward<Args>(args))... };
	return ctor->NewInstance(ctx, sizeof...(Args), argv).ToLocalChecked();
}
