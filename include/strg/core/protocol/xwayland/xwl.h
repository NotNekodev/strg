#ifndef STRG_XWL_H
#define STRG_XWL_H

#include <wlr/xwayland.h>
#include <wayland-server-core.h>
#include <wlr/util/box.h>

struct strg_xwayland {
    struct wlr_xwayland *xwayland;
    struct strg_server *server;

    struct wl_listener new_surface;
    struct wl_listener ready;
    struct wl_listener remove;

    struct wlr_seat *seat;
};

struct strg_xwayland_surface {
    struct strg_xwayland *xwl;
    struct wlr_xwayland_surface *xwayland_surface;
    struct wlr_scene_surface *scene_surface;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener request_configure;
    struct wl_listener request_fullscreen;
    struct wl_listener request_maximize;
    struct wl_listener request_minimize;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_activate;
    struct wl_listener set_title;
    struct wl_listener set_class;
    struct wl_listener associate;
    struct wl_listener dissociate;

    struct wlr_scene_tree *scene_tree;
    struct wl_list link;

    bool is_maximized;
    bool mapped;
    struct wlr_box pre_maximize_geometry;
};

bool xwl_init(struct strg_xwayland *xwl, struct wl_display *display,
              struct wlr_compositor *compositor, bool lazy);
void xwl_finish(struct strg_xwayland *xwl);

void xwl_set_seat(struct strg_xwayland *xwl, struct wlr_seat *seat);

#endif // STRG_XWL_H
