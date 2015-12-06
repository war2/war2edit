/*
 * toolbar.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

#define ICON_WIDTH      50
#define ICON_HEIGHT     50

/* Shortcuts, to index an array */
enum
{
   SN, SC, SR, /* spreads */
   RS, RM, RB, /* radius */
   TL, TD, /* tints */
   AS, AW, AN, AC, AT, AR, AH, AO /* actions */
};

/* Used to avoid to manage dynamic memory allocation for every single
 * toolbar subitem. 16bits to occupy less memory, and because
 * Editor_Sel will never overflow 16 bits. */
static const uint16_t _selections[] = {
   [SN] = EDITOR_SEL_SPREAD_NORMAL,
   [SC] = EDITOR_SEL_SPREAD_CIRCLE,
   [SR] = EDITOR_SEL_SPREAD_RANDOM,

   [RS] = EDITOR_SEL_RADIUS_SMALL,
   [RM] = EDITOR_SEL_RADIUS_MEDIUM,
   [RB] = EDITOR_SEL_RADIUS_BIG,

   [TL] = EDITOR_SEL_TINT_LIGHT,
   [TD] = EDITOR_SEL_TINT_DARK,

   [AS] = EDITOR_SEL_ACTION_SELECTION,
   [AW] = EDITOR_SEL_ACTION_WATER,
   [AN] = EDITOR_SEL_ACTION_NON_CONSTRUCTIBLE,
   [AC] = EDITOR_SEL_ACTION_CONSTRUCTIBLE,
   [AT] = EDITOR_SEL_ACTION_TREES,
   [AR] = EDITOR_SEL_ACTION_ROCKS,
   [AH] = EDITOR_SEL_ACTION_HUMAN_WALLS,
   [AO] = EDITOR_SEL_ACTION_ORCS_WALLS
};


/*============================================================================*
 *                                  Callbacks                                 *
 *============================================================================*/

static void
_seg_changed_cb(void        *data,
                Evas_Object *obj  EINA_UNUSED,
                void        *info EINA_UNUSED)
{
   Editor *ed = data;
   Editor_Sel sel;
   Elm_Object_Item *eoi;

   eoi = elm_segment_control_item_selected_get(obj);
   sel = *((uint16_t *)elm_object_item_data_get(eoi));

   editor_tb_sel_set(ed, sel);
   if (editor_sel_action_get(ed) == EDITOR_SEL_ACTION_SELECTION)
     elm_bitmap_cursor_visibility_set(ed->bitmap, EINA_FALSE);
   else
     {
        elm_bitmap_cursor_visibility_set(ed->bitmap, EINA_TRUE);
        elm_bitmap_cursor_size_set(ed->bitmap, 1, 1);
     }

   // TODO Reset stuff
   /* Safely unset the unit selection */
   menu_unit_selection_reset(ed);
}


/*============================================================================*
 *                                 Private API                                *
 *============================================================================*/


static Evas_Object *
_segment_add(const Editor *ed,
             Evas_Object  *box)
{
   Evas_Object *o;

   o = elm_segment_control_add(box);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, 0.0, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "changed", _seg_changed_cb, ed);
   evas_object_show(o);

   elm_box_pack_end(box, o);

   return o;
}

static Elm_Object_Item *
_segment_item_add(Evas_Object  *seg,
                  const char   *img,
                  unsigned int  val)
{
   Elm_Object_Item *eoi;
   Evas_Object *ic;

   ic = elm_icon_add(seg);
   elm_image_file_set(ic, img, NULL);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(ic, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
   evas_object_show(ic);

   eoi = elm_segment_control_item_add(seg, ic, NULL);
   elm_object_item_data_set(eoi, (void *)(&(_selections[val])));

   return eoi;
}

static void
_segment_size_autoset(Evas_Object  *seg,
                      unsigned int  count)
{
   const unsigned int w = count * ICON_WIDTH;

   evas_object_size_hint_min_set(seg, w, ICON_HEIGHT);
   evas_object_size_hint_max_set(seg, w, ICON_HEIGHT);
}

/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

Eina_Bool
toolbar_add(Editor      *ed,
            Evas_Object *box)
{
   Evas_Object *s[4];
   Elm_Object_Item *eoi;
   unsigned int i;

#define SEG_ADD() \
   _segment_add(ed, box)

#define SEG_IT_ADD(seg_, icon_, value_) \
   _segment_item_add(seg_, DATA_DIR"/images/"icon_, value_)

   /* Tint segment */
   s[0] = SEG_ADD();
   SEG_IT_ADD(s[0], "efl.png", TL);
   SEG_IT_ADD(s[0], "efl.png", TD);
   _segment_size_autoset(s[0], 2);

   /* Spread segmen,t */
   s[1] = SEG_ADD();
   SEG_IT_ADD(s[1], "efl.png", SN);
   SEG_IT_ADD(s[1], "efl.png", SC);
   SEG_IT_ADD(s[1], "efl.png", SR);
   _segment_size_autoset(s[1], 3);

   /* Radius segment */
   s[2] = SEG_ADD();
   SEG_IT_ADD(s[2], "efl.png", RS);
   SEG_IT_ADD(s[2], "efl.png", RM);
   SEG_IT_ADD(s[2], "efl.png", RB);
   _segment_size_autoset(s[2], 3);

   /* Action segment */
   s[3] = SEG_ADD();
   SEG_IT_ADD(s[3], "efl.png", AS);
   SEG_IT_ADD(s[3], "efl.png", AW);
   SEG_IT_ADD(s[3], "efl.png", AN);
   SEG_IT_ADD(s[3], "efl.png", AC);
   SEG_IT_ADD(s[3], "efl.png", AT);
   SEG_IT_ADD(s[3], "efl.png", AR);
   SEG_IT_ADD(s[3], "efl.png", AH);
   SEG_IT_ADD(s[3], "efl.png", AO);
   _segment_size_autoset(s[3], 8);

   /* Always select the first item */
   for (i = 0; i < EINA_C_ARRAY_LENGTH(s); i++)
     {
        eoi = elm_segment_control_item_get(s[i], 0);
        elm_segment_control_item_selected_set(eoi, EINA_TRUE);
     }

   /* Keep track of the action segment */
   ed->actions = s[3];

   return EINA_TRUE;
}

void
toolbar_actions_segment_unselect(const Editor *ed)
{
   Elm_Object_Item *eoi;

   eoi = elm_segment_control_item_selected_get(ed->actions);
   elm_segment_control_item_selected_set(eoi, EINA_FALSE);
}

