target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        Camera.cppm
        Entity.cppm
        Texture.cppm
        Vertex.cppm
        EntityNode.cppm
        Components.cppm
  PRIVATE
    Camera.cpp
    Entity.cpp)