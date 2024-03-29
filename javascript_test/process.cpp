#include "process.h"

void version_callback(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = v8::Isolate::GetCurrent();
	auto context = isolate->GetCurrentContext();
	auto versions = v8::Object::New(isolate);
	versions->Set(context, v8_str("node"), v8_str("1.0.0"));
	versions->Set(context, v8_str("v8"), v8_str(v8::V8::GetVersion()));
	versions->Set(context, v8_str("uv"), v8_str(uv_version_string()));
	args.GetReturnValue().Set(versions);
}

void Process::compile(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = v8::Isolate::GetCurrent();
	auto context = isolate->GetCurrentContext();
	v8::ScriptOrigin origin(isolate, args[0]->ToString(context).ToLocalChecked());
	v8::ScriptCompiler::Source script_source(args[1]->ToString(context).ToLocalChecked(), origin);
	v8::Local<v8::String> params[] = {
		v8_str("require"),
		v8_str("exports"),
		v8_str("module"),
	};
	auto func = v8::ScriptCompiler::CompileFunctionInContext(context, &script_source, 3, params, 0, nullptr);
	if (func.IsEmpty())
		args.GetReturnValue().Set(v8::Undefined(isolate));
	else
		args.GetReturnValue().Set(func.ToLocalChecked());
}

void get_file_contents(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::HandleScope scope(isolate);
	v8::String::Utf8Value str(isolate, args[0]);
	auto filename = ToCString(str);
	auto content = read_file(filename);
	if (content.empty())
		args.GetReturnValue().Set(v8_str(""));
	else
		args.GetReturnValue().Set(v8_str(content.c_str()));
}

void Process::chdir(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::HandleScope scope(isolate);
	v8::String::Utf8Value str(isolate, args[0]);
	auto dir = ToCString(str);
	SetCurrentDirectoryA(dir);
	args.GetReturnValue().SetUndefined();
}

void Process::cwd(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::HandleScope scope(isolate);
	char dir[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, dir);
	args.GetReturnValue().Set(v8_str(dir));
}

/*void Process::init(int argc, char* argv[]) {
	auto global = context->Global();

	auto process = v8::Object::New(isolate);
	process->Set(context, v8_str("versions"), v8::Function::New(context, version_callback).ToLocalChecked());
	auto arguments = v8::Array::New(isolate, argc);
	for (auto i = 0; i < argc; ++i)
		arguments->Set(context, v8_num(i), v8_str(argv[i]));
	process->Set(context, v8_str("ARGV"), arguments);
	process->Set(context, v8_str("Compile"), v8::Function::New(context, compile).ToLocalChecked());
	process->Set(context, v8_str("ReadFile"), v8::Function::New(context, get_file_contents).ToLocalChecked());
	process->Set(context, v8_str("Print"), v8::Function::New(context, print).ToLocalChecked());
	process->Set(context, v8_str("PrintError"), v8::Function::New(context, print_error).ToLocalChecked());
	global->Set(context, v8_str("process"), process);
}*/