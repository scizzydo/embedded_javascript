#include "console.h"

void Console::log(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	v8::HandleScope scope(isolate);
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

void Console::error(v8::FunctionCallbackInfo<v8::Value> const& args) {
	auto isolate = args.GetIsolate();
	printf("Error: ");
	v8::HandleScope scope(isolate);
	for (auto i = 0; i < args.Length(); ++i) {
		if (i > 0)
			fprintf(stderr, " ");
		v8::String::Utf8Value str(isolate, args[i]);
		auto cstr = ToCString(str);
		fprintf(stderr, "%s", cstr);
	}
	fprintf(stderr, "\n");
	fflush(stderr);
}