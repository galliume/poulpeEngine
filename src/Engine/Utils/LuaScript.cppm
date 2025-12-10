module;
extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}

export module Engine.Utils.LuaScript;

import std;

namespace Poulpe
{
  export bool checkLua(lua_State* L, int r);
}
