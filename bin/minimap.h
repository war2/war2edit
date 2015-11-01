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

#endif /* ! _MINIMAP_H_ */

