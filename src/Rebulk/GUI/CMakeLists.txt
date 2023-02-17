set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/ILayerManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/LayerManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/LayerManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Window.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Window.hpp)

target_sources(Rebulkan PRIVATE ${SRC_LIST})

add_subdirectory(ImGui)
add_subdirectory(Layer)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_LIST})
