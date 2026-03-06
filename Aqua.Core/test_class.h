#pragma once
#include<v8pp/class.hpp>
#include"js_types.h"
#include"system.h"

// C++ code
struct X
{
	bool b;
	int c;
	X(bool b) : b(b), c(123) {}
};

struct Y : X
{
	Y(int v, bool u) : X(u), var(v) {}
	int var;
	int fun() const { return var * 2; }
	int get() const { return var; }
	void set(int x) { var = x; }
	~Y() {
		System::PrintLn(LEVEL::WARNING, "Y::~Y() called, var = {}", var);
	}
};

struct Z
{
	int z;
	int get() const { return z; }
	explicit Z(int z) : z(z) {}
	void say() {
		System::PrintLn(LEVEL::INFO, "Z::say() called, z = {}", z);
	}
};

static void ext_fun(v8::FunctionCallbackInfo<v8::Value> const& args)
{
	Y* self = v8pp::class_<Y>::unwrap_object(args.GetIsolate(), args.This());
	if (self) args.GetReturnValue().Set(self->b);
	else args.GetReturnValue().Set(args[0]);
}

class TestClass {
public:
	static void Register(js_State* J) {
		v8::Isolate* isolate = J->isolate();
		// bind class X
		v8pp::class_<X> X_class(isolate);
		X_class
			// specify X constructor signature
			.ctor<bool>()
			.auto_wrap_objects(true)
			// bind class member variable
			.var("b", &X::b)
			// set const property
			.const_("str", "abc")
			;

		// Bind class Y
		v8pp::class_<Y> Y_class(isolate);
		Y_class
			// specify Y constructor signature
			.ctor<int, bool>()
			.auto_wrap_objects(true)
			// inherit bindings from base class
			.inherit<X>()
			// bind member functions
			.function("get", &Y::get)
			.function("set", &Y::set)
			// bind read-only property
			.property("prop", &Y::fun)
			// bind read-write property
			.property("wprop", &Y::get, &Y::set)
			// bind a static function
			.function("ext_fun", &ext_fun)
			;

		// Extend existing X bindings 已存在类的追加
		v8pp::class_<X>::extend(isolate).var("c", &X::c);

		//std::vector<std::string> strings = { "abc", "xyz" };
		//auto js_array = v8pp::to_v8(isolate, strings);

		J->class_("X", X_class);
		J->class_("Y", Y_class);


		J->function("makeY", [](int y, bool b)->Y { return Y(y, b); });
		J->function("getArr", [isolate]()->auto { std::vector<Y> strings = { Y(100,true), Y(200,true) };
		return strings; });
	}
};


