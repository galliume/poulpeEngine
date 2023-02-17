set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/IRendererAdapter.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/VulkanAdapter.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/VulkanAdapter.hpp)

target_sources(Rebulkan PRIVATE ${SRC_LIST})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_LIST})
