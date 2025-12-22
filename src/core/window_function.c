#include <strg/window_function.h>

void window_maximize(struct strg_toplevel *toplevel) {
    if (toplevel->xdg_toplevel->base->initialized) {
		if (toplevel->use_internal_maximize_geometry) {
			if (toplevel->is_maximized) {
				wlr_xdg_toplevel_set_maximized(toplevel->xdg_toplevel, false);
				wlr_scene_node_set_position(&toplevel->scene_tree->node,
					toplevel->pre_maximize_geometry.x,
					toplevel->pre_maximize_geometry.y);
				wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel,
					toplevel->pre_maximize_geometry.width,
					toplevel->pre_maximize_geometry.height);
				toplevel->is_maximized = false;
			} else {
				struct wlr_output *output = wlr_output_layout_output_at(
						toplevel->server->output_layout, toplevel->server->cursor->x, toplevel->server->cursor->y);
				if (output) {
					toplevel->pre_maximize_geometry.x = toplevel->scene_tree->node.x;
					toplevel->pre_maximize_geometry.y = toplevel->scene_tree->node.y;
					toplevel->pre_maximize_geometry.width =
						toplevel->xdg_toplevel->base->current.geometry.width;
					toplevel->pre_maximize_geometry.height =
						toplevel->xdg_toplevel->base->current.geometry.height;

					wlr_xdg_toplevel_set_maximized(toplevel->xdg_toplevel, true);
					wlr_scene_node_set_position(&toplevel->scene_tree->node,
						toplevel->internal_maximized_geometry.x, toplevel->internal_maximized_geometry.y);
					wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel,
						toplevel->internal_maximized_geometry.width, toplevel->internal_maximized_geometry.height);
					toplevel->is_maximized = true;
				}
			}
		} else {
			if (toplevel->is_maximized) {
				wlr_xdg_toplevel_set_maximized(toplevel->xdg_toplevel, false);
				wlr_scene_node_set_position(&toplevel->scene_tree->node,
					toplevel->pre_maximize_geometry.x,
					toplevel->pre_maximize_geometry.y);
				wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel,
					toplevel->pre_maximize_geometry.width,
					toplevel->pre_maximize_geometry.height);
				toplevel->is_maximized = false;
			} else {
				struct wlr_output *output = wlr_output_layout_output_at(
						toplevel->server->output_layout, toplevel->server->cursor->x, toplevel->server->cursor->y);
				if (output) {
					toplevel->pre_maximize_geometry.x = toplevel->scene_tree->node.x;
					toplevel->pre_maximize_geometry.y = toplevel->scene_tree->node.y;
					toplevel->pre_maximize_geometry.width =
						toplevel->xdg_toplevel->base->current.geometry.width;
					toplevel->pre_maximize_geometry.height =
						toplevel->xdg_toplevel->base->current.geometry.height;

					struct wlr_box output_box;
					wlr_output_layout_get_box(toplevel->server->output_layout, output, &output_box);
					wlr_xdg_toplevel_set_maximized(toplevel->xdg_toplevel, true);
					wlr_scene_node_set_position(&toplevel->scene_tree->node,
						output_box.x, output_box.y);
					wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel,
						output_box.width, output_box.height);
					toplevel->is_maximized = true;
				}
			}

		}

		wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
	}
}