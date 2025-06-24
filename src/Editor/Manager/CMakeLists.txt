target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        EditorManager.cppm
        LevelManager.cppm
        ExplorerManager.cppm

  PRIVATE
    EditorManager.cpp
    LevelManager.cpp
    ExplorerManager.cpp)