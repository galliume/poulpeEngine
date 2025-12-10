set(CMAKE_SYSTEM_NAME Linux)

find_program(CMAKE_C_COMPILER NAMES clang-21 clang-20 clang-19 clang)
find_program(CMAKE_CXX_COMPILER NAMES clang++-21 clang++-20 clang++-19 clang++)

if(NOT CMAKE_C_COMPILER OR NOT CMAKE_CXX_COMPILER)
    message(FATAL_ERROR "Clang compiler not found. Please install clang or ensure it is in your PATH.")
endif()