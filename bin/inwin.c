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

static void
_inwin_close_cb(void        *data,
                Evas_Object *obj  EINA_UNUSED,
                void        *evt  EINA_UNUSED)
{
   Editor *ed = data;
   inwin_dismiss(ed);
}

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
   ed->inwin.id = INWIN_GENERIC;

   return obj;
}

void
inwin_set(Editor        *ed,
          Evas_Object   *obj,
          Inwin          id,
          const char    *ok_label,
          Evas_Smart_Cb  ok_smart_cb,
          const char    *cancel_label,
          Evas_Smart_Cb  cancel_smart_cb)
{
   Evas_Object *vbox, *hbox, *o;

   /* Super box: holds everything */
   vbox = elm_box_add(ed->inwin.obj);
   elm_box_horizontal_set(vbox, EINA_FALSE);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(vbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_start(vbox, obj);
   evas_object_show(vbox);

   /* Box to hold buttons */
   hbox = elm_box_add(vbox);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(vbox, hbox);
   evas_object_show(hbox);

   /* Create button */
   if (ok_label || ok_smart_cb)
     {
        o = elm_button_add(hbox);
        elm_object_text_set(o, ok_label ? ok_label : "Ok");
        evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_smart_callback_add(
           o, "clicked",
           ok_smart_cb ? ok_smart_cb : _inwin_close_cb, ed);
        elm_box_pack_end(hbox, o);
        evas_object_show(o);
     }

   /* Cancel button */
   if (cancel_label || cancel_smart_cb)
     {
        o = elm_button_add(hbox);
        elm_object_text_set(o, cancel_label ? cancel_label : "Cancel");
        evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_smart_callback_add(
           o, "clicked",
           cancel_smart_cb ? cancel_smart_cb : _inwin_close_cb, ed);
        elm_box_pack_start(hbox, o);
        evas_object_show(o);
     }

   ed->inwin.id = id;
   elm_win_inwin_content_set(ed->inwin.obj, vbox);
   inwin_activate(ed);
}

void
inwin_dismiss(Editor *ed)
{
   evas_object_hide(ed->inwin.obj);
}

void
inwin_activate(Editor *ed)
{
   elm_win_inwin_activate(ed->inwin.obj);
   evas_object_show(ed->inwin.obj);
}

Eina_Bool
inwin_id_is(const Editor *ed,
            const Inwin   id)
{
   return (ed->inwin.id == id);
}
