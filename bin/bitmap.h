/*
 * bitmap.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef _GRID_H_
#define _GRID_H_

Eina_Bool bitmap_add(Editor *ed);
Eina_Bool bitmap_tile_set(Editor * restrict ed, int x, int y, unsigned int key);
void bitmap_unit_set(Editor *restrict ed, Pud_Unit unit, Pud_Player color, unsigned int orient, int x, int y, unsigned int w, unsigned int h, uint16_t alter);

//void
//bitmap_refresh_zone(Editor *restrict ed,
//                    int              x,
//                    int              y,
//                    unsigned int     w,
//                    unsigned int     h);

#endif /* ! _GRID_H_ */

