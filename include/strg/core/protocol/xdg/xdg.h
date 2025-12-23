#ifndef STRG_XDG_SHELL_H
#define STRG_XDG_SHELL_H

#include <strg/strg.h>

enum strg_decoration_type {
    STRG_DECORATION_CLIENT,
    STRG_DECORATION_SERVER,
};

enum strg_decoration_pref {
    STRG_DECORATION_PREF_UNKNOWN,
    STRG_DECORATION_PREF_CLIENT,
    STRG_DECORATION_PREF_SERVER,
};

struct strg_xdg_decoration {
    struct wlr_xdg_toplevel_decoration_v1 *decoration;
    struct wl_listener surface_commit;
    struct wl_listener request_mode;
    struct wl_listener destroy;

    struct strg_toplevel *toplevel;
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
    struct wl_listener configure;

    enum strg_decoration_type type;
    bool has_xdg_decoration;
    enum strg_decoration_pref decoration_pref;
    bool decorations_applied;

    struct wlr_scene_rect *titlebar;
    struct wlr_scene_buffer *title_text;
    struct wlr_scene_buffer *close_button_buffer;
    struct wlr_scene_buffer *maximize_button_buffer;
    struct wlr_scene_buffer *minimize_button_buffer;
    struct wlr_scene_rect *border_left;
    struct wlr_scene_rect *border_right;
    struct wlr_scene_rect *border_bottom;

    bool is_maximized;
    struct wlr_box pre_maximize_geometry;
    bool pending_demax_restore;

    struct wlr_box internal_maximized_geometry; // basically so the decorations are still visible
    bool use_internal_maximize_geometry;
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
void xdg_toplevel_create(struct wl_listener *listener, void *data);
void xdg_popup_commit(struct wl_listener *listener, void *data);
void xdg_popup_destroy(struct wl_listener *listener, void *data);
void xdg_popup_create(struct wl_listener *listener, void *data);
void xdg_decoration_request_mode_handler(struct wl_listener *listener, void *data);
void xdg_new_decoration_handler(struct wl_listener *listener, void *data);

#endif //STRG_XDG_SHELL_H