set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/ILayer.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/VulkanLayer.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/VulkanLayer.hpp)

target_sources(Rebulkan PRIVATE ${SRC_LIST})