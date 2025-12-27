// this is kinda like main.c but as a header, thats why it isnt a a subdirectory

#ifndef STRG_STRG_H
#define STRG_STRG_H

#include <wayland-server-core.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/util/log.h>
#include <time.h>

enum strg_cursor_mode {
    STRG_CURSOR_PASSTHROUGH,
    STRG_CURSOR_MOVE,
    STRG_CURSOR_RESIZE,
};

enum strg_window_type {
    STRG_WINDOW_XDG,
    STRG_WINDOW_XWAYLAND,
};

struct strg_window {
    enum strg_window_type type;
    void *window;
};

struct strg_server {
    struct wl_display *wl_display;
    struct wl_event_loop *event_loop;
    struct strg_xwayland *xwayland;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    struct wlr_scene *scene;
    struct wlr_scene_output_layout *scene_layout;

    struct wlr_xdg_shell *xdg_shell;
    struct wl_listener new_xdg_toplevel;
    struct wl_listener new_xdg_popup;
    struct wl_list toplevels;

    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *cursor_mgr;
    struct wl_listener cursor_motion;
    struct wl_listener cursor_motion_absolute;
    struct wl_listener cursor_button;
    struct wl_listener cursor_axis;
    struct wl_listener cursor_frame;

    struct wlr_seat *seat;
    struct wl_listener new_input;
    struct wl_listener request_cursor;
    struct wl_listener pointer_focus_change;
    struct wl_listener request_set_selection;
    struct wl_list keyboards;
    enum strg_cursor_mode cursor_mode;
    struct strg_window *grabbed_window;
    double grab_x, grab_y;
    struct wlr_box grab_geobox;
    uint32_t resize_edges;

    struct wlr_output_layout *output_layout;
    struct wl_list outputs;
    struct wl_listener new_output;

    struct wlr_xdg_decoration_manager_v1 *xdg_decoration_manager;
    struct wl_listener new_xdg_decoration;

    int logfile_fd;
    struct timespec start; // time when the compositor started

    struct strg_config *config;
};

void strg_wlr_log_callback(enum wlr_log_importance importance, const char *fmt, va_list args);

#endif //STRG_STRG_H
