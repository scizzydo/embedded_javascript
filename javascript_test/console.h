#pragma once
#include "v8_type.h"

struct Console {
	void log(v8::FunctionCallbackInfo<v8::Value> const& args);
	void error(v8::FunctionCallbackInfo<v8::Value> const& args);
};