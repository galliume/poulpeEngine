set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/Camera.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Camera.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Entity.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Entity.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Mesh.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Mesh.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Mesh2D.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Mesh2D.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Object.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Vertex.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Texture.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Texture.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/Vertex2D.hpp
			 ${CMAKE_CURRENT_SOURCE_DIR}/EntityNode.hpp
			 ${CMAKE_CURRENT_SOURCE_DIR}/WorldEntity.hpp)

target_sources(PoulpeEngine PRIVATE ${SRC_LIST})