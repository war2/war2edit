/*
 * bitmap.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef _BITMAP_H_
#define _BITMAP_H_

typedef enum
{
   BITMAP_UNIT_BELOW,
   BITMAP_UNIT_ABOVE,
   BITMAP_UNIT_START_LOCATION
} Bitmap_Unit;

Eina_Bool bitmap_add(Editor *ed);
void bitmap_unit_set(Editor * ed, Pud_Unit unit, Pud_Player color, unsigned int orient, unsigned int x, unsigned int y, unsigned int w, unsigned int h, uint16_t alter);

void bitmap_refresh(Editor *ed,
                    const Eina_Rectangle *zone);

void
bitmap_visible_zone_cells_get(const Editor   *ed,
                              Eina_Rectangle *zone);

void
bitmap_selections_draw(Editor * ed,
                       int              x,
                       int              y,
                       unsigned int     w,
                       unsigned int     h);

void bitmap_unit_draw(Editor * ed,
                      unsigned int x,
                      unsigned int y,
                      Bitmap_Unit unit_type);

void bitmap_tile_draw(Editor * ed,
                      unsigned int x,
                      unsigned int y);
void
bitmap_unit_del_at(Editor * ed,
                   unsigned int     x,
                   unsigned int     y,
                   Eina_Bool        below);

Eina_Bool
bitmap_tile_set(Editor *  ed,
                int               x,
                int               y,
                uint8_t           tl,
                uint8_t           tr,
                uint8_t           bl,
                uint8_t           br,
                uint8_t           seed,
                Eina_Bool         force);

void
bitmap_cursor_state_evaluate(Editor       *ed,
                             unsigned int  x,
                             unsigned int  y);

void
bitmap_coords_to_cells(const Editor *ed,
                       int           x,
                       int           y,
                       int          *cx,
                       int          *cy);

void
bitmap_cells_to_coords(const Editor *ed,
                       int           cx,
                       int           cy,
                       int          *x,
                       int          *y);

Eina_Bool
bitmap_tile_calculate(Editor           *ed,
                      int               px,
                      int               py,
                      Tile_Propagation *prop);

void
bitmap_cell_size_get(const Editor *ed,
                     int          *w,
                     int          *h);

void
bitmap_cursor_size_set(Editor *ed,
                       int     cw,
                       int     ch);

void
bitmap_cursor_size_get(const Editor *ed,
                       int          *w,
                       int          *h);

Eina_Bool
bitmap_cursor_enabled_get(const Editor *ed);

void
bitmap_cursor_enabled_set(Editor     *ed,
                          Eina_Bool  enabled);

void bitmap_cursor_move(Editor *ed, int cx, int cy);

void bitmap_cursor_visibility_set(Editor *ed, Eina_Bool visible);

#endif /* ! _BITMAP_H_ */
