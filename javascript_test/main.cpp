#include "stdafx.h"
#include <v8pp/call_v8.hpp>
#include <v8pp/function.hpp>
#include <v8pp/object.hpp>
#include <v8pp/module.hpp>
#include <filesystem>

static std::string v8_path;

void console_log(v8::FunctionCallbackInfo<v8::Value> const& args);

void require(v8::FunctionCallbackInfo<v8::Value> const& args);

void setTimeout(v8::FunctionCallbackInfo<v8::Value> const& args);

const char* ToCString(v8::String::Utf8Value const& value) {
	return *value ? *value : "<string conversion failed>";
}

std::vector<std::pair<std::string, void(*)(v8::FunctionCallbackInfo<v8::Value> const& args)>> default_functions{
	{ "require", require },
	{ "setTimeout", setTimeout }
};

void register_default_functions(v8pp::context* context) {
	v8pp::module m(context->isolate());
	m.function("log", &console_log);
	context->module("console", m);
	for (auto& [function, callback] : default_functions) {
		context->function(function, callback);
	}
}

std::string executable_path() {
	char buffer[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of('\\');
	return std::string(buffer).substr(0, pos);
}

std::string get_path() {
	char buf[2048];
	std::size_t bufsize = 2048;
	auto err = getenv_s(&bufsize, buf, bufsize, "PATH");
	if (err)
		return std::string();
	return std::string(buf);
}

int add_path(const char* path) {
	std::string cur_path = get_path();
	if (cur_path.empty())
		return EXIT_FAILURE;
	cur_path += ";";
	cur_path += path;
	auto err = _putenv_s("PATH", cur_path.c_str());
	if (err) {
		std::cerr << "`_putenv_s` failed, returned " << err << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void console_log(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	for (auto i = 0; i < args.Length(); ++i) {
		if (i > 0)
			printf(" ");
		v8::String::Utf8Value str(isolate, args[i]);
		auto cstr = ToCString(str);
		printf("%s", cstr);
	}
	printf("\n");
	fflush(stdout);
}

void setTimeout(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	if (args.Length() != 2)
		throw std::runtime_error("2 arguments required for setTimeout");
	if (!args[0]->IsFunction())
		throw std::runtime_error("setTimeout argument 1 is requierd to be a function");
	if (!args[1]->IsNumber())
		throw std::runtime_error("setTimeout argument 2 is required to be a number");
	v8::HandleScope scope(isolate);
}

void require(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	if (!args.Length())
		throw std::runtime_error("no file specified for include");
	v8::HandleScope scope(isolate);
	v8::String::Utf8Value str(isolate, args[0]);
	std::string content{};
	std::filesystem::path full_path;
	std::filesystem::path file(*str);
	if (file.is_absolute())
		full_path = file;
	else {
		std::filesystem::path dir(v8_path);
		full_path = dir / file;
	}
	if (!full_path.has_extension()) {
		full_path += ".js";
	}
	if (!std::filesystem::exists(full_path))
		throw std::runtime_error("could not locate file specified");
	std::ifstream ifs(full_path);
	if (!ifs.good())
		throw std::runtime_error("Could not load file into stream");
	std::istreambuf_iterator<char> begin(ifs), end;
	content = std::string(begin, end);
	if (content.empty())
		throw std::runtime_error("No contents in file to load");
	v8pp::context context(isolate, nullptr, false, true, {});
	register_default_functions(&context);
	auto cur_context = isolate->GetCurrentContext();
	auto global = context.global();
	auto export_key = v8pp::to_v8(isolate, "exports");
	auto module_key = v8pp::to_v8(isolate, "module");
	global->Set(cur_context, export_key, v8::Object::New(isolate));
	global->Set(cur_context, module_key, global); 
	auto exports = global->GetRealNamedProperty(cur_context, export_key).ToLocalChecked();
	global->Set(cur_context, export_key, exports);
	context.run_script(content, full_path.filename().string());
	auto ret = global->GetRealNamedProperty(cur_context, export_key).ToLocalChecked();
	args.GetReturnValue().Set(ret);
}

uv_loop_t* loop;

int main(int argc, char* argv[]) {
	loop = uv_default_loop();
	auto module_path = executable_path();
	v8_path = module_path + "\\modules";
	add_path(v8_path.c_str());
#if V8_MAJOR_VERSION >= 7
	std::unique_ptr<v8::Platform> platform(v8::platform::NewDefaultPlatform());
#else
	std::unique_ptr<v8::Platform> platform(v8::platform::CreateDefaultPlatform());
#endif
	v8::V8::InitializePlatform(platform.get());
	v8::V8::Initialize();
	{
		v8pp::context context(nullptr, nullptr, false, true, {});
		auto isolate = context.isolate();
		v8::HandleScope scope(isolate);
		v8::TryCatch trycatch(isolate);
		trycatch.SetVerbose(true);
		try {
			register_default_functions(&context);
			if (context.run_file(module_path + "\\index.js").IsEmpty()) {
				auto exception = trycatch.Exception();
				v8::String::Utf8Value exception_str(isolate, exception);
				std::cerr << "Exception: " << *exception_str << std::endl;
			}
			else {
				while (v8::platform::PumpMessageLoop(platform.get(), isolate))
					continue;
			}
		}
		catch (std::exception const& ex) {
			std::cerr << "Exception: " << ex.what() << std::endl;
		}
	}
	v8::V8::Dispose();
	v8::V8::ShutdownPlatform();
	return EXIT_SUCCESS;
}