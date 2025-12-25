#ifndef STRG_LUA_FUNCS_H
#define STRG_LUA_FUNCS_H

#include <lua.h>
#include <strg/config/config.h>

int strg_lua_funcs_register(lua_State *L, struct strg_config *config);

int strg_spawn(lua_State *L);
int strg_set_keybind(lua_State *L);

#endif //STRG_LUA_FUNCS_H