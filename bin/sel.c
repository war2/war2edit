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
   ed->sel.active = EINA_TRUE;
   ed->sel.x = x;
   ed->sel.y = y;
   evas_object_move(ed->sel.obj, ed->sel.x, ed->sel.y);
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
   unsigned int i, j;
   int cx1, cy1, cx2, cy2;
   Cell **cells = ed->cells;
   Cell *anchor, *c;

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

   for (j = (unsigned int) cy1; j < (unsigned int) cy2; ++j)
     {
        for (i = (unsigned int) cx1; i < (unsigned int) cx2; ++i)
          {
             c = &(cells[j][i]);

             anchor = &(cells[j - c->spread_y_below][i - c->spread_x_below]);
             if ((anchor->anchor_below) &&
                 (!(anchor->selected_below) && !(anchor->pre_selected_below)))
               {
                  anchor->pre_selected_below = 1;
                  // TODO Visual hint
               }
             anchor = &(cells[j - c->spread_y_above][i - c->spread_x_above]);
             if ((anchor->anchor_above) &&
                 (!(anchor->selected_above) && !(anchor->pre_selected_above)))
               {
                  anchor->pre_selected_above = 1;
                  // TODO Visual hint
               }
          }
     }

   DBG("Sel (cells): %i %i ; %i %i", cx1, cy1, cx2, cy2);

   evas_object_move(ed->sel.obj, x, y);
   evas_object_resize(ed->sel.obj, w, h);
}

void
sel_end(Editor *restrict ed)
{
   unsigned int i, j;
   Cell *c;
   Eina_Bool redraw = EINA_FALSE;

   for (j = ed->sel.rel1.y; j < ed->sel.rel2.y; ++j)
     {
        for (i = ed->sel.rel1.x; i < ed->sel.rel2.x; ++i)
          {
             c = &(ed->cells[j][i]);
             if (c->pre_selected_below || c->selected_below)
               {
                  c->pre_selected_below = 0;
                  c->selected_below = 1;
                  DBG("Was selected below: (%i,%i)", i, j);
                  redraw = EINA_TRUE;
               }
             if (c->pre_selected_above || c->selected_above)
               {
                  c->pre_selected_above = 0;
                  c->selected_above = 1;
                  DBG("Was selected above: (%i,%i)", i, j);
                  redraw = EINA_TRUE;
               }
          }
     }

   // FIXME Optimize this later. We could just use selections_redraw
   // since selections are always on top, but I call the big fat function
   // for now to collect all calls for later...
   if (redraw)
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

