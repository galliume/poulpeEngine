
message(NOTICE "Fetching SQLite from https://www.sqlite.org/2025/sqlite-amalgamation-${SQLITE_TAG}.zip ...")

FetchContent_Declare(
  fetch_sqlite
  URL https://www.sqlite.org/2025/sqlite-amalgamation-${SQLITE_TAG}.zip
  TLS_VERIFY OFF) #@todo fix this
FetchContent_MakeAvailable(fetch_sqlite)

add_library(sqlite3
SHARED
  ${fetch_sqlite_SOURCE_DIR}/sqlite3.c
  ${fetch_sqlite_SOURCE_DIR}/sqlite3.h
  ${fetch_sqlite_SOURCE_DIR}/sqlite3ext.h
)

target_include_directories(sqlite3
SYSTEM PUBLIC
  ${fetch_sqlite_SOURCE_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set_target_properties(sqlite3
  PROPERTIES
    LINK_FLAGS "/DEF:${fetch_sqlite_SOURCE_DIR}/auto.def")
endif()
