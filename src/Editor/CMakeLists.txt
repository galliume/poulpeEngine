add_library(Editor STATIC)

target_sources(Editor
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      BASE_DIRS .
  PRIVATE)

target_link_libraries(
  Editor
  PRIVATE
    EngineCommon
  PUBLIC
    tcl)

set_poulpe_definitions(Editor)
set_poulpe_links(Editor)
set_poulpe_compiler_flags(Editor)