module;

#include <string>

extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}

module Engine.Utils.LuaScript;

import Engine.Core.Logger;

namespace Poulpe
{
  bool checkLua(lua_State* L, int r)
  {
    if (r != LUA_OK) {
      std::string err = ::lua_tostring(L, -1);
      Logger::warn("LUA error: {}", err);
      return false;
    }
    return true;
  }
}
