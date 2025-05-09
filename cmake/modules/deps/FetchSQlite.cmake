message(NOTICE "Fetching SQLite from https://github.com/sqlite/sqlite ...")
# FetchContent_Declare(
#   fetch_sqlite
#   GIT_REPOSITORY https://github.com/sqlite/sqlite
#   GIT_TAG version-3.49.2
# )
# FetchContent_MakeAvailable(fetch_sqlite)

include(ExternalProject)

set(SQLITE_SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/fetch_sqlite-src)
set(SQLITE_INSTALL_DIR ${CMAKE_BINARY_DIR}/_deps/sqlite)
set(TCLLIBDIR ${CMAKE_BINARY_DIR}/_deps/tcl)

ExternalProject_Add(
  sqlite
  PREFIX ${SQLITE_SOURCE_DIR}
  SOURCE_DIR ${SQLITE_SOURCE_DIR}
  INSTALL_DIR ${SQLITE_INSTALL_DIR}
  GIT_REPOSITORY https://github.com/sqlite/sqlite
  GIT_TAG version-3.49.2
  GIT_SHALLOW FALSE
  GIT_PROGRESS TRUE
  GIT_CONFIG advice.detachedHead=false
  BUILD_IN_SOURCE 1
  UPDATE_DISCONNECTED FALSE
  CONFIGURE_COMMAND ${SQLITE_SOURCE_DIR}/configure TCLLIBDIR=${TCLLIBDIR} --prefix=${SQLITE_INSTALL_DIR}
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  STAMP_DIR ${CMAKE_BINARY_DIR}/_deps/stamp
  TMP_DIR ${CMAKE_BINARY_DIR}/_deps/tmp
)

add_library(sqlite_dummy INTERFACE)
add_dependencies(sqlite_dummy sqlite)
add_dependencies(${PROJECT_NAME} sqlite_dummy)

target_include_directories(${PROJECT_NAME} PRIVATE ${SQLITE_INSTALL_DIR}/include)
target_link_directories(${PROJECT_NAME} PRIVATE ${SQLITE_INSTALL_DIR}/lib)
target_link_libraries(${PROJECT_NAME} PRIVATE sqlite3)

set_target_properties(sqlite PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  RUNTIME_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
)

