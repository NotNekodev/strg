#ifndef STRG_OUTPUT_H
#define STRG_OUTPUT_H

#include <strg/strg.h>

struct strg_output {
    struct wl_list link;
    struct strg_server *server;
    struct wlr_output *wlr_output;
    struct wl_listener frame;
    struct wl_listener request_state;
    struct wl_listener destroy;
};

void output_frame(struct wl_listener *listener, void *data);
void output_request_state(struct wl_listener *listener, void *data);
void output_destroy(struct wl_listener *listener, void *data);
void server_new_output(struct wl_listener *listener, void *data);

#endif //STRG_OUTPUT_H