set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/DeviceMemory.cpp 
	     ${CMAKE_CURRENT_SOURCE_DIR}/DeviceMemory.hpp
	     ${CMAKE_CURRENT_SOURCE_DIR}/DeviceMemoryPool.cpp 
	     ${CMAKE_CURRENT_SOURCE_DIR}/DeviceMemoryPool.hpp
	     ${CMAKE_CURRENT_SOURCE_DIR}/VulkanRenderer.cpp 
	     ${CMAKE_CURRENT_SOURCE_DIR}/VulkanRenderer.hpp)

target_sources(PoulpeEngine PRIVATE ${SRC_LIST})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_LIST})
