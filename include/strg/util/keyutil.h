#ifndef STRG_KEYUTIL_H
#define STRG_KEYUTIL_H

#include <xkbcommon/xkbcommon.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

xkb_keysym_t *parse_modifiers(const char *key_combination, size_t *count);

xkb_keysym_t *parse_keys(const char *key_combination, size_t *count);

bool strg_is_mod_needed(const char *key_combination);

uint32_t mod_to_wlr_modifier(xkb_keysym_t mod_sym);

bool modifiers_match(uint32_t current_mods, xkb_keysym_t *required_mods, 
                     size_t mod_count, bool needs_mod);

bool keysym_array_equal_unordered(xkb_keysym_t *a, xkb_keysym_t *b, size_t n);

#endif // STRG_KEYUTIL_H
