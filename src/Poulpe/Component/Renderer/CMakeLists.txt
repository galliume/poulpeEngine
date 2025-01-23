set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/Crosshair.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Crosshair.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Basic.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Basic.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Grid.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Grid.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Skybox.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Skybox.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ShadowMap.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ShadowMap.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Terrain.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Terrain.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/RendererFactory.hpp)

target_sources(PoulpeEngine PRIVATE ${SRC_LIST})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_LIST})
