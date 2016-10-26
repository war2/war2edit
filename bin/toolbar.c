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

#define TB_ICON_WIDTH      50
#define TB_ICON_HEIGHT     50

typedef enum
{
   SEG_TINT   = 0,
   SEG_SPREAD = 1,
   SEG_RADIUS = 2,
   SEG_ACTION = 3,
} Seg;


/* Shortcuts, to index an array */
enum
{
   SN, SC, SS, /* spreads */
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
   [SS] = EDITOR_SEL_SPREAD_SPECIAL,

   [RS] = EDITOR_SEL_RADIUS_SMALL,
   [RM] = EDITOR_SEL_RADIUS_MEDIUM,
   [RB] = EDITOR_SEL_RADIUS_BIG,

   [TL] = EDITOR_SEL_TINT_LIGHT,
   [TD] = EDITOR_SEL_TINT_DARK,

   [AS] = EDITOR_SEL_ACTION_SELECTION,
   [AW] = EDITOR_SEL_ACTION_WATER,
   [AN] = EDITOR_SEL_ACTION_GROUND,
   [AC] = EDITOR_SEL_ACTION_GRASS,
   [AT] = EDITOR_SEL_ACTION_TREES,
   [AR] = EDITOR_SEL_ACTION_ROCKS,
   [AH] = EDITOR_SEL_ACTION_HUMAN_WALLS,
   [AO] = EDITOR_SEL_ACTION_ORC_WALLS
};



static void
_enable_all_segments(Editor *ed)
{
   unsigned int i, c, j;
   Elm_Object_Item *eoi;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(ed->segs); i++)
     {
        elm_object_disabled_set(ed->segs[i], EINA_FALSE);
        c = (unsigned int)elm_segment_control_item_count_get(ed->segs[i]);
        for (j = 0; j < c; j++)
          {
             eoi = elm_segment_control_item_get(ed->segs[i], j);
             elm_object_item_disabled_set(eoi, EINA_FALSE);
          }
     }
}

static inline void
_subitem_disable(const Editor *ed,
                 Seg           segment,
                 unsigned int  item)
{
   Elm_Object_Item *eoi;
   eoi = elm_segment_control_item_get(ed->segs[segment], item);
   elm_object_item_disabled_set(eoi, EINA_TRUE);
}


/*============================================================================*
 *                                  Callbacks                                 *
 *============================================================================*/

static void
_seg_changed_cb(void        *data,
                Evas_Object *obj,
                void        *info EINA_UNUSED)
{
   Editor *const ed = data;
   Editor_Sel sel;
   Elm_Object_Item *eoi;

   eoi = elm_segment_control_item_selected_get(obj);
   sel = *((uint16_t *)elm_object_item_data_get(eoi));

   editor_tb_sel_set(ed, sel);

   _enable_all_segments(ed);

   bitmap_cursor_visibility_set(ed, EINA_TRUE);
   bitmap_cursor_size_set(ed, 1, 1);

   switch (editor_sel_action_get(ed))
     {
      case EDITOR_SEL_ACTION_SELECTION:
         bitmap_cursor_visibility_set(ed, EINA_FALSE);
         break;

      case EDITOR_SEL_ACTION_ORC_WALLS:
      case EDITOR_SEL_ACTION_HUMAN_WALLS:
         editor_sel_radius_set(ed, EDITOR_SEL_RADIUS_SMALL);
         elm_object_disabled_set(ed->segs[SEG_RADIUS], EINA_TRUE);
         elm_object_disabled_set(ed->segs[SEG_SPREAD], EINA_TRUE);
         /* Fall through */

      case EDITOR_SEL_ACTION_TREES:
      case EDITOR_SEL_ACTION_ROCKS:
         editor_sel_tint_set(ed, EDITOR_SEL_TINT_LIGHT);
         elm_object_disabled_set(ed->segs[SEG_TINT], EINA_TRUE);
         /* Fall through */

      case EDITOR_SEL_ACTION_WATER:
         if (editor_sel_spread_get(ed) == EDITOR_SEL_SPREAD_SPECIAL)
           editor_sel_spread_set(ed, EDITOR_SEL_SPREAD_NORMAL);
         _subitem_disable(ed, SEG_SPREAD, 2);
         break;

      default:
        bitmap_cursor_visibility_set(ed, EINA_TRUE);
        bitmap_cursor_size_set(ed, 1, 1);
        break;
     }

   /* Safely unset the unit selection */
   menu_unit_selection_reset(ed);
}

static void
_seg_action_changed_cb(void        *data,
                       Evas_Object *obj,
                       void        *info)
{
   Editor *const ed = data;
   _seg_changed_cb(ed, obj, info);
}


/*============================================================================*
 *                                 Private API                                *
 *============================================================================*/

static Evas_Object *
_segment_add(const Editor  *ed,
             Evas_Object   *parent,
             Evas_Smart_Cb  cb)
{
   Evas_Object *o;

   o = elm_segment_control_add(parent);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, 0.0, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "changed", cb, ed);
   evas_object_show(o);

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
   evas_object_size_hint_align_set(ic, 0.0, EVAS_HINT_FILL);
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
   const unsigned int w = count * TB_ICON_WIDTH;

   evas_object_size_hint_min_set(seg, w, TB_ICON_HEIGHT);
   evas_object_size_hint_max_set(seg, w, TB_ICON_HEIGHT);
}

/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

Eina_Bool
toolbar_add(Editor      *ed,
            Evas_Object *parent)
{
   Elm_Object_Item *eoi;
   unsigned int i;
   char path[PATH_MAX];

#define SEG_ADD(cb) \
   _segment_add(ed, parent, cb)

#define SEG_IT_ADD(seg_, icon_, value_) \
   do { \
      snprintf(path, sizeof(path), "%s/images/%s", \
               elm_app_data_dir_get(), icon_); \
      _segment_item_add(seg_, path, value_); \
   } while (0)

   /* Tint segment */
   ed->segs[SEG_TINT] = SEG_ADD(_seg_changed_cb);
   SEG_IT_ADD(ed->segs[SEG_TINT], "light.png", TL);
   SEG_IT_ADD(ed->segs[SEG_TINT], "dark.png", TD);
   _segment_size_autoset(ed->segs[SEG_TINT], 2);

   /* Spread segmen,t */
   ed->segs[SEG_SPREAD] = SEG_ADD(_seg_changed_cb);
   SEG_IT_ADD(ed->segs[SEG_SPREAD], "sel_squared.png", SN);
   SEG_IT_ADD(ed->segs[SEG_SPREAD], "sel_circular.png", SC);
   SEG_IT_ADD(ed->segs[SEG_SPREAD], "sel_sparkle.png", SS);
   _segment_size_autoset(ed->segs[SEG_SPREAD], 3);

   /* Radius segment */
   ed->segs[SEG_RADIUS] = SEG_ADD(_seg_changed_cb);
   SEG_IT_ADD(ed->segs[SEG_RADIUS], "brush_small.png", RS);
   SEG_IT_ADD(ed->segs[SEG_RADIUS], "brush_medium.png", RM);
   SEG_IT_ADD(ed->segs[SEG_RADIUS], "brush_big.png", RB);
   _segment_size_autoset(ed->segs[SEG_RADIUS], 3);

   /* Action segment */
   ed->segs[SEG_ACTION] = SEG_ADD(_seg_action_changed_cb);
   SEG_IT_ADD(ed->segs[SEG_ACTION], "selection.png", AS);
   SEG_IT_ADD(ed->segs[SEG_ACTION], "water.png", AW);
   SEG_IT_ADD(ed->segs[SEG_ACTION], "ground.png", AN);
   SEG_IT_ADD(ed->segs[SEG_ACTION], "grass.png", AC);
   SEG_IT_ADD(ed->segs[SEG_ACTION], "trees.png", AT);
   SEG_IT_ADD(ed->segs[SEG_ACTION], "rocks.png", AR);
   SEG_IT_ADD(ed->segs[SEG_ACTION], "human_walls.png", AH);
   SEG_IT_ADD(ed->segs[SEG_ACTION], "orc_walls.png", AO);
   _segment_size_autoset(ed->segs[SEG_ACTION], 8);

   /* Always select the first item */
   for (i = 0; i < EINA_C_ARRAY_LENGTH(ed->segs); i++)
     {
        eoi = elm_segment_control_item_get(ed->segs[i], 0);
        elm_segment_control_item_selected_set(eoi, EINA_TRUE);
     }

   return EINA_TRUE;
}

void
toolbar_actions_segment_unselect(const Editor *ed)
{
   Elm_Object_Item *eoi;

   eoi = elm_segment_control_item_selected_get(ed->segs[SEG_ACTION]);
   elm_segment_control_item_selected_set(eoi, EINA_FALSE);
}
