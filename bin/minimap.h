/*
 * minimap.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef _MINIMAP_H_
#define _MINIMAP_H_

Eina_Bool minimap_add(Editor *ed);
void minimap_del(Editor *ed);
Eina_Bool minimap_update(Editor *restrict ed, unsigned int x, unsigned int y);
void
minimap_render(const Editor *restrict ed,
               unsigned int           x,
               unsigned int           y,
               unsigned int           w,
               unsigned int           h);

void
minimap_render_unit(const Editor *restrict ed,
                    unsigned int           x,
                    unsigned int           y,
                    Pud_Unit               u);

void
minimap_view_move(Editor *restrict ed,
                  int              x,
                  int              y,
                  Eina_Bool        clicked);

void minimap_view_resize(Editor *restrict ed, unsigned int w, unsigned int h);

#endif /* ! _MINIMAP_H_ */

