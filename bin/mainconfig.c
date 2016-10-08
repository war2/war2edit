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

/*============================================================================*
 *                                  Callbacks                                 *
 *============================================================================*/

static void
_mc_cancel_cb(void        *data,
              Evas_Object *obj  EINA_UNUSED,
              void        *evt  EINA_UNUSED)
{
   Editor *ed = data;
   editor_free(ed);
}

static void
_mc_create_cb(void        *data,
              Evas_Object *obj  EINA_UNUSED,
              void        *evt  EINA_UNUSED)
{
   Editor *ed = data;

   mainconfig_hide(ed);
}

static void
_mc_open_cb(void        *data,
            Evas_Object *obj        EINA_UNUSED,
            void        *event_info EINA_UNUSED)
{
   Editor *const ed = data;
   mainconfig_hide(ed);
   editor_file_selector_add(ed, EINA_FALSE);
}

static void
_size_changed_cb(void        *data,
                 Evas_Object *obj,
                 void        *event_info EINA_UNUSED)
{
   Editor *ed = data;
   int id;

   id = elm_radio_value_get(obj);
   switch (id)
     {
      case 1: pud_dimensions_set(ed->pud, PUD_DIMENSIONS_32_32);   break;
      case 2: pud_dimensions_set(ed->pud, PUD_DIMENSIONS_64_64);   break;
      case 3: pud_dimensions_set(ed->pud, PUD_DIMENSIONS_96_96);   break;
      case 4: pud_dimensions_set(ed->pud, PUD_DIMENSIONS_128_128); break;

      default:
              CRI("Invalid ID for size radio group [%i]", id);
              return;
     }
   bitmap_resize(ed);
   pud_era_set(ed->pud, ed->pud->era);
}


static void
_has_extension_cb(void        *data,
                  Evas_Object *obj,
                  void        *event_info EINA_UNUSED)
{
   Editor *ed = data;
   const Eina_Bool with_extension = !!elm_check_state_get(obj);
   const Eina_Bool no_extension = !with_extension;

   elm_object_disabled_set(ed->menu_swamp_radio, no_extension);
   if ((no_extension) &&
       (elm_radio_value_get(ed->menu_map_radio_group) == PUD_ERA_SWAMP))
     {
        elm_radio_value_set(ed->menu_map_radio_group, PUD_ERA_FOREST);
     }

   ed->pud->extension_pack = with_extension;
}


/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

void
mainconfig_show(Editor *ed)
{
   Evas_Object *o, *box, *b2, *b3, *f, *b, *grp, *bb, *ic;
   struct {
      const char *label;
      const char *icon;
      Evas_Smart_Cb cb;
   } const ctor[] = {
        { "Quit", "document-close", _mc_cancel_cb },
        { "Open", "document-open", _mc_open_cb },
        { "Create", "document-new", _mc_create_cb }
   };
   unsigned int i;

   editor_inwin_add(ed);

   /* Default values */
   ed->pud->extension_pack = EINA_TRUE;
   ed->pud->era = PUD_ERA_FOREST;

   /* Create main box (mainconfig) */
   box = elm_box_add(ed->inwin);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(box, EINA_FALSE);
   evas_object_show(box);

   /* Box to hold map and menus */
   b2 = elm_box_add(box);
   elm_box_horizontal_set(b2, EINA_TRUE);
   evas_object_size_hint_weight_set(b2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_start(box, b2);
   evas_object_show(b2);

   /* Extension checker */
   o = elm_check_add(box);
   elm_object_text_set(o, "Use Warcraft II extension - Beyond the Dark Portal");
   elm_check_state_set(o, EINA_TRUE);
   evas_object_smart_callback_add(o, "changed", _has_extension_cb, ed);
   elm_box_pack_start(box, o);
   evas_object_show(o);

   /* Box to put commands */
   b3 = elm_box_add(b2);
   elm_box_horizontal_set(b3, EINA_FALSE);
   evas_object_size_hint_weight_set(b3, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(b2, b3);
   evas_object_show(b3);

   /* Frame for map size */
   f = elm_frame_add(b3);
   elm_object_text_set(f, "Map Size");
   evas_object_size_hint_weight_set(f, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(f, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_start(b3, f);
   evas_object_show(f);
   b = elm_box_add(f); /* Box */
   elm_object_content_set(f, b);
   elm_box_align_set(b, 0.0, 0.0);
   evas_object_show(b);
   o = elm_radio_add(b); /* Size item 1 */
   elm_radio_state_value_set(o, 1);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(o, "32 x 32");
   elm_box_pack_end(b, o);
   evas_object_show(o);
   grp = o;
   evas_object_smart_callback_add(o, "changed", _size_changed_cb, ed);
   o = elm_radio_add(b); /* Size item 2 */
   elm_radio_state_value_set(o, 2);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(o, "64 x 64");
   elm_box_pack_end(b, o);
   evas_object_show(o);
   elm_radio_group_add(o, grp);
   evas_object_smart_callback_add(o, "changed", _size_changed_cb, ed);
   o = elm_radio_add(b); /* Size item 3 */
   elm_radio_state_value_set(o, 3);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(o, "96 x 96");
   elm_box_pack_end(b, o);
   evas_object_show(o);
   elm_radio_group_add(o, grp);
   evas_object_smart_callback_add(o, "changed", _size_changed_cb, ed);
   o = elm_radio_add(b); /* Size item 4 */
   elm_radio_state_value_set(o, 4);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(o, "128 x 128");
   elm_box_pack_end(b, o);
   evas_object_show(o);
   elm_radio_group_add(o, grp);
   evas_object_smart_callback_add(o, "changed", _size_changed_cb, ed);
   elm_radio_value_set(grp, 1);

   elm_box_pack_end(b3, menu_map_properties_new(ed, b3));

   /* Box of buttons */
   bb = elm_box_add(b3);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(bb, EINA_TRUE);
   evas_object_show(bb);

   for (i = 0; i < EINA_C_ARRAY_LENGTH(ctor); i++)
     {
        o = elm_button_add(bb);
        evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(o, "clicked", ctor[i].cb, ed);
        elm_object_text_set(o, ctor[i].label);
        if (ctor[i].icon)
          {
             ic = elm_icon_add(o);
             elm_icon_standard_set(ic, ctor[i].icon);
             elm_object_part_content_set(o, "icon", ic);
          }
        evas_object_show(o);
        elm_box_pack_end(bb, o);
     }

   elm_box_pack_end(b3, bb);

   /* Show inwin */
   editor_inwin_set(ed, box, "minimal", NULL, NULL, NULL, NULL);
   editor_partial_load(ed);
}

void
mainconfig_hide(Editor *ed)
{
   editor_inwin_dismiss(ed);
}
