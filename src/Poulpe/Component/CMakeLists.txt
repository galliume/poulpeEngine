target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        Camera.cppm
        Component.cppm
        Entity.cppm
        EntityNode.cppm
        Mesh.cppm
        RenderComponent.cppm
        Texture.cppm
        Vertex.cppm
  PRIVATE
    Camera.cpp
    Component.cpp
    Entity.cpp
    Mesh.cpp)