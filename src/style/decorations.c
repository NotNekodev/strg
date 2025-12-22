#include <strg/decorations.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_buffer.h>
#include <wlr/interfaces/wlr_buffer.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <string.h>
#include <stdlib.h>
#include <drm_fourcc.h>
#include "strg/xdg_shell.h"

struct cairo_buffer {
    struct wlr_buffer base;
    cairo_surface_t *surface;
    void *data;
    size_t stride;
    int width;
    int height;
};

static void cairo_buffer_destroy(struct wlr_buffer *wlr_buffer) {
    struct cairo_buffer *buffer = wl_container_of(wlr_buffer, buffer, base);
    cairo_surface_destroy(buffer->surface);
    free(buffer);
}

static bool cairo_buffer_get_dmabuf(struct wlr_buffer *wlr_buffer,
        struct wlr_dmabuf_attributes *attribs) {
    (void)wlr_buffer;
    (void)attribs;
    return false;
}

static bool cairo_buffer_get_shm(struct wlr_buffer *wlr_buffer,
        struct wlr_shm_attributes *attribs) {
    struct cairo_buffer *buffer = wl_container_of(wlr_buffer, buffer, base);

    attribs->fd = -1;
    attribs->format = DRM_FORMAT_ARGB8888;
    attribs->width = buffer->width;
    attribs->height = buffer->height;
    attribs->stride = buffer->stride;
    attribs->offset = 0;

    return true;
}

static bool cairo_buffer_begin_data_ptr_access(struct wlr_buffer *wlr_buffer,
        uint32_t flags, void **data, uint32_t *format, size_t *stride) {
    (void)flags;
    struct cairo_buffer *buffer = wl_container_of(wlr_buffer, buffer, base);

    *data = buffer->data;
    *format = DRM_FORMAT_ARGB8888;
    *stride = buffer->stride;

    return true;
}

static void cairo_buffer_end_data_ptr_access(struct wlr_buffer *wlr_buffer) {
    (void)wlr_buffer;
}

static const struct wlr_buffer_impl cairo_buffer_impl = {
    .destroy = cairo_buffer_destroy,
    .get_dmabuf = cairo_buffer_get_dmabuf,
    .get_shm = cairo_buffer_get_shm,
    .begin_data_ptr_access = cairo_buffer_begin_data_ptr_access,
    .end_data_ptr_access = cairo_buffer_end_data_ptr_access,
};

static struct wlr_buffer *create_cairo_buffer(int width, int height) {
    struct cairo_buffer *buffer = calloc(1, sizeof(*buffer));
    if (!buffer) {
        return NULL;
    }

    buffer->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    if (cairo_surface_status(buffer->surface) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(buffer->surface);
        free(buffer);
        return NULL;
    }

    buffer->width = width;
    buffer->height = height;
    buffer->data = cairo_image_surface_get_data(buffer->surface);
    buffer->stride = cairo_image_surface_get_stride(buffer->surface);

    wlr_buffer_init(&buffer->base, &cairo_buffer_impl, width, height);

    return &buffer->base;
}

static struct wlr_scene_buffer* create_text_buffer(struct wlr_scene_tree *parent,
                                                    const char *text,
                                                    int width,
                                                    int height) {
    struct wlr_buffer *wlr_buffer = create_cairo_buffer(width, height);
    if (!wlr_buffer) {
        return NULL;
    }

    struct cairo_buffer *buffer = wl_container_of(wlr_buffer, buffer, base);
    cairo_t *cr = cairo_create(buffer->surface);

    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    PangoLayout *layout = pango_cairo_create_layout(cr);

    PangoFontDescription *desc = pango_font_description_from_string("Sans Bold 10");
    pango_layout_set_font_description(layout, desc);

    pango_layout_set_text(layout, text, -1);

    int text_width = width - BUTTON_SIZE * 3 - BUTTON_MARGIN * 5;
    pango_layout_set_width(layout, text_width * PANGO_SCALE);
    pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);
    pango_layout_set_alignment(layout, PANGO_ALIGN_LEFT);

    int text_height;
    pango_layout_get_pixel_size(layout, NULL, &text_height);

    cairo_set_source_rgb(cr, 0.95, 0.95, 0.95);
    cairo_move_to(cr, BUTTON_MARGIN, (height - text_height) / 2);
    pango_cairo_show_layout(cr, layout);

    pango_font_description_free(desc);
    g_object_unref(layout);
    cairo_destroy(cr);

    cairo_surface_flush(buffer->surface);

    struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_create(parent, wlr_buffer);

    wlr_buffer_drop(wlr_buffer);

    return scene_buffer;
}

static void draw_button_icon(cairo_t *cr, const char *type, int size) {
    double center = size / 2.0;
    double icon_size = size * 0.5;

    cairo_set_line_width(cr, 2.0);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);

    if (strcmp(type, "close") == 0) {
        double offset = icon_size / 2.0;
        cairo_move_to(cr, center - offset, center - offset);
        cairo_line_to(cr, center + offset, center + offset);
        cairo_move_to(cr, center + offset, center - offset);
        cairo_line_to(cr, center - offset, center + offset);
        cairo_stroke(cr);
    } else if (strcmp(type, "maximize") == 0) {
        double offset = icon_size / 2.0;
        cairo_rectangle(cr, center - offset, center - offset, icon_size, icon_size);
        cairo_stroke(cr);
    } else if (strcmp(type, "minimize") == 0) {
        double offset = icon_size / 2.0;
        cairo_move_to(cr, center - offset, center);
        cairo_line_to(cr, center + offset, center);
        cairo_stroke(cr);
    }
}

static struct wlr_scene_buffer* create_button_buffer(struct wlr_scene_tree *parent,
                                                      const char *type,
                                                      float *bg_color) {
    struct wlr_buffer *wlr_buffer = create_cairo_buffer(BUTTON_SIZE, BUTTON_SIZE);
    if (!wlr_buffer) {
        return NULL;
    }

    struct cairo_buffer *buffer = wl_container_of(wlr_buffer, buffer, base);
    cairo_t *cr = cairo_create(buffer->surface);

    double radius = 3.0;
    double x = 0, y = 0;
    double width = BUTTON_SIZE, height = BUTTON_SIZE;

    cairo_new_sub_path(cr);
    cairo_arc(cr, x + width - radius, y + radius, radius, -M_PI/2, 0);
    cairo_arc(cr, x + width - radius, y + height - radius, radius, 0, M_PI/2);
    cairo_arc(cr, x + radius, y + height - radius, radius, M_PI/2, M_PI);
    cairo_arc(cr, x + radius, y + radius, radius, M_PI, 3*M_PI/2);
    cairo_close_path(cr);

    cairo_set_source_rgba(cr, bg_color[0], bg_color[1], bg_color[2], bg_color[3]);
    cairo_fill(cr);

    draw_button_icon(cr, type, BUTTON_SIZE);

    cairo_destroy(cr);
    cairo_surface_flush(buffer->surface);

    struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_create(parent, wlr_buffer);
    wlr_buffer_drop(wlr_buffer);

    return scene_buffer;
}

void create_decorations(struct strg_toplevel *toplevel) {
    float titlebar_color[4] = {0.2f, 0.2f, 0.25f, 1.0f};
    float border_color[4] = {0.15f, 0.15f, 0.2f, 1.0f};
    float close_color[4] = {0.8f, 0.2f, 0.2f, 1.0f};
    float maximize_color[4] = {0.2f, 0.6f, 0.2f, 1.0f};
    float minimize_color[4] = {0.6f, 0.6f, 0.2f, 1.0f};

    struct wlr_box geometry = toplevel->xdg_toplevel->base->current.geometry;

    if (geometry.width == 0 || geometry.height == 0) {
        geometry.width = toplevel->xdg_toplevel->base->surface->current.width;
        geometry.height = toplevel->xdg_toplevel->base->surface->current.height;
    }

    toplevel->titlebar = wlr_scene_rect_create(toplevel->scene_tree,
        geometry.width + 2 * BORDER_WIDTH, TITLEBAR_HEIGHT, titlebar_color);
    wlr_scene_node_set_position(&toplevel->titlebar->node,
        -BORDER_WIDTH, -TITLEBAR_HEIGHT);

    toplevel->close_button_buffer = create_button_buffer(toplevel->scene_tree,
        "close", close_color);
    if (toplevel->close_button_buffer) {
        wlr_scene_node_set_position(&toplevel->close_button_buffer->node,
            geometry.width - BUTTON_SIZE - BUTTON_MARGIN,
            -TITLEBAR_HEIGHT + (TITLEBAR_HEIGHT - BUTTON_SIZE) / 2);
    }

    toplevel->maximize_button_buffer = create_button_buffer(toplevel->scene_tree,
        "maximize", maximize_color);
    if (toplevel->maximize_button_buffer) {
        wlr_scene_node_set_position(&toplevel->maximize_button_buffer->node,
            geometry.width - BUTTON_SIZE * 2 - BUTTON_MARGIN * 2,
            -TITLEBAR_HEIGHT + (TITLEBAR_HEIGHT - BUTTON_SIZE) / 2);
    }

    toplevel->minimize_button_buffer = create_button_buffer(toplevel->scene_tree,
        "minimize", minimize_color);
    if (toplevel->minimize_button_buffer) {
        wlr_scene_node_set_position(&toplevel->minimize_button_buffer->node,
            geometry.width - BUTTON_SIZE * 3 - BUTTON_MARGIN * 3,
            -TITLEBAR_HEIGHT + (TITLEBAR_HEIGHT - BUTTON_SIZE) / 2);
    }

    toplevel->border_left = wlr_scene_rect_create(toplevel->scene_tree,
        BORDER_WIDTH, geometry.height, border_color);
    wlr_scene_node_set_position(&toplevel->border_left->node, -BORDER_WIDTH, 0);

    toplevel->border_right = wlr_scene_rect_create(toplevel->scene_tree,
        BORDER_WIDTH, geometry.height, border_color);
    wlr_scene_node_set_position(&toplevel->border_right->node, geometry.width, 0);

    toplevel->border_bottom = wlr_scene_rect_create(toplevel->scene_tree,
        geometry.width + 2 * BORDER_WIDTH, BORDER_WIDTH, border_color);
    wlr_scene_node_set_position(&toplevel->border_bottom->node,
        -BORDER_WIDTH, geometry.height);

    struct wlr_box maximized_geo = {
        BORDER_WIDTH,
        BORDER_WIDTH + TITLEBAR_HEIGHT,
        geometry.width - 2 * BORDER_WIDTH,
        geometry.height - (BORDER_WIDTH + TITLEBAR_HEIGHT),
    };

    memcpy(&toplevel->internal_maximized_geometry, &maximized_geo, sizeof(struct wlr_box)); // just dont ask about this bs
    toplevel->use_internal_maximize_geometry = true;

    update_title(toplevel);
}

void update_title(struct strg_toplevel *toplevel) {
    if (toplevel->title_text) {
        wlr_scene_node_destroy(&toplevel->title_text->node);
        toplevel->title_text = NULL;
    }

    const char *title = toplevel->xdg_toplevel->title;
    if (!title || strlen(title) == 0) {
        title = "Untitled";
    }

    struct wlr_box geometry = toplevel->xdg_toplevel->base->current.geometry;
    if (geometry.width == 0) {
        geometry.width = toplevel->xdg_toplevel->base->surface->current.width;
    }

    toplevel->title_text = create_text_buffer(toplevel->scene_tree,
        title, geometry.width + 2 * BORDER_WIDTH, TITLEBAR_HEIGHT);

    if (toplevel->title_text) {
        wlr_scene_node_set_position(&toplevel->title_text->node,
            -BORDER_WIDTH, -TITLEBAR_HEIGHT);
    }
}

void destroy_decorations(struct strg_toplevel *toplevel) {
    if (toplevel->titlebar) {
        wlr_scene_node_destroy(&toplevel->titlebar->node);
        toplevel->titlebar = NULL;
    }
    if (toplevel->title_text) {
        wlr_scene_node_destroy(&toplevel->title_text->node);
        toplevel->title_text = NULL;
    }
    if (toplevel->close_button_buffer) {
        wlr_scene_node_destroy(&toplevel->close_button_buffer->node);
        toplevel->close_button_buffer = NULL;
    }
    if (toplevel->maximize_button_buffer) {
        wlr_scene_node_destroy(&toplevel->maximize_button_buffer->node);
        toplevel->maximize_button_buffer = NULL;
    }
    if (toplevel->minimize_button_buffer) {
        wlr_scene_node_destroy(&toplevel->minimize_button_buffer->node);
        toplevel->minimize_button_buffer = NULL;
    }
    if (toplevel->border_left) {
        wlr_scene_node_destroy(&toplevel->border_left->node);
        toplevel->border_left = NULL;
    }
    if (toplevel->border_right) {
        wlr_scene_node_destroy(&toplevel->border_right->node);
        toplevel->border_right = NULL;
    }
    if (toplevel->border_bottom) {
        wlr_scene_node_destroy(&toplevel->border_bottom->node);
        toplevel->border_bottom = NULL;
    }
}

void update_decoration_geometry(struct strg_toplevel *toplevel) {
    if (toplevel->type != STRG_DECORATION_SERVER) {
        return;
    }

    struct wlr_box geometry = toplevel->xdg_toplevel->base->current.geometry;
    if (geometry.width == 0 || geometry.height == 0) {
        geometry.width = toplevel->xdg_toplevel->base->surface->current.width;
        geometry.height = toplevel->xdg_toplevel->base->surface->current.height;
    }

    if (toplevel->titlebar) {
        wlr_scene_rect_set_size(toplevel->titlebar,
            geometry.width + 2 * BORDER_WIDTH, TITLEBAR_HEIGHT);
    }

    if (toplevel->close_button_buffer) {
        wlr_scene_node_set_position(&toplevel->close_button_buffer->node,
            geometry.width - BUTTON_SIZE - BUTTON_MARGIN,
            -TITLEBAR_HEIGHT + (TITLEBAR_HEIGHT - BUTTON_SIZE) / 2);
    }

    if (toplevel->maximize_button_buffer) {
        wlr_scene_node_set_position(&toplevel->maximize_button_buffer->node,
            geometry.width - BUTTON_SIZE * 2 - BUTTON_MARGIN * 2,
            -TITLEBAR_HEIGHT + (TITLEBAR_HEIGHT - BUTTON_SIZE) / 2);
    }

    if (toplevel->minimize_button_buffer) {
        wlr_scene_node_set_position(&toplevel->minimize_button_buffer->node,
            geometry.width - BUTTON_SIZE * 3 - BUTTON_MARGIN * 3,
            -TITLEBAR_HEIGHT + (TITLEBAR_HEIGHT - BUTTON_SIZE) / 2);
    }

    if (toplevel->border_left) {
        wlr_scene_rect_set_size(toplevel->border_left, BORDER_WIDTH, geometry.height);
    }

    if (toplevel->border_right) {
        wlr_scene_rect_set_size(toplevel->border_right, BORDER_WIDTH, geometry.height);
        wlr_scene_node_set_position(&toplevel->border_right->node, geometry.width, 0);
    }

    if (toplevel->border_bottom) {
        wlr_scene_rect_set_size(toplevel->border_bottom,
            geometry.width + 2 * BORDER_WIDTH, BORDER_WIDTH);
        wlr_scene_node_set_position(&toplevel->border_bottom->node,
            -BORDER_WIDTH, geometry.height);
    }

    update_title(toplevel);
}


bool is_click_on_titlebar(struct strg_toplevel *toplevel, double sx, double sy) {
    if (!toplevel->titlebar) return false;

    double local_x = sx - toplevel->scene_tree->node.x;
    double local_y = sy - toplevel->scene_tree->node.y;

    bool in_titlebar = (local_x >= -BORDER_WIDTH &&
                        local_x < toplevel->titlebar->width - BORDER_WIDTH &&
                        local_y >= -TITLEBAR_HEIGHT &&
                        local_y < 0);

    if (!in_titlebar) return false;

    struct wlr_box geometry = toplevel->xdg_toplevel->base->current.geometry;
    if (geometry.width == 0) {
        geometry.width = toplevel->xdg_toplevel->base->surface->current.width;
    }

    int button_area_start = geometry.width - BUTTON_SIZE * 3 - BUTTON_MARGIN * 4;
    if (local_x >= button_area_start) {
        return false;
    }

    return true;
}

bool is_click_on_close_button(struct strg_toplevel *toplevel, double sx, double sy) {
    if (!toplevel->close_button_buffer) return false;

    double local_x = sx - toplevel->scene_tree->node.x;
    double local_y = sy - toplevel->scene_tree->node.y;

    int button_x = toplevel->close_button_buffer->node.x;
    int button_y = toplevel->close_button_buffer->node.y;

    return (local_x >= button_x && local_x < button_x + BUTTON_SIZE &&
            local_y >= button_y && local_y < button_y + BUTTON_SIZE);
}

bool is_click_on_maximize_button(struct strg_toplevel *toplevel, double sx, double sy) {
    if (!toplevel->maximize_button_buffer) return false;

    double local_x = sx - toplevel->scene_tree->node.x;
    double local_y = sy - toplevel->scene_tree->node.y;

    int button_x = toplevel->maximize_button_buffer->node.x;
    int button_y = toplevel->maximize_button_buffer->node.y;

    return (local_x >= button_x && local_x < button_x + BUTTON_SIZE &&
            local_y >= button_y && local_y < button_y + BUTTON_SIZE);
}

bool is_click_on_minimize_button(struct strg_toplevel *toplevel, double sx, double sy) {
    if (!toplevel->minimize_button_buffer) return false;

    double local_x = sx - toplevel->scene_tree->node.x;
    double local_y = sy - toplevel->scene_tree->node.y;

    int button_x = toplevel->minimize_button_buffer->node.x;
    int button_y = toplevel->minimize_button_buffer->node.y;

    return (local_x >= button_x && local_x < button_x + BUTTON_SIZE &&
            local_y >= button_y && local_y < button_y + BUTTON_SIZE);
}