#pragma once
#include <uv.h>
#include "v8_type.h"
#include "PersistentHandleWrapper.h"

typedef PersistentHandleWrapper<v8::Function> PersistentFunction;

struct Timer {
	uv_timer_t handle;
	PersistentFunction callback;
	bool is_interval;
	Timer() :
		handle(),
		callback(),
		is_interval(false)
	{
		handle.data = this;
	}
	void start(v8::FunctionCallbackInfo<v8::Value> const& args);
	void stop();
};