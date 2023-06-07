enable_testing()

project(rbkTests
  VERSION 0.1.1
  DESCRIPTION "Tests for Rebulkan"
  HOMEPAGE_URL "https://github.com/galliume/rebulkan"
  LANGUAGES CXX
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

include_directories("${PROJECT_SOURCE_DIR}/../vendor/gtest/googletest/include")
include(GoogleTest)

add_subdirectory("${PROJECT_SOURCE_DIR}/../vendor/gtest" "${PROJECT_BINARY_DIR}/lib")
add_executable(rbkTests "test.cpp")

target_link_libraries(rbkTests PRIVATE GTest::gtest_main)

gtest_discover_tests(rbkTests)