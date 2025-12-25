#ifndef STRG_CONFIG_H
#define STRG_CONFIG_H

#include <lua.h>
#include <strg/util/dynarray.h>
#include <strg/strg.h>

struct strg_keybind {
    char key_combination[128]; // might be inefficent, when strdup exists, but hell do i look like i care? i mean this is way better than do some bs and freeing stuff becauser i use strdup
                               // even if, this is enough space, for your extra specific keybind to open that one exact hentai
                               // if you want to change this open a pr, because i wont
    int lua_callback_ref;
};

struct strg_config {
    struct dynarray *keybinds; // of struct strg_keybind
    lua_State *L; // needed to free the lua callback ref
    struct strg_server *server;
};

struct strg_config *strg_config_load(const char *filepath, struct strg_server *server);
void strg_config_destroy(struct strg_config *config);

#endif //STRG_CONFIG_H