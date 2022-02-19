#pragma once
#include "v8_type.h"
#include <uv.h>

class Process {
	v8::Local<v8::Context> context;
	v8::Isolate* isolate;
public:
	void init(int argc, char* argv[]);
	Process(v8::Local<v8::Context> context, v8::Isolate* isolate) :
		context(context),
		isolate(isolate)
	{}
};