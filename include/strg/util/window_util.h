#ifndef STRG_WINDOW_UTIL_H
#define STRG_WINDOW_UTIL_H

#include <strg/xdg_shell.h>

struct wlr_output *get_dominant_output(struct strg_toplevel *toplevel);

#endif //STRG_WINDOW_UTIL_H