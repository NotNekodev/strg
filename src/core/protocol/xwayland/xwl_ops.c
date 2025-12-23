#include "strg/core/protocol/xwayland/xwl_ops.h"

#include <wlr/util/edges.h>
#include <wlr/util/log.h>

#include "strg/core/protocol/xdg/xdg.h"
#include "strg/style/decorations.h"
#include "strg/util/window_util.h"

void xwl_surface_configure(struct strg_xwayland_surface *surface,
                           int16_t x, int16_t y, uint16_t width, uint16_t height) {
    if (!surface->mapped) {
        return;
    }
    if (surface->scene_tree == NULL) {
        wlr_log(WLR_ERROR, "Attempted to configure XWayland surface without scene_tree");
        return;
    }

    wlr_scene_node_set_position(&surface->scene_tree->node, x, y);

    wlr_xwayland_surface_configure(surface->xwayland_surface, x, y, width, height);
}

void xwl_surface_close(struct strg_xwayland_surface *surface) {
    wlr_xwayland_surface_close(surface->xwayland_surface);
}

void xwl_surface_set_maximized(struct strg_xwayland_surface *surface) {
    struct wlr_output *output = get_dominant_output(&(struct strg_window){ .type = STRG_WINDOW_XWAYLAND, .window = surface });
    if (!output) {
        return;
    }

    if (!surface->mapped) {
        return;
    }

    if (surface->is_maximized) {
        wlr_xwayland_surface_set_maximized(surface->xwayland_surface, true, true);

        xwl_surface_configure(surface,
            surface->pre_maximize_geometry.x,
            surface->pre_maximize_geometry.y,
            surface->pre_maximize_geometry.width,
            surface->pre_maximize_geometry.height);
        surface->is_maximized = false;
    } else {
        struct wlr_box current_geo = {
            .x = surface->xwayland_surface->x,
            .y = surface->xwayland_surface->y,
            .width = surface->xwayland_surface->width,
            .height = surface->xwayland_surface->height
        };
        if (current_geo.width == 0 || current_geo.height == 0) {
            current_geo.width = surface->xwayland_surface->surface->current.width;
            current_geo.height = surface->xwayland_surface->surface->current.height;
        }

        surface->pre_maximize_geometry.x = surface->scene_tree->node.x;
        surface->pre_maximize_geometry.y = surface->scene_tree->node.y;
        surface->pre_maximize_geometry.width = current_geo.width;
        surface->pre_maximize_geometry.height = current_geo.height;

        struct wlr_box output_box;
        wlr_output_layout_get_box(surface->xwl->server->output_layout, output, &output_box);

        struct wlr_box target_box;
        /*if (toplevel->type == STRG_DECORATION_SERVER && toplevel->decorations_applied) {
            target_box = (struct wlr_box){
                .x = output_box.x,
                .y = output_box.y + TITLEBAR_HEIGHT,
                .width = output_box.width - 2 * BORDER_WIDTH,
                .height = output_box.height - TITLEBAR_HEIGHT - BORDER_WIDTH,
            };
        } else {

        }*/
        target_box = output_box;

        wlr_xwayland_surface_set_maximized(surface->xwayland_surface, true, true);
        xwl_surface_configure(surface,
            target_box.x,
            target_box.y,
            target_box.width,
            target_box.height);
        surface->is_maximized = true;
    }
}

void xwl_surface_set_fullscreen(struct strg_xwayland_surface *surface, bool fullscreen) {
    wlr_xwayland_surface_set_fullscreen(surface->xwayland_surface, fullscreen);
}

void xwl_surface_move(struct strg_xwayland_surface *surface) {
    if (!surface->mapped) {
        return;
    }

    if (surface->is_maximized) {
        const double cursor_x = surface->xwl->server->cursor->x;
        const double cursor_y = surface->xwl->server->cursor->y;

        struct wlr_box current_geo = {
            .x = surface->xwayland_surface->x,
            .y = surface->xwayland_surface->y,
            .width = surface->xwayland_surface->surface->current.width,
            .height = surface->xwayland_surface->surface->current.height
        };

        if (current_geo.width == 0 || current_geo.height == 0) {
            current_geo.width = surface->xwayland_surface->surface->current.width;
            current_geo.height = surface->xwayland_surface->surface->current.height;
        }

        const double rel_x = (cursor_x - surface->scene_tree->node.x) / (double)current_geo.width;
        double rel_y = (cursor_y - surface->scene_tree->node.y) / (double)current_geo.height;

        /*if (toplevel->type == STRG_DECORATION_SERVER && rel_y < 0) {
            rel_y = 0.5;
        }*/

        xwl_surface_set_maximized(surface);

        int new_width = surface->pre_maximize_geometry.width;
        int new_height = surface->pre_maximize_geometry.height;

        int new_x = cursor_x - (int)(new_width * rel_x);
        int new_y = cursor_y - (int)(new_height * rel_y);

        /*if (toplevel->type == STRG_DECORATION_SERVER && cursor_y < toplevel->scene_tree->node.y) {
            new_y = cursor_y - TITLEBAR_HEIGHT / 2;
        }*/

        wlr_scene_node_set_position(&surface->scene_tree->node, new_x, new_y);
        surface->pre_maximize_geometry.x = new_x;
        surface->pre_maximize_geometry.y = new_y;

        surface->xwl->server->grabbed_window = malloc(sizeof(struct strg_window));
        memset(surface->xwl->server->grabbed_window, 0, sizeof(struct strg_window));
        surface->xwl->server->grabbed_window->type = STRG_WINDOW_XWAYLAND;
        surface->xwl->server->grabbed_window->window = surface;

        surface->xwl->server->cursor_mode = STRG_CURSOR_MOVE;
        surface->xwl->server->grab_x = cursor_x - new_x;
        surface->xwl->server->grab_y = cursor_y - new_y;

        return;
    }

    xwl_begin_interactive(surface, STRG_CURSOR_MOVE, 0);
}

void xwl_surface_set_minimized(struct strg_xwayland_surface *surface, bool minimized) {
    wlr_xwayland_surface_set_minimized(surface->xwayland_surface, minimized);
}

void xwl_surface_set_activated(struct strg_xwayland_surface *surface, bool activated) {
    wlr_xwayland_surface_activate(surface->xwayland_surface, activated);
    wlr_xwayland_surface_restack(surface->xwayland_surface, NULL, XCB_STACK_MODE_ABOVE);
}

void xwl_begin_interactive(struct strg_xwayland_surface *window, enum strg_cursor_mode mode, uint32_t edges) {
    struct strg_server *server = window->xwl->server;

    if (!window->mapped) {
        return;
    }

    server->grabbed_window = malloc(sizeof(struct strg_window));
    memset(server->grabbed_window, 0, sizeof(struct strg_window));
    server->grabbed_window->type = STRG_WINDOW_XWAYLAND;
    server->grabbed_window->window = window;

    server->cursor_mode = mode;

    if (mode == STRG_CURSOR_MOVE) {
        server->grab_x = server->cursor->x - window->scene_tree->node.x;
        server->grab_y = server->cursor->y - window->scene_tree->node.y;
    } else {
        struct wlr_box *geo_box = &(struct wlr_box){
            .x = window->xwayland_surface->x,
            .y = window->xwayland_surface->y,
            .width = window->xwayland_surface->surface->current.width,
            .height = window->xwayland_surface->surface->current.height
        };

        double border_x = (window->scene_tree->node.x + geo_box->x) +
            ((edges & WLR_EDGE_RIGHT) ? geo_box->width : 0);
        double border_y = (window->scene_tree->node.y + geo_box->y) +
            ((edges & WLR_EDGE_BOTTOM) ? geo_box->height : 0);
        server->grab_x = server->cursor->x - border_x;
        server->grab_y = server->cursor->y - border_y;

        server->grab_geobox = *geo_box;
        server->grab_geobox.x += window->scene_tree->node.x;
        server->grab_geobox.y += window->scene_tree->node.y;

        server->resize_edges = edges;
    }
}

void xwl_surface_resize(struct strg_xwayland_surface *surface, const uint32_t edges) {
    if (!surface->mapped) {
        return;
    }

    if (surface->is_maximized) {
        const double cursor_x = surface->xwl->server->cursor->x;
        const double cursor_y = surface->xwl->server->cursor->y;

        struct wlr_box current_geo = {
            .x = surface->xwayland_surface->x,
            .y = surface->xwayland_surface->y,
            .width = surface->xwayland_surface->surface->current.width,
            .height = surface->xwayland_surface->surface->current.height
        };

        if (current_geo.width == 0 || current_geo.height == 0) {
            current_geo.width = surface->xwayland_surface->surface->current.width;
            current_geo.height = surface->xwayland_surface->surface->current.height;
        }

        const double rel_x = (cursor_x - surface->scene_tree->node.x) / (double)current_geo.width;
        double rel_y = (cursor_y - surface->scene_tree->node.y) / (double)current_geo.height;

        /*if (surface->type == STRG_DECORATION_SERVER && rel_y < 0) {
            rel_y = 0.5;
        }*/

        xwl_surface_set_maximized(surface);

        int new_width = surface->pre_maximize_geometry.width;
        int new_height = surface->pre_maximize_geometry.height;

        int new_x = cursor_x - (int)(new_width * rel_x);
        int new_y = cursor_y - (int)(new_height * rel_y);

        /*if (surface->type == STRG_DECORATION_SERVER && cursor_y < surface->scene_tree->node.y) {
            new_y = cursor_y - TITLEBAR_HEIGHT / 2;
        }*/

        wlr_scene_node_set_position(&surface->scene_tree->node, new_x, new_y);
        surface->pre_maximize_geometry.x = new_x;
        surface->pre_maximize_geometry.y = new_y;

        surface->xwl->server->grabbed_window = malloc(sizeof(struct strg_window));
        memset(surface->xwl->server->grabbed_window, 0, sizeof(struct strg_window));
        surface->xwl->server->grabbed_window->type = STRG_WINDOW_XWAYLAND;
        surface->xwl->server->grabbed_window->window = surface;

        surface->xwl->server->cursor_mode = STRG_CURSOR_MOVE;
        surface->xwl->server->grab_x = cursor_x - new_x;
        surface->xwl->server->grab_y = cursor_y - new_y;

        return;
    }

    xwl_begin_interactive(surface, STRG_CURSOR_RESIZE, edges);
}
