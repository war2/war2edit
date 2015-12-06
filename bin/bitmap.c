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

   /* XXX Use OpenMP there??? */
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

static uint16_t
_tile_solid_mask_get(Editor_Sel action,
                     Editor_Sel tint)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL((tint != EDITOR_SEL_TINT_LIGHT) &&
                                  (tint != EDITOR_SEL_TINT_DARK), 0x0000);
   uint16_t mask;

   switch (action)
     {
      case EDITOR_SEL_ACTION_WATER:
         if (tint  == EDITOR_SEL_TINT_LIGHT) mask = 0x0010;
         else                                mask = 0x0020;
         break;

      case EDITOR_SEL_ACTION_NON_CONSTRUCTIBLE:
         if (tint  == EDITOR_SEL_TINT_LIGHT) mask = 0x0030;
         else                                mask = 0x0040;
         break;

      case EDITOR_SEL_ACTION_CONSTRUCTIBLE:
         if (tint  == EDITOR_SEL_TINT_LIGHT) mask = 0x0050;
         else                                mask = 0x0060;
         break;

      case EDITOR_SEL_ACTION_TREES:
         mask = 0x0070;
         break;

      case EDITOR_SEL_ACTION_ROCKS:
         mask = 0x0080;
         break;

      case EDITOR_SEL_ACTION_HUMAN_WALLS:
         mask = 0x0090;
         break;

      case EDITOR_SEL_ACTION_ORCS_WALLS:
         mask = 0x00a0;
         break;

      default:
         CRI("Unhandled action %x", action);
         return 0x0000;
     }

   return mask;
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
                  minimap_update(ed, lx, ly);
                  // FIXME bitmap_refresh_zone(ed, lx - 1, ly - 1, 3, 3);
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
        bitmap_redraw(ed); // FIXME ZONE!
     }
   else
     {
        uint16_t tile = 0x0000;

        action = editor_sel_action_get(ed);
        switch (action)
          {
           case EDITOR_SEL_ACTION_SELECTION:
              /* Handled by mouse,down mouse,move mouve,up callbacks */
              break;

              /* Tiles drawing */
           case EDITOR_SEL_ACTION_WATER:
           case EDITOR_SEL_ACTION_NON_CONSTRUCTIBLE:
           case EDITOR_SEL_ACTION_CONSTRUCTIBLE:
           case EDITOR_SEL_ACTION_TREES:
           case EDITOR_SEL_ACTION_ROCKS:
           case EDITOR_SEL_ACTION_HUMAN_WALLS:
           case EDITOR_SEL_ACTION_ORCS_WALLS:
              tile |= _tile_solid_mask_get(action, editor_sel_tint_get(ed));
              tile |= 0x0001;
              bitmap_tile_set(ed, x, y, tile);
              bitmap_redraw(ed); // FIXME Bad
              break;

           case EDITOR_SEL_ACTION_NONE:
           default:
              CRI("Unhandled action <0x%x>", action);
              break;
          }
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

   /* If we are playing with tiles, algorithm below is pointless */
   if (ed->sel_unit == PUD_UNIT_NONE)
     goto end;

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

end:
   if (ev->mouse.buttons & 1)
     _click_handle(ed, x, y);
}

static void
_mouse_down_cb(void        *data,
               Evas_Object *bmp  EINA_UNUSED,
               void        *info)
{
   Elm_Bitmap_Event_Mouse_Down *ev = info;
   Editor *ed = data;

   if (ed->xdebug)
     {
        Cell *c = &(ed->cells[ev->cell_y][ev->cell_x]);
        fprintf(stdout, "[%u,%u] =", ev->cell_x, ev->cell_y);
        cell_dump(c, stdout);
     }
   _click_handle(ed, ev->cell_x, ev->cell_y);
}

static void
_sel_start_cb(void        *data,
              Evas_Object *obj  EINA_UNUSED,
              void        *info)
{
   Editor *ed = data;
   Evas_Event_Mouse_Down *ev = info;

   if (editor_sel_action_get(ed) != EDITOR_SEL_ACTION_SELECTION)
     return;
   if (!sel_active_is(ed))
     sel_start(ed, ev->canvas.x, ev->canvas.y,
               evas_key_modifier_is_set(ev->modifiers, "Shift"));
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

void
bitmap_unit_draw(Editor *restrict ed,
                 unsigned int     x,
                 unsigned int     y,
                 Eina_Bool        unit_below)
{
   const Cell *c = &(ed->cells[y][x]);
   unsigned char *sprite;
   Eina_Bool flip;
   int at_x, at_y;
   unsigned int w, h, sw, sh;
   Pud_Unit unit = PUD_UNIT_NONE;
   Pud_Player col;
   unsigned int orient;

   if ((unit_below == EINA_TRUE) && (c->anchor_below == 1))
     {
        unit = c->unit_below;
        col = c->player_below;
        orient = c->orient_below;
        w = c->spread_x_below;
        h = c->spread_y_below;
     }
   else if (c->anchor_above == 1) // unit_below == FALSE is implied by else
     {
        unit = c->unit_above;
        col = c->player_above;
        orient = c->orient_above;
        w = c->spread_x_above;
        h = c->spread_y_above;
     }

   /* Don't draw if unit anchor was not found */
   if (unit == PUD_UNIT_NONE)
     return;

   sprite = sprite_get(unit, ed->pud->era, orient, NULL, NULL, &sw, &sh, &flip);
   EINA_SAFETY_ON_NULL_RETURN(sprite);

   at_x = (x * TEXTURE_WIDTH) + ((w * TEXTURE_WIDTH) - sw) / 2;
   at_y = (y * TEXTURE_HEIGHT) + ((h * TEXTURE_HEIGHT) - sh) / 2;

   _draw(ed, sprite, at_x, at_y, sw, sh, flip, col);
}

/* FIXME Zones */
void
bitmap_selections_draw(Editor *restrict ed)
{
   const unsigned int map_w = ed->pud->map_w;
   const unsigned int map_h = ed->pud->map_h;
   unsigned int i, j;
   Cell *c;

   for (j = 0; j < map_h; ++j)
     for (i = 0; i < map_w; ++i)
       {
          // TODO Pre-selections

          c = &(ed->cells[j][i]);
          if ((c->anchor_below) && (c->selected_below))
            {
               _draw(ed, sprite_selection_get(c->spread_x_below),
                     i * TEXTURE_WIDTH, j * TEXTURE_HEIGHT,
                     c->spread_x_below * TEXTURE_WIDTH,
                     c->spread_y_below * TEXTURE_HEIGHT,
                     EINA_FALSE, -1);
            }
          if ((c->anchor_above) && (c->selected_above))
            {
               _draw(ed, sprite_selection_get(c->spread_x_above),
                     i * TEXTURE_WIDTH, j * TEXTURE_HEIGHT,
                     c->spread_x_above * TEXTURE_WIDTH,
                     c->spread_y_above * TEXTURE_HEIGHT,
                     EINA_FALSE, -1);
            }
       }
}

void
bitmap_unit_del_at(Editor *restrict ed,
                   unsigned int     x,
                   unsigned int     y,
                   Eina_Bool        below)
{
   Cell *c, *anchor;
   unsigned int rx, ry, sx, sy, i, j;

   anchor = cell_anchor_pos_get(ed->cells, x, y, &rx, &ry, below);
   sx = (below) ? anchor->spread_x_below : anchor->spread_x_above;
   sy = (below) ? anchor->spread_y_below : anchor->spread_y_above;
   for (j = ry; j < ry + sy; ++j)
     for (i = rx; i < rx + sx; ++i)
       {
          c = &(ed->cells[j][i]);
          if (below)
            {
               c->unit_below = PUD_UNIT_NONE;
               c->spread_x_below = 0;
               c->spread_y_below = 0;
               c->anchor_below = 0;
               c->selected_below = 0;
               c->start_location = 0;
            }
          else
            {
               c->unit_above = PUD_UNIT_NONE;
               c->spread_x_above = 0;
               c->spread_y_above = 0;
               c->anchor_above = 0;
               c->selected_above = 0;
            }
          minimap_update(ed, i, j);
       }
}

void
bitmap_unit_set(Editor *restrict ed,
                Pud_Unit         unit,
                Pud_Player       color,
                unsigned int     orient,
                unsigned int     x,
                unsigned int     y,
                unsigned int     w,
                unsigned int     h,
                uint16_t         alter)
{
   unsigned int i, j;
   unsigned int spread_x, spread_y;
   Eina_Bool flying;
   const unsigned int map_w = ed->pud->map_w;
   const unsigned int map_h = ed->pud->map_h;
   Cell *c;

   /* Don't draw */
   if (unit == PUD_UNIT_NONE)
     return;

   flying = pud_unit_flying_is(unit);
   for (spread_y = 0, j = y; j < y + h; ++j, ++spread_y)
     {
        for (spread_x = 0, i = x; i < x + w; ++i, ++spread_x)
          {
             if ((i >= map_w) || (j >= map_h))
               break;

             c = &(ed->cells[j][i]);
             if (flying)
               {
                  c->unit_above = unit;
                  c->orient_above = orient;
                  c->player_above = color;
                  c->anchor_above = 0;
                  c->spread_x_above = spread_x;
                  c->spread_y_above = spread_y;
               }
             else
               {
                  c->unit_below = unit;
                  c->orient_below = orient;
                  c->player_below = color;
                  c->anchor_below = 0;
                  c->spread_x_below = spread_x;
                  c->spread_y_below = spread_y;
               }
             c->alter = alter;
          }
     }

   c = &(ed->cells[y][x]);
   if (flying)
     {
        c->anchor_above = 1;
        c->spread_x_above = w;
        c->spread_y_above = h;
     }
   else
     {
        c->anchor_below = 1;
        c->spread_x_below = w;
        c->spread_y_below = h;

        /* Pud locations never fly! */
        if (pud_unit_start_location_is(unit))
          {
             c->start_location = color;
             c->start_location_human = (unit == PUD_UNIT_HUMAN_START);
          }
     }

   minimap_update(ed, x, y);
}

void
bitmap_tile_draw(Editor *restrict ed,
                 unsigned int     x,
                 unsigned int     y)
{
   unsigned char *tex;
   Eina_Bool missing;

   tex = texture_get(ed->cells[y][x].tile, ed->pud->era, &missing);
   /* If the texture could not be loaded because of an internal error,
    * return TRUE because we can do nothing about it.
    * If the texture was non-existant, let's try again: the tileset
    * is not helping us */
   // FIXME I don't understand what is going on this the "missing" thing...
   if (!tex) return;

   _draw(ed, tex, x * TEXTURE_WIDTH, y * TEXTURE_HEIGHT,
         TEXTURE_WIDTH, TEXTURE_HEIGHT, EINA_FALSE, -1);
   minimap_render(ed, x, y, 1, 1);
}

Eina_Bool
bitmap_tile_set(Editor * restrict ed,
                int               x,
                int               y,
                unsigned int      key)
{
   Cell *c = &(ed->cells[y][x]);

   if (c->unit_below != PUD_UNIT_NONE)
     {
        /*
         * Big fat-*ss condition!
         * When NOT to delete a unit:
         *  - unit is marine and tile is water
         *  - unit is flying
         *  - unit is land and
         *    - if unit is a building and tile is constructible
         *    - else (unit is not a building) if tile is walkable
         */
        if (!((texture_water_is(key) && pud_unit_marine_is(c->unit_below)) ||
              (pud_unit_flying_is(c->unit_below)) ||
              (pud_unit_land_is(c->unit_below) &&
               ((!pud_unit_building_is(c->unit_below) &&
                 texture_walkable_is(key)) ||
                (pud_unit_building_is(c->unit_below) &&
                 texture_constructible_is(key))))))
          bitmap_unit_del_at(ed, x, y, EINA_TRUE);
     }
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
      elm_obj_bitmap_cursor_visibility_set(EINA_FALSE),
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
bitmap_redraw(Editor *restrict ed)
{
   const unsigned int map_w = ed->pud->map_w;
   const unsigned int map_h = ed->pud->map_h;
   unsigned int i, j;

   /* FIXME  Sooo innefficient. Introduce zones */

   /* Tiles first */
   for (j = 0; j < map_h; ++j)
     for (i = 0; i < map_w; ++i)
       bitmap_tile_draw(ed, i, j);

   /* Units below */
   for (j = map_h - 1; (int) j >= 0; --j)
     for (i = map_w - 1; (int) i >= 0; --i)
       bitmap_unit_draw(ed, i, j, BITMAP_UNIT_BELOW);

   /* Units above */
   for (j = map_h - 1; (int) j >= 0; --j)
     for (i = map_w - 1; (int) i >= 0; --i)
       bitmap_unit_draw(ed, i, j, BITMAP_UNIT_ABOVE);

   /* (Pre)Selections last */
   bitmap_selections_draw(ed);
}

