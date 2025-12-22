#include <assert.h>
#include <stdlib.h>
#include <strg/xdg_shell.h>
#include <wlr/util/edges.h>
#include <wlr/util/log.h>

#include "strg/decorations.h"
#include "strg/input.h"
#include "strg/window_function.h"

void xdg_decoration_surface_commit_handler(struct wl_listener *listener, void *data) {
	(void)data;
	struct strg_xdg_decoration *dec =
		wl_container_of(listener, dec, surface_commit);

	struct wlr_xdg_surface *xdg_surface =
		dec->decoration->toplevel->base;

	if (!xdg_surface->initialized) {
		return;
	}

	struct strg_toplevel *toplevel = xdg_surface->data;

	if (dec->decoration->current.mode == WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE) {
		toplevel->type = STRG_DECORATION_SERVER;
		create_decorations(toplevel);
	} else {
		toplevel->type = STRG_DECORATION_CLIENT;
	}
}

static void xdg_decoration_destroy_handler(struct wl_listener *listener, void *data) {
	(void)data;
	struct strg_xdg_decoration *dec = wl_container_of(listener, dec, destroy);

	if (!wl_list_empty(&dec->request_mode.link)) {
		wl_list_remove(&dec->request_mode.link);
	}
	if (!wl_list_empty(&dec->destroy.link)) {
		wl_list_remove(&dec->destroy.link);
	}

	free(dec);
}

void xdg_toplevel_map(struct wl_listener *listener, void *data) {
	(void)data;
	struct strg_toplevel *toplevel =
		wl_container_of(listener, toplevel, map);

	wl_list_insert(&toplevel->server->toplevels, &toplevel->link);

	wlr_scene_node_set_position(
		&toplevel->scene_tree->node, 500, 500
	);

	if (!toplevel->has_xdg_decoration) {
		toplevel->type = STRG_DECORATION_CLIENT;
		goto done;
	}

	if (toplevel->decoration_pref == STRG_DECORATION_PREF_CLIENT) {
		toplevel->type = STRG_DECORATION_CLIENT;
		goto done;
	}

	toplevel->type = STRG_DECORATION_SERVER;
	create_decorations(toplevel);
	toplevel->decorations_applied = true;

	done:
		focus_toplevel(toplevel);
}


void xdg_toplevel_unmap(struct wl_listener *listener, void *data) {
	(void)data;
	struct strg_toplevel *toplevel = wl_container_of(listener, toplevel, unmap);

	if (toplevel == toplevel->server->grabbed_toplevel) {
		reset_cursor_mode(toplevel->server);
	}

	wl_list_remove(&toplevel->link);
}

void xdg_toplevel_commit(struct wl_listener *listener, void *data) {
	(void)data;
	struct strg_toplevel *toplevel = wl_container_of(listener, toplevel, commit);

	if (toplevel->xdg_toplevel->base->initial_commit) {
		wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, 0, 0);
	}

	if (toplevel->pending_demax_restore) {
		wlr_scene_node_set_position(&toplevel->scene_tree->node,
			toplevel->pre_maximize_geometry.x,
			toplevel->pre_maximize_geometry.y);
		wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel,
			toplevel->pre_maximize_geometry.width,
			toplevel->pre_maximize_geometry.height);
		toplevel->pending_demax_restore = false;
	}

	if (toplevel->type == STRG_DECORATION_SERVER && toplevel->decorations_applied) {
		update_title(toplevel);
		update_decoration_geometry(toplevel);
	}
}

void xdg_toplevel_destroy(struct wl_listener *listener, void *data) {
	(void)data;
	struct strg_toplevel *toplevel = wl_container_of(listener, toplevel, destroy);

	destroy_decorations(toplevel);

	wl_list_remove(&toplevel->map.link);
	wl_list_remove(&toplevel->unmap.link);
	wl_list_remove(&toplevel->commit.link);
	wl_list_remove(&toplevel->destroy.link);
	wl_list_remove(&toplevel->request_move.link);
	wl_list_remove(&toplevel->request_resize.link);
	wl_list_remove(&toplevel->request_maximize.link);
	wl_list_remove(&toplevel->request_fullscreen.link);
	wl_list_remove(&toplevel->configure.link);

	free(toplevel);
}

void xdg_decoration_request_mode_handler(struct wl_listener *listener, void *data) {
	(void)data;
	struct strg_xdg_decoration *dec =
		wl_container_of(listener, dec, request_mode);

	struct strg_toplevel *toplevel = dec->toplevel;
	struct wlr_xdg_toplevel_decoration_v1 *wlr_dec = dec->decoration;

	if (wlr_dec->requested_mode ==
		WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE) {
		toplevel->decoration_pref = STRG_DECORATION_PREF_CLIENT;
		} else {
			toplevel->decoration_pref = STRG_DECORATION_PREF_SERVER;
		}

	if (!wlr_dec->toplevel->base->initialized) {
		return;
	}

	wlr_xdg_toplevel_decoration_v1_set_mode(
		wlr_dec,
		wlr_dec->requested_mode
	);
}

void xdg_new_decoration_handler(struct wl_listener *listener, void *data) {
	(void)listener;
	struct wlr_xdg_toplevel_decoration_v1 *decoration = data;

	struct strg_toplevel *toplevel =
		decoration->toplevel->base->data;

	struct strg_xdg_decoration *dec = calloc(1, sizeof(*dec));
	dec->decoration = decoration;
	dec->toplevel = toplevel;

	toplevel->has_xdg_decoration = true;

	dec->request_mode.notify = xdg_decoration_request_mode_handler;
	wl_signal_add(&decoration->events.request_mode, &dec->request_mode);

	dec->destroy.notify = xdg_decoration_destroy_handler;
	wl_signal_add(&decoration->events.destroy, &dec->destroy);
}

void begin_interactive(struct strg_toplevel *toplevel, enum strg_cursor_mode mode, uint32_t edges) {
	struct strg_server *server = toplevel->server;

	server->grabbed_toplevel = toplevel;
	server->cursor_mode = mode;

	if (mode == STRG_CURSOR_MOVE) {
		server->grab_x = server->cursor->x - toplevel->scene_tree->node.x;
		server->grab_y = server->cursor->y - toplevel->scene_tree->node.y;
	} else {
		struct wlr_box *geo_box = &toplevel->xdg_toplevel->base->geometry;

		double border_x = (toplevel->scene_tree->node.x + geo_box->x) +
			((edges & WLR_EDGE_RIGHT) ? geo_box->width : 0);
		double border_y = (toplevel->scene_tree->node.y + geo_box->y) +
			((edges & WLR_EDGE_BOTTOM) ? geo_box->height : 0);
		server->grab_x = server->cursor->x - border_x;
		server->grab_y = server->cursor->y - border_y;

		server->grab_geobox = *geo_box;
		server->grab_geobox.x += toplevel->scene_tree->node.x;
		server->grab_geobox.y += toplevel->scene_tree->node.y;

		server->resize_edges = edges;
	}
}

void xdg_toplevel_request_move(struct wl_listener *listener, void *data) {
	(void)data;
	struct strg_toplevel *toplevel = wl_container_of(listener, toplevel, request_move);

	window_move(toplevel);
}

void xdg_toplevel_request_resize(struct wl_listener *listener, void *data) {
	struct wlr_xdg_toplevel_resize_event *event = data;
	struct strg_toplevel *toplevel = wl_container_of(listener, toplevel, request_resize);
	begin_interactive(toplevel, STRG_CURSOR_RESIZE, event->edges);
}

void xdg_toplevel_request_maximize(struct wl_listener *listener, void *data) {
	(void)data;

	struct strg_toplevel *toplevel =
		wl_container_of(listener, toplevel, request_maximize);

	window_maximize(toplevel);
}

void xdg_toplevel_request_fullscreen(struct wl_listener *listener, void *data) {
	(void)data;
	/* Just as with request_maximize, we must send a configure here. */
	struct strg_toplevel *toplevel =
		wl_container_of(listener, toplevel, request_fullscreen);
	if (toplevel->xdg_toplevel->base->initialized) {
		wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
	}
}

void xdg_configure_configure(struct wl_listener *listener, void *data) {
	(void)data;
	struct strg_toplevel *toplevel =
		wl_container_of(listener, toplevel, configure);

	if (toplevel->type == STRG_DECORATION_SERVER && toplevel->decorations_applied) {
		update_decoration_geometry(toplevel);
	}
}

void xdg_toplevel_create(struct wl_listener *listener, void *data) {
	/* This event is raised when a client creates a new toplevel (application window). */
	struct strg_server *server = wl_container_of(listener, server, new_xdg_toplevel);
	struct wlr_xdg_toplevel *xdg_toplevel = data;

	/* Allocate a strg_toplevel for this surface */
	struct strg_toplevel *toplevel = calloc(1, sizeof(*toplevel));
	toplevel->server = server;
	toplevel->xdg_toplevel = xdg_toplevel;
	toplevel->scene_tree =
		wlr_scene_xdg_surface_create(&toplevel->server->scene->tree, xdg_toplevel->base);
	toplevel->scene_tree->node.data = toplevel;
	xdg_toplevel->base->data = toplevel->scene_tree;

	/* Listen to the various events it can emit */
	toplevel->map.notify = xdg_toplevel_map;
	wl_signal_add(&xdg_toplevel->base->surface->events.map, &toplevel->map);
	toplevel->unmap.notify = xdg_toplevel_unmap;
	wl_signal_add(&xdg_toplevel->base->surface->events.unmap, &toplevel->unmap);
	toplevel->commit.notify = xdg_toplevel_commit;
	wl_signal_add(&xdg_toplevel->base->surface->events.commit, &toplevel->commit);
	toplevel->destroy.notify = xdg_toplevel_destroy;
	wl_signal_add(&xdg_toplevel->events.destroy, &toplevel->destroy);
	toplevel->configure.notify = xdg_configure_configure;
	wl_signal_add(&xdg_toplevel->base->events.configure, &toplevel->configure);

	/* cotd */
	toplevel->request_move.notify = xdg_toplevel_request_move;
	wl_signal_add(&xdg_toplevel->events.request_move, &toplevel->request_move);
	toplevel->request_resize.notify = xdg_toplevel_request_resize;
	wl_signal_add(&xdg_toplevel->events.request_resize, &toplevel->request_resize);
	toplevel->request_maximize.notify = xdg_toplevel_request_maximize;
	wl_signal_add(&xdg_toplevel->events.request_maximize, &toplevel->request_maximize);
	toplevel->request_fullscreen.notify = xdg_toplevel_request_fullscreen;
	wl_signal_add(&xdg_toplevel->events.request_fullscreen, &toplevel->request_fullscreen);

	xdg_toplevel->base->data = toplevel;
	toplevel->has_xdg_decoration = false;
	toplevel->decoration_pref = STRG_DECORATION_PREF_UNKNOWN;
	toplevel->decorations_applied = false;

	toplevel->title_text = NULL;
	toplevel->close_button_buffer = NULL;
	toplevel->maximize_button_buffer = NULL;
	toplevel->minimize_button_buffer = NULL;
	toplevel->titlebar = NULL;
	toplevel->border_left = NULL;
	toplevel->border_right = NULL;
	toplevel->border_bottom = NULL;

	toplevel->is_maximized = false;
	toplevel->pre_maximize_geometry = (struct wlr_box){0};
	toplevel->internal_maximized_geometry = (struct wlr_box){0};

	toplevel->use_internal_maximize_geometry = false; // we arent sure if we use CSD or SSD at this point
	toplevel->pending_demax_restore = false;
}

void xdg_popup_commit(struct wl_listener *listener, void *data) {
	(void)data;
	/* Called when a new surface state is committed. */
	struct strg_popup *popup = wl_container_of(listener, popup, commit);

	if (popup->xdg_popup->base->initial_commit) {
		wlr_xdg_surface_schedule_configure(popup->xdg_popup->base);
	}
}

void xdg_popup_destroy(struct wl_listener *listener, void *data) {
	(void)data;
	/* Called when the xdg_popup is destroyed. */
	struct strg_popup *popup = wl_container_of(listener, popup, destroy);

	wl_list_remove(&popup->commit.link);
	wl_list_remove(&popup->destroy.link);

	free(popup);
}

void xdg_popup_create(struct wl_listener *listener, void *data) {
	(void)listener;
	struct wlr_xdg_popup *xdg_popup = data;

	struct strg_popup *popup = calloc(1, sizeof(*popup));
	popup->xdg_popup = xdg_popup;

	struct wlr_xdg_surface *parent =
		wlr_xdg_surface_try_from_wlr_surface(xdg_popup->parent);
	assert(parent != NULL);

	if (!parent->data) {
		wlr_log(WLR_ERROR, "xdg_popup_create: parent->data is null\n");
		free(popup);
		return;
	}

	struct wlr_scene_tree *parent_tree = ((struct strg_toplevel *)parent->data)->scene_tree;
	xdg_popup->base->data = wlr_scene_xdg_surface_create(parent_tree, xdg_popup->base);

	popup->commit.notify = xdg_popup_commit;
	wl_signal_add(&xdg_popup->base->surface->events.commit, &popup->commit);

	popup->destroy.notify = xdg_popup_destroy;
	wl_signal_add(&xdg_popup->events.destroy, &popup->destroy);
}