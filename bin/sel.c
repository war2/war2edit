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
sel_add(Editor *ed)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, NULL);

   Evas_Object *o;
   Evas *e;

   e = evas_object_evas_get(ed->win);
   o = evas_object_rectangle_add(e);
   evas_object_smart_member_add(o, ed->scroller);
   evas_object_color_set(o, 0, 0, 100, 100);
   ed->sel.active = EINA_FALSE;

   ed->sel.obj = o;

   return o;
}

void
sel_start(Editor    *ed,
          int        x,
          int        y,
          Eina_Bool  inclusive)
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
        bitmap_refresh(ed, NULL); // FIXME BAD
     }
   ed->sel.active = EINA_TRUE;
   ed->sel.inclusive = inclusive;
   ed->sel.x = x;
   ed->sel.y = y;
   sel_update(ed, 0, 0);
   evas_object_show(ed->sel.obj);
}

void
sel_update(Editor *ed,
           int     w,
           int     h)
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
   elm_scroller_region_get(ed->scroller, &sx, &sy, NULL, NULL);
   evas_object_geometry_get(ed->scroller, &rx, &ry, NULL, NULL);
   bitmap_cell_size_get(ed, &cell_w, &cell_h);

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

   if (ed->sel.rel2.x >= ed->pud->map_w) ed->sel.rel2.x = ed->pud->map_w - 1;
   if (ed->sel.rel2.y >= ed->pud->map_h) ed->sel.rel2.y = ed->pud->map_h - 1;

   //DBG("Sel (cells): %i %i ; %i %i", cx1, cy1, cx2, cy2);

   evas_object_move(ed->sel.obj, x, y);
   evas_object_resize(ed->sel.obj, w, h);
}

void
sel_end(Editor *ed)
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
             if (anchor)
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
             if (anchor)
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
                       if (!(anchor->selected_above & SEL_MARK))
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
          if (anchor) anchor->selected_below &= (~SEL_MARK);
          anchor = cell_anchor_get(cells, i, j, EINA_FALSE);
          if (anchor) anchor->selected_above &= (~SEL_MARK);
       }

   bitmap_refresh(ed, NULL); // FIXME BAD

   evas_object_hide(ed->sel.obj);
   evas_object_resize(ed->sel.obj, 1, 1);
   ed->sel.active = EINA_FALSE;
}

Eina_Bool
sel_active_is(const Editor *ed)
{
   return ed->sel.active;
}

Eina_Bool
sel_empty_is(const Editor *ed)
{
   return (ed->sel.selections == 0);
}

void
sel_del(Editor *ed)
{
   unsigned int i, j;
   Cell *c;

   snapshot_push(ed);
   for (j = 0; j < ed->pud->map_h; ++j)
     for (i = 0; i < ed->pud->map_w; ++i)
       {
          c = &(ed->cells[j][i]);
          if (c->selected_below == SEL_SET)
            {
               if (c->anchor_below)
                 bitmap_unit_del_at(ed, i, j, UNIT_BELOW);
               if (c->start_location != CELL_NOT_START_LOCATION)
                 bitmap_unit_del_at(ed, i, j, UNIT_START_LOCATION);
            }
          if ((c->anchor_above) && (c->selected_above == SEL_SET))
            bitmap_unit_del_at(ed, i, j, UNIT_ABOVE);
       }
   snapshot_push_done(ed);
   bitmap_refresh(ed, NULL); // FIXME BAD
}
