#include <strg/util/keyutil.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/util/log.h>
#include <strings.h>

static void trim_whitespace(char *str) {
    if (!str || !*str) return;
    
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    
    size_t len = (end - start) + 1;
    if (start != str) {
        memmove(str, start, len);
    }
    str[len] = '\0';
}

static xkb_keysym_t parse_single_modifier(const char *mod_str) {
    if (!mod_str || !*mod_str) return XKB_KEY_NoSymbol;
    
    if (strcasecmp(mod_str, "shift") == 0) return XKB_KEY_Shift_L;
    if (strcasecmp(mod_str, "shiftl") == 0) return XKB_KEY_Shift_L;
    if (strcasecmp(mod_str, "shiftr") == 0) return XKB_KEY_Shift_R;
    
    if (strcasecmp(mod_str, "ctrl") == 0) return XKB_KEY_Control_L;
    if (strcasecmp(mod_str, "control") == 0) return XKB_KEY_Control_L;
    if (strcasecmp(mod_str, "ctrll") == 0) return XKB_KEY_Control_L;
    if (strcasecmp(mod_str, "ctrlr") == 0) return XKB_KEY_Control_R;
    if (strcasecmp(mod_str, "controll") == 0) return XKB_KEY_Control_L;
    if (strcasecmp(mod_str, "controlr") == 0) return XKB_KEY_Control_R;
    
    if (strcasecmp(mod_str, "alt") == 0) return XKB_KEY_Alt_L;
    if (strcasecmp(mod_str, "altl") == 0) return XKB_KEY_Alt_L;
    if (strcasecmp(mod_str, "altr") == 0) return XKB_KEY_Alt_R;
    if (strcasecmp(mod_str, "meta") == 0) return XKB_KEY_Alt_L;
    
    if (strcasecmp(mod_str, "super") == 0) return XKB_KEY_Super_L;
    if (strcasecmp(mod_str, "logo") == 0) return XKB_KEY_Super_L;
    if (strcasecmp(mod_str, "win") == 0) return XKB_KEY_Super_L;
    if (strcasecmp(mod_str, "windows") == 0) return XKB_KEY_Super_L;
    if (strcasecmp(mod_str, "superl") == 0) return XKB_KEY_Super_L;
    if (strcasecmp(mod_str, "superr") == 0) return XKB_KEY_Super_R;
    if (strcasecmp(mod_str, "cmd") == 0) return XKB_KEY_Super_L;
    if (strcasecmp(mod_str, "command") == 0) return XKB_KEY_Super_L;
    
    if (strcasecmp(mod_str, "mod1") == 0) return XKB_KEY_Alt_L;
    if (strcasecmp(mod_str, "mod2") == 0) return XKB_KEY_Num_Lock;
    if (strcasecmp(mod_str, "mod3") == 0) return XKB_KEY_Hyper_L;
    if (strcasecmp(mod_str, "mod4") == 0) return XKB_KEY_Super_L;
    if (strcasecmp(mod_str, "mod5") == 0) return XKB_KEY_ISO_Level3_Shift;
    
    if (strcasecmp(mod_str, "mod") == 0) return XKB_KEY_VoidSymbol;
    
    return XKB_KEY_NoSymbol;
}

static bool is_modifier_key(xkb_keysym_t sym) {
    return sym == XKB_KEY_Shift_L || sym == XKB_KEY_Shift_R ||
           sym == XKB_KEY_Control_L || sym == XKB_KEY_Control_R ||
           sym == XKB_KEY_Alt_L || sym == XKB_KEY_Alt_R ||
           sym == XKB_KEY_Super_L || sym == XKB_KEY_Super_R ||
           sym == XKB_KEY_Hyper_L || sym == XKB_KEY_Hyper_R ||
           sym == XKB_KEY_Meta_L || sym == XKB_KEY_Meta_R ||
           sym == XKB_KEY_Num_Lock || sym == XKB_KEY_Caps_Lock ||
           sym == XKB_KEY_ISO_Level3_Shift || sym == XKB_KEY_VoidSymbol;
}

bool strg_is_mod_needed(const char *key_combination) {
    if (!key_combination || !*key_combination) return false;
    
    char *copy = strdup(key_combination);
    if (!copy) return false;
    
    char *saveptr;
    char *token = strtok_r(copy, "+", &saveptr);
    bool has_mod = false;
    
    while (token) {
        trim_whitespace(token);
        
        if (strcasecmp(token, "mod") == 0) {
            has_mod = true;
            break;
        }
        
        token = strtok_r(NULL, "+", &saveptr);
    }
    
    free(copy);
    return has_mod;
}

xkb_keysym_t *parse_modifiers(const char *key_combination, size_t *count) {
    if (!key_combination || !count) {
        if (count) *count = 0;
        return NULL;
    }
    
    *count = 0;
    
    xkb_keysym_t *mods = calloc(8, sizeof(xkb_keysym_t));
    if (!mods) {
        wlr_log(WLR_ERROR, "Failed to allocate memory for modifiers");
        return NULL;
    }
    
    char *copy = strdup(key_combination);
    if (!copy) {
        free(mods);
        wlr_log(WLR_ERROR, "Failed to duplicate key combination string");
        return NULL;
    }
    
    char *saveptr;
    char *token = strtok_r(copy, "+", &saveptr);
    
    while (token && *count < 8) {
        trim_whitespace(token);
        
        if (!*token) {
            token = strtok_r(NULL, "+", &saveptr);
            continue;
        }
        
        xkb_keysym_t sym = parse_single_modifier(token);
        
        if (sym != XKB_KEY_NoSymbol && is_modifier_key(sym)) {
            bool duplicate = false;
            for (size_t i = 0; i < *count; i++) {
                if (mods[i] == sym) {
                    duplicate = true;
                    break;
                }
            }
            
            if (!duplicate) {
                mods[*count] = sym;
                (*count)++;
            }
        }
        
        token = strtok_r(NULL, "+", &saveptr);
    }
    
    free(copy);
    
    if (*count == 0) {
        free(mods);
        return NULL;
    }
    
    return mods;
}

xkb_keysym_t *parse_keys(const char *key_combination, size_t *count) {
    if (!key_combination || !count) {
        if (count) *count = 0;
        return NULL;
    }
    
    *count = 0;
    
    xkb_keysym_t *keys = calloc(128, sizeof(xkb_keysym_t));
    if (!keys) {
        wlr_log(WLR_ERROR, "Failed to allocate memory for keys");
        return NULL;
    }
    
    char *copy = strdup(key_combination);
    if (!copy) {
        free(keys);
        wlr_log(WLR_ERROR, "Failed to duplicate key combination string");
        return NULL;
    }
    
    char *saveptr;
    char *token = strtok_r(copy, "+", &saveptr);
    
    while (token && *count < 128) {
        trim_whitespace(token);
        
        if (!*token) {
            token = strtok_r(NULL, "+", &saveptr);
            continue;
        }
        
        xkb_keysym_t mod_check = parse_single_modifier(token);
        
        if (mod_check == XKB_KEY_NoSymbol) {
            xkb_keysym_t sym = xkb_keysym_from_name(token, XKB_KEYSYM_CASE_INSENSITIVE);
            
            if (sym == XKB_KEY_NoSymbol) {
                char prefixed[256];
                snprintf(prefixed, sizeof(prefixed), "XKB_KEY_%s", token);
                sym = xkb_keysym_from_name(prefixed, XKB_KEYSYM_NO_FLAGS);
            }
            
            if (sym != XKB_KEY_NoSymbol) {
                bool duplicate = false;
                for (size_t i = 0; i < *count; i++) {
                    if (keys[i] == sym) {
                        duplicate = true;
                        break;
                    }
                }
                
                if (!duplicate) {
                    keys[*count] = sym;
                    (*count)++;
                }
            } else {
                wlr_log(WLR_ERROR, "Unknown key name: '%s' in combination '%s'", 
                       token, key_combination);
            }
        }
        
        token = strtok_r(NULL, "+", &saveptr);
    }
    
    free(copy);
    
    if (*count == 0) {
        free(keys);
        return NULL;
    }
    
    return keys;
}

uint32_t mod_to_wlr_modifier(xkb_keysym_t mod_sym) {
    switch (mod_sym) {
        case XKB_KEY_Shift_L:
        case XKB_KEY_Shift_R:
            return WLR_MODIFIER_SHIFT;
            
        case XKB_KEY_Control_L:
        case XKB_KEY_Control_R:
            return WLR_MODIFIER_CTRL;
            
        case XKB_KEY_Alt_L:
        case XKB_KEY_Alt_R:
        case XKB_KEY_Meta_L:
        case XKB_KEY_Meta_R:
            return WLR_MODIFIER_ALT;
            
        case XKB_KEY_Super_L:
        case XKB_KEY_Super_R:
        case XKB_KEY_Hyper_L:
        case XKB_KEY_Hyper_R:
            return WLR_MODIFIER_LOGO;
            
        case XKB_KEY_Caps_Lock:
            return WLR_MODIFIER_CAPS;
            
        case XKB_KEY_Num_Lock:
            return WLR_MODIFIER_MOD2;
            
        default:
            return 0;
    }
}

bool modifiers_match(uint32_t current_mods, xkb_keysym_t *required_mods, 
                     size_t mod_count, bool needs_mod) {
    if (needs_mod) {
        uint32_t any_mod = WLR_MODIFIER_ALT | WLR_MODIFIER_LOGO | 
                          WLR_MODIFIER_CTRL | WLR_MODIFIER_SHIFT;
        return (current_mods & any_mod) != 0;
    }
    
    if (mod_count == 0 || !required_mods) {
        uint32_t filtered_mods = current_mods & ~(WLR_MODIFIER_CAPS | WLR_MODIFIER_MOD2);
        return filtered_mods == 0;
    }
    
    uint32_t required_mask = 0;
    for (size_t i = 0; i < mod_count; i++) {
        uint32_t mod_flag = mod_to_wlr_modifier(required_mods[i]);
        if (mod_flag != 0) {
            required_mask |= mod_flag;
        }
    }
    
    uint32_t filtered_current = current_mods & ~(WLR_MODIFIER_CAPS | WLR_MODIFIER_MOD2);
    
    return filtered_current == required_mask;
}

bool keysym_array_equal_unordered(xkb_keysym_t *a, xkb_keysym_t *b, size_t n) {
    if (!a || !b) return false;
    if (n == 0) return true;
    
    bool *used = calloc(n, sizeof(bool));
    if (!used) return false;
    
    bool result = true;
    
    for (size_t i = 0; i < n; i++) {
        bool found = false;
        for (size_t j = 0; j < n; j++) {
            if (!used[j] && a[i] == b[j]) {
                used[j] = true;
                found = true;
                break;
            }
        }
        if (!found) {
            result = false;
            break;
        }
    }
    
    free(used);
    return result;
}
