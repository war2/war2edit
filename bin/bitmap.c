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

static uint8_t
_solid_component_get(const Editor_Sel action,
                     const Editor_Sel tint)
{
   uint8_t component = TILE_NONE;

   switch (action)
     {
      case EDITOR_SEL_ACTION_WATER:
         if (tint == EDITOR_SEL_TINT_LIGHT)
           component = TILE_WATER_LIGHT;
         else
           component = TILE_WATER_DARK;
         break;

      case EDITOR_SEL_ACTION_GRASS:
         if (tint == EDITOR_SEL_TINT_LIGHT)
           component = TILE_GRASS_LIGHT;
         else
           component = TILE_GRASS_DARK;
         break;

      case EDITOR_SEL_ACTION_GROUND:
         if (tint == EDITOR_SEL_TINT_LIGHT)
           component = TILE_GROUND_LIGHT;
         else
           component = TILE_GROUND_DARK;
         break;

      case EDITOR_SEL_ACTION_TREES:
         component = TILE_TREES;
         break;

      case EDITOR_SEL_ACTION_ROCKS:
         component = TILE_ROCKS;
         break;

      default:
         CRI("Unhandled action %x", action);
         break;
     }

   return component;
}

static void
_place_selected_tile(Editor             *ed,
                     const Editor_Sel    action,
                     const Editor_Sel    spread,
                     const Editor_Sel    tint,
                     const unsigned int  x,
                     const unsigned int  y)
{
   uint8_t component;
   uint8_t randomize = TILE_RANDOMIZE;

   if (spread == EDITOR_SEL_SPREAD_SPECIAL)
     randomize |= TILE_SPECIAL;

   component = _solid_component_get(action, tint);
   bitmap_tile_set(ed, x, y, component, component,
                   component, component, randomize);
   bitmap_tile_calculate(ed, x, y, NULL);
}

static void
_click_handle(Editor *ed,
              int     x,
              int     y)
{
   Sprite_Info orient;
   unsigned int w, h;

   if (!elm_bitmap_cursor_enabled_get(ed->bitmap)) return;

   const Editor_Sel action = editor_sel_action_get(ed);
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
                  ed->cells[ly][lx].start_location = CELL_NOT_START_LOCATION;
                  editor_unit_unref(ed);
                  minimap_update(ed, lx, ly);
                  bitmap_redraw(ed, lx - 1, ly - 1, 3, 3);
               }

             ed->start_locations[ed->sel_player].x = x;
             ed->start_locations[ed->sel_player].y = y;
          }

        /* Draw the unit, and therefore lock the cursor. */
        orient = sprite_info_random_get();

        elm_bitmap_cursor_size_get(ed->bitmap, (int*)(&w), (int*)(&h));
        editor_unit_ref(ed);
        bitmap_unit_set(ed, ed->sel_unit, ed->sel_player,
                         orient, x, y, w, h,
                         editor_alter_defaults_get(ed, ed->sel_unit));
        minimap_render_unit(ed, x, y, ed->sel_unit);
        elm_bitmap_cursor_enabled_set(ed->bitmap, EINA_FALSE);
        bitmap_redraw(ed, x - 6, y - 6, 12, 12); // XXX Zone is random
     }
   else if (action != EDITOR_SEL_ACTION_SELECTION)
     {
        _place_selected_tile(ed, action,
                             editor_sel_spread_get(ed),
                             editor_sel_tint_get(ed), x, y);
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

static inline Eina_Bool
_cells_type_get(Cell         **cells,
                unsigned int   ox,
                unsigned int   oy,
                unsigned int   w,
                unsigned int   h,
                Eina_Bool    (*iterator)(const uint8_t, const uint8_t,
                                         const uint8_t, const uint8_t))
{
   unsigned int i, j;
   Eina_Bool res = EINA_FALSE;
   Cell *c;

   for (j = oy; j < oy + h; ++j)
     for (i = ox; i < ox + w; ++i)
       {
          c = &(cells[j][i]);
          res |= iterator(c->tile_tl, c->tile_tr, c->tile_bl, c->tile_br);
       }

   return res;
}

void
bitmap_cursor_state_evaluate(Editor       *ed,
                             unsigned int  x,
                             unsigned int  y)
{
   unsigned int cw, ch;

   elm_bitmap_cursor_size_get(ed->bitmap, (int*)(&cw), (int*)&ch); // FIXME cast

   if (_cells_type_get(ed->cells, x, y, cw, ch, tile_rocks_is) ||
       //TILE_WALL_IS(c) || // FIXME
       _cells_type_get(ed->cells, x, y, cw, ch, tile_trees_is))
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
             if (_cells_type_get(ed->cells, x, y, cw, ch, tile_water_is)) /* water */
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

   /* Disabled by elm_bitmap: don't even bother to set the cursor status */
   if (!elm_bitmap_cursor_enabled_get(ed->bitmap))
     return;

   /* If we are playing with tiles, algorithm below is pointless */
   if (ed->sel_unit == PUD_UNIT_NONE)
     goto end;

   bitmap_cursor_state_evaluate(ed, ev->cell_x, ev->cell_y);

end:
   if (ev->mouse.buttons & 1)
     _click_handle(ed, ev->cell_x, ev->cell_y);
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
                 Bitmap_Unit      unit_type)
{
   Cell **cells = ed->cells;
   const Cell *c = &(cells[y][x]);
   unsigned char *sprite;
   Eina_Bool flip;
   int at_x, at_y;
   unsigned int w, h, sw, sh;
   Pud_Unit unit = PUD_UNIT_NONE;
   Pud_Player col;
   unsigned int orient;

   if (unit_type == BITMAP_UNIT_BELOW)
     {
        if (c->anchor_below != 1)
          {
             x -= c->spread_x_below;
             y -= c->spread_y_below;
             c = &(cells[y][x]);
          }
        unit = c->unit_below;
        col = c->player_below;
        orient = c->orient_below;
        w = c->spread_x_below;
        h = c->spread_y_below;
     }
   else if (unit_type == BITMAP_UNIT_ABOVE)
     {
        if (c->anchor_above != 1)
          {
             x -= c->spread_x_above;
             y -= c->spread_y_above;
             c = &(cells[y][x]);
          }
        unit = c->unit_above;
        col = c->player_above;
        orient = c->orient_above;
        w = c->spread_x_above;
        h = c->spread_y_above;
     }
   else if ((unit_type == BITMAP_UNIT_START_LOCATION) &&
            (c->start_location != CELL_NOT_START_LOCATION))
     {
        unit = (c->start_location_human)
           ? PUD_UNIT_HUMAN_START
           : PUD_UNIT_ORC_START;
        col = c->start_location;
        orient = 0;
        w = 1;
        h = 1;
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

void
bitmap_selections_draw(Editor *restrict ed,
                       int              x,
                       int              y,
                       unsigned int     w,
                       unsigned int     h)
{
   unsigned int x1 = (x < 0) ? 0 : x;
   unsigned int y1 = (y < 0) ? 0 : y;
   unsigned int x2 = x1 + w;
   unsigned int y2 = y1 + h;
   unsigned int i, j;
   Cell *c;

   if (ed->sel.selections <= 0) return;

   if (x2 >= ed->pud->map_w) x2 = ed->pud->map_w - 1;
   if (y2 >= ed->pud->map_h) y2 = ed->pud->map_h - 1;

   for (j = y1; j < y2; ++j)
     for (i = x1; i < x2; ++i)
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
               c->start_location = CELL_NOT_START_LOCATION;
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
   editor_unit_unref(ed);
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

   if (pud_unit_start_location_is(unit))
     {
        c = &(ed->cells[y][x]);
        c->start_location = color;
        c->start_location_human = (unit == PUD_UNIT_HUMAN_START);
        goto end;
     }

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
     }

end:
   minimap_update(ed, x, y);
}

void
bitmap_tile_draw(Editor *restrict ed,
                 unsigned int     x,
                 unsigned int     y)
{
   unsigned char *tex;

   tex = texture_get(ed->cells[y][x].tile, ed->pud->era);
   if (EINA_UNLIKELY(!tex)) return;

   _draw(ed, tex, x * TEXTURE_WIDTH, y * TEXTURE_HEIGHT,
         TEXTURE_WIDTH, TEXTURE_HEIGHT, EINA_FALSE, -1);
   minimap_render(ed, x, y, 1, 1);
}

enum
{
   TL, T, TR,
   L,     R,
   BL, B, BR
};

static inline uint8_t
_conflit_solve(const uint8_t  imposed,
               const uint8_t  conflict,
               Eina_Bool     *result_is_conflict)
{
   /* If the imposed and potential conflictual fragments are compatible,
    * the potential conflict is not a conflict. Otherwise,
    * force a compatible tile */
   if (tile_fragments_compatible_are(imposed, conflict) == EINA_FALSE)
     {
        *result_is_conflict = EINA_TRUE;
        return tile_conflict_resolve_get(imposed);
     }
   return conflict;
}


Eina_Bool
bitmap_tile_calculate(Editor           *ed,
                      int               px,
                      int               py,
                      Tile_Propagation *prop)
{
   Cell *const *const cells = ed->cells;
   Eina_Bool ok = EINA_TRUE;
   Tile_Propagation next[8] = { {0} };
   const Tile_Propagate current_prop = (prop) ? prop->prop : TILE_PROPAGATE_FULL;
   const int x = (prop) ? prop->x : px;
   const int y = (prop) ? prop->y : py;
   unsigned int k;
   uint8_t imposed;

#define _TILE_RESOLVE(T, SUB, X, Y) \
   next[T].SUB = _conflit_solve(imposed, cells[Y][X].tile_ ## SUB, \
                                &(next[T].conflict))

   if (x > 0)
     {
        next[L].x = x - 1;
        next[L].y = y;
        next[L].valid = EINA_TRUE;
        next[L].prop = TILE_PROPAGATE_L;

        imposed = cells[y][x].tile_tl;
        _TILE_RESOLVE(L, tl, x - 1, y);
        next[L].tr = cells[y][x].tile_tl;
        _TILE_RESOLVE(L, bl, x - 1, y);
        next[L].br = cells[y][x].tile_bl;

        if (y > 0)
          {
             next[TL].x = x - 1;
             next[TL].y = y - 1;
             next[TL].valid = EINA_TRUE;
             next[TL].prop = TILE_PROPAGATE_TL;

             imposed = cells[y][x].tile_tl;
             _TILE_RESOLVE(TL, tl, x - 1, y - 1);
             _TILE_RESOLVE(TL, tr, x - 1, y - 1);
             _TILE_RESOLVE(TL, bl, x - 1, y - 1);
             next[TL].br = imposed;
          }
        if (y < (int)ed->pud->map_h - 1)
          {
             next[BL].x = x - 1;
             next[BL].y = y + 1;
             next[BL].valid = EINA_TRUE;
             next[BL].prop = TILE_PROPAGATE_BL;

             imposed = cells[y][x].tile_bl;
             _TILE_RESOLVE(BL, tl, x - 1, y + 1);
             next[BL].tr = imposed;
             _TILE_RESOLVE(BL, bl, x - 1, y + 1);
             _TILE_RESOLVE(BL, br, x - 1, y + 1);
          }
     }
   if (x < (int)ed->pud->map_w - 1)
     {
        next[R].x = x + 1;
        next[R].y = y;
        next[R].valid = EINA_TRUE;
        next[R].prop = TILE_PROPAGATE_R;

        imposed = cells[y][x].tile_tr;
        next[R].tl = imposed;
        _TILE_RESOLVE(R, tr, x + 1, y);
        next[R].bl = imposed;
        _TILE_RESOLVE(R, br, x + 1, y);

        if (y > 0)
          {
             next[TR].x = x + 1;
             next[TR].y = y - 1;
             next[TR].valid = EINA_TRUE;
             next[TR].prop = TILE_PROPAGATE_TR;

             imposed = cells[y][x].tile_tr;
             _TILE_RESOLVE(TR, tl, x + 1, y - 1);
             _TILE_RESOLVE(TR, tr, x + 1, y - 1);
             next[TR].bl = imposed;
             _TILE_RESOLVE(TR, br, x + 1, y - 1);
          }
        if (y < (int)ed->pud->map_h - 1)
          {
             next[BR].x = x + 1;
             next[BR].y = y + 1;
             next[BR].valid = EINA_TRUE;
             next[BR].prop = TILE_PROPAGATE_BR;

             imposed = cells[y][x].tile_br;
             next[BR].tl = imposed;
             _TILE_RESOLVE(BR, tr, x + 1, y + 1);
             _TILE_RESOLVE(BR, bl, x + 1, y + 1);
             _TILE_RESOLVE(BR, br, x + 1, y + 1);
          }
     }
   if (y > 0)
     {
        next[T].x = x;
        next[T].y = y - 1;
        next[T].valid = EINA_TRUE;
        next[T].prop = TILE_PROPAGATE_T;

        imposed = cells[y][x].tile_tl;
        _TILE_RESOLVE(T, tl, x, y - 1);
        _TILE_RESOLVE(T, tr, x, y - 1);
        next[T].bl = imposed;
        next[T].br = imposed;
     }
   if (y < (int)ed->pud->map_h - 1)
     {
        next[B].x = x;
        next[B].y = y + 1;
        next[B].valid = EINA_TRUE;
        next[B].prop = TILE_PROPAGATE_B;

        imposed = cells[y][x].tile_bl;
        next[B].tl = imposed;
        next[B].tr = imposed;
        _TILE_RESOLVE(B, bl, x, y + 1);
        _TILE_RESOLVE(B, br, x, y + 1);
     }

#undef _TILE_RESOLVE

   for (k = 0; k < EINA_C_ARRAY_LENGTH(next); ++k)
     {
        if ((next[k].valid) && (next[k].prop & current_prop))
          {
             ok &= bitmap_tile_set(ed, next[k].x, next[k].y,
                                   next[k].tl, next[k].tr,
                                   next[k].bl, next[k].br,
                                   TILE_RANDOMIZE);

             if (next[k].conflict == EINA_FALSE)
               continue;

             ok &= bitmap_tile_calculate(ed, x, y, &(next[k]));
          }
     }

   bitmap_redraw(ed, x - 1, y - 1, 3, 3);

   return ok;
}


Eina_Bool
bitmap_tile_set(Editor * restrict ed,
                int               x,
                int               y,
                uint8_t           tl,
                uint8_t           tr,
                uint8_t           bl,
                uint8_t           br,
                uint8_t           seed)
{
   /* Safety checks */
   EINA_SAFETY_ON_TRUE_RETURN_VAL((x < 0) || (y < 0) ||
                                  (x >= (int)ed->pud->map_w) ||
                                  (y >= (int)ed->pud->map_h),
                                  EINA_FALSE);

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
        if (!((TILE_WATER_IS(c) && pud_unit_marine_is(c->unit_below)) ||
              (pud_unit_flying_is(c->unit_below)) ||
              (pud_unit_land_is(c->unit_below) &&
               ((!pud_unit_building_is(c->unit_below) &&
                 TILE_WALKABLE_IS(c)) ||
                (pud_unit_building_is(c->unit_below) &&
                 TILE_GRASS_IS(c))))))
          bitmap_unit_del_at(ed, x, y, EINA_TRUE);
     }

   /* Set tile internals */
   if (tl != TILE_NONE) c->tile_tl = tl;
   if (tr != TILE_NONE) c->tile_tr = tr;
   if (bl != TILE_NONE) c->tile_bl = bl;
   if (br != TILE_NONE) c->tile_br = br;

   c->tile = tile_calculate(c->tile_tl, c->tile_tr,
                            c->tile_bl, c->tile_br,
                            seed, ed->pud->era);

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
bitmap_redraw(Editor *restrict ed,
              int              x,
              int              y,
              unsigned int     w,
              unsigned int     h)
{
   int x1 = (x < 0) ? 0 : x;
   int y1 = (y < 0) ? 0 : y;
   int x2 = x1 + w;
   int y2 = y1 + h;
   int i, j;

   if (x2 > (int)ed->pud->map_w) x2 = ed->pud->map_w;
   if (y2 > (int)ed->pud->map_h) y2 = ed->pud->map_h;


   /*
    * XXX Not great... Simple, Stupid yet
    */

   /* Tiles first (plus start locations) */
   for (j = y1; j < y2; ++j)
     for (i = x1; i < x2; ++i)
       {
          bitmap_tile_draw(ed, i, j);
          bitmap_unit_draw(ed, i, j, BITMAP_UNIT_START_LOCATION);
       }

   /* Units below */
   for (j = y2 - 1; j >= y1; --j)
     for (i = x2 - 1; i >= x1; --i)
       bitmap_unit_draw(ed, i, j, BITMAP_UNIT_BELOW);

   /* Units above */
   for (j = y2 - 1; j >= y1; --j)
     for (i = x2 - 1; i >= x1; --i)
       bitmap_unit_draw(ed, i, j, BITMAP_UNIT_ABOVE);

   /* (Pre)Selections last */
   bitmap_selections_draw(ed, x1, y1, w, h);
}

