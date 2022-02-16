# C++ Embedded JavaScript V8 engine

## Build steps:
1. Follow steps from https://v8.dev/docs/source-code to get the basics (depot_tools, gclient setup etc.)

    **<pulled_dir> below will reference the directory the source code is inserted to from the steps above**

2. Once v8 is pulled with `fetch v8` run the following commands for this project setup:
```
gn args out/x86.no-i18n.release
```

Copy the following contents into the pop up:
```
is_clang = false
target_cpu = "x86"
is_debug = false
v8_monolithic = true
v8_use_external_startup_data = false
use_custom_libcxx = false
use_custom_libcxx_for_host = false
v8_static_library = true
use_glib = false
visual_studio_version = "2022"
treat_warnings_as_errors = false
v8_enable_i18n_support = false
```
Save and close the file for it to pull all the build items

3. Verify the pulled directory now has the contents of `<pulled_dir>\out\x86.no-i18n.release`. If there is only about 3 files then the args command didn't generate the folder. Issue the following to command to ensure it generates: `gn gen out/x86.no-i18n.release`

4. Run `ninja -C out/x86.no-i18n.release v8_monolith`

5. Update the include x86 Release include directory to: `<pulled_dir>` and the library directory to: `<pulled_dir>\out\x86.no-i18n.release\obj`
