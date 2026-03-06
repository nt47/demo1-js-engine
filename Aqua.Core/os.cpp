#include "os.h"
#include"utils.h"
auto ExecuteCommand(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();
	if (args.Length() < 1 || !args[0]->IsString()) {
		isolate->ThrowException(v8::String::NewFromUtf8Literal(isolate, "String expected"));
		return v8pp::to_v8(isolate, -1);
	}

	// 转成 UTF-16（V8 内部就是 UTF-16）
	v8::String::Value command(isolate, args[0]);
	int result = _wsystem(reinterpret_cast<const wchar_t*>(*command));; // 调用系统命令
	//args.GetReturnValue().Set(v8::Integer::New(isolate, result));
	return v8pp::to_v8(isolate, result);
}

auto GetCurrentDirectoryV1() {
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	std::wstring dir = utils::GetModuleDirectoryW();
	return v8pp::to_v8(isolate, dir);
}

void OS::Register() {
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	v8pp::module os(isolate);
	os
		.function("execute", &ExecuteCommand)
		.function("getCurrentDirectory", &GetCurrentDirectoryV1);

	isolate->GetCurrentContext()->Global()->Set(context, v8pp::to_v8(isolate, "os"), os.new_instance());
}