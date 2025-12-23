#ifndef STRG_XWL_OPS_H
#define STRG_XWL_OPS_H

#include "xwl.h"
#include <strg/strg.h>

void xwl_surface_configure(struct strg_xwayland_surface *surface,
                           int16_t x, int16_t y, uint16_t width, uint16_t height);
void xwl_surface_close(struct strg_xwayland_surface *surface);
void xwl_surface_set_maximized(struct strg_xwayland_surface *surface);
void xwl_surface_set_fullscreen(struct strg_xwayland_surface *surface, bool fullscreen);
void xwl_surface_set_minimized(struct strg_xwayland_surface *surface, bool minimized);
void xwl_surface_set_activated(struct strg_xwayland_surface *surface, bool activated);
void xwl_begin_interactive(struct strg_xwayland_surface *window, enum strg_cursor_mode mode, uint32_t edges);
void xwl_surface_move(struct strg_xwayland_surface *surface);
void xwl_surface_resize(struct strg_xwayland_surface *surface, uint32_t edges);

#endif // STRG_XWL_OPS_H
