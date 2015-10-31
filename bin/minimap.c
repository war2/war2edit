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

   Evas_Object *o;

   ed->minimap.win = elm_win_add(ed->win, "Minimap", ELM_WIN_UTILITY);
   evas_object_show(ed->minimap.win);

   o = ed->minimap.map = elm_button_add(ed->minimap.win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(o, "Quiche");
   evas_object_show(o);

   elm_win_resize_object_add(ed->minimap.win, o);
   evas_object_resize(ed->minimap.win, 200, 200);
   


   return EINA_TRUE;
}

