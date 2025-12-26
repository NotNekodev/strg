#include <ctype.h>
#include <errno.h>
#include <strg/config/lua_funcs.h>

#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int strg_lua_funcs_register(lua_State *L, struct strg_config *config) {
    lua_pushlightuserdata(L, config);
    lua_pushcclosure(L, strg_set_keybind, 1);
    lua_setglobal(L, "bind_key");

    lua_pushcfunction(L, strg_spawn);
    lua_setglobal(L, "spawn");

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

    dynarray_add(config->keybinds, keybind);

	wlr_log(WLR_INFO, "Bound key combination %s to lua reference %d", key, ref);

    return 0;
}
