#include <ctype.h>
#include <errno.h>
#include <strg/config/lua_funcs.h>

#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <strg/util/keyutil.h>

static int keyboard_index(lua_State *L) {
    struct strg_config *config = lua_touserdata(L, lua_upvalueindex(1));
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "layout") == 0) {
        lua_pushstring(L, config->kb_conf.layout);
        return 1;
    }
    if (strcmp(key, "rule") == 0) {
        lua_pushstring(L, config->kb_conf.rule);
        return 1;
    }
    if (strcmp(key, "model") == 0) {
        lua_pushstring(L, config->kb_conf.model);
        return 1;
    }
    if (strcmp(key, "variant") == 0) {
        lua_pushstring(L, config->kb_conf.variant);
        return 1;
    }
    if (strcmp(key, "options") == 0) {
        lua_pushstring(L, config->kb_conf.options);
        return 1;
    }

    return luaL_error(L, "Unknown field '%s' in keyboard", key);
}

static int keyboard_newindex(lua_State *L) {
    struct strg_config *config = lua_touserdata(L, lua_upvalueindex(1));
    const char *key = luaL_checkstring(L, 2);
    const char *val = luaL_checkstring(L, 3);

    if (strcmp(key, "layout") == 0) {
        strncpy(config->kb_conf.layout, val, sizeof(config->kb_conf.layout)-1);
        config->kb_conf.layout[sizeof(config->kb_conf.layout)-1] = '\0';
        return 0;
    }
    if (strcmp(key, "rule") == 0) {
        strncpy(config->kb_conf.rule, val, sizeof(config->kb_conf.rule)-1);
        config->kb_conf.rule[sizeof(config->kb_conf.rule)-1] = '\0';
        return 0;
    }
    if (strcmp(key, "model") == 0) {
        strncpy(config->kb_conf.model, val, sizeof(config->kb_conf.model)-1);
        config->kb_conf.model[sizeof(config->kb_conf.model)-1] = '\0';
        return 0;
    }
    if (strcmp(key, "variant") == 0) {
        strncpy(config->kb_conf.variant, val, sizeof(config->kb_conf.variant)-1);
        config->kb_conf.variant[sizeof(config->kb_conf.variant)-1] = '\0';
        return 0;
    }
    if (strcmp(key, "options") == 0) {
        strncpy(config->kb_conf.options, val, sizeof(config->kb_conf.options)-1);
        config->kb_conf.options[sizeof(config->kb_conf.options)-1] = '\0';
        return 0;
    }

    return luaL_error(L, "Unknown or read-only field '%s' in keyboard", key);
}

int strg_lua_funcs_register(lua_State *L, struct strg_config *config) {
    lua_pushlightuserdata(L, config);
    lua_pushcclosure(L, strg_set_keybind, 1);
    lua_setglobal(L, "bind_key");

    lua_pushcfunction(L, strg_spawn);
    lua_setglobal(L, "spawn");

	// config table stuff
	lua_newtable(L);

    lua_newtable(L); 
	lua_newtable(L);
	lua_pushlightuserdata(L, config);
    lua_pushcclosure(L, keyboard_index, 1);
    lua_setfield(L, -2, "__index");
    lua_pushlightuserdata(L, config);
    lua_pushcclosure(L, keyboard_newindex, 1);
    lua_setfield(L, -2, "__newindex");

    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "keyboard"); 

    lua_setglobal(L, "opt");

    return 0;
}

int strg_spawn(lua_State *L) {
    const char *command = luaL_checkstring(L, 1);
    if (!command || !*command) {
        luaL_error(L, "Command string is empty");
        return 0;
    }

    size_t max_args = 64;
    char **argv = malloc((max_args + 1) * sizeof(char *));
    if (!argv)
        luaL_error(L, "Memory allocation failed: %s", strerror(errno));

    size_t argc = 0;
    const char *p = command;

    while (*p) {
        while (isspace(*p)) p++;
        if (!*p) break;

        char quote = 0;
        if (*p == '"' || *p == '\'') {
            quote = *p++;
        }

        const char *start = p;
        char *arg = NULL;
        size_t len = 0;

        if (quote) {
            while (*p && *p != quote) p++;
            if (*p != quote) {
                free(argv);
                luaL_error(L, "Unclosed quote in command");
                return 0;
            }
            len = p - start;
            p++;
        } else {
            while (*p && !isspace(*p)) p++;
            len = p - start;
        }

        arg = malloc(len + 1);
        if (!arg) {
            for (size_t j = 0; j < argc; j++) free(argv[j]);
            free(argv);
            luaL_error(L, "Memory allocation failed: %s", strerror(errno));
            return 0;
        }
        strncpy(arg, start, len);
        arg[len] = '\0';
        argv[argc++] = arg;

        if (argc >= max_args) break;
    }

    argv[argc] = NULL;

    pid_t pid = fork();
    if (pid < 0) {
        for (size_t j = 0; j < argc; j++) free(argv[j]);
        free(argv);
        luaL_error(L, "fork() failed: %s", strerror(errno));
        return 0;
    } else if (pid == 0) {
        if (setsid() < 0) _exit(EXIT_FAILURE);
        execvp(argv[0], argv);
        _exit(EXIT_FAILURE);
    }

    for (size_t j = 0; j < argc; j++) free(argv[j]);
    free(argv);

    lua_pushinteger(L, pid);
    return 1;
}

int strg_set_keybind(lua_State *L) {
    const char *key = luaL_checkstring(L, 1);

    if (!lua_isfunction(L, 2)) {
        return luaL_error(L, "Action argument is not a function");
    }

    if (strlen(key) >= 128) {
        return luaL_error(L, "Key combination too long");
    }

    struct strg_config *config = lua_touserdata(L, lua_upvalueindex(1));

    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    struct strg_keybind *keybind = malloc(sizeof(struct strg_keybind));
    memset(keybind, 0, sizeof(struct strg_keybind));
    strncpy(keybind->key_combination, key, sizeof(keybind->key_combination) - 1);
    keybind->lua_callback_ref = ref;

	xkb_keysym_t *modifiers = parse_modifiers(keybind->key_combination, &keybind->keycode.mod_count);
	xkb_keysym_t *keys = parse_keys(keybind->key_combination, &keybind->keycode.key_count);

	keybind->keycode.needs_mod = strg_is_mod_needed(keybind->key_combination);

	if (keys) {
		memcpy(&keybind->keycode.keys, keys, sizeof(keybind->keycode.keys));
	}

	if (modifiers) {
		memcpy(&keybind->keycode.mods, modifiers, sizeof(keybind->keycode.mods));
	}

	wlr_log(WLR_INFO, "New Keybind:");
	wlr_log(WLR_INFO, "    Keys:");
	for (size_t i = 0; i < keybind->keycode.key_count; i++) {
		char name[64];
 		xkb_keysym_get_name(keybind->keycode.keys[i], name, sizeof(name));
		wlr_log(WLR_INFO, "    - XKB_KEY_%s", name);
	}
	wlr_log(WLR_INFO, "    Modifiers:");
	if (keybind->keycode.mod_count == 0) {
		wlr_log(WLR_INFO, "    NONE");
	} else {
		for (size_t i = 0; i < keybind->keycode.mod_count; i++) {
			char name[64];
 			xkb_keysym_get_name(keybind->keycode.mods[i], name, sizeof(name));
			wlr_log(WLR_INFO, "    - XKB_KEY_%s", name);
		}
	}
	wlr_log(WLR_INFO, "    Uses `Mod`: %s", keybind->keycode.needs_mod ? "yes" : "no");

    dynarray_add(config->keybinds, keybind);

	free(modifiers);
	free(keys);

    return 0;
}
