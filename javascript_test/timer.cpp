#include "timer.h"

void timer_callback(uv_timer_t* handle) {
	Timer* data = (Timer*)handle->data;
	if (!data->is_interval)
		uv_timer_stop(handle);
	auto isolate = v8::Isolate::GetCurrent();
	v8::HandleScope scope(isolate);
	auto context = isolate->GetCurrentContext();
	v8::Local<v8::Function> cb = data->callback.Extract();

	cb->Call(context, v8::Null(isolate), 0, nullptr);
}

void Timer::start(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::HandleScope scope(isolate);
	auto context = isolate->GetCurrentContext();
	auto delay = args[0]->IntegerValue(context).ToChecked();
	auto repeat = args[1]->Int32Value(context).ToChecked();
	callback = PersistentFunction(isolate, v8::Local<v8::Function>::Cast(args[2]));
	is_interval = repeat ? true : false;
	uv_timer_init(uv_default_loop(), &handle);
	uv_timer_start(&handle, timer_callback, delay, repeat);
}

void Timer::stop() {
	uv_timer_stop(&handle);
}