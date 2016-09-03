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
   SEG_RUNNER = 4,
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
   DBG("GUI func");

   _enable_all_segments(ed);
   /* FIXME - remove this when circular brush is implemented */
   _subitem_disable(ed, SEG_SPREAD, 1);

   bitmap_cursor_visibility_set(ed, EINA_TRUE);
   switch (editor_sel_action_get(ed))
     {
      case EDITOR_SEL_ACTION_SELECTION:
         bitmap_cursor_visibility_set(ed, EINA_FALSE);
         break;

      case EDITOR_SEL_ACTION_ORC_WALLS:
      case EDITOR_SEL_ACTION_HUMAN_WALLS:
         editor_tb_sel_set(ed, EDITOR_SEL_RADIUS_SMALL);
         elm_object_disabled_set(ed->segs[SEG_RADIUS], EINA_TRUE);
         elm_object_disabled_set(ed->segs[SEG_SPREAD], EINA_TRUE);
         /* Fall through */

      case EDITOR_SEL_ACTION_TREES:
      case EDITOR_SEL_ACTION_ROCKS:
         editor_tb_sel_set(ed, EDITOR_SEL_TINT_LIGHT);
         elm_object_disabled_set(ed->segs[SEG_TINT], EINA_TRUE);
         /* Fall through */

      case EDITOR_SEL_ACTION_WATER:
         editor_tb_sel_set(ed, EDITOR_SEL_SPREAD_NORMAL);
         //_subitem_disable(ed, SEG_SPREAD, 2);
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
_run_cb(void        *data,
        Evas_Object *obj  EINA_UNUSED,
        void        *info EINA_UNUSED)
{
   Editor *ed = data;
   Ecore_Exe *exe;
   char cmd[1024];
   char *dosbox = prefs_value_string_get(PREFS_DOSBOX, "main:dosbox/bin");
   char *disk = prefs_value_string_get(PREFS_DOSBOX, "main:dosbox/bdp/disk");
   char *cdrom = prefs_value_string_get(PREFS_DOSBOX, "main:dosbox/bdp/cdrom");
   char *path = prefs_value_string_get(PREFS_DOSBOX, "main:dosbox/bdp/path");
   char *extra_cmd = prefs_value_string_get(PREFS_DOSBOX, "main:dosbox/cmd");

   snprintf(cmd, sizeof(cmd),
            "%s"
            " -c \"mount C: %s\""
            " -c \"mount D: %s -t cdrom\""
            " %s"
            " -c \"C:\" -c \"%s\"",
            dosbox, disk, cdrom, extra_cmd, path);
   cmd[sizeof(cmd) - 1] = '\0';

   free(dosbox);
   free(disk);
   free(cdrom);
   free(path);
   free(extra_cmd);

   exe = ecore_exe_pipe_run(cmd,
                            ECORE_EXE_PIPE_READ_LINE_BUFFERED |
                            ECORE_EXE_PIPE_ERROR_LINE_BUFFERED, ed);
   if (EINA_UNLIKELY(!exe))
     {
        CRI("Failed to start command [%s]", cmd);
        toolbar_runner_segment_selected_set(ed, EINA_FALSE);
        return;
     }
}


/*============================================================================*
 *                                 Private API                                *
 *============================================================================*/


static Evas_Object *
_segment_add(const Editor  *ed,
             Evas_Object   *box,
             Evas_Smart_Cb  cb)
{
   Evas_Object *o;

   o = elm_segment_control_add(box);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, 0.0, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "changed", cb, ed);
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
   const unsigned int w = count * TB_ICON_WIDTH;

   evas_object_size_hint_min_set(seg, w, TB_ICON_HEIGHT);
   evas_object_size_hint_max_set(seg, w, TB_ICON_HEIGHT);
}

/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

Eina_Bool
toolbar_add(Editor      *ed,
            Evas_Object *box)
{
   Elm_Object_Item *eoi;
   unsigned int i;
   char path[PATH_MAX];

#define SEG_ADD(cb) \
   _segment_add(ed, box, cb)

#define SEG_IT_ADD(seg_, icon_, value_) \
   do { \
      snprintf(path, sizeof(path), "%s/images/%s", \
               elm_app_data_dir_get(), icon_); \
      _segment_item_add(seg_, path, value_); \
   } while (0)

   /* Tint segment */
   ed->segs[0] = SEG_ADD(_seg_changed_cb);
   SEG_IT_ADD(ed->segs[0], "light.png", TL);
   SEG_IT_ADD(ed->segs[0], "dark.png", TD);
   _segment_size_autoset(ed->segs[0], 2);

   /* Spread segmen,t */
   ed->segs[1] = SEG_ADD(_seg_changed_cb);
   SEG_IT_ADD(ed->segs[1], "sel_squared.png", SN);
   SEG_IT_ADD(ed->segs[1], "sel_circular.png", SC);
   SEG_IT_ADD(ed->segs[1], "sel_sparkle.png", SS);
   _segment_size_autoset(ed->segs[1], 3);

   /* Radius segment */
   ed->segs[2] = SEG_ADD(_seg_changed_cb);
   SEG_IT_ADD(ed->segs[2], "brush_small.png", RS);
   SEG_IT_ADD(ed->segs[2], "brush_medium.png", RM);
   SEG_IT_ADD(ed->segs[2], "brush_big.png", RB);
   _segment_size_autoset(ed->segs[2], 3);

   /* Action segment */
   ed->segs[3] = SEG_ADD(_seg_changed_cb);
   SEG_IT_ADD(ed->segs[3], "selection.png", AS);
   SEG_IT_ADD(ed->segs[3], "water.png", AW);
   SEG_IT_ADD(ed->segs[3], "ground.png", AN);
   SEG_IT_ADD(ed->segs[3], "grass.png", AC);
   SEG_IT_ADD(ed->segs[3], "trees.png", AT);
   SEG_IT_ADD(ed->segs[3], "rocks.png", AR);
   SEG_IT_ADD(ed->segs[3], "human_walls.png", AH);
   SEG_IT_ADD(ed->segs[3], "orc_walls.png", AO);
   _segment_size_autoset(ed->segs[3], 8);

   /* Run segment */
   ed->segs[4] = SEG_ADD(_run_cb);
   elm_object_disabled_set(ed->segs[4], ipc_disabled_get());
   SEG_IT_ADD(ed->segs[4], "efl.png", 0);
   _segment_size_autoset(ed->segs[4], 1);

   /* Always select the first item */
   for (i = 0; i < EINA_C_ARRAY_LENGTH(ed->segs); i++)
     {
        if (i == 4) continue; /* Skip run button */

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

void
toolbar_runner_segment_selected_set(Editor    *ed,
                                    Eina_Bool  select)
{
   Elm_Object_Item *eoi;

   /* Reset selection on button */
   eoi = elm_segment_control_item_selected_get(ed->segs[SEG_RUNNER]);
   elm_segment_control_item_selected_set(eoi, select);
}
