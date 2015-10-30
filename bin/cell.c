/*
 * cell.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
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
cell_matrix_zero(Cell   **cells,
                 size_t   count)
{
   if (cells)
     memset(cells[0], 0, count);
}

