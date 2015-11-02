/*
 * toolbar.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

typedef struct
{
   Editor       *ed;
   unsigned int  val;
} Btn_Data;

static Btn_Data *_btn_data_new(unsigned int val, Editor *ed);
static void _btn_data_free(Btn_Data *data);

/*============================================================================*
 *                                  Callbacks                                 *
 *============================================================================*/

static void
_btn_free_cb(void        *data,
             Evas        *e    EINA_UNUSED,
             Evas_Object *obj  EINA_UNUSED,
             void        *info EINA_UNUSED)
{
   _btn_data_free(data);
}

static void
_click_cb(void        *data,
          Evas_Object *obj  EINA_UNUSED,
          void        *info EINA_UNUSED)
{
   Btn_Data *sd = data;

   editor_tb_sel_set(sd->ed, sd->val);

   if (editor_sel_action_get(sd->ed) == EDITOR_SEL_ACTION_SELECTION)
     elm_bitmap_cursor_visibility_set(sd->ed->bitmap, EINA_FALSE);
   else
     elm_bitmap_cursor_visibility_set(sd->ed->bitmap, EINA_TRUE);

   // TODO Reset stuff
   /* Safely unset the unit selection */
   menu_unit_selection_reset(sd->ed);
}


/*============================================================================*
 *                                 Private API                                *
 *============================================================================*/

static Btn_Data *
_btn_data_new(unsigned int  val,
              Editor       *ed)
{
   Btn_Data *data;

   data = malloc(sizeof(*data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);

   data->ed = ed;
   data->val = val;

   return data;
}

static void
_btn_data_free(Btn_Data *data)
{
   free(data);
}

static Evas_Object *
_btn_add(Editor       *ed,
         Evas_Object  *box,
         const char   *img   EINA_UNUSED,
         unsigned int  value)
{
   Evas_Object *o/*, *icon*/;
   Btn_Data *data;

   o = elm_button_add(box);
   EINA_SAFETY_ON_NULL_RETURN_VAL(o, NULL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(o);

   //icon = elm_icon_add(o);
   //EINA_SAFETY_ON_NULL_RETURN_VAL(icon, NULL);
   //elm_image_file_set(icon, img, NULL);
   //elm_image_resizable_set(icon, EINA_FALSE, EINA_FALSE);
   //evas_object_show(icon);

   data = _btn_data_new(value, ed);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);

   if (value == EDITOR_SEL_ACTION_SELECTION)
     elm_object_text_set(o, "O"); // FIXME for now... because my icons are too big
   else
     elm_object_text_set(o, "x"); // FIXME for now... because my icons are too big
   //elm_object_part_content_set(o, "icon", icon);
   evas_object_smart_callback_add(o, "clicked", _click_cb, data);
   evas_object_event_callback_add(o, EVAS_CALLBACK_DEL, _btn_free_cb, data);

   elm_box_pack_end(box, o);
   return o;
}


/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

Eina_Bool
toolbar_add(Editor      *ed,
            Evas_Object *box)
{
#define BTN_ADD(icon_, value_) \
   _btn_add(ed, box, DATA_DIR"/images/"icon_, value_)

   ed->tb.tint[0] = BTN_ADD("light.png", EDITOR_SEL_TINT_LIGHT);
   ed->tb.tint[1] = BTN_ADD("dark.png",  EDITOR_SEL_TINT_DARK);
   ed->tb.spread[0] = BTN_ADD("spread_normal.png", EDITOR_SEL_SPREAD_NORMAL);
   ed->tb.spread[1] = BTN_ADD("spread_circle.png", EDITOR_SEL_SPREAD_CIRCLE);
   ed->tb.spread[2] = BTN_ADD("spread_random.png", EDITOR_SEL_SPREAD_RANDOM);
   ed->tb.radius[0] = BTN_ADD("radius_small.png", EDITOR_SEL_RADIUS_SMALL);
   ed->tb.radius[1] = BTN_ADD("radius_medium.png", EDITOR_SEL_RADIUS_MEDIUM);
   ed->tb.radius[2] = BTN_ADD("radius_big.png", EDITOR_SEL_RADIUS_BIG);

   ed->tb.action[0] = BTN_ADD("magnifying_glass.png", EDITOR_SEL_ACTION_SELECTION);
   ed->tb.action[1] = BTN_ADD("water.png", EDITOR_SEL_ACTION_WATER);
   ed->tb.action[2] = BTN_ADD("mud.png", EDITOR_SEL_ACTION_NON_CONSTRUCTIBLE);
   ed->tb.action[3] = BTN_ADD("grass.png", EDITOR_SEL_ACTION_CONSTRUCTIBLE);
   ed->tb.action[4] = BTN_ADD("trees.png", EDITOR_SEL_ACTION_TREES);
   ed->tb.action[5] = BTN_ADD("rocks.png", EDITOR_SEL_ACTION_ROCKS);
   ed->tb.action[6] = BTN_ADD("human_wall.png", EDITOR_SEL_ACTION_HUMAN_WALLS);
   ed->tb.action[7] = BTN_ADD("orc_wall.png", EDITOR_SEL_ACTION_ORCS_WALLS);

#undef BTN_ADD

   return EINA_TRUE;
}

