set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/Crosshair.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Crosshair.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/EntityMesh.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/EntityMesh.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Grid.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Grid.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Skybox.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Skybox.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Splash.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Splash.hpp)

target_sources(PoulpeEngine PRIVATE ${SRC_LIST})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_LIST})
