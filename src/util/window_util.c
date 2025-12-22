#include <strg/util/window_util.h>
#include <wlr/backend.h>

#include "strg/decorations.h"

struct wlr_output *get_dominant_output(struct strg_toplevel *toplevel) {
    struct wlr_output_layout *layout = toplevel->server->output_layout;

    struct wlr_box current_geo = toplevel->xdg_toplevel->base->current.geometry;

    struct wlr_box window_box = {
        .x = toplevel->scene_tree->node.x,
        .y = toplevel->scene_tree->node.y,
        .width = current_geo.width,
        .height = current_geo.height
    };

    if (window_box.width == 0 || window_box.height == 0) {
        window_box.width = toplevel->xdg_toplevel->base->surface->current.width;
        window_box.height = toplevel->xdg_toplevel->base->surface->current.height;
    }

    window_box.x += current_geo.x;
    window_box.y += current_geo.y;

    if (toplevel->type == STRG_DECORATION_SERVER && toplevel->decorations_applied) {
        window_box.x -= BORDER_WIDTH;
        window_box.y -= TITLEBAR_HEIGHT;
        window_box.width += 2 * BORDER_WIDTH;
        window_box.height += TITLEBAR_HEIGHT + BORDER_WIDTH;
    }

    struct wlr_output *best_output = NULL;
    int max_area = 0;
    struct wlr_output *center_output = NULL;

    int window_center_x = window_box.x + window_box.width / 2;
    int window_center_y = window_box.y + window_box.height / 2;

    struct wlr_output_layout_output *layout_output;
    wl_list_for_each(layout_output, &layout->outputs, link) {
        struct wlr_output *output = layout_output->output;

        if (!output->enabled) {
            continue;
        }

        struct wlr_box output_box;
        wlr_output_layout_get_box(layout, output, &output_box);

        struct wlr_box intersection;
        bool intersects = wlr_box_intersection(&intersection, &window_box, &output_box);

        if (intersects) {
            int area = intersection.width * intersection.height;

            if (area > max_area) {
                max_area = area;
                best_output = output;
            }
        }

        if (window_center_x >= output_box.x &&
            window_center_x < output_box.x + output_box.width &&
            window_center_y >= output_box.y &&
            window_center_y < output_box.y + output_box.height) {
            center_output = output;
        }
    }

    if (best_output) {
        return best_output;
    }

    if (center_output) {
        return center_output;
    }

    struct wlr_output *cursor_output = wlr_output_layout_output_at(
        layout,
        toplevel->server->cursor->x,
        toplevel->server->cursor->y);

    if (cursor_output) {
        return cursor_output;
    }

    return wlr_output_layout_get_center_output(layout);
}
