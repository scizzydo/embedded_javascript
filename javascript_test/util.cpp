#include "util.h"

char* read_file(const char* filename) {
	auto file = fopen(filename, "rb");
	if (!file)
		return nullptr;
	fseek(file, 0, SEEK_END);
	auto size = ftell(file);
	rewind(file);
	auto chars = new char[size + 1];
	chars[size] = '\0';

	for (auto i = 0; i < size;) {
		i += fread(&chars[i], 1, size - i, file);
		if (ferror(file)) {
			fclose(file);
			delete[] chars;
			return nullptr;
		}
	}
	fclose(file);
	return chars;
}