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
_segment_add(const Editor *ed,
             Evas_Object  *box)
{
   Evas_Object *o;

   o = elm_segment_control_add(box);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(o);

   return o;
}

static Elm_Object_Item *
_segment_item_add(const Editor *ed,
                  Evas_Object  *seg,
                  const char   *img,
                  unsigned int  val)
{
   Elm_Object_Item *eoi;
   Evas_Object *ic;

   ic = elm_icon_add(seg);
   elm_image_file_set(ic, img, NULL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
   evas_object_show(ic);

   eoi = elm_segment_control_item_add(seg, ic, NULL);
   return eoi;
}



/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

Eina_Bool
toolbar_add(Editor      *ed,
            Evas_Object *box)
{
   Evas_Object *s1, *s2, *s3, *s4;

#define SEG_ADD() \
   _segment_add(ed, box)

#define SEG_IT_ADD(seg_icon_, value_) \
   _segment_item_add(ed, seg_, DATA_DIR"/images/"icon_, value_)

   s1 = SEG_ADD();
   SEG_IT_ADD(s1, "efl.png", EDITOR_SEL_TINT_LIGHT);
   SEG_IT_ADD(s1, "efl.png", EDITOR_SEL_TINT_DARK);

   s2 = SEG_ADD();
   SEG_IT_ADD(s2, "efl.png", EDITOR_SEL_SPREAD_NORMAL);
   SEG_IT_ADD(s2, "efl.png", EDITOR_SEL_SPREAD_CIRCLE);
   SEG_IT_ADD(s2, "efl.png", EDITOR_SEL_SPREAD_RANDOM);

   s3 = SEG_ADD();
   SEG_IT_ADD(s3, "efl.png", EDITOR_SEL_RADIUS_SMALL);
   SEG_IT_ADD(s3, "efl.png", EDITOR_SEL_RADIUS_MEDIUM);
   SEG_IT_ADD(s3, "efl.png", EDITOR_SEL_RADIUS_BIG);

   s4 = SEG_ADD();
   SEG_IT_ADD(s4, "efl.png", EDITOR_SEL_ACTION_SELECTION);
   SEG_IT_ADD(s4, "efl.png", EDITOR_SEL_ACTION_WATER);
   SEG_IT_ADD(s4, "efl.png", EDITOR_SEL_ACTION_NON_CONSTRUCTIBLE);
   SEG_IT_ADD(s4, "efl.png", EDITOR_SEL_ACTION_CONSTRUCTIBLE);
   SEG_IT_ADD(s4, "efl.png", EDITOR_SEL_ACTION_TREES);
   SEG_IT_ADD(s4, "efl.png", EDITOR_SEL_ACTION_ROCKS);
   SEG_IT_ADD(s4, "efl.png", EDITOR_SEL_ACTION_HUMAN_WALLS);
   SEG_IT_ADD(s4, "efl.png", EDITOR_SEL_ACTION_ORCS_WALLS);


   return EINA_TRUE;
}

