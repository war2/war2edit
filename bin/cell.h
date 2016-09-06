/*
 * Copyright (c) 2015-2016 Jean Guyomarc'h
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _CELL_H_
#define _CELL_H_

struct _Cell
{
   uint16_t     alter_below; // FIXME _bits are sufficient
   unsigned int tile : 12;
   unsigned int unit_below : 7;
   unsigned int unit_above : 7;
   unsigned int orient_below : 3; /* 8 values */
   unsigned int orient_above : 3; /* 8 values */
   unsigned int player_below : 4;
   unsigned int player_above : 4;
   unsigned int spread_x_below : 3; /* 0-4 */
   unsigned int spread_y_below : 3; /* 0-4 */
   unsigned int spread_x_above : 3; /* 0-4 */
   unsigned int spread_y_above : 3; /* 0-4 */
   unsigned int anchor_below : 1;
   unsigned int anchor_above : 1;
   unsigned int alter_above : 1;
   unsigned int start_location : 4;
   unsigned int start_location_human : 1;
   unsigned int selected_below : 2;
   unsigned int selected_above : 2;

   /* Top Left, Top Right, Bottom Left, Bottom Right */
   uint8_t tile_tl;
   uint8_t tile_tr;
   uint8_t tile_bl;
   uint8_t tile_br;
};

#define CELL_NOT_START_LOCATION 0x0f

Cell **cell_matrix_new(unsigned int w, unsigned int h);
void cell_matrix_free(Cell **cells);
void cell_dump(Cell *cell, FILE *stream);
Cell *cell_anchor_get(Cell         **cells,
                      unsigned int   x,
                      unsigned int   y,
                      Eina_Bool      below);
Cell *cell_anchor_pos_get(Cell         **cells,
                          unsigned int   x,
                          unsigned int   y,
                          unsigned int  *ax,
                          unsigned int  *ay,
                          Eina_Bool      below);

void
cell_matrix_copy(Cell         **src,
                 Cell         **dst,
                 unsigned int   w,
                 unsigned int   h);

void
cell_matrix_bindump(Cell **cells,
                    unsigned int w,
                    unsigned int h,
                    FILE *stream);

Eina_Bool
cell_unit_get(const Cell *c,
              Unit        type,
              Pud_Unit   *unit,
              Pud_Player *owner);

#endif /* ! _CELL_H_ */
