set(CMAKE_SYSTEM_NAME Linux)

find_program(CMAKE_C_COMPILER NAMES clang-22 clang-21 clang)
find_program(CMAKE_CXX_COMPILER NAMES clang++-22 clang++-21 clang++)

if(NOT CMAKE_C_COMPILER OR NOT CMAKE_CXX_COMPILER)
    message(FATAL_ERROR "Clang compiler not found. Please install clang or ensure it is in your PATH.")
endif()


file(GLOB_RECURSE STD_MODULE_JSON 
    "/usr/lib/llvm-21/lib/libc++.modules.json"
    "/usr/lib/llvm-22/lib/libc++.modules.json"
    "/usr/share/libc++/v1/libc++.modules.json"
)

if(STD_MODULE_JSON)
    list(GET STD_MODULE_JSON 0 SELECTED_JSON)
    set(CMAKE_CXX_STDLIB_MODULES_JSON "${SELECTED_JSON}")
    message(NOTICE "Using Standard Module Manifest: ${SELECTED_JSON}")
endif()

if(EXISTS "/usr/lib/llvm-22/share/libc++/v1/std.cppm")
  message(NOTICE "Adding std.cppm for llvm 22")
  set(CMAKE_CXX_COMPILER_CLANG_MODULES_SOURCE_DIR "/usr/lib/llvm-22/share/libc++/v1")
elseif(EXISTS "/usr/lib/llvm-21/share/libc++/v1/std.cppm")
  message(NOTICE "Adding std.cppm for llvm 21")
  set(CMAKE_CXX_COMPILER_CLANG_MODULES_SOURCE_DIR "/usr/lib/llvm-21/share/libc++/v1")
endif()