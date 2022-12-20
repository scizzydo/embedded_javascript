#pragma once
#include <v8.h>
#include "util.h"

static inline v8::Local<v8::String> v8_str(v8::Isolate* isolate, const char* str) {
	return v8::String::NewFromUtf8(isolate, str).ToLocalChecked();
}

static inline v8::Local<v8::String> v8_str(const char* str) {
	return v8_str(v8::Isolate::GetCurrent(), str);
}

static inline v8::Local<v8::Boolean> v8_bool(v8::Isolate* isolate, bool val) {
	return v8::Boolean::New(isolate, val);
}

static inline v8::Local<v8::Boolean> v8_bool(bool val) {
	return v8_bool(v8::Isolate::GetCurrent(), val);
}

static inline v8::Local<v8::Value> v8_num(v8::Isolate* isolate, double val) {
	return v8::Number::New(isolate, val);
}

static inline v8::Local<v8::Value> v8_num(double val) {
	return v8_num(v8::Isolate::GetCurrent(), val);
}

static inline v8::Local<v8::Integer> v8_int(v8::Isolate* isolate, std::int32_t val) {
	return v8::Integer::New(isolate, val);
}

static inline v8::Local<v8::Integer> v8_int(std::int32_t val) {
	return v8_int(v8::Isolate::GetCurrent(), val);
}

static inline v8::Local<v8::BigInt> v8_bigint(v8::Isolate* isolate, std::int64_t val) {
	return v8::BigInt::New(isolate, val);
}

static inline v8::Local<v8::BigInt> v8_bigint(std::int64_t val) {
	return v8_bigint(v8::Isolate::GetCurrent(), val);
}

static inline v8::Local<v8::Symbol> v8_symbol(v8::Isolate* isolate, const char* name) {
	return v8::Symbol::New(isolate, v8_str(name));
}

static inline v8::Local<v8::Symbol> v8_symbol(const char* name) {
	return v8_symbol(v8::Isolate::GetCurrent(), name);
}