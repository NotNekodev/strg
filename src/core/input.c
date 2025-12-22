#include <stdlib.h>
#include <unistd.h>
#include <strg/input.h>
#include <strg/xdg_shell.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/util/edges.h>
#include <stdio.h>
#include <linux/input-event-codes.h>

#include "strg/decorations.h"
#include "strg/window_function.h"

void focus_toplevel(struct strg_toplevel *toplevel) {
	if (toplevel == NULL) {
		return;
	}
	struct strg_server *server = toplevel->server;
	struct wlr_seat *seat = server->seat;
	struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;
	struct wlr_surface *surface = toplevel->xdg_toplevel->base->surface;
	if (prev_surface == surface) {
		return;
	}
	if (prev_surface) {
		struct wlr_xdg_toplevel *prev_toplevel =
			wlr_xdg_toplevel_try_from_wlr_surface(prev_surface);
		if (prev_toplevel != NULL) {
			wlr_xdg_toplevel_set_activated(prev_toplevel, false);
		}
	}
	struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
	wlr_scene_node_raise_to_top(&toplevel->scene_tree->node);
	wl_list_remove(&toplevel->link);
	wl_list_insert(&server->toplevels, &toplevel->link);
	wlr_xdg_toplevel_set_activated(toplevel->xdg_toplevel, true);
	if (keyboard != NULL) {
		wlr_seat_keyboard_notify_enter(seat, surface,
			keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
	}
}

void keyboard_handle_modifiers(struct wl_listener *listener, void *data) {
	(void)data;
	struct strg_keyboard *keyboard = wl_container_of(listener, keyboard, modifiers);
	wlr_seat_set_keyboard(keyboard->server->seat, keyboard->wlr_keyboard);
	wlr_seat_keyboard_notify_modifiers(keyboard->server->seat,
		&keyboard->wlr_keyboard->modifiers);
}

bool handle_keybinding(struct strg_server *server, xkb_keysym_t sym) {
	switch (sym) {
	case XKB_KEY_Escape:
		wl_display_terminate(server->wl_display);
		break;
	case XKB_KEY_F1:
		if (wl_list_length(&server->toplevels) < 2) {
			break;
		}
		struct strg_toplevel *next_toplevel = wl_container_of(server->toplevels.prev, next_toplevel, link);
		focus_toplevel(next_toplevel);
		break;
	case XKB_KEY_F2:
		pid_t pid = fork();

		if (pid < 0) {
			perror("fork");
			break;
		} else if (pid == 0) {
			execl("/bin/weston-terminal", "weston-terminal", NULL);
			perror("execl");
			break;
		}
		break;
	case XKB_KEY_F9:
		wl_display_terminate(server->wl_display);
		break;
	default:
		return false;
	}
	return true;
}

void keyboard_handle_key(struct wl_listener *listener, void *data) {
	/* This event is raised when a key is pressed or released. */
	struct strg_keyboard *keyboard =
		wl_container_of(listener, keyboard, key);
	struct strg_server *server = keyboard->server;
	struct wlr_keyboard_key_event *event = data;
	struct wlr_seat *seat = server->seat;

	uint32_t keycode = event->keycode + 8;
	const xkb_keysym_t *syms;
	int nsyms = xkb_state_key_get_syms(
			keyboard->wlr_keyboard->xkb_state, keycode, &syms);

	bool handled = false;
	uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);
	if ((modifiers & WLR_MODIFIER_ALT) &&
			event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		for (int i = 0; i < nsyms; i++) {
			handled = handle_keybinding(server, syms[i]);
		}
	}

	if (!handled) {
		wlr_seat_set_keyboard(seat, keyboard->wlr_keyboard);
		wlr_seat_keyboard_notify_key(seat, event->time_msec,
			event->keycode, event->state);
	}
}

void keyboard_handle_destroy(struct wl_listener *listener, void *data) {
	struct strg_keyboard *keyboard = wl_container_of(listener, keyboard, destroy);
	wl_list_remove(&keyboard->modifiers.link);
	wl_list_remove(&keyboard->key.link);
	wl_list_remove(&keyboard->destroy.link);
	wl_list_remove(&keyboard->link);
	free(keyboard);
}

void server_new_keyboard(struct strg_server *server, struct wlr_input_device *device, const char* keyboard_layout) {
	struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);

	struct strg_keyboard *keyboard = calloc(1, sizeof(*keyboard));
	keyboard->server = server;
	keyboard->wlr_keyboard = wlr_keyboard;

	struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	struct xkb_keymap *keymap = xkb_keymap_new_from_names(context,
		&(struct xkb_rule_names){
			.rules = "evdev",
			.model = "pc105",
			.layout = keyboard_layout,
			.variant = NULL,
			.options = "grp:alt_shift_toggle"
		},
		XKB_KEYMAP_COMPILE_NO_FLAGS);

	wlr_keyboard_set_keymap(wlr_keyboard, keymap);
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

	keyboard->modifiers.notify = keyboard_handle_modifiers;
	wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->modifiers);
	keyboard->key.notify = keyboard_handle_key;
	wl_signal_add(&wlr_keyboard->events.key, &keyboard->key);
	keyboard->destroy.notify = keyboard_handle_destroy;
	wl_signal_add(&device->events.destroy, &keyboard->destroy);

	wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);

	wl_list_insert(&server->keyboards, &keyboard->link);
}

void server_new_pointer(struct strg_server *server, struct wlr_input_device *device) {
	/*TODO: for config add options for accerlation, sensitivity etc*/
	wlr_cursor_attach_input_device(server->cursor, device);
}

void server_new_input(struct wl_listener *listener, void *data) {
	struct strg_server *server = wl_container_of(listener, server, new_input);
	struct wlr_input_device *device = data;
	switch (device->type) {
		case WLR_INPUT_DEVICE_KEYBOARD:
		server_new_keyboard(server, device, server->kb_layout);
		break;
	case WLR_INPUT_DEVICE_POINTER:
		server_new_pointer(server, device);
		break;
	default:
		break;
	}
	uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
	if (!wl_list_empty(&server->keyboards)) {
		caps |= WL_SEAT_CAPABILITY_KEYBOARD;
	}
	wlr_seat_set_capabilities(server->seat, caps);
}

void seat_request_cursor(struct wl_listener *listener, void *data) {
	struct strg_server *server = wl_container_of(
			listener, server, request_cursor);
	struct wlr_seat_pointer_request_set_cursor_event *event = data;
	struct wlr_seat_client *focused_client =
		server->seat->pointer_state.focused_client;
	if (focused_client == event->seat_client) {
		wlr_cursor_set_surface(server->cursor, event->surface,
				event->hotspot_x, event->hotspot_y);
	}
}

void seat_pointer_focus_change(struct wl_listener *listener, void *data) {
	struct strg_server *server = wl_container_of(
			listener, server, pointer_focus_change);
	struct wlr_seat_pointer_focus_change_event *event = data;
	if (event->new_surface == NULL) {
		wlr_cursor_set_xcursor(server->cursor, server->cursor_mgr, "default");
	}
}

void seat_request_set_selection(struct wl_listener *listener, void *data) {
	struct strg_server *server = wl_container_of(
			listener, server, request_set_selection);
	struct wlr_seat_request_set_selection_event *event = data;
	wlr_seat_set_selection(server->seat, event->source, event->serial);
}

struct strg_toplevel *desktop_toplevel_at(struct strg_server *server, double lx, double ly, struct wlr_surface **surface, double *sx, double *sy) {
	struct wlr_scene_node *node = wlr_scene_node_at(
		&server->scene->tree.node, lx, ly, sx, sy);
	if (node == NULL || node->type != WLR_SCENE_NODE_BUFFER) {
		return NULL;
	}
	struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
	struct wlr_scene_surface *scene_surface =
		wlr_scene_surface_try_from_buffer(scene_buffer);
	if (!scene_surface) {
		return NULL;
	}

	*surface = scene_surface->surface;
	struct wlr_scene_tree *tree = node->parent;
	while (tree != NULL && tree->node.data == NULL) {
		tree = tree->node.parent;
	}
	return tree->node.data;
}

void reset_cursor_mode(struct strg_server *server) {
	server->cursor_mode = STRG_CURSOR_PASSTHROUGH;
	server->grabbed_toplevel = NULL;
}

void process_cursor_move(struct strg_server *server) {
	struct strg_toplevel *toplevel = server->grabbed_toplevel;
	wlr_scene_node_set_position(&toplevel->scene_tree->node,
		server->cursor->x - server->grab_x,
		server->cursor->y - server->grab_y);
}

void process_cursor_resize(struct strg_server *server) {
	struct strg_toplevel *toplevel = server->grabbed_toplevel;
	double border_x = server->cursor->x - server->grab_x;
	double border_y = server->cursor->y - server->grab_y;
	int new_left = server->grab_geobox.x;
	int new_right = server->grab_geobox.x + server->grab_geobox.width;
	int new_top = server->grab_geobox.y;
	int new_bottom = server->grab_geobox.y + server->grab_geobox.height;

	if (server->resize_edges & WLR_EDGE_TOP) {
		new_top = border_y;
		if (new_top >= new_bottom) {
			new_top = new_bottom - 1;
		}
	} else if (server->resize_edges & WLR_EDGE_BOTTOM) {
		new_bottom = border_y;
		if (new_bottom <= new_top) {
			new_bottom = new_top + 1;
		}
	}
	if (server->resize_edges & WLR_EDGE_LEFT) {
		new_left = border_x;
		if (new_left >= new_right) {
			new_left = new_right - 1;
		}
	} else if (server->resize_edges & WLR_EDGE_RIGHT) {
		new_right = border_x;
		if (new_right <= new_left) {
			new_right = new_left + 1;
		}
	}

	struct wlr_box *geo_box = &toplevel->xdg_toplevel->base->geometry;
	wlr_scene_node_set_position(&toplevel->scene_tree->node,
		new_left - geo_box->x, new_top - geo_box->y);

	int new_width = new_right - new_left;
	int new_height = new_bottom - new_top;
	wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, new_width, new_height);
}

void process_cursor_motion(struct strg_server *server, uint32_t time) {
	if (server->cursor_mode == STRG_CURSOR_MOVE) {
		process_cursor_move(server);
		return;
	} else if (server->cursor_mode == STRG_CURSOR_RESIZE) {
		process_cursor_resize(server);
		return;
	}

	double sx, sy;
	struct wlr_seat *seat = server->seat;
	struct wlr_surface *surface = NULL;
	struct strg_toplevel *toplevel = desktop_toplevel_at(server,
			server->cursor->x, server->cursor->y, &surface, &sx, &sy);
	if (!toplevel) {
		wlr_cursor_set_xcursor(server->cursor, server->cursor_mgr, "default");
	}
	if (surface) {
		wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
		wlr_seat_pointer_notify_motion(seat, time, sx, sy);
	} else {
		wlr_seat_pointer_clear_focus(seat);
	}
}

void server_cursor_motion(struct wl_listener *listener, void *data) {
	struct strg_server *server =
		wl_container_of(listener, server, cursor_motion);
	struct wlr_pointer_motion_event *event = data;
	wlr_cursor_move(server->cursor, &event->pointer->base,
			event->delta_x, event->delta_y);
	process_cursor_motion(server, event->time_msec);
}

void server_cursor_motion_absolute(
		struct wl_listener *listener, void *data) {
	struct strg_server *server =
		wl_container_of(listener, server, cursor_motion_absolute);
	struct wlr_pointer_motion_absolute_event *event = data;
	wlr_cursor_warp_absolute(server->cursor, &event->pointer->base, event->x,
		event->y);
	process_cursor_motion(server, event->time_msec);
}

void server_cursor_button(struct wl_listener *listener, void *data) {
    struct strg_server *server = wl_container_of(listener, server, cursor_button);
    struct wlr_pointer_button_event *event = data;

    double sx, sy;
    struct wlr_surface *surface = NULL;
    struct strg_toplevel *toplevel = desktop_toplevel_at(server,
                                                          server->cursor->x,
                                                          server->cursor->y,
                                                          &surface, &sx, &sy);

    if (event->state == WL_POINTER_BUTTON_STATE_PRESSED && event->button == BTN_LEFT) {
        if (!toplevel) {
            struct strg_toplevel *tl;
            wl_list_for_each(tl, &server->toplevels, link) {
                if (tl->type == STRG_DECORATION_SERVER) {
                    double local_x = server->cursor->x - tl->scene_tree->node.x;
                    double local_y = server->cursor->y - tl->scene_tree->node.y;

                    struct wlr_box geometry = tl->xdg_toplevel->base->current.geometry;
                    if (geometry.width == 0) {
                        geometry.width = tl->xdg_toplevel->base->surface->current.width;
                    }
                    if (geometry.height == 0) {
                        geometry.height = tl->xdg_toplevel->base->surface->current.height;
                    }

                    // Check if click is within decoration area
                    if (local_x >= -BORDER_WIDTH &&
                        local_x < geometry.width + BORDER_WIDTH &&
                        local_y >= -TITLEBAR_HEIGHT &&
                        local_y < geometry.height + BORDER_WIDTH) {
                        toplevel = tl;
                        break;
                    }
                }
            }
        }

        if (toplevel && toplevel->type == STRG_DECORATION_SERVER) {
            if (is_click_on_close_button(toplevel, server->cursor->x, server->cursor->y)) {
                wlr_xdg_toplevel_send_close(toplevel->xdg_toplevel);
                return;
            }

            if (is_click_on_maximize_button(toplevel, server->cursor->x, server->cursor->y)) {
            	window_maximize(toplevel);
                return;
            }

            if (is_click_on_minimize_button(toplevel, server->cursor->x, server->cursor->y)) {
                wlr_xdg_toplevel_set_activated(toplevel->xdg_toplevel, false);
                return;
            }

            if (is_click_on_titlebar(toplevel, server->cursor->x, server->cursor->y)) {
                focus_toplevel(toplevel);
            	window_move(toplevel);
                return;
            }
        }

        if (toplevel) {
            focus_toplevel(toplevel);
        }
    }

    uint32_t modifiers = wlr_keyboard_get_modifiers(server->seat->keyboard_state.keyboard);
    if (event->state == WL_POINTER_BUTTON_STATE_PRESSED &&
        event->button == BTN_LEFT &&
        (modifiers & WLR_MODIFIER_ALT) &&
        toplevel) {
        focus_toplevel(toplevel);
    	window_move(toplevel);
        return;
    }

    wlr_seat_pointer_notify_button(server->seat,
                                   event->time_msec,
                                   event->button,
                                   event->state);

    if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
        reset_cursor_mode(server);
    }
}


void server_cursor_axis(struct wl_listener *listener, void *data) {
	struct strg_server *server =
		wl_container_of(listener, server, cursor_axis);
	struct wlr_pointer_axis_event *event = data;
	/* Notify the client with pointer focus of the axis event. */
	wlr_seat_pointer_notify_axis(server->seat,
			event->time_msec, event->orientation, event->delta,
			event->delta_discrete, event->source, event->relative_direction);
}

void server_cursor_frame(struct wl_listener *listener, void *data) {
	(void)data;
	struct strg_server *server =
		wl_container_of(listener, server, cursor_frame);
	/* Notify the client with pointer focus of the frame event. */
	wlr_seat_pointer_notify_frame(server->seat);
}