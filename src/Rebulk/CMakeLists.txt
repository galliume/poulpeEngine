set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/Application.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Application.hpp)

target_sources(Rebulkan PRIVATE ${SRC_LIST})

add_subdirectory(Component)
add_subdirectory(Core)
add_subdirectory(GUI)
add_subdirectory(Manager)
add_subdirectory(Renderer)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_LIST})
