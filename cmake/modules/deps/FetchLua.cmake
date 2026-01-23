message(NOTICE "Fetching LUA from https://github.com/lua/lua ...")

FetchContent_Declare(
  fetch_lua
  GIT_REPOSITORY https://github.com/lua/lua
  GIT_TAG ${LUA_TAG}
  GIT_SHALLOW ${FETCH_GIT_SHALLOW}
  GIT_PROGRESS ${FETCH_GIT_PROGRESS}
  UPDATE_DISCONNECTED ${FETCH_UPDATE_DISCONNECTED}
)
FetchContent_MakeAvailable(fetch_lua)

target_include_directories(${PROJECT_NAME}
SYSTEM PRIVATE
  ${fetch_lua_SOURCE_DIR})

add_library(
  lua STATIC
  ${fetch_lua_SOURCE_DIR}/lapi.c
  ${fetch_lua_SOURCE_DIR}/lauxlib.c
  ${fetch_lua_SOURCE_DIR}/lbaselib.c
  ${fetch_lua_SOURCE_DIR}/lcode.c
  ${fetch_lua_SOURCE_DIR}/lcorolib.c
  ${fetch_lua_SOURCE_DIR}/lctype.c
  ${fetch_lua_SOURCE_DIR}/ldblib.c
  ${fetch_lua_SOURCE_DIR}/ldebug.c
  ${fetch_lua_SOURCE_DIR}/ldo.c
  ${fetch_lua_SOURCE_DIR}/ldump.c
  ${fetch_lua_SOURCE_DIR}/lfunc.c
  ${fetch_lua_SOURCE_DIR}/lgc.c
  ${fetch_lua_SOURCE_DIR}/linit.c
  ${fetch_lua_SOURCE_DIR}/liolib.c
  ${fetch_lua_SOURCE_DIR}/llex.c
  ${fetch_lua_SOURCE_DIR}/lmathlib.c
  ${fetch_lua_SOURCE_DIR}/lmem.c
  ${fetch_lua_SOURCE_DIR}/loadlib.c
  ${fetch_lua_SOURCE_DIR}/lobject.c
  ${fetch_lua_SOURCE_DIR}/lopcodes.c
  ${fetch_lua_SOURCE_DIR}/loslib.c
  ${fetch_lua_SOURCE_DIR}/lparser.c
  ${fetch_lua_SOURCE_DIR}/lstate.c
  ${fetch_lua_SOURCE_DIR}/lstring.c
  ${fetch_lua_SOURCE_DIR}/lstrlib.c
  ${fetch_lua_SOURCE_DIR}/ltable.c
  ${fetch_lua_SOURCE_DIR}/ltablib.c
  ${fetch_lua_SOURCE_DIR}/ltests.c
  ${fetch_lua_SOURCE_DIR}/ltm.c
  ${fetch_lua_SOURCE_DIR}/lundump.c
  ${fetch_lua_SOURCE_DIR}/lutf8lib.c
  ${fetch_lua_SOURCE_DIR}/lvm.c
  ${fetch_lua_SOURCE_DIR}/lzio.c
  ${fetch_lua_SOURCE_DIR}/lauxlib.h
  ${fetch_lua_SOURCE_DIR}/lua.h
  ${fetch_lua_SOURCE_DIR}/luaconf.h
  ${fetch_lua_SOURCE_DIR}/lualib.h
)

target_include_directories(lua
  SYSTEM INTERFACE ${fetch_lua_SOURCE_DIR})