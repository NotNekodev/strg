#ifndef STRG_DECORATIONS_H
#define STRG_DECORATIONS_H

#include <strg/strg.h>

void create_decorations(struct strg_toplevel *toplevel);
void update_title(struct strg_toplevel *toplevel);
void destroy_decorations(struct strg_toplevel *toplevel);
bool is_click_on_titlebar(struct strg_toplevel *toplevel, double sx, double sy);
bool is_click_on_close_button(struct strg_toplevel *toplevel, double sx, double sy);
bool is_click_on_maximize_button(struct strg_toplevel *toplevel, double sx, double sy);
bool is_click_on_minimize_button(struct strg_toplevel *toplevel, double sx, double sy);


#endif //STRG_DECORATIONS_H