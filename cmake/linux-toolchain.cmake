set(CMAKE_SYSTEM_NAME Linux)

find_program(CMAKE_C_COMPILER NAMES clang-22 clang-21 clang)
find_program(CMAKE_CXX_COMPILER NAMES clang++-22 clang++-21 clang++)

if(NOT CMAKE_C_COMPILER OR NOT CMAKE_CXX_COMPILER)
    message(FATAL_ERROR "Clang compiler not found. Please install clang or ensure it is in your PATH.")
endif()
