/*
 * Copyright (c) 2016 Jean Guyomarc'h
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

typedef struct
{
   Editor *ed;
   Evas_Object *lay;
   Cell *c;
   Unit type;
   Pud_Unit unit;
   unsigned int x;
   unsigned int y;
} Udata;

typedef Evas_Object *(*Ctor)(Evas_Object *vbox, Udata *u);


static Udata *
udata_new(Editor      *ed,
          Evas_Object *lay,
          Cell        *c,
          Unit         type,
          Pud_Unit     unit,
          unsigned int x,
          unsigned int y)
{
   Udata *u;

   u = malloc(sizeof(*u));
   if (EINA_UNLIKELY(!u))
     {
        CRI("Failed to allocate memory");
        return NULL;
     }

   u->ed = ed;
   u->lay = lay;
   u->c = c;
   u->type = type;
   u->unit = unit;
   u->x = x;
   u->y = y;
   return u;
}

static void
udata_free(Udata *u)
{
   free(u);
}

static Eina_Bool
_update_icon(Editor      *ed,
             Evas_Object *lay,
             Pud_Player   col,
             Pud_Unit     unit)
{
   Evas_Object *im;
   const char part[] = "war2edit.unitselector.icon";

   /* We may want to update the image */
   im = elm_layout_content_unset(lay, part);
   if (im) evas_object_del(im);

   im = editor_icon_image_new(lay, pud_unit_icon_get(unit), ed->pud->era, col);
   elm_layout_content_set(lay, part, im);

   return EINA_TRUE;
}

static void
_free_cb(void        *data,
         Evas        *e    EINA_UNUSED,
         Evas_Object *obj  EINA_UNUSED,
         void        *info EINA_UNUSED)
{
   Udata *const u = data;
   udata_free(u);
}

static void
_radio_cb(void        *data,
          Evas_Object *obj,
          void        *info EINA_UNUSED)
{
   Udata *const u = data;
   Pud_Player sel, old;

   snapshot_push(u->ed);
   sel = elm_radio_value_get(obj);
   switch (u->type)
     {
      case UNIT_BELOW:
         old = u->c->player_below;
         u->c->player_below = sel;
         break;

      case UNIT_ABOVE:
         old = u->c->player_above;
         u->c->player_above = sel;
         break;

      default:
         CRI("Unhandled type 0x%x", u->type);
         snapshot_push_done(u->ed);
         return;
     }

   if (pud_side_for_player_get(u->ed->pud, old) != pud_side_for_player_get(u->ed->pud, sel))
     {

        u->unit = pud_unit_switch_side(u->unit);
        if (u->type == UNIT_BELOW)
          u->c->unit_below = u->unit;
        else
          u->c->unit_above = u->unit;

        editor_unit_unref(u->ed, u->x, u->y, u->type);
        editor_unit_ref(u->ed, u->x, u->y, u->type);

        elm_layout_text_set(u->lay, "war2edit.unitselector.name",
                            pud_unit_to_string(u->unit, PUD_TRUE));
     }
   snapshot_push_done(u->ed);
   _update_icon(u->ed, u->lay, sel, u->unit);
   bitmap_refresh(u->ed, NULL); // XXX Not cool
}


static Evas_Object *
_radio_add(Evas_Object *parent,
           Evas_Object *group,
           Pud_Player   player,
           int          col,
           int          row,
           Udata       *u)
{
   Evas_Object *o;
   const char *strings[8] = {
      STR_PLAYER_1, STR_PLAYER_2, STR_PLAYER_3, STR_PLAYER_4,
      STR_PLAYER_5, STR_PLAYER_6, STR_PLAYER_7, STR_PLAYER_8,
   };

   o = elm_radio_add(parent);
   elm_radio_state_value_set(o, player);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(o, strings[player]);
   elm_table_pack(parent, o, col, row, 1, 1);
   evas_object_show(o);

   if (group) elm_radio_group_add(o, group);
   evas_object_smart_callback_add(o, "changed", _radio_cb, u);

   return o;
}

static void
_res_edit_cb(void        *data,
             Evas_Object *obj,
             void        *info EINA_UNUSED)
{
   Udata *const u = data;
   const char *text;
   int len;
   char *ptr;
   unsigned long int res;

   text = elm_object_text_get(obj);
   len = strlen(text);

   res = strtoul(text, &ptr, 10);
   if ((res == 0) || (ptr != text + len) || (res > 490000) || (res < 2500))
     {
        ERR("Invalid value %lu (text: \"%s\")", res, text);
        // TODO Visual info
     }
   else
     {
        DBG("Resource value is %lu", res);
        u->c->alter_below = res / 2500;
     }
}


static Evas_Object *
_player_ctor(Evas_Object *vbox,
             Udata       *u)
{
   Evas_Object *f, *t, *grp = NULL;
   unsigned int i;

   f = elm_frame_add(vbox);
   elm_object_text_set(f, "Player's Allegiance");
   evas_object_size_hint_weight_set(f, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(f, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(f);

   t = elm_table_add(f);
   evas_object_size_hint_weight_set(t, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(t, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_homogeneous_set(t, EINA_TRUE);
   elm_object_content_set(f, t);
   evas_object_show(t);

   for (i = PUD_PLAYER_RED; i <= PUD_PLAYER_YELLOW; i++)
     grp = _radio_add(t, grp, i, i & 0x3, i >> 2, u);

   cell_unit_get(u->c, u->type, NULL, &i);

   elm_radio_value_set(grp, i);

   return f;
}

static Evas_Object *
_res_ctor(Evas_Object *vbox,
          Udata       *u)
{
   Evas_Object *f, *e;
   char buf[32];

   f = elm_frame_add(vbox);
   elm_object_text_set(f, "Natural Resources");
   evas_object_size_hint_weight_set(f, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(f, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(f);

   e = elm_entry_add(f);
   evas_object_size_hint_weight_set(e, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(e, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_entry_single_line_set(e, EINA_TRUE);
   elm_entry_editable_set(e, EINA_TRUE);
   elm_entry_scrollable_set(e, EINA_TRUE);
   evas_object_show(e);

   snprintf(buf, sizeof(buf), "%u", u->c->alter_below * 2500);
   buf[sizeof(buf) - 1] = '\0';

   evas_object_smart_callback_add(e, "changed,user", _res_edit_cb, u);
   elm_object_content_set(f, e);
   elm_object_text_set(e, buf);

   return f;
}

static Evas_Object *
_provide_unit_handler(Editor *ed,
                      Evas_Object *parent,
                      const Cell  *c,
                      unsigned int x,
                      unsigned int y,
                      Unit         unit_type)
{
   const char wdg_group[] = "war2edit/unitselector/widget";
   Evas_Object *lay, *vbox, *o, *f;
   Eina_Bool chk;
   Pud_Player player;
   const Ctor ctor[2] = {
      _player_ctor,
      _res_ctor,
   };
   enum {
      CTOR_NONE = 0,
      CTOR_PLAYER = (1 << 0),
      CTOR_RES = (1 << 1),
   };
   unsigned int ctor_taken = CTOR_NONE, i;
   const char *desc;
   Pud_Unit unit;
   char buf[32];
   Udata *u;

   f = elm_frame_add(parent);
   evas_object_size_hint_weight_set(f, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(f, EVAS_HINT_FILL, 0.0);
   evas_object_show(f);

   lay = elm_layout_add(f);
   chk = elm_layout_file_set(lay, main_edje_file_get(), wdg_group);
   if (EINA_UNLIKELY(!chk))
     {
        CRI("Failed to set edje file from %s for group %s",
            main_edje_file_get(), wdg_group);
        evas_object_del(lay);
        return NULL;
     }
   evas_object_size_hint_weight_set(lay, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lay, EVAS_HINT_FILL, 0.0);

   vbox = elm_box_add(lay);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(vbox, EVAS_HINT_FILL, 0.0);
   elm_box_horizontal_set(vbox, EINA_FALSE);
   elm_box_homogeneous_set(vbox, EINA_FALSE);
   evas_object_show(vbox);

   switch (unit_type)
     {
      case UNIT_BELOW:
         if ((c->unit_below == PUD_UNIT_GOLD_MINE) ||
             (c->unit_below == PUD_UNIT_OIL_PATCH))
           {
              ctor_taken |= CTOR_RES;
           }
         else if ((c->unit_below == PUD_UNIT_HUMAN_OIL_WELL) ||
                  (c->unit_below == PUD_UNIT_ORC_OIL_WELL))
           {
              ctor_taken |= CTOR_RES;
              ctor_taken |= CTOR_PLAYER;
           }
         else if ((c->unit_below != PUD_UNIT_CRITTER) &&
                  (c->unit_below != PUD_UNIT_CIRCLE_OF_POWER) &&
                  (c->unit_below != PUD_UNIT_DARK_PORTAL))
           {
              ctor_taken |= CTOR_PLAYER;
           }
         desc = "Bottom Layer Unit";
         unit = c->unit_below;
         player = c->player_below;
         break;

      case UNIT_START_LOCATION:
         if (c->start_location_human)
           unit = PUD_UNIT_HUMAN_START;
         else
           unit = PUD_UNIT_ORC_START;
         desc = "Player Start Location";
         player = c->start_location;
         break;

      case UNIT_ABOVE:
         desc = "Top Layer Unit";
         ctor_taken |= CTOR_PLAYER;
         unit = c->unit_above;
         player = c->player_above;
         break;

      case UNIT_NONE:
      default:
         CRI("Invalid unit type 0x%x", unit_type);
         evas_object_del(f);
         return NULL;
     }


   _update_icon(ed, lay, player, unit);

   for (i = 0; i < EINA_C_ARRAY_LENGTH(ctor); i++)
     {
        if ((ctor_taken >> i) & 0x1)
          {
             /* FIXME Where do we free this? And check for return */
             u = udata_new(ed, lay, (Cell *)c, unit_type, unit, x, y);
             o = ctor[i](vbox, u);
             evas_object_event_callback_add(o, EVAS_CALLBACK_FREE, _free_cb, u);
             elm_box_pack_end(vbox, o);
             evas_object_show(o);
          }
     }

   snprintf(buf, sizeof(buf), "Coordinates: X=%u, Y=%u",
            ed->unitselector.x, ed->unitselector.y);
   elm_layout_text_set(lay, "war2edit.unitselector.name", pud_unit_to_string(unit, PUD_TRUE));
   elm_layout_text_set(lay, "war2edit.unitselector.info", buf);
   elm_layout_content_set(lay, "war2edit.unitselector.contents", vbox);
   evas_object_show(lay);
   elm_object_text_set(f, desc);
   elm_object_content_set(f, lay);

   return f;
}

static Edje_Object *
_sel_add(Editor *ed,
         unsigned int cx,
         unsigned int cy,
         unsigned int w,
         unsigned int h)
{
   int px, py, sx, sy, rx, ry, cell_w, cell_h, cx1, cy1, cw, ch;
   Edje_Object *o;
   Eina_Bool chk;
   const char group[] = "war2edit/unitselector/sel";

   bitmap_cells_to_coords(ed, cx, cy, &px, &py);
   elm_scroller_region_get(ed->scroller, &sx, &sy, NULL, NULL);
   evas_object_geometry_get(ed->scroller, &rx, &ry, NULL, NULL);
   bitmap_cell_size_get(ed, &cell_w, &cell_h);

   cx1 = rx - sx + (cx * cell_w);
   cy1 = ry - sy + (cy * cell_h);
   cw = w * cell_w;
   ch = h * cell_h;

   o = edje_object_add(ed->lay);
   chk = edje_object_file_set(o, main_edje_file_get(), group);
   if (EINA_UNLIKELY(!chk))
     {
        CRI("Failed to set edje object layout");
        evas_object_del(o);
        return NULL;
     }
   evas_object_smart_member_add(o, ed->edje);

   evas_object_resize(o, cw, ch);
   evas_object_move(o, cx1, cy1);
   evas_object_show(o);

   return o;
}

static Evas_Object *
_unitselector_add(Editor       *ed,
                  Evas_Object  *parent,
                  unsigned int  x,
                  unsigned int  y)
{
   Evas_Object *vbox, *o;
   unsigned int cx, cy;
   Cell *c;

   vbox = elm_box_add(parent);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(vbox, EVAS_HINT_FILL, 0.0);
   elm_box_horizontal_set(vbox, EINA_FALSE);
   elm_box_homogeneous_set(vbox, EINA_FALSE);
   evas_object_show(vbox);

   c = cell_anchor_pos_get(ed->cells, x, y, &cx, &cy, EINA_TRUE);
   if (c && (c->unit_below != PUD_UNIT_NONE))
     {
        o = _provide_unit_handler(ed, vbox, c, cx, cy, UNIT_BELOW);
        elm_box_pack_end(vbox, o);
        ed->unitselector.sel[0] = _sel_add(ed, cx, cy,
                                           c->spread_x_below,
                                           c->spread_y_below);
     }
   c = cell_anchor_pos_get(ed->cells, x, y, &cx, &cy, EINA_FALSE);
   if (c && (c->unit_above != PUD_UNIT_NONE))
     {
        o = _provide_unit_handler(ed, vbox, c, cx, cy, UNIT_ABOVE);
        elm_box_pack_end(vbox, o);
        ed->unitselector.sel[1] = _sel_add(ed, cx, cy,
                                           c->spread_x_above,
                                           c->spread_y_above);
     }
   c = &(ed->cells[y][x]);
   if (c->start_location != CELL_NOT_START_LOCATION)
     {
        o = _provide_unit_handler(ed, vbox, c, cx, cy, UNIT_START_LOCATION);
        elm_box_pack_end(vbox, o);
        ed->unitselector.sel[2] = _sel_add(ed, cx, cy, 1, 1);
     }
   evas_object_smart_member_add(vbox, ed->edje);

   return vbox;
}

static void
_unitselector_event_cb(void        *data,
                       Evas_Object *obj      EINA_UNUSED,
                       const char  *emission EINA_UNUSED,
                       const char  *source   EINA_UNUSED)
{
   Editor *const ed = data;
   unitselector_hide(ed);
}

void
unitselector_hide(Editor *ed)
{
   Evas_Object *o;
   unsigned int i;

   elm_layout_signal_emit(ed->lay, "war2edit,unitselector,hide", "war2edit");
   o = elm_layout_content_unset(ed->lay, "war2edit.main.unitselector");

   for (i = 0; i < EINA_C_ARRAY_LENGTH(ed->unitselector.sel); i++)
     {
        evas_object_del(ed->unitselector.sel[i]);
        ed->unitselector.sel[i] = NULL;
     }
   evas_object_del(o);
}

void
unitselector_show(Editor       *ed,
                  unsigned int  x,
                  unsigned int  y)
{
   const Cell *c;
   Evas_Object *o;
   const char part[] = "war2edit.main.unitselector";
   unsigned int i;

   c = &(ed->cells[y][x]);

   /* No unit on the cell - do nothing */
   if ((c->unit_above == PUD_UNIT_NONE) &&
       (c->unit_below == PUD_UNIT_NONE) &&
       (c->start_location == CELL_NOT_START_LOCATION))
     {
        return;
     }

   ed->unitselector.x = x;
   ed->unitselector.y = y;

   o = _unitselector_add(ed, ed->lay, x, y);

   elm_layout_content_set(ed->lay, part, o);
   elm_layout_signal_emit(ed->lay, "war2edit,unitselector,show", "war2edit");
   elm_layout_signal_callback_add(ed->lay,
                                  "war2edit,unitselector,event", "war2edit",
                                  _unitselector_event_cb, ed);
   for (i = 0; i < EINA_C_ARRAY_LENGTH(ed->unitselector.sel); i++)
     {
        evas_object_stack_below(ed->unitselector.sel[i], o);
     }
}
