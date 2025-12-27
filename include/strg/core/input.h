#ifndef STRG_INPUT_H
#define STRG_INPUT_H

#include <strg/strg.h>

struct strg_keyboard {
    struct wl_list link;
    struct strg_server *server;
    struct wlr_keyboard *wlr_keyboard;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;
};

void focus_window(struct strg_window *toplevel);
void keyboard_handle_modifiers(struct wl_listener *listener, void *data);
void keyboard_handle_key(struct wl_listener *listener, void *data);
void keyboard_handle_destroy(struct wl_listener *listener, void *data);
void server_new_keyboard(struct strg_server *server, struct wlr_input_device *device);
void server_new_pointer(struct strg_server *server, struct wlr_input_device *device);
void server_new_input(struct wl_listener *listener, void *data);
void seat_request_cursor(struct wl_listener *listener, void *data);
void seat_pointer_focus_change(struct wl_listener *listener, void *data);
void seat_request_set_selection(struct wl_listener *listener, void *data);
struct strg_window *desktop_window_at(struct strg_server *server, double lx, double ly, struct wlr_surface **surface, double *sx, double *sy);
void reset_cursor_mode(struct strg_server *server); 
void process_cursor_move(struct strg_server *server);
void process_cursor_resize(struct strg_server *server);
void process_cursor_motion(struct strg_server *server, uint32_t time);
void server_cursor_motion(struct wl_listener *listener, void *data);
void server_cursor_motion_absolute(struct wl_listener *listener, void *data);
void server_cursor_button(struct wl_listener *listener, void *data);
void server_cursor_axis(struct wl_listener *listener, void *data);
void server_cursor_frame(struct wl_listener *listener, void *data);

#endif //STRG_INPUT_H
