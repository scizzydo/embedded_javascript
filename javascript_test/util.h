#pragma once
#include <v8.h>

static inline const char* ToCString(v8::String::Utf8Value const& value) {
	return *value ? *value : "<string conversion failed>";
}

std::string read_file(const char* filename);