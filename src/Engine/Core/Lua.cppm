module;

extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}

export module Engine.Core.LUA;

#undef LUA_OK
#undef LUA_MULTRET
#undef LUA_TFUNCTION
#undef lua_register
#undef luaL_dofile
#undef lua_tostring
#undef lua_isfunction
#undef lua_pcall
#undef lua_tonumber
#undef lua_pushcfunction
#undef luaL_loadfile

export
{
  inline constexpr int LUA_OK = 0;
  inline constexpr int LUA_MULTRET = -1;
  inline constexpr int LUA_TFUNCTION = 6;

  using ::lua_State;
  using ::lua_CFunction;
  using ::lua_Number;
  using ::lua_Integer;
  
  using ::lua_close;
  using ::lua_getglobal;
  using ::lua_isnumber;
  using ::lua_pushlightuserdata;
  using ::lua_pushnumber;
  using ::lua_touserdata;
  using ::lua_setglobal;
  using ::lua_type;
  using ::lua_tolstring;
  
  using ::luaL_newstate;
  using ::luaL_openlibs;
  using ::luaL_loadfilex;
  using ::lua_pcallk;
  using ::lua_tonumberx;
  using ::lua_pushcclosure;

  inline void lua_register(lua_State* L, const char* n, lua_CFunction f) {
    ::lua_pushcclosure(L, f, 0);
    ::lua_setglobal(L, n);
  }

  inline int luaL_dofile(lua_State* L, const char* fn) {
    return (::luaL_loadfilex(L, fn, nullptr) || ::lua_pcallk(L, 0, LUA_MULTRET, 0, 0, nullptr));
  }

  inline const char* lua_tostring(lua_State* L, int i) {
    return ::lua_tolstring(L, i, nullptr);
  }

  inline bool lua_isfunction(lua_State* L, int n) {
    return ::lua_type(L, n) == LUA_TFUNCTION;
  }

  inline int lua_pcall(lua_State* L, int n, int r, int f) {
    return ::lua_pcallk(L, n, r, f, 0, nullptr);
  }

  inline lua_Number lua_tonumber(lua_State* L, int i) {
    return ::lua_tonumberx(L, i, nullptr);
  }

  inline void lua_pushcfunction(lua_State* L, lua_CFunction f) {
    ::lua_pushcclosure(L, f, 0);
  }

  inline int luaL_loadfile(lua_State* L, const char* fn) {
    return ::luaL_loadfilex(L, fn, nullptr);
  }
}
