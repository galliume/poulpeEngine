message(NOTICE "Fetching SQLite from https://github.com/sqlite/sqlite ...")
# FetchContent_Declare(
#   fetch_sqlite
#   GIT_REPOSITORY https://github.com/sqlite/sqlite
#   GIT_TAG version-3.49.2
# )
# FetchContent_MakeAvailable(fetch_sqlite)

include(ExternalProject)

set(SQLITE_SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/fetch_sqlite-src)
set(TCLLIBDIR ${CMAKE_BINARY_DIR}/_deps/tcl)
set(TCLSH "tclsh90")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(SQLITE_CONFIGURE_COMMAND 
            ${CMAKE_COMMAND} -E env
            CC=clang
            CXX=clang++
            LD=lld-link
            AR=llvm-ar
            bash ${SQLITE_SOURCE_DIR}/configure --disable-static --out-implib=none)
else()
  set(SQLITE_CONFIGURE_COMMAND ${SQLITE_SOURCE_DIR}/configure TCLLIBDIR=${TCLLIBDIR})
endif()

ExternalProject_Add(
  sqlite
  PREFIX ${SQLITE_SOURCE_DIR}
  SOURCE_DIR ${SQLITE_SOURCE_DIR}
  INSTALL_DIR ${SQLITE_INSTALL_DIR}
  URL https://www.sqlite.org/snapshot/sqlite-snapshot-202505081618.tar.gz
  CONFIGURE_COMMAND ${SQLITE_CONFIGURE_COMMAND}
  BUILD_IN_SOURCE 1
  BUILD_COMMAND make
  STAMP_DIR ${SQLITE_SOURCE_DIR}
  TMP_DIR ${SQLITE_INSTALL_DIR}
  INSTALL_COMMAND ""
)

add_library(sqlite_dummy INTERFACE)
add_dependencies(sqlite_dummy sqlite)
add_dependencies(${PROJECT_NAME} sqlite_dummy)

target_include_directories(${PROJECT_NAME} PRIVATE ${SQLITE_SOURCE_DIR})
target_link_directories(${PROJECT_NAME} PRIVATE ${SQLITE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE sqlite3)
