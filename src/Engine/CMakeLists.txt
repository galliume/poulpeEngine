add_library(PoulpeEngineLib STATIC)

target_sources(PoulpeEngineLib
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      BASE_DIRS .
      FILES
        Application.cppm
  PRIVATE
    Application.cpp)

target_include_directories(PoulpeEngineLib PRIVATE ${CMAKE_BINARY_DIR})


target_link_libraries(
  PoulpeEngineLib

  PRIVATE
    miniaudio
    EngineManagers

  PUBLIC
    tcl)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
    PoulpeEngineLib
    EditorManagers)

set_poulpe_definitions(PoulpeEngineLib)
set_poulpe_links(PoulpeEngineLib)