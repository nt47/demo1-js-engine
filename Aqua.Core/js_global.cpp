#include"js_global.h"
#include"console.h"
#include"test_class.h"
#include"os.h"
#include"io.h"
#include"event_paint.h"
#include"event_tick.h"
#include"timer_wrapper.h"
#include"test_promise.h"

void RegisterGlobalFunctions(js_State* J) {// 凡是绑定类的，J必须传入
	Console::Register();//绑定模块不需要传入J
	TestClass::Register(J);
	OS::Register();
	IO::Register();
	PaintHookListener::Register(J);
	TickEventListener::Register(J);
	TimerWrapper::Register(J);
	TestPromise::Register(J);
}