#include"console.h"
#include<iostream>
#include<unordered_set>
using namespace v8;
// 循环引用检测
std::unordered_set<void*> seen;

void Print(const v8::FunctionCallbackInfo<v8::Value>& args, fmt::color color) {
	v8::Isolate* isolate = args.GetIsolate();
	v8::HandleScope handle_scope(isolate);

	for (int i = 0; i < args.Length(); i++) {
		v8::String::Utf8Value str(isolate, args[i]);
		if (*str) {
			fmt::print(fg(color), "{}", *str);
		}
		if (i + 1 < args.Length()) {
			printf(" ");
		}
	}
	printf("\n");
}

void Log(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();
	v8::HandleScope handle_scope(isolate);

	for (int i = 0; i < args.Length(); i++) {
		v8::String::Utf8Value str(isolate, args[i]);
		if (*str) {
			printf("%s", *str);
		}
		if (i + 1 < args.Length()) {
			printf(" ");
		}
	}
	printf("\n");
}

void Warn(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Print(args, fmt::color::yellow);
}

void Info(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Print(args, fmt::color::aqua);
}

void Error(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Print(args, fmt::color::red);
}

void Success(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Print(args, fmt::color::green);
}

void Debug(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Print(args, fmt::color::pink);
}


void PrintValue(Local<Value> value, Isolate* isolate, int indent = 0);

void PrintObject(Local<Object> obj, Isolate* isolate, int indent) {
	void* ptr = *obj;
	if (seen.count(ptr)) {
		std::cout << "[Circular]";
		return;
	}
	seen.insert(ptr);

	Local<Context> context = isolate->GetCurrentContext();

	// 判断是否是数组
	if (obj->IsArray()) {
		Local<Array> arr = obj.As<Array>();
		uint32_t length = arr->Length();
		std::cout << "[\n";
		for (uint32_t i = 0; i < length; ++i) {
			Local<Value> val = arr->Get(context, i).ToLocalChecked();

			// 缩进
			for (int j = 0; j < indent + 2; ++j) std::cout << ' ';
			PrintValue(val, isolate, indent + 2);
			if (i != length - 1) std::cout << ",";
			std::cout << "\n";
		}
		for (int j = 0; j < indent; ++j) std::cout << ' ';
		std::cout << "]";
		return;
	}

	// 普通对象
	Local<Array> keys = obj->GetOwnPropertyNames(context).ToLocalChecked();
	uint32_t length = keys->Length();

	std::cout << "{\n";
	for (uint32_t i = 0; i < length; ++i) {
		Local<Value> key = keys->Get(context, i).ToLocalChecked();
		Local<Value> val = obj->Get(context, key).ToLocalChecked();

		for (int j = 0; j < indent + 2; ++j) std::cout << ' ';
		std::cout << *String::Utf8Value(isolate, key) << ": ";
		PrintValue(val, isolate, indent + 2);
		if (i != length - 1) std::cout << ",";
		std::cout << "\n";
	}
	for (int j = 0; j < indent; ++j) std::cout << ' ';
	std::cout << "}";
}

void PrintValue(Local<Value> value, Isolate* isolate, int indent) {
	if (value->IsObject()) {
		Local<Object> obj = value->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
		PrintObject(obj, isolate, indent);
	}
	else if (value->IsString()) {
		std::cout << *String::Utf8Value(isolate, value);
	}
	else if (value->IsNumber()) {
		std::cout << value->NumberValue(isolate->GetCurrentContext()).FromMaybe(0);
	}
	else if (value->IsBoolean()) {
		std::cout << (value->BooleanValue(isolate) ? "true" : "false");
	}
	else if (value->IsNull()) {
		std::cout << "null";
	}
	else if (value->IsUndefined()) {
		std::cout << "undefined";
	}
	else {
		std::cout << "[Unknown]";
	}
}

// C++ 绑定的 console.dir 回调
void ConsoleDirCallback(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
	if (args.Length() < 1) return;

	seen.clear(); // 每次调用清空循环引用集合
	PrintValue(args[0], isolate, 0);
	std::cout << std::endl;
}



void Console::Register() {
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	v8pp::module console(isolate);
	console
		.function("log", &Log)
		.function("warn", &Warn)
		.function("info", &Info)
		.function("error", &Error)
		.function("success", &Success)
		.function("debug", &Debug)
		.function("dir", &ConsoleDirCallback);
	isolate->GetCurrentContext()->Global()->Set(context, v8pp::to_v8(isolate, "console"), console.new_instance());
}