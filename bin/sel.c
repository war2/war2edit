/*
 * sel.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

/*
 * This applies to anchors...
 * cell->selected_xxx will have the following bits
 *  - [0] SEL_SET if the selection is effective
 *  - [1] SEL_MARK if the anchor was proccessed during the seletion
 *    Indeed, for NxN units, N>1 the anchor will likely to be processed
 *    several times. This system is used to handle the "inclusive"
 *    selection that allows to de-select previously selected units.
 */
enum {
   SEL_MARK = (1 << 1),
   SEL_SET  = (1 << 0)
};

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
          const int        y,
          Eina_Bool        inclusive)
{
   unsigned int i, j;
   Cell *c;

   if (!inclusive)
     {
        for (j = 0; j < ed->pud->map_h; ++j)
          for (i = 0; i < ed->pud->map_w; ++i)
            {
               c = &(ed->cells[j][i]);
               c->selected_above = 0;
               c->selected_below = 0;
            }
        ed->sel.selections = 0;
     }
   ed->sel.active = EINA_TRUE;
   ed->sel.inclusive = inclusive;
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
   Cell *anchor;
   Cell **cells = ed->cells;
   const Eina_Bool inclusive = ed->sel.inclusive;

   for (j = ed->sel.rel1.y; j <= ed->sel.rel2.y; ++j)
     {
        for (i = ed->sel.rel1.x; i <= ed->sel.rel2.x; ++i)
          {
             /* Selections for units below */
             anchor = cell_anchor_get(cells, i, j, EINA_TRUE);
             if (anchor->unit_below != PUD_UNIT_NONE)
               {
                  if (inclusive)
                    {
                       if (!(anchor->selected_below & SEL_MARK))
                         {
                            anchor->selected_below = ~(anchor->selected_below);
                            if (anchor->selected_below & SEL_SET)
                              ++ed->sel.selections;
                            else
                              --ed->sel.selections;
                         }
                    }
                  else
                    {
                       if (!(anchor->selected_below & SEL_MARK))
                         ++ed->sel.selections;
                       anchor->selected_below |= (SEL_MARK | SEL_SET);
                    }
               }

             /* Selection for units above */
             anchor = cell_anchor_get(cells, i, j, EINA_FALSE);
             if (anchor->unit_above != PUD_UNIT_NONE)
               {
                  if (inclusive)
                    {
                       if (!(anchor->selected_above & SEL_MARK))
                         {
                            anchor->selected_above = ~(anchor->selected_above);
                            if (anchor->selected_above & SEL_SET)
                              ++ed->sel.selections;
                            else
                              --ed->sel.selections;
                         }
                    }
                  else
                    {
                       if (!(anchor->selected_below & SEL_MARK))
                         ++ed->sel.selections;
                       anchor->selected_above |= (SEL_MARK | SEL_SET);
                    }
               }
          }
     }

   /* Reset the marks */
   for (j = ed->sel.rel1.y; j <= ed->sel.rel2.y; ++j)
     for (i = ed->sel.rel1.x; i <= ed->sel.rel2.x; ++i)
       {
          anchor = cell_anchor_get(cells, i, j, EINA_TRUE);
          anchor->selected_below &= (~SEL_MARK);
          anchor = cell_anchor_get(cells, i, j, EINA_FALSE);
          anchor->selected_above &= (~SEL_MARK);
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

Eina_Bool
sel_empty_is(const Editor *restrict ed)
{
   return (ed->sel.selections == 0);
}

void
sel_del(Editor *restrict ed)
{
   unsigned int i, j;
   Cell *c;

   for (j = 0; j < ed->pud->map_h; ++j)
     for (i = 0; i < ed->pud->map_w; ++i)
       {
          c = &(ed->cells[j][i]);
          if ((c->anchor_below) && (c->selected_below == SEL_SET))
            bitmap_unit_del_at(ed, i, j, EINA_TRUE);
          if ((c->anchor_above) && (c->selected_above == SEL_SET))
            bitmap_unit_del_at(ed, i, j, EINA_FALSE);
       }
   bitmap_redraw(ed);
}

