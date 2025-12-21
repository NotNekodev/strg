#include <strg/decorations.h>

#include <wlr/types/wlr_xdg_shell.h>

#include "strg/xdg_shell.h"

void create_decorations(struct strg_toplevel *toplevel) {
    float titlebar_color[4] = {0.2f, 0.2f, 0.25f, 1.0f}; // RGBA
    float border_color[4] = {0.15f, 0.15f, 0.2f, 1.0f};

    struct wlr_box geometry = toplevel->xdg_toplevel->base->current.geometry;

    if (geometry.width == 0 || geometry.height == 0) {
        geometry.width = toplevel->xdg_toplevel->base->surface->current.width;
        geometry.height = toplevel->xdg_toplevel->base->surface->current.height;
    }

    toplevel->titlebar = wlr_scene_rect_create(toplevel->scene_tree,
        geometry.width + 2 * 2, 30, titlebar_color);
    wlr_scene_node_set_position(&toplevel->titlebar->node,
        -2, -30);

}
