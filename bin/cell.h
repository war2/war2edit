/*
 * cell.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef _CELL_H_
#define _CELL_H_

struct _Cell
{
   uint16_t     alter_below;
   unsigned int tile : 12;
   unsigned int unit_below : 7;
   unsigned int unit_above : 7;
   unsigned int orient_below : 3; /* 8 values */
   unsigned int orient_above : 3; /* 8 values */
   unsigned int player_below : 3; /* 8 values */
   unsigned int player_above : 3; /* 8 values */
   unsigned int spread_x_below : 3; /* 0-4 */
   unsigned int spread_y_below : 3; /* 0-4 */
   unsigned int spread_x_above : 3; /* 0-4 */
   unsigned int spread_y_above : 3; /* 0-4 */
   unsigned int anchor_below : 1;
   unsigned int anchor_above : 1;
   unsigned int alter_above : 1;
   unsigned int alter_start_location : 1;
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

Cell **cell_matrix_new(const unsigned int w, const unsigned int h);
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

#endif /* ! _CELL_H_ */

