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

#include "war2edit.h"

Cell **
cell_matrix_new(const unsigned int w,
                const unsigned int h)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL((w == 0) || (h == 0), NULL);

   Cell **ptr;
   unsigned int i, j;

   /* Iliffe vector allocation */

   ptr = malloc(h * sizeof(*ptr));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ptr, NULL);

   ptr[0] = calloc(w * h, sizeof(**ptr));
   EINA_SAFETY_ON_NULL_GOTO(ptr[0], fail);

   for (i = 1; i < h; ++i)
     ptr[i] = ptr[i - 1] + w;

   for (i = 0; i < h; i++)
     for (j = 0; j < w; j++)
       {
          ptr[i][j].unit_above = PUD_UNIT_NONE;
          ptr[i][j].unit_below = PUD_UNIT_NONE;
          ptr[i][j].start_location = CELL_NOT_START_LOCATION;
       }
   /* Other fields are set to 0 */

   return ptr;

fail:
   free(ptr);
   return NULL;
}

void
cell_matrix_free(Cell **cells)
{
   if (cells)
     {
        free(cells[0]);
        free(cells);
     }
}

void
cell_dump(Cell *cell,
          FILE *stream)
{
   fprintf(
      stream,
      "{\n"
      "    alter (below).......: %u\n"
      "    alter (above).......: %u\n"
      "    tile................: 0x%04x\n"
      "    unit_below..........: 0x%02x\n"
      "    unit_above..........: 0x%02x\n"
      "    orient_below........: 0x%02x\n"
      "    orient_above........: 0x%02x\n"
      "    player_below........: 0x%02x\n"
      "    player_above........: 0x%02x\n"
      "    spread_x_below......: %u\n"
      "    spread_y_below......: %u\n"
      "    spread_x_above......: %u\n"
      "    spread_y_above......: %u\n"
      "    anchor_below........: %u\n"
      "    anchor_above........: %u\n"
      "    start_location......: %u\n"
      "    start_location_human: %u\n"
      "    selected_below......: %u\n"
      "    selected_above......: %u\n"
      "    tile TL.............: 0x%02x\n"
      "    tile TR.............: 0x%02x\n"
      "    tile BL.............: 0x%02x\n"
      "    tile BR.............: 0x%02x\n"
      "}\n",
      cell->alter_below,
      cell->alter_above,
      cell->tile,
      cell->unit_below,
      cell->unit_above,
      cell->orient_below,
      cell->orient_above,
      cell->player_below,
      cell->player_above,
      cell->spread_x_below,
      cell->spread_y_below,
      cell->spread_x_above,
      cell->spread_y_above,
      cell->anchor_below,
      cell->anchor_above,
      cell->start_location,
      cell->start_location_human,
      cell->selected_below,
      cell->selected_above,
      cell->tile_tl,
      cell->tile_tr,
      cell->tile_bl,
      cell->tile_br
   );
}

Cell *
cell_anchor_get(Cell         **cells,
                unsigned int   x,
                unsigned int   y,
                Eina_Bool      below)
{
   return cell_anchor_pos_get(cells, x, y, NULL, NULL, below);
}

Cell *
cell_anchor_pos_get(Cell         **cells,
                    unsigned int   x,
                    unsigned int   y,
                    unsigned int  *ax,
                    unsigned int  *ay,
                    Eina_Bool      below)
{
   Cell *c = &(cells[y][x]);
   unsigned int rx, ry;

   if (below)
     {
        if (c->anchor_below)
          {
             if (ax) *ax = x;
             if (ay) *ay = y;
             return c;
          }
        rx = x - c->spread_x_below;
        ry = y - c->spread_y_below;
        if (ax) *ax = rx;
        if (ay) *ay = ry;
        return &(cells[ry][rx]);
     }
   else
     {
        if (c->anchor_above)
          {
             if (ax) *ax = x;
             if (ay) *ay = y;
             return c;
          }
        rx = x - c->spread_x_above;
        ry = y - c->spread_y_above;
        if (ax) *ax = rx;
        if (ay) *ay = ry;
        return &(cells[ry][rx]);
     }
}

