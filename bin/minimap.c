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

typedef struct
{
   Evas_Object    *win;
   Evas_Object    *map;
   Evas_Object    *rect;
   Minimap_Data   *active;
   unsigned int    ratio;
} Minimap;

static Minimap _minimap;

static void
_mouse_down_cb(void        *data EINA_UNUSED,
               Evas        *e    EINA_UNUSED,
               Evas_Object *obj  EINA_UNUSED,
               void        *info)
{
   Evas_Event_Mouse_Down *const down = info;
   Editor *ed;
   int cx, cy;
   float ratio;

   /*
    * Black magick to get the editor out of the active
    * minimap field.
    */
   ed = (Editor *)(void *)((unsigned char *)_minimap.active - offsetof(Editor, minimap));

   ratio = (float)ed->minimap.ratio;
   cx = rintf((float)down->output.x / ratio);
   cy = rintf((float)down->output.y / ratio);

   minimap_view_move(ed, cx, cy, EINA_TRUE);
}

static void
_mouse_move_cb(void        *data EINA_UNUSED,
               Evas        *e    EINA_UNUSED,
               Evas_Object *obj  EINA_UNUSED,
               void        *info EINA_UNUSED)
{
   // FIXME broken feature due to scrolling callback
 //  Editor *ed = data;
 //  Evas_Event_Mouse_Move *move = info;

 //  if (move->buttons & 1)
 //    minimap_view_move(ed, move->cur.output.x, move->cur.output.y, EINA_TRUE);
}

static void
_minimap_win_close_cb(void        *data EINA_UNUSED,
                      Evas_Object *obj  EINA_UNUSED,
                      void        *evt  EINA_UNUSED)
{
   evas_object_hide(_minimap.win);
}

Eina_Bool
minimap_init(void)
{
   Evas_Object *o, *win;
   Evas *evas;

   win = _minimap.win = elm_win_util_standard_add("Minimap", "Minimap");
   evas_object_smart_callback_add(win, "delete,request",
                                  _minimap_win_close_cb, NULL);
   evas_object_show(win);

   evas = evas_object_evas_get(win);
   o = _minimap.map = evas_object_image_filled_add(evas);
   evas_object_image_colorspace_set(o, EVAS_COLORSPACE_ARGB8888);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(o);
   elm_win_resize_object_add(win, o);

   evas_object_event_callback_add(_minimap.map, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down_cb, NULL);
   evas_object_event_callback_add(_minimap.map, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move_cb, NULL);

   /* Current view mask */
   o = _minimap.rect = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 100, 100, 100, 100);
   evas_object_resize(o, 1, 1);
   evas_object_move(o, 0, 0);
   evas_object_show(o);

   return EINA_TRUE;
}

Eina_Bool
minimap_add(Editor *ed)
{
   unsigned int w, i;

   /* Define a ratio to resize the minimap (way too small overwise) */
   switch (ed->pud->dims)
     {
      case PUD_DIMENSIONS_32_32:   ed->minimap.ratio = 5; break;
      case PUD_DIMENSIONS_64_64:   ed->minimap.ratio = 3; break;
      case PUD_DIMENSIONS_96_96:   ed->minimap.ratio = 2; break;
      case PUD_DIMENSIONS_128_128: ed->minimap.ratio = 2; break;
      default:
         CRI("ed->pud->dims is %i. This MUST NEVER happen", ed->pud->dims);
         goto fail;
     }

   ed->minimap.w = ed->pud->map_w * ed->minimap.ratio;
   ed->minimap.h = ed->pud->map_h * ed->minimap.ratio;

   /* Colorspace width */
   w = ed->pud->map_w * 4;

   /* Allocate Iliffe vector to hold the minimap */
   ed->minimap.data = malloc(ed->pud->map_h * sizeof(unsigned char *));
   if (EINA_UNLIKELY(!ed->minimap.data))
     {
        CRI("Failed to alloc memory");
        goto fail;
     }
   ed->minimap.data[0] = malloc(ed->pud->map_w * ed->pud->map_h * 4);
   if (EINA_UNLIKELY(!ed->minimap.data[0]))
     {
        CRI("Failed to alloc memory");
        goto fail_free;
     }
   for (i = 1; i < ed->pud->map_h; ++i)
     ed->minimap.data[i] = ed->minimap.data[i - 1] + w;

   return EINA_TRUE;

fail_free:
   free(ed->minimap.data);
fail:
   return EINA_FALSE;
}

Eina_Bool
minimap_attach(Editor *ed)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);

   Minimap_Data *d;

   if (_minimap.active == &ed->minimap)
     return EINA_TRUE;

   /* We now receive a new editor to attach to the minimap */
   d = _minimap.active = &(ed->minimap);

   /* Resize window for current minimap */
   evas_object_size_hint_max_set(_minimap.win, d->w, d->h);
   evas_object_size_hint_min_set(_minimap.win, d->w, d->h);
   evas_object_resize(_minimap.win, d->w, d->h);

   /* Configure minimap image */
   evas_object_image_size_set(_minimap.map, ed->pud->map_w, ed->pud->map_h);
   evas_object_image_data_set(_minimap.map, d->data[0]);

   minimap_render(0, 0, ed->pud->map_w, ed->pud->map_h);

   return EINA_TRUE;
}

void
minimap_show(void)
{
   evas_object_show(_minimap.win);
}

void
minimap_shutdown(void)
{
   evas_object_del(_minimap.win);
}

void
minimap_del(Editor *ed)
{
   EINA_SAFETY_ON_NULL_RETURN(ed);

   if (ed->minimap.data)
     {
        free(ed->minimap.data[0]);
        free(ed->minimap.data);
     }
}

Eina_Bool
minimap_update(Editor       *ed,
               unsigned int  x,
               unsigned int  y)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((x >= ed->pud->map_w) ||
                                  (y >= ed->pud->map_h), EINA_FALSE);

   const Cell *c = &(ed->cells[y][x]);
   uint8_t player;
   unsigned int px, py;
   Pud_Unit u = PUD_UNIT_NONE;
   Pud_Color col;
   unsigned char *ptr;
   unsigned int i, j;
   unsigned int ry, rx;
   unsigned int w, h;

   if (c->unit_above != PUD_UNIT_NONE)
     {
        player = c->player_above;
        u = c->unit_above;
     }
   else if (c->unit_below != PUD_UNIT_NONE)
     {
        player = c->player_below;
        u = c->unit_below;
     }

   if (u == PUD_UNIT_NONE)
     {
        col = pud_tile_to_color(ed->pud->era, c->tile);
        w = 1;
        h = 1;
     }
   else
     {
        col = pud_color_for_unit(u, player);
        sprite_tile_size_get(u, &w, &h);
     }

   /* Format ARGB8888: each pixel is 4 bytes long */
   px = x * 4;
   py = y;
   rx = px + (w * 4);
   ry = py + h;

   if (rx > ed->pud->map_w * 4) rx = (ed->pud->map_w - 1) * 4;
   if (ry > ed->pud->map_h) ry = ed->pud->map_h - 1;

   for (j = py; j < ry; ++j)
     {
        for (i = px; i < rx; i += 4)
          {
             ptr = &(_minimap.active->data[j][i]); /* Raw data */
             ptr[0] = col.b;
             ptr[1] = col.g;
             ptr[2] = col.r;
             ptr[3] = col.a;
          }
     }

   /*
    * Don't update the image!
    * Use minimap_render() to do so (for perfs).
    */

   return EINA_TRUE;
}

void
minimap_render(unsigned int  x,
               unsigned int  y,
               unsigned int  w,
               unsigned int  h)
{
   evas_object_image_data_update_add(_minimap.map, x, y, w, h);
}

void
minimap_render_unit(const Editor *ed,
                    unsigned int  x,
                    unsigned int  y,
                    Pud_Unit      u)
{
   minimap_render(x, y, ed->pud->unit_data[u].size_w,
                  ed->pud->unit_data[u].size_h);
}

void
minimap_view_move(Editor    *ed,
                  int         x,
                  int         y,
                  Eina_Bool   clicked)
{
   int bx, by, rw = 0, rh = 0, srw, srh, cx, cy;

   evas_object_geometry_get(_minimap.rect, NULL, NULL, &rw, &rh);

   if (clicked)
     {
        x -= rw / (2 * ed->minimap.ratio);
        y -= rh / (2 * ed->minimap.ratio);
     }

   if (x < 0) x = 0;
   if (y < 0) y = 0;

   evas_object_move(_minimap.rect,
                    x * ed->minimap.ratio, y * ed->minimap.ratio);

   if (clicked)
     {
        elm_interface_scrollable_content_region_get(ed->scroller,
                                                    NULL, NULL, &srw, &srh);
        bitmap_cell_size_get(ed, &cx, &cy);
        bx = x * cx;
        by = y * cy;
        elm_scroller_region_bring_in(ed->scroller, bx, by, srw, srh);
     }
}

void
minimap_view_resize(Editor       *ed,
                    unsigned int  w,
                    unsigned int  h)
{
   evas_object_resize(_minimap.rect,
                      w * ed->minimap.ratio, h * ed->minimap.ratio);
}
