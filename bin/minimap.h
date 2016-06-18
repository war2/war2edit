/*
 * minimap.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef _MINIMAP_H_
#define _MINIMAP_H_

Eina_Bool minimap_add(Editor *ed);
void minimap_del(Editor *ed);
Eina_Bool minimap_update(Editor *ed, unsigned int x, unsigned int y);
void
minimap_render(const Editor *ed,
               unsigned int  x,
               unsigned int  y,
               unsigned int  w,
               unsigned int  h);

void
minimap_render_unit(const Editor *ed,
                    unsigned int  x,
                    unsigned int  y,
                    Pud_Unit      u);

void
minimap_view_move(Editor    *ed,
                  int        x,
                  int        y,
                  Eina_Bool  clicked);

void minimap_view_resize(Editor *ed, unsigned int w, unsigned int h);

void minimap_show(Editor *ed);

#endif /* ! _MINIMAP_H_ */

