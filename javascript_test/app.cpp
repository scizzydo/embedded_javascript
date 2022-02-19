#include "app.h"
#include <cassert>
#include "timer.h"
#include "process.h"
#include <v8pp/module.hpp>
#include <v8pp/class.hpp>

void App::create_platform() {
	platform = v8::platform::NewDefaultPlatform();
	v8::V8::InitializePlatform(platform.get());
	v8::V8::Initialize();
}

void App::create_vm() {
	create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	isolate = v8::Isolate::New(create_params);
}

void App::shutdown_vm() {
	isolate->Dispose();
	v8::V8::Dispose();
	v8::V8::ShutdownPlatform();
	delete create_params.array_buffer_allocator;
}

void App::init_global() {
	global = v8::ObjectTemplate::New(isolate);
}

void App::setup_global(int argc, char* argv[]) {
	v8pp::module timer(isolate);
	v8pp::class_<Timer> timer_class(isolate);
	timer_class.ctor()
		.function("start", &Timer::start)
		.function("stop", &Timer::stop);
	timer.class_("Timer", timer_class);
	context->Global()->Set(context, v8_str("node"), timer.new_instance());

	Process process(context, isolate);
	process.init(argc, argv);
}

void report_exception(v8::Isolate* isolate, v8::TryCatch* try_catch) {
	v8::HandleScope handle_scope(isolate);
	v8::String::Utf8Value exception(isolate, try_catch->Exception());
	auto exception_string = ToCString(exception);
	v8::Local<v8::Message> message = try_catch->Message();
	if (message.IsEmpty()) {
		fprintf(stderr, "%s\n", exception_string);
	}
	else {
		v8::String::Utf8Value filename(isolate, message->GetScriptOrigin().ResourceName());
		v8::Local<v8::Context> context(isolate->GetCurrentContext());
		auto filename_string = ToCString(filename);
		auto line_number = message->GetLineNumber(context).FromJust();
		fprintf(stderr, "%s:%i: %s\n", filename_string, line_number, exception_string);
		v8::String::Utf8Value sourceline(isolate, message->GetSourceLine(context).ToLocalChecked());
		auto sourceline_string = ToCString(sourceline);
		fprintf(stderr, "%s\n", sourceline_string);
		auto start = message->GetStartColumn(context).FromJust();
		for (auto i = 0; i < start; ++i)
			fprintf(stderr, " ");
		auto end = message->GetEndColumn(context).FromJust();
		for (auto i = start; i < end; ++i)
			fprintf(stderr, "^");
		fprintf(stderr, "\n");
		v8::Local<v8::Value> stack_trace_string;
		if (try_catch->StackTrace(context).ToLocal(&stack_trace_string)
			&& stack_trace_string->IsString()
			&& stack_trace_string.As<v8::String>()->Length() > 0) {
			v8::String::Utf8Value stack_trace(isolate, stack_trace_string);
			auto err = ToCString(stack_trace);
			fprintf(stderr, "%s\n", err);
		}
	}
}

bool execute_string(v8::Isolate* isolate, v8::Local<v8::String> source, v8::Local<v8::Value> name) {
	v8::HandleScope handle_scope(isolate);
	v8::TryCatch try_catch(isolate);
	v8::ScriptOrigin origin(isolate, name);
	v8::Local<v8::Context> context(isolate->GetCurrentContext());
	v8::Local<v8::Script> script;
	if (!v8::Script::Compile(context, source, &origin).ToLocal(&script)) {
		report_exception(isolate, &try_catch);
		return false;
	}
	else {
		v8::Local<v8::Value> result;
		if (!script->Run(context).ToLocal(&result)) {
			assert(try_catch.HasCaught());
			report_exception(isolate, &try_catch);
			return false;
		}
		else {
			assert(!try_catch.HasCaught());
			if (!result->IsUndefined()) {
				v8::String::Utf8Value str(isolate, result);
				auto cstr = ToCString(str);
				printf("%s\n", cstr);
			}
			uv_run(uv_default_loop(), UV_RUN_DEFAULT);
			return true;
		}
	}
}

void App::run_script_string(int argc, char* argv[], const char* filename, const char* script_string) {
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);
	init_global();
	context = v8::Context::New(isolate, nullptr, global);
	v8::Context::Scope context_scope(context);
	setup_global(argc, argv);
	auto source = v8_str(script_string);
	delete[] script_string;
	execute_string(isolate, source, v8_str(filename));
}

void App::run_file(int argc, char* argv[], const char* filename) {
	auto script = read_file(filename);
	if (script) {
		run_script_string(argc, argv, filename, script);
	}
	else {
		throw std::runtime_error("Unable to read content to buffer");
	}
}