#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <libplatform/libplatform.h>
#include <v8.h>
#include <uv.h>
#include "app.h"
#include "process.h"

int main(int argc, char* argv[]) {
	App app;
	app.create_platform();
	app.create_vm();
	app.run_file(argc, argv, "./index.js");
	app.shutdown_vm();
	return EXIT_SUCCESS;
}