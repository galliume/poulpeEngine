#pragma once

#include "Poulpe/Core/Log.hpp"

#include <string>

extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}

namespace Poulpe
{
  [[maybe_unused]] static bool checkLua(lua_State* L, int r)
  {
    if (r != LUA_OK) {
      std::string err = lua_tostring(L, -1);
      PLP_WARN("LUA error: {}", err);
      return false;
    }
    return true;
  }
}
