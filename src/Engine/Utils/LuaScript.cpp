module Engine.Utils.LuaScript;

import std;

import Engine.Core.LUA;
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
