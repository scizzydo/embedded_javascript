#pragma once
#include <v8.h>
#include "util.h"

static inline v8::Local<v8::String> v8_str(const char* str) {
	return v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), str).ToLocalChecked();
}

static inline v8::Local<v8::Boolean> v8_bool(bool val) {
	return v8::Boolean::New(v8::Isolate::GetCurrent(), val);
}

static inline v8::Local<v8::Value> v8_num(double val) {
	return v8::Number::New(v8::Isolate::GetCurrent(), val);
}

static inline v8::Local<v8::Integer> v8_int(std::int32_t val) {
	return v8::Integer::New(v8::Isolate::GetCurrent(), val);
}

static inline v8::Local<v8::BigInt> v8_bigint(std::int64_t val) {
	return v8::BigInt::New(v8::Isolate::GetCurrent(), val);
}

static inline v8::Local<v8::Symbol> v8_symbol(const char* name) {
	return v8::Symbol::New(v8::Isolate::GetCurrent(), v8_str(name));
}