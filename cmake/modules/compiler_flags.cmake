message(STATUS "Setting compiler flags.")

target_compile_features(${PROJECT_NAME}
  PRIVATE   cxx_std_23
  INTERFACE cxx_std_23)

set(DEBUG_OPTIONS
  -g
  -O0
  -fno-inline
  -fno-inline-functions
  -fno-omit-frame-pointer)

set(RELEASE_OPTIONS
  -O3
  -ffast-math
  -march=native
  -fvisibility=hidden
  -DNDEBUG)

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>
  $<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>
  -Weverything
  -fno-strict-aliasing
  -Wno-c++98-compat
  -Wno-old-style-cast
  -Wno-padded
  -Wno-documentation
  -Wno-documentation-unknown-command
  -Wno-decls-in-multiple-modules
  -Wno-unsafe-buffer-usage
  -Wno-c++98-compat
  -Wno-c++98-compat-pedantic
  -Wno-covered-switch-default
  -Wno-deprecated-declarations
  -Wno-c++20-compat)

message(STATUS "compiler definitions OK.")