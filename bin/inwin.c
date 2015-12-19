/*
 * inwin.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

Evas_Object *
inwin_add(Editor *ed)
{
   Evas_Object *obj;

   obj = elm_win_inwin_add(ed->win);
   if (EINA_UNLIKELY(!obj))
     {
        CRI("Failed to create Inwin");
        return NULL;
     }
   ed->inwin.obj = obj;

   return obj;
}

void
inwin_activate(Editor      *ed,
               Evas_Object *obj,
               Inwin        id)
{
   ed->inwin.id = id;
   elm_win_inwin_content_set(ed->inwin.obj, obj);
   elm_win_inwin_activate(ed->inwin.obj);
   evas_object_show(ed->inwin.obj);
}

void
inwin_dismiss(Editor *ed)
{
   evas_object_hide(ed->inwin.obj);
}

Eina_Bool
inwin_id_is(const Editor *ed,
            const Inwin   id)
{
   return (ed->inwin.id == id);
}

