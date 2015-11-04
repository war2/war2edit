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
   evas_object_color_set(o, 0, 0, 255, 100);
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
   int x = ed->sel.x;
   int y = ed->sel.y;

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

   evas_object_move(ed->sel.obj, x, y);
   evas_object_resize(ed->sel.obj, w, h);
}

void
sel_end(Editor *restrict ed)
{
   evas_object_hide(ed->sel.obj);
   evas_object_resize(ed->sel.obj, 1, 1);
   ed->sel.active = EINA_FALSE;
}

Eina_Bool
sel_active_is(const Editor *restrict ed)
{
   return ed->sel.active;
}

