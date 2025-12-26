#include <strg/util/keyutil.h>

#include <stdlib.h>

bool strg_is_mod_needed(const char *combination) {
	if (!combination)
		return false;

	char *tmp = strdup(combination);
	char *save;
	char *tok = strtok_r(tmp, "+", &save);

	while (tok) {
		while (isspace((unsigned char)*tok))
			tok++;

		char *end = tok + strlen(tok) - 1;
		while (end > tok && isspace((unsigned char)*end)) {
			*end = '\0';
			end--;
		}

		if (strlen(tok) == 3) {
            char m[4];
            for (int i = 0; i < 3; i++)
                m[i] = tolower((unsigned char)tok[i]);
            m[3] = '\0';

            if (strcmp(m, "mod") == 0) {
                free(tmp);
                return true;
            }
        }

        tok = strtok_r(NULL, "+", &save);
    }

    free(tmp);
    return false;
}

xkb_keysym_t modifier_from_name(const char *name) {
	if (!name) return XKB_KEY_NoSymbol;

    if (!strcmp(name, "shift"))   return XKB_KEY_Shift_L;
    if (!strcmp(name, "ctrl") ||
        !strcmp(name, "control")) return XKB_KEY_Control_L;
    if (!strcmp(name, "alt"))     return XKB_KEY_Alt_L;
    if (!strcmp(name, "super") ||
        !strcmp(name, "win"))     return XKB_KEY_Super_L;
    if (!strcmp(name, "meta"))    return XKB_KEY_Meta_L;

    return XKB_KEY_NoSymbol;
}

xkb_keysym_t key_from_name(const char *name) {
	if (!name || !*name)
        return XKB_KEY_NoSymbol;

    if (strlen(name) == 1) {
        char c = name[0];
        if (isprint((unsigned char)c)) {
            char buf[2] = { (char)tolower(c), 0 };
            return xkb_keysym_from_name(buf, XKB_KEYSYM_CASE_INSENSITIVE);
        }
    }

    if (name[0] == 'f' && isdigit((unsigned char)name[1])) {
        return xkb_keysym_from_name(name, XKB_KEYSYM_CASE_INSENSITIVE);
    }

    for (size_t i = 0; i < sizeof(special_keys) / sizeof(special_keys[0]); i++) {
        if (!strcmp(name, special_keys[i].name))
            return special_keys[i].sym;
    }

    return xkb_keysym_from_name(name, XKB_KEYSYM_CASE_INSENSITIVE);
}

xkb_keysym_t *parse_modifiers(const char *combo, size_t *out_count) {
	*out_count = 0;
    xkb_keysym_t *mods = NULL;

    char *tmp = strdup(combo);
    char *save;
    char *tok = strtok_r(tmp, "+", &save);

    while (tok) {
        char *t = str_tolower(tok);

        if (strcmp(t, "mod") != 0) {
            xkb_keysym_t sym = modifier_from_name(t);
            if (sym != XKB_KEY_NoSymbol) {
                mods = realloc(mods, sizeof(xkb_keysym_t) * (*out_count + 1));
                mods[*out_count] = sym;
                (*out_count)++;
            }
        }

        free(t);
        tok = strtok_r(NULL, "+", &save);
    }

    free(tmp);
    return mods;
}

xkb_keysym_t *parse_keys(const char *combo, size_t *out_count) {
    *out_count = 0;
    xkb_keysym_t *keys = NULL;

    char *tmp = strdup(combo);
    char *save;
    char *tok = strtok_r(tmp, "+", &save);

    while (tok) {
        char *t = str_tolower(tok);

        if (!modifier_from_name(t) && strcmp(t, "mod") != 0) {
            xkb_keysym_t sym = key_from_name(t);
            if (sym != XKB_KEY_NoSymbol) {
                keys = realloc(keys, sizeof(xkb_keysym_t) * (*out_count + 1));
                keys[*out_count] = sym;
                (*out_count)++;
            }
        }

        free(t);
        tok = strtok_r(NULL, "+", &save);
    }

    free(tmp);
    return keys;
}


