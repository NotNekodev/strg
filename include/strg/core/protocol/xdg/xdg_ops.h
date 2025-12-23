#ifndef STRG_WINDOW_FUNCTION_H
#define STRG_WINDOW_FUNCTION_H

#include <strg/core/protocol/xdg/xdg.h>

void xdg_window_maximize(struct strg_toplevel *toplevel);
void xdg_window_move(struct strg_toplevel *toplevel);
void xdg_window_resize(struct strg_toplevel *toplevel);
void xdg_window_fullscreen(struct strg_toplevel *toplevel);

void xdg_window_map(struct strg_toplevel *toplevel);
void xdg_window_unmap(struct strg_toplevel *toplevel);

void xdg_begin_interactive(struct strg_toplevel *window, enum strg_cursor_mode mode, uint32_t edges);

#endif //STRG_WINDOW_FUNCTION_H