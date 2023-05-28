enable_testing()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_executable(rbkTests "test.cpp")
include_directories("${PROJECT_SOURCE_DIR}/vendor/gtest/googletest/include")

include(GoogleTest)

target_link_libraries(rbkTests PRIVATE GTest::gtest_main)

gtest_discover_tests(rbkTests)