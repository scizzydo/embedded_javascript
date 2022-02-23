#pragma once
#include "v8_type.h"
#include <uv.h>

struct Constants {
};

class OS {
public:
	const char* EOL;
	void arch(v8::FunctionCallbackInfo<v8::Value> const& args);
//	Constants constants;
	void cpus(v8::FunctionCallbackInfo<v8::Value> const& args);
	const char* devNull;
	void endianness(v8::FunctionCallbackInfo<v8::Value> const& args);
	void freemem(v8::FunctionCallbackInfo<v8::Value> const& args);
	void getPriority(v8::FunctionCallbackInfo<v8::Value> const& args);
	void homedir(v8::FunctionCallbackInfo<v8::Value> const& args);
	void hostname(v8::FunctionCallbackInfo<v8::Value> const& args);
	void loadavg(v8::FunctionCallbackInfo<v8::Value> const& args);
	void networkInterfaces(v8::FunctionCallbackInfo<v8::Value> const& args);
	void platform(v8::FunctionCallbackInfo<v8::Value> const& args);
	void release(v8::FunctionCallbackInfo<v8::Value> const& args);
	void setPriority(v8::FunctionCallbackInfo<v8::Value> const& args);
	void tmpdir(v8::FunctionCallbackInfo<v8::Value> const& args);
	void totalmem(v8::FunctionCallbackInfo<v8::Value> const& args);
	void type(v8::FunctionCallbackInfo<v8::Value> const& args);
	void uptime(v8::FunctionCallbackInfo<v8::Value> const& args);
	void userInfo(v8::FunctionCallbackInfo<v8::Value> const& args);
	void version(v8::FunctionCallbackInfo<v8::Value> const& args);
};
