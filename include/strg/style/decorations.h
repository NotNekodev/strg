#ifndef STRG_DECORATIONS_H
#define STRG_DECORATIONS_H


#define TITLEBAR_HEIGHT 30
#define BORDER_WIDTH 2
#define BUTTON_SIZE 20
#define BUTTON_MARGIN 5

#define M_PI 3.14159265358979323846

void create_decorations(struct strg_toplevel *toplevel);
void update_title(struct strg_toplevel *toplevel);
void destroy_decorations(struct strg_toplevel *toplevel);
bool is_click_on_titlebar(struct strg_toplevel *toplevel, double sx, double sy);
bool is_click_on_close_button(struct strg_toplevel *toplevel, double sx, double sy);
bool is_click_on_maximize_button(struct strg_toplevel *toplevel, double sx, double sy);
bool is_click_on_minimize_button(struct strg_toplevel *toplevel, double sx, double sy);
void update_decoration_geometry(struct strg_toplevel *toplevel);


#endif //STRG_DECORATIONS_H