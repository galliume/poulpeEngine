module;
#include <string>

extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}

export module Engine.Utils.LuaScript;

namespace Poulpe
{
  export bool checkLua(lua_State* L, int r);
}
