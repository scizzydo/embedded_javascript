#include "app.h"
#include <cassert>
#include "timer.h"
#include "process.h"
#include <v8pp/module.hpp>
#include <v8pp/class.hpp>
#include <v8pp/function.hpp>
#include "console.h"
#include <filesystem>
#include <map>
#include "os.h"

std::int32_t last_id = 0;
std::map<std::int32_t, std::string> module_paths;
std::unordered_map<std::string, PersistentHandleWrapper<v8::Module>> loaded_modules{};

class RAII {
	std::function<void()> cb;
public:
	RAII(std::function<void()> cb) :
		cb(cb) {}
	~RAII() {
		cb();
	}
};

void App::create_platform() {
	platform = v8::platform::NewDefaultPlatform();
	v8::V8::InitializePlatform(platform.get());
	v8::V8::Initialize();
}

v8::Local<v8::Module> load_module(const char* code,
	const char* name, std::string file,
	v8::Local<v8::Context> cx);

v8::Local<v8::Value> exec_module(v8::Local<v8::Module> mod,
	v8::Local<v8::Context> cx,
	bool nsObject = false);

v8::MaybeLocal<v8::Module> call_resolve(
	v8::Local<v8::Context> context, v8::Local<v8::String> specifier,
	v8::Local<v8::FixedArray> import_assertions, v8::Local<v8::Module> referrer);

v8::MaybeLocal<v8::Promise> call_dynamic(v8::Local<v8::Context> context,
	v8::Local<v8::ScriptOrModule> referrer,
	v8::Local<v8::String> specifier,
	v8::Local<v8::FixedArray> import_assertions);

void call_meta(v8::Local<v8::Context> context,
	v8::Local<v8::Module> module,
	v8::Local<v8::Object> meta);

std::filesystem::path get_module_path(v8::String::Utf8Value const& str) {
	std::string file = ToCString(str);
	std::filesystem::path file_path(file);
	std::filesystem::path load_file;
	if (!file_path.has_extension()) {
		if (std::filesystem::is_directory(file_path))
			load_file = file_path / "index.js";
		else
			load_file = file_path.string() + ".js";
	}
	else
		load_file = file_path;
	std::filesystem::path absolute;
	if (!load_file.is_absolute())
		absolute = std::filesystem::current_path() / load_file;
	else
		absolute = load_file;
	return absolute;
}

std::string get_file_path(const char* file) {
	std::filesystem::path path(file);
	if (path.is_absolute())
		return path.parent_path().string();
	auto current = std::filesystem::current_path();
	return (current / path).parent_path().string();
}

void App::create_vm() {
	create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	isolate = v8::Isolate::New(create_params);
	isolate->SetHostImportModuleDynamicallyCallback(call_dynamic);
	isolate->SetHostInitializeImportMetaObjectCallback(call_meta);
}

void App::shutdown_vm() {
	loaded_modules.clear();
	isolate->Dispose();
	v8::V8::Dispose();
	v8::V8::ShutdownPlatform();
	delete create_params.array_buffer_allocator;
}

void App::init_global() {
	global = v8::ObjectTemplate::New(isolate);
}

void require(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::HandleScope scope(isolate);
	auto context = isolate->GetCurrentContext();
	v8::String::Utf8Value str(isolate, args[0]);
	if (strcmp(*str, "os") == 0) {
		v8pp::class_<OS> os(isolate);
		os.ctor()
			.const_("EOL", v8_str(R"(\r\n)"))
			.const_("devNull", v8_str(R"(\\.\nul)"))
			.function("arch", &OS::arch)
			.function("cpus", &OS::cpus)
			.function("endianness", &OS::endianness)
			.function("freemem", &OS::freemem)
			.function("getPriority", &OS::getPriority)
			.function("homedir", &OS::homedir)
			.function("hostname", &OS::hostname)
			.function("loadavg", &OS::loadavg)
			.function("networkInterfaces", &OS::networkInterfaces)
			.function("platform", &OS::platform)
			.function("release", &OS::release)
			.function("setPriority", &OS::setPriority)
			.function("tmpdir", &OS::tmpdir)
			.function("totalmem", &OS::totalmem)
			.function("type", &OS::type)
			.function("uptime", &OS::uptime)
			.function("userInfo", &OS::userInfo)
			.function("version", &OS::version);
		args.GetReturnValue().Set(os.create_object(isolate));
		return;
	}
	auto path = get_module_path(str);
	auto file = path.string();
	if (loaded_modules.count(file)) {
		auto mod = loaded_modules.at(file).Extract();
		auto ret = exec_module(mod, context, true);
		args.GetReturnValue().Set(ret);
	}
	else {
		std::string previous;
		RAII raii([&previous]() {
			if (!previous.empty())
				SetCurrentDirectory(previous.c_str());
			});
		std::string content;
		std::filesystem::path temp_path(*str);
		if ((*str)[0] == '.') {
			if (temp_path.has_parent_path()) {
				previous = std::filesystem::current_path().string();
				SetCurrentDirectory(temp_path.parent_path().string().c_str());
				content = read_file(temp_path.filename().string().c_str());
			}
			else {
				content = read_file(file.c_str());
			}
		}
		else {
			previous = std::filesystem::current_path().string();
			SetCurrentDirectory(path.parent_path().string().c_str());
			content = read_file(path.string().c_str());
		}
		auto mod = load_module(content.c_str(), *str, file, context);
		loaded_modules.insert({ file, PersistentHandleWrapper<v8::Module>(isolate, mod) });
		auto ret = exec_module(mod, context, true);
		args.GetReturnValue().Set(ret);
	}
}

void App::setup_global(int argc, char* argv[]) {
	auto global = context->Global();

	v8pp::class_<Timer> timer_class(isolate);
	timer_class.ctor()
		.function("start", &Timer::start)
		.function("stop", &Timer::stop);
	global->Set(context, v8_str("Timer"), timer_class.js_function_template()->GetFunction(context).ToLocalChecked());

	v8pp::class_<Console> console_class(isolate);
	console_class.ctor()
		.function("log", &Console::log)
		.function("error", &Console::error);
	global->Set(context, v8_str("console"), console_class.create_object(isolate));

	v8pp::class_<Process> process_class(isolate);
	process_class.ctor()
		.function("cwd", &Process::cwd)
		.function("chdir", &Process::chdir);
	global->Set(context, v8_str("process"), process_class.create_object(isolate));

	global->Set(context, v8_str("require"), v8::FunctionTemplate::New(isolate, require)->GetFunction(context).ToLocalChecked());
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

bool execute_string(v8::Isolate* isolate, v8::Local<v8::String> source, const char* name) {
	v8::HandleScope handle_scope(isolate);
	v8::TryCatch try_catch(isolate);
	auto current_id = last_id++;
	auto file_path = get_file_path(name);
	v8::ScriptOrigin origin(isolate, v8_str(name), 0,
		0, false, current_id, v8::Local<v8::Value>(), false, false, false);	
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
	execute_string(isolate, source, filename);
}

void App::run_file(int argc, char* argv[], const char* filename) {
	auto script = read_file(filename);
	if (script.empty()) {
		throw std::runtime_error("Unable to read content to buffer");
	}
	else {
		run_script_string(argc, argv, filename, script.c_str());
	}
}

v8::Local<v8::Module> load_module(const char* code, const char* name,
	std::string file, v8::Local<v8::Context> cx) {
	auto isolate = cx->GetIsolate();
	v8::Local<v8::String> vcode = v8_str(isolate, code);
	auto current_id = last_id++;
	auto file_path = get_file_path(name);
	v8::ScriptOrigin origin(isolate, v8_str(isolate, name), 0,
		0, false, current_id, v8::Local<v8::Value>(), false, false, true);
	v8::ScriptCompiler::Source source(vcode, origin);
	auto ret = v8::ScriptCompiler::CompileModule(isolate, &source);
	v8::Local<v8::Module> mod;
	if (!ret.ToLocal(&mod)) {
		throw std::runtime_error("Error loading module");
	}
	auto result = mod->InstantiateModule(cx, call_resolve);
	if (result.IsNothing())
		throw std::runtime_error("Error instantiating module");
	loaded_modules.insert({ file, PersistentHandleWrapper<v8::Module>(isolate, mod)});
	return mod;
}

v8::Local<v8::Module> check_module(v8::MaybeLocal<v8::Module> maybe_module,
	v8::Local<v8::Context> cx) {
	v8::Local<v8::Module> mod;
	if (!maybe_module.ToLocal(&mod))
		throw std::runtime_error("Error loading module");
	auto result = mod->InstantiateModule(cx, call_resolve);
	if (result.IsNothing())
		throw std::runtime_error("Error instantiating module");
	return mod;
}

v8::Local<v8::Value> exec_module(
	v8::Local<v8::Module> mod, v8::Local<v8::Context> cx,
	bool nsObject) {
	v8::Local<v8::Value> ret;
	if (!mod->Evaluate(cx).ToLocal(&ret))
		throw std::runtime_error("Error evaluating module");
	if (nsObject)
		return mod->GetModuleNamespace();
	return ret;
}

v8::MaybeLocal<v8::Module> call_resolve(
	v8::Local<v8::Context> context, v8::Local<v8::String> specifier,
	v8::Local<v8::FixedArray> import_assertions, v8::Local<v8::Module> referrer) {
	v8::String::Utf8Value str(context->GetIsolate(), specifier);
	std::string previous;
	std::string file;
	RAII raii([&previous]() {
		if (!previous.empty())
			SetCurrentDirectory(previous.c_str());
		});
	std::string content;
	if ((*str)[0] == '.') {
		auto path = get_module_path(str);
		file = path.string();
		if (loaded_modules.count(file)) {
			auto mod = loaded_modules.at(file).Extract();
			return mod;
		}
		content = read_file(file.c_str());
	}
	else {
		auto path = get_module_path(str);
		file = path.string();
		if (loaded_modules.count(file)) {
			auto mod = loaded_modules.at(file).Extract();
			return mod;
		}
		previous = std::filesystem::current_path().string();
		SetCurrentDirectory(path.parent_path().string().c_str());
		content = read_file(path.string().c_str());
	}
	return load_module(content.c_str(), *str, file, context);
}

v8::MaybeLocal<v8::Promise> call_dynamic(
	v8::Local<v8::Context> context, v8::Local<v8::ScriptOrModule> referrer,
	v8::Local<v8::String> specifier, v8::Local<v8::FixedArray> import_assertions) {
	v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
	v8::MaybeLocal<v8::Promise> promise(resolver->GetPromise());
	v8::String::Utf8Value name(context->GetIsolate(), specifier);
	auto path = get_module_path(name);
	auto file = path.string();
	std::string content;
	content = read_file(file.c_str());
	auto mod = check_module(load_module(content.c_str(), *name, file, context), context);
	auto ret = exec_module(mod, context, true);
	resolver->Resolve(context, ret);
	return promise;
}

void call_meta(
	v8::Local<v8::Context> context, v8::Local<v8::Module> module,
	v8::Local<v8::Object> meta) {
	auto isolate = context->GetIsolate();
	meta->Set(context, v8_str(isolate, "url"), v8_str(isolate, "https://throwaway.sh"));
}