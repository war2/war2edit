/*
 * toolbar.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

typedef struct
{
   unsigned int *bind;
   Editor       *ed;
   unsigned int  val;
   unsigned int  type;
} Btn_Data;

static Btn_Data *_btn_data_new(unsigned int *bind, unsigned int val,
                               Editor *ed, unsigned int type);
static void _btn_data_free(Btn_Data *data);

typedef enum
{
   TINT,
   SPREAD,
   RADIUS,
   ACTION
} Btn_Type;


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

   *(sd->bind) = sd->val;

   if ((sd->type == ACTION) && (sd->val == EDITOR_ACTION_SELECTION))
     {
        elm_bitmap_cursor_visibility_set(sd->ed->bitmap, EINA_FALSE);
     }
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
_btn_data_new(unsigned int *bind,
              unsigned int  val,
              Editor       *ed,
              unsigned int  type)
{
   Btn_Data *data;

   data = malloc(sizeof(*data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);

   data->bind = bind;
   data->ed = ed;
   data->val = val;
   data->type = type;

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
         unsigned int *bind,
         unsigned int  value,
         unsigned int  type)
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

   data = _btn_data_new(bind, value, ed, type);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);

   if (type == ACTION && value == EDITOR_ACTION_SELECTION)
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
#define BTN_ADD(icon_, bind_, value_, type_) \
   _btn_add(ed, box, DATA_DIR"/images/"icon_, bind_, value_, type_)

   ed->tb.tint[0] = BTN_ADD("light.png", &(ed->tint), EDITOR_TINT_LIGHT, TINT);
   ed->tb.tint[1] = BTN_ADD("dark.png",  &(ed->tint), EDITOR_TINT_DARK,  TINT);

   ed->tb.spread[0] = BTN_ADD("spread_normal.png", &(ed->spread),
                              EDITOR_SPREAD_NORMAL, SPREAD);
   ed->tb.spread[1] = BTN_ADD("spread_circle.png", &(ed->spread),
                              EDITOR_SPREAD_CIRCLE, SPREAD);
   ed->tb.spread[2] = BTN_ADD("spread_random.png", &(ed->spread),
                              EDITOR_SPREAD_RANDOM, SPREAD);

   ed->tb.radius[0] = BTN_ADD("radius_small.png", &(ed->radius),
                              EDITOR_RADIUS_SMALL, RADIUS);
   ed->tb.radius[1] = BTN_ADD("radius_medium.png", &(ed->radius),
                              EDITOR_RADIUS_MEDIUM, RADIUS);
   ed->tb.radius[2] = BTN_ADD("radius_big.png",&(ed->radius),
                              EDITOR_RADIUS_BIG, RADIUS);

   ed->tb.action[0] = BTN_ADD("magnifying_glass.png", &(ed->action),
                              EDITOR_ACTION_SELECTION, ACTION);
   ed->tb.action[1] = BTN_ADD("water.png", &(ed->action),
                              EDITOR_ACTION_WATER, ACTION);
   ed->tb.action[2] = BTN_ADD("mud.png", &(ed->action),
                              EDITOR_ACTION_NON_CONSTRUCTIBLE, ACTION);
   ed->tb.action[3] = BTN_ADD("grass.png", &(ed->action),
                              EDITOR_ACTION_CONSTRUCTIBLE, ACTION);
   ed->tb.action[4] = BTN_ADD("trees.png", &(ed->action),
                              EDITOR_ACTION_TREES, ACTION);
   ed->tb.action[5] = BTN_ADD("rocks.png", &(ed->action),
                              EDITOR_ACTION_ROCKS, ACTION);
   ed->tb.action[6] = BTN_ADD("human_wall.png", &(ed->action),
                              EDITOR_ACTION_HUMAN_WALLS, ACTION);
   ed->tb.action[7] = BTN_ADD("orc_wall.png", &(ed->action),
                              EDITOR_ACTION_ORCS_WALLS, ACTION);

#undef BTN_ADD

   return EINA_TRUE;
}

