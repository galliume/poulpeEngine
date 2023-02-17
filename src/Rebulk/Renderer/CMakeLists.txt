set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/IRenderer.hpp)

target_sources(Rebulkan PRIVATE ${SRC_LIST})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_LIST})
