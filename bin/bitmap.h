/*
 * bitmap.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef _BITMAP_H_
#define _BITMAP_H_

#define BITMAP_UNIT_BELOW ((Eina_Bool) EINA_TRUE)
#define BITMAP_UNIT_ABOVE ((Eina_Bool) EINA_FALSE)

Eina_Bool bitmap_add(Editor *ed);
Eina_Bool bitmap_tile_set(Editor * restrict ed, int x, int y, unsigned int key);
void bitmap_unit_set(Editor *restrict ed, Pud_Unit unit, Pud_Player color, unsigned int orient, unsigned int x, unsigned int y, unsigned int w, unsigned int h, uint16_t alter);

void bitmap_redraw(Editor *restrict ed);

void bitmap_unit_draw(Editor *restrict ed,
                      unsigned int x,
                      unsigned int y,
                      Eina_Bool unit_below);

void bitmap_tile_draw(Editor *restrict ed,
                      unsigned int x,
                      unsigned int y);
void
bitmap_unit_del_at(Editor *restrict ed,
                   unsigned int     x,
                   unsigned int     y,
                   Eina_Bool        below);

#endif /* ! _BITMAP_H_ */

