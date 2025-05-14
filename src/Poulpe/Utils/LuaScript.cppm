export module Poulpe.Utils:LuaScript;

import Poulpe.Core.Log;

import <string>;

extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}

export static bool checkLua(lua_State* L, int r)
{
  if (r != LUA_OK) {
    std::string err = lua_tostring(L, -1);
    PLP_WARN("LUA error: {}", err);
    return false;
  }
  return true;
}
