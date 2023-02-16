set(SRC_LIST "Application.cpp" "Application.hpp")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_LIST})
target_sources(Rebulkan PRIVATE ${SRC_LIST})