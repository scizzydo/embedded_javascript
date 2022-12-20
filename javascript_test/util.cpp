#include "util.h"
#include <fstream>

std::string read_file(const char* filename) {
	std::string content;
	std::ifstream ifs(filename);
	if (!ifs.good())
		return content;
	std::istreambuf_iterator<char> begin(ifs), end;
	content = std::string(begin, end);
	return content;
}