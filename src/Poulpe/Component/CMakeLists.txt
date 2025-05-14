target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Components.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/Camera.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/Entity.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/Mesh.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/Vertex.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/Texture.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/Vertex.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/EntityNode.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/Component.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/RenderComponent.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/AnimationComponent.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/MeshComponent.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/AnimationScript.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/BoneAnimationComponent.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/BoneAnimationScript.cppm

  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Mesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AnimationScript.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BoneAnimationScript.cpp)