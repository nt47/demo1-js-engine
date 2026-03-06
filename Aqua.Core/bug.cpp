#include"bug.h"
void Bug::PromiseRejectCallback(v8::PromiseRejectMessage message) {
	if (message.GetEvent() != v8::kPromiseRejectWithNoHandler)
		return;

	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::HandleScope handle_scope(isolate);

	v8::Local<v8::Value> exception = message.GetValue();
	v8::Local<v8::Message> msg = v8::Exception::CreateMessage(isolate, exception);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	// 文件名
	v8::String::Utf8Value filename(isolate, msg->GetScriptOrigin().ResourceName());

	// 行号
	int line = msg->GetLineNumber(context).FromMaybe(-1);

	// 列号
	int column = msg->GetStartColumn();

	// 源码片段
	v8::Local<v8::String> source_line;
	if (msg->GetSourceLine(context).ToLocal(&source_line)) {
		v8::String::Utf8Value sourceline(isolate, source_line);

		printf("\nUnhandled Promise rejection at %s:%d:%d\n", *filename, line, column);
		printf("    %s\n", *sourceline);

		std::string indicator(column + 4, ' ');
		indicator += "^";
		printf("%s\n", indicator.c_str());
	}

	// 打印堆栈
	v8::String::Utf8Value stacktrace(isolate, exception);
	printf("%s\n", *stacktrace);
}

void Bug::MessageCallback(v8::Local<v8::Message> message, v8::Local<v8::Value> data) {
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	// 文件名
	v8::String::Utf8Value filename(isolate, message->GetScriptOrigin().ResourceName());

	// 行号
	int line = message->GetLineNumber(context).FromMaybe(-1);

	// 列号
	int column = message->GetStartColumn();

	// 源码片段
	v8::Local<v8::String> source_line;
	if (message->GetSourceLine(context).ToLocal(&source_line)) {
		v8::String::Utf8Value sourceline(isolate, source_line);

		printf("\nJS Error at %s:%d:%d\n", *filename, line, column);
		printf("    %s\n", *sourceline);

		// 打印 ^ 指示符
		std::string indicator(column + 4, ' ');
		indicator += "^";
		printf("%s\n", indicator.c_str());
	}

	// 打印堆栈（如果有）
	if (data->IsObject()) {
		v8::Local<v8::Object> obj = data.As<v8::Object>();
		v8::Local<v8::Value> stack;
		if (obj->Get(context, v8::String::NewFromUtf8Literal(isolate, "stack")).ToLocal(&stack)) {
			v8::String::Utf8Value stacktrace(isolate, stack);
			printf("%s\n", *stacktrace);
		}
	}
}