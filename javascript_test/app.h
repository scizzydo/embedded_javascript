#pragma once
#include <libplatform/libplatform.h>
#include "v8_type.h"

class App {
	std::unique_ptr<v8::Platform> platform;
	v8::Isolate::CreateParams create_params;
	v8::Isolate* isolate;
	v8::Local<v8::Context> context;
	v8::Handle<v8::ObjectTemplate> global;
public:
	void create_platform();
	void create_vm();
	void shutdown_vm();
	void init_global();
	void setup_global(int argc, char* argv[]);
	void run_script_string(int argc, char* argv[], const char* filename, const char* script_string);
	void run_file(int argc, char* argv[], const char* filename);
};

