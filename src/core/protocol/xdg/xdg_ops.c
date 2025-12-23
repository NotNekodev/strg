#include <strg/core/protocol/xdg/xdg_ops.h>
#include <strg/style/decorations.h>
#include <strg/util/window_util.h>

void window_maximize(struct strg_toplevel *toplevel) {
    if (!toplevel->xdg_toplevel->base->initialized) {
        return;
    }

    struct wlr_output *output = get_dominant_output(toplevel);
    if (!output) {
        return;
    }

    if (toplevel->is_maximized) {
        wlr_xdg_toplevel_set_maximized(toplevel->xdg_toplevel, false);
        wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel,
            toplevel->pre_maximize_geometry.width,
            toplevel->pre_maximize_geometry.height);
        toplevel->pending_demax_restore = true;
        toplevel->is_maximized = false;
    } else {
        struct wlr_box current_geo = toplevel->xdg_toplevel->base->current.geometry;
        if (current_geo.width == 0 || current_geo.height == 0) {
            current_geo.width = toplevel->xdg_toplevel->base->surface->current.width;
            current_geo.height = toplevel->xdg_toplevel->base->surface->current.height;
        }

        toplevel->pre_maximize_geometry.x = toplevel->scene_tree->node.x;
        toplevel->pre_maximize_geometry.y = toplevel->scene_tree->node.y;
        toplevel->pre_maximize_geometry.width = current_geo.width;
        toplevel->pre_maximize_geometry.height = current_geo.height;

        struct wlr_box output_box;
        wlr_output_layout_get_box(toplevel->server->output_layout, output, &output_box);

        struct wlr_box target_box;
        if (toplevel->type == STRG_DECORATION_SERVER && toplevel->decorations_applied) {
            target_box = (struct wlr_box){
                .x = output_box.x,
                .y = output_box.y + TITLEBAR_HEIGHT,
                .width = output_box.width - 2 * BORDER_WIDTH,
                .height = output_box.height - TITLEBAR_HEIGHT - BORDER_WIDTH,
            };
        } else {
            target_box = output_box;
        }

        wlr_xdg_toplevel_set_maximized(toplevel->xdg_toplevel, true);
        wlr_scene_node_set_position(&toplevel->scene_tree->node,
            target_box.x, target_box.y);
        wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel,
            target_box.width, target_box.height);
        toplevel->is_maximized = true;
    }

    wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
}

void window_move(struct strg_toplevel *toplevel) {
    if (toplevel->is_maximized) {
        const double cursor_x = toplevel->server->cursor->x;
        const double cursor_y = toplevel->server->cursor->y;

        struct wlr_box current_geo = toplevel->xdg_toplevel->base->current.geometry;
        if (current_geo.width == 0 || current_geo.height == 0) {
            current_geo.width = toplevel->xdg_toplevel->base->surface->current.width;
            current_geo.height = toplevel->xdg_toplevel->base->surface->current.height;
        }

        const double rel_x = (cursor_x - toplevel->scene_tree->node.x) / (double)current_geo.width;
        double rel_y = (cursor_y - toplevel->scene_tree->node.y) / (double)current_geo.height;

        if (toplevel->type == STRG_DECORATION_SERVER && rel_y < 0) {
            rel_y = 0.5;
        }

        window_maximize(toplevel);

        int new_width = toplevel->pre_maximize_geometry.width;
        int new_height = toplevel->pre_maximize_geometry.height;

        int new_x = cursor_x - (int)(new_width * rel_x);
        int new_y = cursor_y - (int)(new_height * rel_y);

        if (toplevel->type == STRG_DECORATION_SERVER && cursor_y < toplevel->scene_tree->node.y) {
            new_y = cursor_y - TITLEBAR_HEIGHT / 2;
        }

        wlr_scene_node_set_position(&toplevel->scene_tree->node, new_x, new_y);
        toplevel->pre_maximize_geometry.x = new_x;
        toplevel->pre_maximize_geometry.y = new_y;

        toplevel->server->grabbed_toplevel = toplevel;
        toplevel->server->cursor_mode = STRG_CURSOR_MOVE;
        toplevel->server->grab_x = cursor_x - new_x;
        toplevel->server->grab_y = cursor_y - new_y;

        return;
    }

    begin_interactive(toplevel, STRG_CURSOR_MOVE, 0);
}
