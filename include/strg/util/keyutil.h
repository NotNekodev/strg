#ifndef STRG_KEYUTIL_H
#define STRG_KEYUTIL_H

#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <xkbcommon/xkbcommon.h>

static const struct {
    const char *name;
    xkb_keysym_t sym;
} special_keys[] = {
    { "enter",     XKB_KEY_Return },
    { "return",    XKB_KEY_Return },
    { "esc",       XKB_KEY_Escape },
    { "escape",    XKB_KEY_Escape },
    { "tab",       XKB_KEY_Tab },
    { "space",     XKB_KEY_space },
    { "backspace", XKB_KEY_BackSpace },
    { "delete",    XKB_KEY_Delete },
    { "insert",    XKB_KEY_Insert },
    { "home",      XKB_KEY_Home },
    { "end",       XKB_KEY_End },
    { "pageup",    XKB_KEY_Page_Up },
    { "pagedown",  XKB_KEY_Page_Down },
    { "left",      XKB_KEY_Left },
    { "right",     XKB_KEY_Right },
    { "up",        XKB_KEY_Up },
    { "down",      XKB_KEY_Down },
};

// todo: if there is demand move this to a seperate strutil.h header
static inline int str_ieq(const char *a, const char *b) {
	while (*a && *b) {
		if (tolower((unsigned char)*a) != tolower((unsigned char)*a))
			return 0;
		a++;
		b++;
	}
	return *a == '\0' && *b == '\0';
}

// todo: if there is demand move this to a seperate strutil.h header
static inline char *str_tolower(const char *s) {
	char *out = strdup(s);
	for (char *p = out; *p; ++p) {
		*p = tolower(*p);
	}
	return out;
}

bool strg_is_mod_needed(const char *combination);

xkb_keysym_t modifier_from_name(const char *name);
xkb_keysym_t key_from_name(const char *name);

xkb_keysym_t *parse_modifiers(const char *combo, size_t *out_count);
xkb_keysym_t *parse_keys(const char *combo, size_t *out_count);

#endif // STRG_KEYUTIL_H 
