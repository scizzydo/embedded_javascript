#include "os.h"
#include <sstream>

typedef void(WINAPI* RtlGetVersion_t)(OSVERSIONINFOEXW*);
void RtlGetVersion(OSVERSIONINFOEXW* osvi) {
	auto hmod = GetModuleHandle("ntdll.dll");
	if (hmod) {
		auto func = GetProcAddress(hmod, "RtlGetVersion");
		if (func) {
			reinterpret_cast<RtlGetVersion_t>(func)(osvi);
		}
	}
}

void OS::arch(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::EscapableHandleScope handle_scope(isolate);
#ifdef _WIN32
	args.GetReturnValue().Set(handle_scope.Escape(v8_str("x32")));
#else
	args.GetReturnValue().Set(handle_scope.Escape(v8_str("x64")));
#endif
}

void OS::cpus(v8::FunctionCallbackInfo<v8::Value> const& args) {

}

void OS::endianness(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::EscapableHandleScope handle_scope(isolate);
	union {
		std::uint32_t i;
		std::uint8_t c[4];
	} bint = { 0x01020304 };
	if (bint.c[0] == 1)
		args.GetReturnValue().Set(handle_scope.Escape(v8_str("BE")));
	else
		args.GetReturnValue().Set(handle_scope.Escape(v8_str("LE")));
}

void OS::freemem(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::EscapableHandleScope handle_scope(isolate);
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	args.GetReturnValue().Set(handle_scope.Escape(v8_bigint(statex.ullAvailPhys)));
}

void OS::getPriority(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::EscapableHandleScope handle_scope(isolate);
	DWORD pid = 0;
	DWORD priority = 0;
	if (args.Length() == 1 && args[0]->IsInt32())
		pid = (DWORD)args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).FromJust();
	if (pid) {
		auto handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
		priority = GetPriorityClass(handle);
		CloseHandle(handle);
	}
	else {
		auto handle = GetCurrentProcess();
		priority = GetPriorityClass(handle);
	}
	args.GetReturnValue().Set(handle_scope.Escape(v8_int(priority)));
}

void OS::homedir(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::EscapableHandleScope handle_scope(isolate);
	args.GetReturnValue().Set(handle_scope.Escape(v8_str(getenv("USERPROFILE"))));
}

void OS::hostname(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::EscapableHandleScope handle_scope(isolate);
#define BUFFER_SIZE (MAX_COMPUTERNAME_LENGTH + 1)
	char buffer[BUFFER_SIZE];
	DWORD buffer_size = BUFFER_SIZE;
#undef BUFFER_SIZE
	GetComputerName(buffer, &buffer_size);
	args.GetReturnValue().Set(handle_scope.Escape(v8_str(buffer)));
}

void OS::loadavg(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::EscapableHandleScope handle_scope(isolate);
	auto context = isolate->GetCurrentContext();
	auto arr = v8::Array::New(isolate, 3);
	arr->Set(context, 0, v8_int(0));
	arr->Set(context, 1, v8_int(0));
	arr->Set(context, 2, v8_int(0));
	args.GetReturnValue().Set(handle_scope.Escape(arr));
}

void OS::networkInterfaces(v8::FunctionCallbackInfo<v8::Value> const& args) {

}

void OS::platform(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::EscapableHandleScope handle_scope(isolate);
	args.GetReturnValue().Set(handle_scope.Escape(v8_str("win32")));
}

void OS::release(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::EscapableHandleScope handle_scope(isolate);
	OSVERSIONINFOEXW osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
	RtlGetVersion(&osvi);
	std::stringstream ss;
	ss << osvi.dwMajorVersion << "." << osvi.dwMinorVersion << "." << osvi.dwBuildNumber;
	args.GetReturnValue().Set(handle_scope.Escape(v8_str(ss.str().c_str())));
}

void OS::setPriority(v8::FunctionCallbackInfo<v8::Value> const& args) {

}

void OS::tmpdir(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::EscapableHandleScope handle_scope(isolate);
#define BUFFER_SIZE (MAX_PATH + 1)
	char buffer[BUFFER_SIZE];
	DWORD length = BUFFER_SIZE;
#undef BUFFER_SIZE
	GetTempPath(length, buffer);
	args.GetReturnValue().Set(handle_scope.Escape(v8_str(buffer)));
}

void OS::totalmem(v8::FunctionCallbackInfo<v8::Value> const& args) {

}

void OS::type(v8::FunctionCallbackInfo<v8::Value> const& args) {

}

void OS::uptime(v8::FunctionCallbackInfo<v8::Value> const& args) {

}

void OS::userInfo(v8::FunctionCallbackInfo<v8::Value> const& args) {

}

void OS::version(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::EscapableHandleScope handle_scope(isolate);
	OSVERSIONINFOEXW osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
	RtlGetVersion(&osvi);
	std::stringstream ss;
	ss << osvi.dwMajorVersion << "." << osvi.dwMinorVersion;
	args.GetReturnValue().Set(handle_scope.Escape(v8_str(ss.str().c_str())));
}
