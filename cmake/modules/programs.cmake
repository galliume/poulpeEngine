message(STATUS "Setting programs.")

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  message(STATUS "Found ccache: ${CCACHE_PROGRAM}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

message(STATUS "programs OK.")