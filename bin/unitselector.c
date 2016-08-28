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
   Cell *c;
   Unit type;
} Udata;

typedef Evas_Object *(*Ctor)(Evas_Object *vbox, Udata *u);


static Udata *
udata_new(Editor *ed,
          Cell   *c,
          Unit    type)
{
   Udata *u;

   u = malloc(sizeof(*u));
   if (EINA_UNLIKELY(!u))
     {
        CRI("Failed to allocate memory");
        return NULL;
     }

   u->ed = ed;
   u->c = c;
   u->type = type;
   return u;
}

static void
udata_free(Udata *u)
{
   free(u);
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
   Pud_Player sel;

   sel = elm_radio_value_get(obj);
   switch (u->type)
     {
      case UNIT_BELOW:
         u->c->player_below = sel;
         break;

      case UNIT_ABOVE:
         u->c->player_above = sel;
         break;

      default:
         CRI("Unhandled type 0x%x", u->type);
         return;
     }

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

   switch (u->type)
     {
      case UNIT_BELOW:
         i = u->c->player_below;
         break;

      case UNIT_ABOVE:
         i = u->c->player_above;
         break;

      default:
         CRI("Unhandled type 0x%x", u->type);
         return f;
     }

   elm_radio_value_set(grp, i);

   return f;
}


static Evas_Object *
_provide_unit_handler(Editor *ed,
                      Evas_Object *parent,
                      const Cell  *c,
                      Unit         unit_type)
{
   const char wdg_group[] = "war2edit/unitselector/widget";
   Evas_Object *lay, *vbox, *o, *f;
   Eina_Bool chk;
   const Ctor ctor[1] = {
      _player_ctor
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
   chk = elm_layout_file_set(lay, ed->edje_file, wdg_group);
   if (EINA_UNLIKELY(!chk))
     {
        CRI("Failed to set edje file from %s for group %s", ed->edje_file, wdg_group);
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
         else
           {
              ctor_taken |= CTOR_PLAYER;
           }
         desc = "Bottom Layer Unit";
         unit = c->unit_below;
         break;

      case UNIT_START_LOCATION:
         if (c->start_location_human)
           unit = PUD_UNIT_HUMAN_START;
         else
           unit = PUD_UNIT_ORC_START;
         desc = "Player Start Location";
         break;

      case UNIT_ABOVE:
         desc = "Top Layer Unit";
         ctor_taken |= CTOR_PLAYER;
         unit = c->unit_above;
         break;

      case UNIT_NONE:
      default:
         CRI("Invalid unit type 0x%x", unit_type);
         evas_object_del(f);
         return NULL;
     }

 
   for (i = 0; i < EINA_C_ARRAY_LENGTH(ctor); i++)
     {
        if ((ctor_taken >> i) & 0x1)
          {
             /* FIXME Where do we free this? And check for return */
             u = udata_new(ed, (Cell *)c, unit_type);
             o = ctor[i](vbox, u);
             evas_object_event_callback_add(o, EVAS_CALLBACK_FREE, _free_cb, u);
             elm_box_pack_end(vbox, o);
             evas_object_show(o);
          }
     }

   snprintf(buf, sizeof(buf), "Coordinates: X=%u, Y=%u",
            ed->unitselector.x, ed->unitselector.y);
   elm_layout_text_set(lay, "war2edit.unitselector.name", pud_unit2str(unit, PUD_TRUE));
   elm_layout_text_set(lay, "war2edit.unitselector.info", buf);
   elm_layout_content_set(lay, "war2edit.unitselector.contents", vbox);
   evas_object_show(lay);
   elm_object_text_set(f, desc);
   elm_object_content_set(f, lay);

   return f;
}

Eina_Bool
unitselector_add(Editor *ed EINA_UNUSED)
{
//   const char sel_group[] = "war2edit/unitselector/sel";
//   Eina_Bool chk;
//   Evas_Object *o;
//
//   o = ed->unitselector.sel = elm_layout_add(ed->win);
//   chk = elm_layout_file_set(o, ed->edje_file, sel_group);
//   if (EINA_UNLIKELY(!chk))
//     {
//        CRI("Failed to set edje file from %s for group %s", ed->edje_file, sel_group);
//        return EINA_FALSE;
//     }
//
   return EINA_TRUE;
}

Evas_Object *
_unitselector_add(Editor       *ed,
                  Cell         *c,
                  unsigned int  cx EINA_UNUSED,
                  unsigned int  cy EINA_UNUSED)
{
   Evas_Object *vbox, *o;
   //int px, py, sx, sy, rx, ry, cell_w, cell_h, cx1, cy1, cw, ch;
   //unsigned int w, h;
   //
   //w = c->spread_x_below;
   //h = c->spread_y_below;
   //
   //bitmap_cells_to_coords(ed, cx, cy, &px, &py);
   //elm_interface_scrollable_content_region_get(ed->scroller, &sx, &sy, NULL, NULL);
   //evas_object_geometry_get(ed->scroller, &rx, &ry, NULL, NULL);
   //bitmap_cell_size_get(ed, &cell_w, &cell_h);
   //
   //cx1 = rx - sx + (cx * cell_w);
   //cy1 = ry - sy + (cy * cell_h);
   //cw = w * cell_w;
   //ch = h * cell_h;
   //evas_object_resize(ed->unitselector.sel, cw, ch);
   //evas_object_move(ed->unitselector.sel, cx1, cy1);
   //evas_object_show(ed->unitselector.sel);

   vbox = elm_box_add(ed->inwin.obj);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(vbox, EVAS_HINT_FILL, 0.0);
   elm_box_horizontal_set(vbox, EINA_FALSE);
   elm_box_homogeneous_set(vbox, EINA_FALSE);
   evas_object_show(vbox);


   if (c->unit_below != PUD_UNIT_NONE)
     {
        o = _provide_unit_handler(ed, vbox, c, UNIT_BELOW);
        elm_box_pack_end(vbox, o);
     }
   if (c->unit_above != PUD_UNIT_NONE)
     {
        o = _provide_unit_handler(ed, vbox, c, UNIT_ABOVE);
        elm_box_pack_end(vbox, o);
     }
   if (c->start_location != CELL_NOT_START_LOCATION)
     {
        o = _provide_unit_handler(ed, vbox, c, UNIT_START_LOCATION);
        elm_box_pack_end(vbox, o);
     }

   return vbox;
}

void
unitselector_show(Editor       *ed,
                  unsigned int  x,
                  unsigned int  y)
{
   Cell *c;
   unsigned int cx, cy;

   c = cell_anchor_pos_get(ed->cells, x, y, &cx, &cy, EINA_TRUE);
   if (!c) return;

   ed->unitselector.x = cx;
   ed->unitselector.y = cy;
   inwin_set(ed, _unitselector_add(ed, c, cx, cy),
             INWIN_UNITSELECTOR,
             "Close", NULL, NULL, NULL);
}
