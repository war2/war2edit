/*
 * minimap.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

Eina_Bool
minimap_add(Editor *ed)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);

   Evas_Object *o, *win;
   unsigned int w = ed->pud->map_w;
   const unsigned int h = ed->pud->map_h;
   unsigned int i;

   win = ed->minimap.win = elm_win_add(ed->win, "Minimap", ELM_WIN_UTILITY);
   evas_object_show(win);

   o = ed->minimap.map = evas_object_image_filled_add(evas_object_evas_get(win));
   evas_object_image_colorspace_set(o, EVAS_COLORSPACE_ARGB8888);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(o);

   /* Define a ratio to resize the minimap (way too small overwise) */
   switch (ed->pud->dims)
     {
      case PUD_DIMENSIONS_32_32:
         ed->minimap.ratio = 8;
         break;

      case PUD_DIMENSIONS_64_64:
         ed->minimap.ratio = 4;
         break;

      case PUD_DIMENSIONS_128_128:
         ed->minimap.ratio = 2;
         break;

      default:
         CRI("ed->pud->dims is %i. This MUST NEVER happen", ed->pud->dims);
         ed->minimap.ratio = 1;
         break;
     }

   ed->minimap.w = w * ed->minimap.ratio;
   ed->minimap.h = h * ed->minimap.ratio;

   elm_win_resize_object_add(win, o);
   evas_object_resize(win, ed->minimap.w, ed->minimap.h);
   evas_object_size_hint_max_set(win, ed->minimap.w, ed->minimap.h);
   evas_object_size_hint_min_set(win, ed->minimap.w, ed->minimap.h);

   /* Colorspace width */
   w = ed->minimap.w * 4;

   /* Allocate Iliffe vector to hold the minimap */
   ed->minimap.data = malloc(ed->minimap.h * sizeof(unsigned char *));
   if (EINA_UNLIKELY(!ed->minimap.data))
     {
        CRI("Failed to alloc memory");
        goto fail;
     }
   INF("Pud tile: %i. Ratio is %u", ed->pud->tiles, ed->minimap.ratio);
   ed->minimap.data[0] = malloc(ed->minimap.w * ed->minimap.h * 4);
   if (EINA_UNLIKELY(!ed->minimap.data[0]))
     {
        CRI("Failed to alloc memory");
        goto fail_free;
     }
   for (i = 1; i < ed->minimap.h; ++i)
     ed->minimap.data[i] = ed->minimap.data[i - 1] + w;

   /* Configure minimap image */
   evas_object_image_size_set(o, ed->minimap.w, ed->minimap.h);
   evas_object_image_data_set(o, ed->minimap.data[0]);

   evas_object_image_data_update_add(ed->minimap.map, 0, 0,
                                     ed->minimap.w, ed->minimap.h);

   return EINA_TRUE;

fail_free:
   free(ed->minimap.data);
fail:
   return EINA_FALSE;
}

void
minimap_del(Editor *ed)
{
   EINA_SAFETY_ON_NULL_RETURN(ed);

   if (ed->minimap.data)
     {
        free(ed->minimap.data[0]);
        free(ed->minimap.data);
     }
   if (ed->minimap.win)
     evas_object_del(ed->minimap.win);
}

Eina_Bool
minimap_update(Editor *restrict ed,
               unsigned int     x,
               unsigned int     y)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((x >= ed->pud->map_w) ||
                                  (y >= ed->pud->map_h), EINA_FALSE);

   const Cell c = ed->cells[y][x];
   uint8_t player;
   unsigned int px, py;
   Pud_Unit u = PUD_UNIT_NONE;
   Pud_Color col;
   unsigned char *ptr;
   unsigned int i, j;
   unsigned ry, rx;

   if (c.unit_above != PUD_UNIT_NONE)
     {
        player = c.player_above;
        u = c.unit_above;
     }
   else if (c.unit_below != PUD_UNIT_NONE)
     {
        player = c.player_below;
        u = c.unit_below;
     }

   if (u == PUD_UNIT_NONE)
     col = pud_tile_to_color(ed->pud->era, c.tile);
   else
     col = pud_color_for_unit(u, player);

   /* Format ARGB8888: each pixel is 4 bytes long */
   px = x * ed->minimap.ratio * 4;
   py = y * ed->minimap.ratio;
   ry = py + ed->minimap.ratio;
   rx = px + ed->minimap.ratio;

   for (j = py; j < ry; ++j)
     {
        for (i = px; i < rx; i += 4)
          {
             ptr = &(ed->minimap.data[j][i]); /* Raw data */
             ptr[0] = col.b;
             ptr[1] = col.g;
             ptr[2] = col.r;
             ptr[3] = col.a;
          }
     }

   /*
    * Don't update the image!
    * Use minimap_render() to do so (for perfs).
    */

   return EINA_TRUE;
}

void
minimap_render(const Editor *restrict ed,
               unsigned int           x,
               unsigned int           y,
               unsigned int           w,
               unsigned int           h)
{
   evas_object_image_data_update_add(ed->minimap.map, x, y,
                                     w * ed->minimap.w,
                                     h * ed->minimap.h);
}

