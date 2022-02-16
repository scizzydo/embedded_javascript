#pragma once
#include <Windows.h>
#include <cstdint>
#include <iostream>

#include "libplatform/libplatform.h"
#include "v8.h"

#pragma comment(lib, "v8_libbase.lib")
#pragma comment(lib, "v8_libplatform.lib")
#pragma comment(lib, "v8_monolith.lib")

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "shlwapi.lib")