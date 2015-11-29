/*
 * sel.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

Evas_Object *
sel_add(Editor *restrict ed)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, NULL);

   Evas_Object *o;
   Evas *e;

   e = evas_object_evas_get(ed->win);
   o = evas_object_rectangle_add(e);
   evas_object_color_set(o, 0, 0, 100, 100);
   ed->sel.active = EINA_FALSE;

   ed->sel.obj = o;

   return o;
}

void
sel_start(Editor *restrict ed,
          const int        x,
          const int        y)
{
   unsigned int i, j;
   Cell *c;

   // TODO Handle shift key. If shift key, then skip this loop
   for (j = 0; j < ed->pud->map_h; ++j)
     for (i = 0; i < ed->pud->map_w; ++i)
       {
          c = &(ed->cells[j][i]);
          c->selected_above = 0;
          c->selected_below = 0;
       }
   ed->sel.active = EINA_TRUE;
   ed->sel.x = x;
   ed->sel.y = y;
   sel_update(ed, 0, 0);
   evas_object_show(ed->sel.obj);
}

void
sel_update(Editor *restrict ed,
           int              w,
           int              h)
{
   const unsigned int map_w = ed->pud->map_w;
   const unsigned int map_h = ed->pud->map_h;
   int x = ed->sel.x;
   int y = ed->sel.y;
   int sx, sy, cell_w, cell_h, rx, ry, relx, rely;
   int cx1, cy1, cx2, cy2;

   if (w < 0)
     {
        x += w; /* w is negative */
        w = -w;
     }
   if (h < 0)
     {
        y += h; /* h is negative */
        h = -h;
     }

   /* Get positions */
   eo_do(
      ed->scroller,
      elm_interface_scrollable_content_region_get(&sx, &sy, NULL, NULL));
   evas_object_geometry_get(ed->scroller, &rx, &ry, NULL, NULL);
   elm_bitmap_cell_size_get(ed->bitmap, &cell_w, &cell_h);

   /* Cache */
   relx = x - rx + sx;
   rely = y - ry + sy;

   /* rel1 */
   cx1 = relx / cell_w;
   cy1 = rely / cell_h;

   /* rel2 */
   cx2 = (relx + w) / cell_w;
   cy2 = (rely + h) / cell_h;

   /* Bounds checking safety */
   if (EINA_UNLIKELY(cx1 >= (int)map_w))
     cx1 = (int)map_w - 1;
   else if (EINA_UNLIKELY(cx1 < 0))
     cx1 = 0;
   if (EINA_UNLIKELY(cx2 >= (int)map_w))
     cx2 = (int)map_w - 1;
   else if (EINA_UNLIKELY(cx2 < 0))
     cx2 = 0;
   if (EINA_UNLIKELY(cy1 >= (int)map_h))
     cy1 = (int)map_h - 1;
   else if (EINA_UNLIKELY(cy1 < 0))
     cy1 = 0;
   if (EINA_UNLIKELY(cy2 >= (int)map_h))
     cy2 = (int)map_h - 1;
   else if (EINA_UNLIKELY(cy2 < 0))
     cy2 = 0;

   /* Cache selection */
   ed->sel.rel1.x = (unsigned int)cx1;
   ed->sel.rel1.y = (unsigned int)cy1;
   ed->sel.rel2.x = (unsigned int)cx2;
   ed->sel.rel2.y = (unsigned int)cy2;

   //DBG("Sel (cells): %i %i ; %i %i", cx1, cy1, cx2, cy2);

   evas_object_move(ed->sel.obj, x, y);
   evas_object_resize(ed->sel.obj, w, h);
}

void
sel_end(Editor *restrict ed)
{
   unsigned int i, j;
   Cell *c;
   Cell *anchor;
   Cell **cells = ed->cells;

   for (j = ed->sel.rel1.y; j <= ed->sel.rel2.y; ++j)
     {
        for (i = ed->sel.rel1.x; i <= ed->sel.rel2.x; ++i)
          {
             c = &(cells[j][i]);

             if (c->anchor_below)
               anchor = c;
             else
               anchor = &(cells[j - c->spread_y_below][i - c->spread_x_below]);
             anchor->selected_below = 1;

             if (c->anchor_above)
               anchor = c;
             else
               anchor = &(cells[j - c->spread_y_above][i - c->spread_x_above]);
             anchor->selected_above = 1;
          }
     }

   // FIXME Optimize this later. We could just use selections_redraw
   // since selections are always on top, but I call the big fat function
   // for now to collect all calls for later...
   bitmap_redraw(ed);

   evas_object_hide(ed->sel.obj);
   evas_object_resize(ed->sel.obj, 1, 1);
   ed->sel.active = EINA_FALSE;
}

Eina_Bool
sel_active_is(const Editor *restrict ed)
{
   return ed->sel.active;
}

