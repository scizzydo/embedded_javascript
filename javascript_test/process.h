#pragma once
#include "v8_type.h"
#include <uv.h>

struct Process {
	std::string execPath;
	int pid;
	void chdir(v8::FunctionCallbackInfo<v8::Value> const& args);
	void cwd(v8::FunctionCallbackInfo<v8::Value> const& args);
	void compile(v8::FunctionCallbackInfo<v8::Value> const& args);
};