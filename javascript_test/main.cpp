#include "stdafx.h"

std::string get_path() {
	char buffer[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of('\\');
	return std::string(buffer).substr(0, pos);
}

int add_path(const char* path) {
	char buf[2048];
	std::size_t bufsize = 2048;
	auto err = getenv_s(&bufsize, buf, bufsize, "PATH");
	if (err) {
		std::cerr << "`getenv_s` failed, returned " << err << std::endl;
		return EXIT_FAILURE;
	}
	std::string cur_path = buf;
	cur_path += ";";
	cur_path += path;
	err = _putenv_s("PATH", cur_path.c_str());
	if (err) {
		std::cerr << "`_putenv_s` failed, returned " << err << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
	auto module_path = get_path();
	auto v8_path = module_path + "\\v8";
	if (add_path(v8_path.c_str()))
		return EXIT_FAILURE;
	std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
	v8::V8::InitializePlatform(platform.get());
	v8::V8::Initialize();

	v8::Isolate::CreateParams create_params;
	create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	v8::Isolate* isolate = v8::Isolate::New(create_params);
	{
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope handle_scope(isolate);
		v8::Local<v8::Context> context = v8::Context::New(isolate);
		v8::Context::Scope context_scope(context);
		{
			v8::Local<v8::String> source = v8::String::NewFromUtf8Literal(isolate, "'Hello' + ', World!'");
			v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();
			v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
			v8::String::Utf8Value utf8(isolate, result);
			printf("%s\n", *utf8);
		}
		{
			auto source = v8::String::NewFromUtf8Literal(isolate, 
				"function fib(n) {"
					"if (n == 0) {"
						"return 0;"
					"}"
					"else if (n == 1) {"
						"return 1;"
					"}"
					"return fib(n - 1) + fib(n - 2);"
				"}"
				"fib(8);");
			auto script = v8::Script::Compile(context, source).ToLocalChecked();
			auto result = script->Run(context).ToLocalChecked();
			v8::String::Utf8Value utf8(isolate, result);
			printf("Result from JavaScript function: %s\n", *utf8);
		}
	}
	isolate->Dispose();
	v8::V8::Dispose();
	v8::V8::ShutdownPlatform();
	delete create_params.array_buffer_allocator;
	return EXIT_SUCCESS;
}