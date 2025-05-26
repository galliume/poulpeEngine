message(NOTICE "Fetching SQLite from https://www.sqlite.org/snapshot ...")

FetchContent_Declare(
  fetch_sqlite
  URL https://www.sqlite.org/snapshot/sqlite-snapshot-202505081618.tar.gz
)
FetchContent_MakeAvailable(fetch_sqlite)

#ugly fix
file(REMOVE 
  ${fetch_sqlite_SOURCE_DIR}/VERSION)

add_library(sqlite3
SHARED
  ${fetch_sqlite_SOURCE_DIR}/sqlite3.c
  ${fetch_sqlite_SOURCE_DIR}/sqlite3.h
  ${fetch_sqlite_SOURCE_DIR}/sqlite3ext.h
)

target_include_directories(sqlite3 
SYSTEM PUBLIC
  ${fetch_sqlite_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
PRIVATE
  sqlite3)

if (CMAKE_CXX_COMPILER STREQUAL "MSVC")
  set_target_properties(sqlite3
  PROPERTIES
    LINK_FLAGS "/DEF:${fetch_sqlite_SOURCE_DIR}/auto.def")
endif()
