#pragma once
#include<v8.h>
using namespace v8;
class Bug
{
public:
	static void PromiseRejectCallback(v8::PromiseRejectMessage message);
	static void MessageCallback(v8::Local<v8::Message> message, v8::Local<v8::Value> data);

};

