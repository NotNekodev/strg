#include "strg/core/protocol/xwayland/xwl.h"
#include "strg/core/protocol/xwayland/xwl_ops.h"

#include <stdlib.h>

#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/xwayland.h>
#include <wlr/util/log.h>

#include "strg/strg.h"

static void xwl_surface_handle_associate(struct wl_listener *listener, void *data) {
    struct strg_xwayland_surface *surface =
        wl_container_of(listener, surface, associate);

    struct wlr_surface *wlr_surface =
        surface->xwayland_surface->surface;

    if (!wlr_surface) {
        wlr_log(WLR_ERROR, "XWayland associate without wlr_surface");
        return;
    }

    surface->scene_tree = wlr_scene_tree_create(&surface->xwl->server->scene->tree);
    surface->scene_surface =
        wlr_scene_surface_create(surface->scene_tree, wlr_surface);

    surface->scene_tree->node.data = surface;
    wlr_scene_node_set_position(&surface->scene_tree->node, 500, 500);

    surface->mapped = true;
}

static void xwl_surface_handle_dissociate(struct wl_listener *listener, void *data) {
    struct strg_xwayland_surface *surface =
        wl_container_of(listener, surface, dissociate);

    surface->mapped = false;

    if (surface->scene_tree) {
        wlr_scene_node_destroy(&surface->scene_tree->node);
        surface->scene_tree = NULL;
        surface->scene_surface = NULL;
    }
}

static void xwl_surface_handle_destroy(struct wl_listener *listener, void *data) {
    struct strg_xwayland_surface *surface =
        wl_container_of(listener, surface, destroy);

    wl_list_remove(&surface->associate.link);
    wl_list_remove(&surface->dissociate.link);
    wl_list_remove(&surface->destroy.link);
    wl_list_remove(&surface->request_configure.link);
    wl_list_remove(&surface->request_fullscreen.link);
    wl_list_remove(&surface->request_minimize.link);
    wl_list_remove(&surface->request_maximize.link);
    wl_list_remove(&surface->request_move.link);
    wl_list_remove(&surface->request_resize.link);
    wl_list_remove(&surface->request_activate.link);
    wl_list_remove(&surface->set_title.link);
    wl_list_remove(&surface->set_class.link);

    free(surface);
}

static void xwl_surface_handle_request_configure(
    struct wl_listener *listener, void *data)
{
    struct strg_xwayland_surface *surface =
        wl_container_of(listener, surface, request_configure);

    struct wlr_xwayland_surface_configure_event *event = data;

    xwl_surface_configure(
        surface,
        event->x,
        event->y,
        event->width,
        event->height
    );
}

static void xwl_surface_handle_request_fullscreen(
    struct wl_listener *listener, void *data)
{
    struct strg_xwayland_surface *surface =
        wl_container_of(listener, surface, request_fullscreen);

    xwl_surface_set_fullscreen(
        surface,
        surface->xwayland_surface->fullscreen
    );
}

static void xwl_surface_handle_request_minimize(
    struct wl_listener *listener, void *data)
{
    struct strg_xwayland_surface *surface =
        wl_container_of(listener, surface, request_minimize);
}

static void xwl_surface_handle_request_move(
    struct wl_listener *listener, void *data)
{
    struct strg_xwayland_surface *surface =
        wl_container_of(listener, surface, request_resize);

    xwl_surface_move(surface);
}

static void xwl_surface_handle_request_resize(
    struct wl_listener *listener, void *data)
{
    struct strg_xwayland_surface *surface =
        wl_container_of(listener, surface, request_resize);

    struct wlr_xwayland_resize_event *event = data;
}

static void xwl_surface_handle_request_activate(
    struct wl_listener *listener, void *data)
{
    struct strg_xwayland_surface *surface =
        wl_container_of(listener, surface, request_activate);

    xwl_surface_set_activated(surface, true);
}

static void xwl_surface_handle_set_title(
    struct wl_listener *listener, void *data)
{
    struct strg_xwayland_surface *surface =
        wl_container_of(listener, surface, set_title);

}

static void xwl_surface_handle_set_class(
    struct wl_listener *listener, void *data)
{
    struct strg_xwayland_surface *surface =
        wl_container_of(listener, surface, set_class);

    wlr_log(
        WLR_DEBUG,
        "XWayland class: %s",
        surface->xwayland_surface->class
    );
}

static void xwl_surface_handle_request_maximize(struct wl_listener *listener, void *data) {
    struct strg_xwayland_surface *surface =
        wl_container_of(listener, surface, request_maximize);

    struct wlr_xwayland_maximize_event *event = data;

    xwl_surface_set_maximized(surface);
}

static void xwl_handle_new_surface(struct wl_listener *listener, void *data) {
    struct strg_xwayland *xwl =
        wl_container_of(listener, xwl, new_surface);

    struct wlr_xwayland_surface *xsurface = data;

    struct strg_xwayland_surface *surface =
        calloc(1, sizeof(*surface));

    if (!surface) {
        return;
    }

    surface->mapped = false;

    surface->xwl = xwl;
    surface->xwayland_surface = xsurface;

    surface->associate.notify = xwl_surface_handle_associate;
    wl_signal_add(&xsurface->events.associate, &surface->associate);

    surface->dissociate.notify = xwl_surface_handle_dissociate;
    wl_signal_add(&xsurface->events.dissociate, &surface->dissociate);

    surface->destroy.notify = xwl_surface_handle_destroy;
    wl_signal_add(&xsurface->events.destroy, &surface->destroy);

    surface->request_configure.notify = xwl_surface_handle_request_configure;
    wl_signal_add(&xsurface->events.request_configure, &surface->request_configure);

    surface->request_fullscreen.notify = xwl_surface_handle_request_fullscreen;
    wl_signal_add(&xsurface->events.request_fullscreen, &surface->request_fullscreen);

    surface->request_minimize.notify = xwl_surface_handle_request_minimize;
    wl_signal_add(&xsurface->events.request_minimize, &surface->request_minimize);

    surface->request_maximize.notify = xwl_surface_handle_request_maximize;
    wl_signal_add(&xsurface->events.request_maximize, &surface->request_maximize);

    surface->request_move.notify = xwl_surface_handle_request_move;
    wl_signal_add(&xsurface->events.request_move, &surface->request_move);

    surface->request_resize.notify = xwl_surface_handle_request_resize;
    wl_signal_add(&xsurface->events.request_resize, &surface->request_resize);

    surface->request_activate.notify = xwl_surface_handle_request_activate;
    wl_signal_add(&xsurface->events.request_activate, &surface->request_activate);

    surface->set_title.notify = xwl_surface_handle_set_title;
    wl_signal_add(&xsurface->events.set_title, &surface->set_title);

    surface->set_class.notify = xwl_surface_handle_set_class;
    wl_signal_add(&xsurface->events.set_class, &surface->set_class);

    wl_list_init(&surface->link);
}


static void xwl_handle_ready(struct wl_listener *listener, void *data) {
    struct strg_xwayland *xwl =
        wl_container_of(listener, xwl, ready);

    wlr_log(WLR_INFO, "XWayland server ready");

    wlr_xwayland_set_seat(xwl->xwayland, xwl->seat);
}

static void xwl_handle_remove(struct wl_listener *listener, void *data) {
    wlr_log(WLR_INFO, "XWayland server destroyed");
}

bool xwl_init(
    struct strg_xwayland *xwl,
    struct wl_display *display,
    struct wlr_compositor *compositor,
    bool lazy)
{
    xwl->xwayland = wlr_xwayland_create(display, compositor, lazy);
    if (!xwl->xwayland) {
        wlr_log(WLR_ERROR, "Failed to create XWayland");
        return false;
    }

    xwl->new_surface.notify = xwl_handle_new_surface;
    wl_signal_add(&xwl->xwayland->events.new_surface, &xwl->new_surface);

    xwl->ready.notify = xwl_handle_ready;
    wl_signal_add(&xwl->xwayland->events.ready, &xwl->ready);

    xwl->remove.notify = xwl_handle_remove;
    wl_signal_add(&xwl->xwayland->events.destroy, &xwl->remove);

    return true;
}

void xwl_set_seat(struct strg_xwayland *xwl, struct wlr_seat *seat) {
    xwl->seat = seat;
    if (xwl->xwayland) {
        wlr_xwayland_set_seat(xwl->xwayland, seat);
    }
}

void xwl_finish(struct strg_xwayland *xwl) {
    if (!xwl->xwayland) {
        return;
    }

    wl_list_remove(&xwl->new_surface.link);
    wl_list_remove(&xwl->ready.link);
    wl_list_remove(&xwl->remove.link);

    wlr_xwayland_destroy(xwl->xwayland);
    xwl->xwayland = NULL;
}
