#ifndef STRG_XDG_SHELL_H
#define STRG_XDG_SHELL_H

#include <strg/strg.h>

enum strg_decoration_type {
    STRG_DECORATION_CLIENT,
    STRG_DECORATION_SERVER,
};

struct strg_toplevel {
    struct wl_list link;
    struct strg_server *server;
    struct wlr_xdg_toplevel *xdg_toplevel;
    struct wlr_scene_tree *scene_tree;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener commit;
    struct wl_listener destroy;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_maximize;
    struct wl_listener request_fullscreen;

    enum strg_decoration_type type;

    struct wlr_scene_rect *titlebar;

    struct wlr_scene_rect *border_left;
    struct wlr_scene_rect *border_right;
    struct wlr_scene_rect *border_bottom;
};

struct strg_popup {
    struct wlr_xdg_popup *xdg_popup;
    struct wl_listener commit;
    struct wl_listener destroy;
};

void xdg_toplevel_map(struct wl_listener *listener, void *data);
void xdg_toplevel_unmap(struct wl_listener *listener, void *data);
void xdg_toplevel_commit(struct wl_listener *listener, void *data);
void xdg_toplevel_destroy(struct wl_listener *listener, void *data);
void xdg_toplevel_request_move(struct wl_listener *listener, void *data);
void xdg_toplevel_request_resize(struct wl_listener *listener, void *data);
void xdg_toplevel_request_maximize(struct wl_listener *listener, void *data);
void xdg_toplevel_request_fullscreen(struct wl_listener *listener, void *data);
void server_new_xdg_toplevel(struct wl_listener *listener, void *data);
void xdg_popup_commit(struct wl_listener *listener, void *data);
void xdg_popup_destroy(struct wl_listener *listener, void *data);
void server_new_xdg_popup(struct wl_listener *listener, void *data);
void begin_interactive(struct strg_toplevel *toplevel, enum strg_cursor_mode mode, uint32_t edges);
void handle_xdg_decoration_request_mode(struct wl_listener *listener, void *data);
void handle_new_xdg_decoration(struct wl_listener *listener, void *data);

#endif //STRG_XDG_SHELL_H