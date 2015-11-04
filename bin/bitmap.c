/*
 * bitmap.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

#define UNIT_BELOW (1 << 0)
#define UNIT_ABOVE (1 << 1)

typedef struct {
   Editor    *ed;
   int        colorize;
   Eina_Bool  hflip;
} Draw_Data;

/*============================================================================*
 *                                 Private API                                *
 *============================================================================*/

static void
_bitmap_draw_func(void                 *data,
                  Evas_Object          *obj  EINA_UNUSED,
                  Elm_Bitmap_Draw_Info *info)
{
   Draw_Data *d = data;
   const int bmp_w = info->bitmap_w * 4;
   int img_y, bmp_y, img_x, bmp_x, k;
   int img_w, at_x;
   unsigned char *restrict bmp = info->pixels;
   unsigned char bgr[4];
   int bmp_x_start;
   int bmp_x_step;

   img_w = info->source_w * 4;
   at_x = info->at_x * 4;

   /* Always used when there is full alpha */
   for (img_y = 0, bmp_y = info->at_y;
        (img_y < info->source_h) && (bmp_y < info->bitmap_h);
        ++img_y, ++bmp_y)
     {
        /* Calculate the horizontal mirroring  */
        if (d->hflip)
          {
             bmp_x_start = at_x + img_w;
             bmp_x_step = -4;
          }
        else
          {
             bmp_x_start = at_x;
             bmp_x_step = 4;
          }

        for (img_x = 0, bmp_x = bmp_x_start;
             (img_x < img_w) && (bmp_x < bmp_w);
             img_x += 4, bmp_x += bmp_x_step)
          {
             k = img_x + (img_y * img_w);
             memcpy(&(bgr[0]), &(info->source[k]), 4);
             if (d->colorize != -1)
               {
                  war2_sprites_color_convert(d->colorize,
                                             bgr[2], bgr[1], bgr[0],
                                             &(bgr[2]), &(bgr[1]), &(bgr[0]));
               }
             if (bgr[3] != 0)
               {
                  memcpy(&(bmp[bmp_x + bmp_y * bmp_w]), &(bgr[0]), 4);
               }
          }
     }
}

static inline void
_draw(Editor *ed,
      unsigned char *restrict img,
      int                     at_x,
      int                     at_y,
      int                     img_w,
      int                     img_h,
      Eina_Bool               hflip,
      int                     colorize)
{
   Draw_Data draw_data;

   draw_data.ed = ed;
   draw_data.hflip = hflip;
   draw_data.colorize = colorize;

   elm_bitmap_abs_draw(ed->bitmap, &draw_data, img, img_w, img_h, at_x, at_y);
}

static void
_click_handle(Editor *ed,
              int     x,
              int     y)
{
   Sprite_Info orient;
   unsigned int w, h;
   Editor_Sel action;

   if (!elm_bitmap_cursor_enabled_get(ed->bitmap)) return;

   action = editor_sel_action_get(ed);
   if (ed->sel_unit != PUD_UNIT_NONE)
     {
        if (pud_unit_start_location_is(ed->sel_unit))
          {
             const int lx = ed->start_locations[ed->sel_player].x;
             const int ly = ed->start_locations[ed->sel_player].y;

             /* Start location did exist: move it. Also refresh
              * the zone where it was to remove it. */
             /* FIXME See Cedric's message on E-phab */
             if (ed->start_locations[ed->sel_player].x != -1)
               {
                  ed->cells[ly][lx].unit_below = PUD_UNIT_NONE;
                  // FIXME bitmap_refresh_zone(ed, lx - 1, ly - 1, 3, 3);
                  //if (ed->pud->units_count > 0) ed->pud->units_count++;
               }

             ed->start_locations[ed->sel_player].x = x;
             ed->start_locations[ed->sel_player].y = y;
          }

        /* Draw the unit, and therefore lock the cursor. */
        orient = sprite_info_random_get();

        // FIXME cast --- change stuff in elm_bitmap
        elm_bitmap_cursor_size_get(ed->bitmap, (int*)(&w), (int*)(&h));
        editor_unit_ref(ed);
        bitmap_unit_set(ed, ed->sel_unit, ed->sel_player,
                         orient, x, y, w, h,
                         editor_alter_defaults_get(ed, ed->sel_unit));
        minimap_render_unit(ed, x, y, ed->sel_unit);
        elm_bitmap_cursor_enabled_set(ed->bitmap, EINA_FALSE);
     }
   else if (action != EDITOR_SEL_ACTION_NONE)
     {
        INF("editor action");
     }
}

static Eina_Bool
_unit_below_cursor_is(const Editor *restrict ed,
                      int                    x,
                      int                    y,
                      unsigned int           cw,
                      unsigned int           ch,
                      unsigned char          types)
{
   unsigned int i, j;
   const Cell *c;
   const Pud *pud = ed->pud;

   for (j = y; j < y + ch; ++j)
     {
        for (i = x; i < x + cw; ++i)
          {
             /* Prevent for accessing invalid memory */
             if ((i >= pud->map_w) || (j >= (pud->map_h)))
               return EINA_FALSE;

             c = &(ed->cells[j][i]);
             if (types & UNIT_BELOW)
               {
                  if (c->unit_below != PUD_UNIT_NONE)
                    return EINA_TRUE;
               }
             if (types & UNIT_ABOVE)
               {
                  if (c->unit_above != PUD_UNIT_NONE)
                    return EINA_TRUE;
               }
          }
     }
   return EINA_FALSE;
}


/*============================================================================*
 *                                   Events                                   *
 *============================================================================*/

static void
_hovered_cb(void        *data,
            Evas_Object *bmp  EINA_UNUSED,
            void        *info)
{
   Editor *ed = data;
   Elm_Bitmap_Event_Hovered *ev = info;
   Cell c;
   int x, y;
   unsigned int cw, ch;

   /* Disabled by elm_bitmap: don't even bother to set the cursor status */
   if (!elm_bitmap_cursor_enabled_get(ed->bitmap))
     return;

   x = ev->cell_x;
   y = ev->cell_y;
   elm_bitmap_cursor_size_get(ed->bitmap, (int*)(&cw), (int*)&ch); // FIXME cast

   c = ed->cells[y][x];

   if (texture_rock_is(c.tile) ||
       texture_wall_is(c.tile) ||
       texture_tree_is(c.tile))
     {
        /* Handle only flying units: they are the only one
         * that can be placed there */
        if (!pud_unit_flying_is(ed->sel_unit))
          elm_bitmap_cursor_enabled_set(ed->bitmap, EINA_FALSE);
        else
          {
             /* Don't collide with another unit */
             if (_unit_below_cursor_is(ed, x, y, cw, ch, UNIT_ABOVE))
               elm_bitmap_cursor_enabled_set(ed->bitmap, EINA_FALSE);
             else
               elm_bitmap_cursor_enabled_set(ed->bitmap, EINA_TRUE);
          }
     }
   else /* Ground, water */
     {
        /* Flying units */
        if (pud_unit_flying_is(ed->sel_unit))
          {
             /* Don't collide with another unit */
             if (_unit_below_cursor_is(ed, x, y, cw, ch, UNIT_ABOVE))
               elm_bitmap_cursor_enabled_set(ed->bitmap, EINA_FALSE);
             else
               elm_bitmap_cursor_enabled_set(ed->bitmap, EINA_TRUE);
          }
        else /* marine,ground units */
          {
             if (texture_water_is(c.tile)) /* water */
               {
                  if (pud_unit_marine_is(ed->sel_unit))
                    {
                       /* Don't collide with another unit */
                       if (_unit_below_cursor_is(ed, x, y, cw, ch, UNIT_BELOW))
                         elm_bitmap_cursor_enabled_set(ed->bitmap, EINA_FALSE);
                       else
                         elm_bitmap_cursor_enabled_set(ed->bitmap, EINA_TRUE);
                    }
                  else
                    elm_bitmap_cursor_enabled_set(ed->bitmap, EINA_FALSE);
               }
             else /* ground */
               {
                  if (pud_unit_marine_is(ed->sel_unit))
                    elm_bitmap_cursor_enabled_set(ed->bitmap, EINA_FALSE);
                  else
                    {
                       if (_unit_below_cursor_is(ed, x, y, cw, ch, UNIT_BELOW))
                         elm_bitmap_cursor_enabled_set(ed->bitmap, EINA_FALSE);
                       else
                         elm_bitmap_cursor_enabled_set(ed->bitmap, EINA_TRUE);
                    }
               }
          }
     }

   if (ev->mouse.buttons & 1)
     _click_handle(data, x, y);
}

static void
_mouse_down_cb(void        *data,
               Evas_Object *bmp  EINA_UNUSED,
               void        *info)
{
   Elm_Bitmap_Event_Mouse_Down *ev = info;
   _click_handle(data, ev->cell_x, ev->cell_y);
}

static void
_sel_start_cb(void        *data,
              Evas_Object *obj  EINA_UNUSED,
              void        *info)
{
   Editor *ed = data;
   Evas_Event_Mouse_Down *ev = info;

   if (!sel_active_is(ed))
     sel_start(ed, ev->canvas.x, ev->canvas.y);
}

static void
_sel_end_cb(void        *data,
            Evas_Object *obj  EINA_UNUSED,
            void        *info EINA_UNUSED)
{
   Editor *ed = data;

   if (sel_active_is(ed))
     sel_end(ed);
}

static void
_sel_update_cb(void        *data,
               Evas_Object *obj  EINA_UNUSED,
               void        *info)
{
   Editor *ed = data;
   Evas_Event_Mouse_Move *ev = info;
   int rx, ry, rw, rh;

   if (sel_active_is(ed))
     {
        eo_do(
           ed->scroller,
           elm_interface_scrollable_content_region_get(&rx, &ry, &rw, &rh)
        );
        sel_update(ed, ev->cur.canvas.x - ed->sel.x,
                   ev->cur.canvas.y - ed->sel.y);
     }
}



/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

//void
//bitmap_refresh_zone(Editor *restrict ed,
//                    int              x,
//                    int              y,
//                    unsigned int     w,
//                    unsigned int     h)
//{
//   unsigned int i, j, sw, sh;
//   Cell c;
//
//   /* Bounds checking - needed */
//   if (x < 0) x = 0;
//   if (y < 0) y = 0;
//   if (x + w >= ed->pud->map_w) w = ed->pud->map_w - x - 1;
//   if (y + h >= ed->pud->map_h) h = ed->pud->map_h - y - 1;
//
//   for (j = y; j < y + h; j++)
//     {
//        for (i = x; i < x + w; i++)
//          {
//             c = ed->cells[j][i];
//             bitmap_tile_set(ed, i, j, c.tile);
//             if (c.anchor_below)
//               {
//                  sprite_tile_size_get(c.unit_below, &sw, &sh);
//                  bitmap_unit_set(ed, c.unit_below, c.player_below,
//                                  c.orient_below, i, j, sw, sh);
//               }
//          }
//     }
//
//   /* FIXME This is pretty bad!! To avoid mixing sprites I do 2 separate passes.
//    * FIXME There is certainly much much better, I'll do it some day. */
//   for (j = y; j < y + h; j++)
//     {
//        for (i = x; i < x + w; i++)
//          {
//             if (c.anchor_above)
//               {
//                  sprite_tile_size_get(c.unit_above, &sw, &sh);
//                  bitmap_unit_set(ed, c.unit_above, c.player_above,
//                                  c.orient_above, i, j, sw, sh);
//               }
//          }
//     }
//}

void
bitmap_unit_set(Editor *restrict ed,
                Pud_Unit         unit,
                Pud_Player       color,
                unsigned int     orient,
                int              x,
                int              y,
                unsigned int     w,
                unsigned int     h,
                uint16_t         alter)
{
   unsigned char *sprite;
   int at_x, at_y;
   unsigned int sw, sh;
   unsigned int i, j;
   Eina_Bool flip;
   Eina_Bool flying;
   const unsigned int map_w = ed->pud->map_w;
   const unsigned int map_h = ed->pud->map_h;

   /* Don't draw */
   if (unit == PUD_UNIT_NONE) return;

   sprite = sprite_get(unit, ed->pud->era, orient, NULL, NULL, &sw, &sh, &flip);
   EINA_SAFETY_ON_NULL_RETURN(sprite);

   at_x = (x * TEXTURE_WIDTH) + (int)((w * TEXTURE_WIDTH) - sw) / 2;
   at_y = (y * TEXTURE_HEIGHT) + (int)((h * TEXTURE_HEIGHT) - sh) / 2;

   _draw(ed, sprite, at_x, at_y, sw, sh, flip, color);

   flying = pud_unit_flying_is(unit);
   for (j = y; j < y + h; ++j)
     {
        for (i = x; i < x + w; ++i)
          {
             if ((i >= map_w) || (j >= map_h))
               break;

             if (flying)
               {
                  ed->cells[j][i].unit_above = unit;
                  ed->cells[j][i].orient_above = orient;
                  ed->cells[j][i].player_above = color;
                  ed->cells[j][i].anchor_above = 0;
                  ed->cells[j][i].alter = alter;
               }
             else
               {
                  ed->cells[j][i].unit_below = unit;
                  ed->cells[j][i].orient_below = orient;
                  ed->cells[j][i].player_below = color;
                  ed->cells[j][i].anchor_below = 0;
                  ed->cells[j][i].alter = alter;
               }
          }
     }
   if (flying)
     ed->cells[y][x].anchor_above = 1;
   else
     ed->cells[y][x].anchor_below = 1;

   minimap_update(ed, x, y);
}

Eina_Bool
bitmap_tile_set(Editor * restrict ed,
                int               x,
                int               y,
                unsigned int      key)
{
   unsigned char *tex;
   Eina_Bool missing;

   tex = texture_get(key, ed->pud->era, &missing);
   /* If the texture could not be loaded because of an internal error,
    * return TRUE because we can do nothing about it.
    * If the texture was non-existant, let's try again: the tileset
    * is not helping us */
   if (!tex) return !missing;

   _draw(ed, tex, x * TEXTURE_WIDTH, y * TEXTURE_HEIGHT,
         TEXTURE_WIDTH, TEXTURE_HEIGHT, EINA_FALSE, -1);
   ed->cells[y][x].tile = key;

   minimap_update(ed, x, y);
   return EINA_TRUE;
}

Eina_Bool
bitmap_add(Editor *ed)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);

   Evas_Object *obj;

   obj = elm_bitmap_init_add(ed->win, 32, 32, ed->pud->map_w, ed->pud->map_h);
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);

   eo_do(
      obj,
      evas_obj_size_hint_align_set(0.0, 0.0),
      evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
      elm_obj_bitmap_resizable_set(EINA_FALSE),
      elm_obj_bitmap_cursor_visibility_set(EINA_TRUE),
      elm_obj_bitmap_draw_func_set(_bitmap_draw_func)
   );
   evas_object_smart_callback_add(obj, "bitmap,mouse,down", _mouse_down_cb, ed);
   evas_object_smart_callback_add(obj, "bitmap,mouse,hovered", _hovered_cb, ed);
   evas_object_smart_callback_add(obj, "mouse,down", _sel_start_cb, ed);
   evas_object_smart_callback_add(obj, "mouse,move", _sel_update_cb, ed);
   evas_object_smart_callback_add(obj, "mouse,up", _sel_end_cb, ed);


   ed->bitmap = obj;
   ed->cells = cell_matrix_new(ed->pud->map_w, ed->pud->map_h);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed->cells, EINA_FALSE);

   elm_object_content_set(ed->scroller, ed->bitmap);
   evas_object_show(ed->bitmap);

   sel_add(ed);

   return EINA_TRUE;
}

void
bitmap_reset(Editor *ed)
{
   cell_matrix_zero(ed->cells, ed->pud->tiles);
   elm_bitmap_clear(ed->bitmap, 0, 255, 0);
}

