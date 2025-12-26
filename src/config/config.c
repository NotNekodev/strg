#include <lauxlib.h>
#include <lualib.h>
#include <stdlib.h>
#include <strg/config/config.h>

#include "strg/config/lua_funcs.h"

struct strg_config *strg_config_load(const char *filepath, struct strg_server *server) {
    struct strg_config *config = malloc(sizeof(struct strg_config));

	config->keybinds = dynarray_create();

	strcpy(config->kb_conf.layout, "us");
	strcpy(config->kb_conf.rule, "evdev");
	strcpy(config->kb_conf.model, "pc105");
	strcpy(config->kb_conf.variant, "\0");
	strcpy(config->kb_conf.options, "grp:");

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    strg_lua_funcs_register(L, config);

    if (luaL_dofile(L, filepath) != LUA_OK) {
        const char *error_msg = lua_tostring(L, -1);
        wlr_log(WLR_ERROR, "Error loading config file: %s\n", error_msg);
        lua_close(L);
        free(config);
        return NULL;
    }

    return config;
}

void strg_config_destroy(struct strg_config *config) {
    if (!config) return;

    DYNARRAY_FOREACH(config->keybinds, i, keybind) {
        struct strg_keybind *kb = keybind;

        if (kb->lua_callback_ref != LUA_NOREF &&
            kb->lua_callback_ref != LUA_REFNIL) {

            luaL_unref(config->L, LUA_REGISTRYINDEX, kb->lua_callback_ref);
            kb->lua_callback_ref = LUA_NOREF;
            }
    }

    dynarray_destroy(config->keybinds);
    lua_close(config->L);
    free(config);
}
