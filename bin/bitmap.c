/*
 * bitmap.c
 *
 * Copyright (c) 2015 - 2016 Jean Guyomarc'h
 */

#include "war2edit.h"

#define UNIT_BELOW (1 << 0)
#define UNIT_ABOVE (1 << 1)

/*============================================================================*
 *                                 Private API                                *
 *============================================================================*/

static void
_draw(Editor        *ed,
      unsigned char *img,
      int            at_x,
      int            at_y,
      int            img_w,
      int            img_h,
      Eina_Bool      hflip,
      int            colorize)
{
   int bmp_w, bmp_h;
   int img_y, bmp_y, img_x, bmp_x, k;
   unsigned char *bmp;
   unsigned char bgr[4];
   int bmp_x_start;
   int bmp_x_step;

   return; // XXX Override draw
#if 1
   bmp = cairo_image_surface_get_data(ed->bitmap.surf);
#else
   bmp = ed->bitmap.pixels;
#endif

   at_x -= ed->bitmap.x_off;
   if (at_x < 0) at_x = 0;
   at_y -= ed->bitmap.y_off;
   if (at_y < 0) at_y = 0;

   img_w *= 4;
   at_x *= 4;

   evas_object_geometry_get(ed->bitmap.img, NULL, NULL, &bmp_w, &bmp_h);
   bmp_w *= 4;

   /*
    * FIXME
    *
    * Ideally, we should not make manual rendering like we do now...
    * We should Cairo handle all that, use GL behind the scenes,
    * and it is likely to be much faster.
    *
    * This is legacy from my elm_bitmap. I'll change it some day...
    *
    * FIXME
    */

   /* Calculate the horizontal mirroring  */
   if (hflip)
     {
        bmp_x_start = at_x + img_w;
        bmp_x_step = -4;
     }
   else
     {
        bmp_x_start = at_x;
        bmp_x_step = 4;
     }

   for (img_y = 0, bmp_y = at_y;
        (img_y < img_h) && (bmp_y < bmp_h);
        ++img_y, ++bmp_y)
     {
        for (img_x = 0, bmp_x = bmp_x_start;
             (img_x < img_w) && (bmp_x < bmp_w);
             img_x += 4, bmp_x += bmp_x_step)
          {
             k = img_x + (img_y * img_w);
             memcpy(&(bgr[0]), &(img[k]), 4);
             if (colorize != -1)
               {
                  war2_sprites_color_convert(colorize,
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

static uint8_t
_solid_component_get(Editor_Sel action,
                     Editor_Sel tint)
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

      case EDITOR_SEL_ACTION_HUMAN_WALLS:
         component = TILE_HUMAN_WALL;
         break;

      case EDITOR_SEL_ACTION_ORC_WALLS:
         component = TILE_ORC_WALL;
         break;

      default:
         CRI("Unhandled action %x", action);
         break;
     }

   return component;
}

static void
_place_selected_tile(Editor           *ed,
                     const Editor_Sel  action,
                     const Editor_Sel  spread,
                     const Editor_Sel  tint,
                     unsigned int      x,
                     unsigned int      y)
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
   int w, h;
   Eina_Rectangle zone;

   if (!bitmap_cursor_enabled_get(ed)) return;

   const Editor_Sel action = editor_sel_action_get(ed);
   if (ed->sel_unit != PUD_UNIT_NONE)
     {
        if (pud_unit_start_location_is(ed->sel_unit))
          {
             const int lx = ed->start_locations[ed->sel_player].x;
             const int ly = ed->start_locations[ed->sel_player].y;

             if (ed->start_locations[ed->sel_player].x != -1)
               {
                  ed->cells[ly][lx].unit_below = PUD_UNIT_NONE;
                  ed->cells[ly][lx].start_location = CELL_NOT_START_LOCATION;
                  editor_unit_unref(ed);
                  minimap_update(ed, lx, ly);
                  EINA_RECTANGLE_SET(&zone, lx - 1, ly - 1, 3, 3);
                  bitmap_refresh(ed, &zone);
               }

             ed->start_locations[ed->sel_player].x = x;
             ed->start_locations[ed->sel_player].y = y;
          }

        /* Draw the unit, and therefore lock the cursor. */
        orient = sprite_info_random_get();

        bitmap_cursor_size_get(ed, &w, &h);
        editor_unit_ref(ed);
        bitmap_unit_set(ed, ed->sel_unit, ed->sel_player,
                         orient, x, y, w, h,
                         editor_alter_defaults_get(ed, ed->sel_unit));
        minimap_render_unit(ed, x, y, ed->sel_unit);
        bitmap_cursor_enabled_set(ed, EINA_FALSE);
        EINA_RECTANGLE_SET(&zone, x - 6, y - 6, 12, 12); // XXX Zone is random
        bitmap_refresh(ed, &zone);
     }
   else if (action != EDITOR_SEL_ACTION_SELECTION)
     {
        _place_selected_tile(ed, action,
                             editor_sel_spread_get(ed),
                             editor_sel_tint_get(ed), x, y);
     }
}

static Eina_Bool
_unit_below_cursor_is(const Editor  *ed,
                      int            x,
                      int            y,
                      unsigned int   cw,
                      unsigned int   ch,
                      unsigned char  types)
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

static Eina_Bool
_inclusive_op(Eina_Bool old, Eina_Bool new)
{
   return old || new;
}

static Eina_Bool
_exclusive_op(Eina_Bool old, Eina_Bool new)
{
   return old && new;
}

static Eina_Bool
_cells_type_get(Cell         **cells,
                unsigned int   ox,
                unsigned int   oy,
                unsigned int   w,
                unsigned int   h,
                Eina_Bool    (*iterator)(const uint8_t, const uint8_t,
                                         const uint8_t, const uint8_t),
                Eina_Bool    (*operator)(Eina_Bool, Eina_Bool))
{
   unsigned int i, j;
   Eina_Bool res;
   Cell *c;

   res = (operator == _inclusive_op) ? EINA_FALSE : EINA_TRUE;

   for (j = oy; j < oy + h; ++j)
     for (i = ox; i < ox + w; ++i)
       {
          c = &(cells[j][i]);
          res = operator(res,
                         iterator(c->tile_tl, c->tile_tr,
                                  c->tile_bl, c->tile_br));
       }

   return res;
}

void
bitmap_cursor_state_evaluate(Editor       *ed,
                             unsigned int  x,
                             unsigned int  y)
{
   int cw, ch;

   bitmap_cursor_size_get(ed, &cw, &ch);

#define CELLS_TYPE(type_, op_) \
   _cells_type_get(ed->cells, x, y, cw, ch, type_, op_)

   if (CELLS_TYPE(tile_rocks_is, _inclusive_op) ||
       CELLS_TYPE(tile_wall_is, _inclusive_op) ||
       CELLS_TYPE(tile_trees_is, _inclusive_op))
     {
        /* Handle only flying units: they are the only one
         * that can be placed there */
        if (pud_unit_flying_is(ed->sel_unit))
          {
             /* Don't collide with another unit */
             if (_unit_below_cursor_is(ed, x, y, cw, ch, UNIT_ABOVE))
               bitmap_cursor_enabled_set(ed, EINA_FALSE);
             else
               bitmap_cursor_enabled_set(ed, EINA_TRUE);
          }
        else
          bitmap_cursor_enabled_set(ed, EINA_FALSE);
     }
   else /* Ground, water */
     {
        /* Flying units */
        if (pud_unit_flying_is(ed->sel_unit))
          {
             /* Don't collide with another unit */
             if (_unit_below_cursor_is(ed, x, y, cw, ch, UNIT_ABOVE))
               bitmap_cursor_enabled_set(ed, EINA_FALSE);
             else
               bitmap_cursor_enabled_set(ed, EINA_TRUE);
          }
        else /* marine,ground units */
          {
             if (CELLS_TYPE(tile_water_is, _exclusive_op)) /* water */
               {
                  if (pud_unit_marine_is(ed->sel_unit))
                    {
                       /* Don't collide with another unit */
                       if (_unit_below_cursor_is(ed, x, y, cw, ch, UNIT_BELOW))
                         bitmap_cursor_enabled_set(ed, EINA_FALSE);
                       else
                         bitmap_cursor_enabled_set(ed, EINA_TRUE);
                    }
                  else
                    bitmap_cursor_enabled_set(ed, EINA_FALSE);
               }
             else /* ground */
               {
                  if (pud_unit_marine_is(ed->sel_unit))
                    bitmap_cursor_enabled_set(ed, EINA_FALSE);
                  else
                    {
                       if (pud_unit_building_is(ed->sel_unit))
                         {
                            if (CELLS_TYPE(tile_grass_is, _exclusive_op) &&
                                (!_unit_below_cursor_is(ed, x, y, cw, ch, UNIT_BELOW)))
                              bitmap_cursor_enabled_set(ed, EINA_TRUE);
                            else
                              bitmap_cursor_enabled_set(ed, EINA_FALSE);
                         }
                       else
                         {
                            if (CELLS_TYPE(tile_walkable_is, _exclusive_op))
                              {
                                 if (_unit_below_cursor_is(ed, x, y, cw, ch, UNIT_BELOW))
                                   bitmap_cursor_enabled_set(ed, EINA_FALSE);
                                 else
                                   bitmap_cursor_enabled_set(ed, EINA_TRUE);
                              }
                            else
                              bitmap_cursor_enabled_set(ed, EINA_FALSE);
                         }
                    }
               }
          }
     }
#undef CELLS_TYPE
}

static void
_bitmap_autoresize(Editor *ed)
{
   int x, y, w, h;
   unsigned char *pixels;


   evas_object_geometry_get(ed->scroller, &x, &y, NULL, NULL);
   elm_interface_scrollable_content_region_get(ed->scroller, NULL, NULL, &w, &h);

   if (w > ed->bitmap.max_w) w = ed->bitmap.max_w;
   if (h > ed->bitmap.max_h) h = ed->bitmap.max_h;

   evas_object_move(ed->bitmap.img, x, y);
   evas_object_move(ed->bitmap.clip, x, y);

   evas_object_resize(ed->bitmap.img, w, h);
   evas_object_resize(ed->bitmap.clip, w, h);

   evas_object_image_data_update_add(ed->bitmap.img, 0, 0, w, h);
}

static void
_bitmap_resize_cb(void        *data,
                  Evas        *e    EINA_UNUSED,
                  Evas_Object *obj  EINA_UNUSED,
                  void        *info EINA_UNUSED)
{
   _bitmap_autoresize(data);
}


/*============================================================================*
 *                                   Events                                   *
 *============================================================================*/

static void
_mouse_move_cb(void        *data,
               Evas_Object *bmp  EINA_UNUSED,
               void        *info)
{
   Editor *ed = data;
   Evas_Event_Mouse_Move *ev = info;
   int cx, cy, ox, oy;

   evas_object_geometry_get(ed->bitmap.img, &ox, &oy, NULL, NULL);
   bitmap_coords_to_cells(ed, ev->cur.canvas.x - ox, ev->cur.canvas.y - oy, &cx, &cy);

   bitmap_cursor_move(ed, cx, cy);

   /* Handle selection */
   if (sel_active_is(ed))
     {
        int rx, ry, rw, rh;
        elm_interface_scrollable_content_region_get(ed->scroller, &rx, &ry, &rw, &rh);
        sel_update(ed, ev->cur.canvas.x - ed->sel.x,
                   ev->cur.canvas.y - ed->sel.y);
     }

   if (ed->sel_unit != PUD_UNIT_NONE)
     bitmap_cursor_state_evaluate(ed, cx, cy);

   /* Out of bounds? */
   if ((ed->bitmap.cx + ed->bitmap.cw > (int)ed->pud->map_w) ||
       (ed->bitmap.cy + ed->bitmap.ch > (int)ed->pud->map_h))
     bitmap_cursor_enabled_set(ed, EINA_FALSE);
   else
     bitmap_cursor_enabled_set(ed, EINA_TRUE);

   if (bitmap_cursor_enabled_get(ed))
     {
        if (ev->buttons & 1)
          _click_handle(ed, cx, cy);
     }
}

static void
_mouse_down_cb(void        *data,
               Evas_Object *bmp  EINA_UNUSED,
               void        *info)
{
   Editor *ed = data;
   Evas_Event_Mouse_Down *ev = info;
   int cx, cy;
   int ox, oy;

   evas_object_geometry_get(ed->bitmap.img, &ox, &oy, NULL, NULL);
   bitmap_coords_to_cells(ed, ev->canvas.x - ox, ev->canvas.y - oy, &cx, &cy);

   if (ed->xdebug)
     {
        Cell *c = &(ed->cells[cy][cx]);
        fprintf(stdout, "[%u,%u] =", cx, cy);
        cell_dump(c, stdout);
     }

   /* Handle selection */
   if ((editor_sel_action_get(ed) == EDITOR_SEL_ACTION_SELECTION) &&
       (!sel_active_is(ed)))
     {
        sel_start(ed, ev->canvas.x, ev->canvas.y,
                  evas_key_modifier_is_set(ev->modifiers, "Shift"));
     }
   _click_handle(ed, cx, cy);
}

static void
_mouse_up_cb(void        *data,
             Evas_Object *obj  EINA_UNUSED,
             void        *info EINA_UNUSED)
{
   Editor *ed = data;

   if (sel_active_is(ed))
     sel_end(ed);
}


/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

void
bitmap_unit_draw(Editor       *ed,
                 unsigned int  x,
                 unsigned int  y,
                 Bitmap_Unit   unit_type)
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
   cairo_surface_t *surf;
   cairo_matrix_t mat;
   cairo_t *const cr = ed->bitmap.cr;

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

   sprite = sprite_get(unit, ed->pud->era, orient, &sw, &sh, &flip);
   if (EINA_UNLIKELY(!sprite))
     {
        CRI("Failed to get sprite 0x%x", unit);
        return;
     }

   at_x = (x * TEXTURE_WIDTH) + ((int)(w * TEXTURE_WIDTH) - (int)sw) / 2;
   at_y = (y * TEXTURE_HEIGHT) + ((int)(h * TEXTURE_HEIGHT) - (int)sh) / 2;

   surf = cairo_image_surface_create_for_data(sprite, CAIRO_FORMAT_ARGB32, sw, sh,
                                              cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, sw));

   if (flip)
     {
        cairo_matrix_init(&mat,
                          -1.0             ,  0.0,
                           0.0             ,  1.0,
                          (2.0 * at_x) + sw ,  0.0);
        cairo_save(cr);
        cairo_transform(cr, &mat);
     }

   /* TODO  Colorize */
   

   cairo_set_source_surface(cr, surf, at_x, at_y);
   cairo_rectangle(cr, at_x, at_y, sw, sh);
   cairo_fill(cr);

   if (flip)
     {
        cairo_restore(cr);
     }

   cairo_surface_destroy(surf);

   //DBG("Draw unit %s at_x=%i, at_y=%i", pud_unit2str(unit), at_x, at_y);
   //_draw(ed, sprite, at_x, at_y, sw, sh, flip, col);
}

void
bitmap_selections_draw(Editor       *ed,
                       int           x,
                       int           y,
                       unsigned int  w,
                       unsigned int  h)
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
bitmap_unit_del_at(Editor       *ed,
                   unsigned int  x,
                   unsigned int  y,
                   Eina_Bool     below)
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
bitmap_unit_set(Editor       *ed,
                Pud_Unit      unit,
                Pud_Player    color,
                unsigned int  orient,
                unsigned int  x,
                unsigned int  y,
                unsigned int  w,
                unsigned int  h,
                uint16_t      alter)
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
        c->alter_start_location = !!alter;
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
                  c->alter_above = alter;
               }
             else
               {
                  c->unit_below = unit;
                  c->orient_below = orient;
                  c->player_below = color;
                  c->anchor_below = 0;
                  c->spread_x_below = spread_x;
                  c->spread_y_below = spread_y;
                  c->alter_below = alter;
               }
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
bitmap_tile_draw(Editor       *ed,
                 unsigned int  x,
                 unsigned int  y)
{
   cairo_surface_t *atlas;
   unsigned int ox, oy, px, py;

   atlas = texture_atlas_get(ed->pud->era);
   if (EINA_UNLIKELY(!atlas))
     {
        ERR("Failed to get atlas for era %s", pud_era2str(ed->pud->era));
        return;
     }

   if (EINA_UNLIKELY(!texture_access_test(ed->cells[y][x].tile, atlas, &ox, &oy)))
     {
        ERR("Cannot map tile texture 0x%04x", ed->cells[y][x].tile);
        return;
     }

   px = x * TEXTURE_WIDTH;
   py = y * TEXTURE_HEIGHT;

   cairo_set_source_surface(ed->bitmap.cr, atlas,
                            (int)px - (int)ox, (int)py - (int)oy);
   cairo_rectangle(ed->bitmap.cr, px, py, TEXTURE_WIDTH, TEXTURE_HEIGHT);
   cairo_fill(ed->bitmap.cr);

   minimap_render(ed, x, y, 1, 1);
}

enum
{
   TL, T, TR,
   L,     R,
   BL, B, BR
};

static inline uint8_t
_conflict_solve(uint8_t    imposed,
                uint8_t    conflict,
                Eina_Bool *result_is_conflict)
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
   Eina_Rectangle zone;

#define _TILE_RESOLVE(T, SUB, X, Y) \
   next[T].SUB = _conflict_solve(imposed, cells[Y][X].tile_ ## SUB, \
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

   EINA_RECTANGLE_SET(&zone, x - 1, y - 1, 3, 3);
   bitmap_refresh(ed, &zone);

   return ok;
}

Eina_Bool
bitmap_full_tile_set(Editor   *ed,
                     int       x,
                     int       y,
                     uint16_t  tile)
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

   c->tile = tile;
   minimap_update(ed, x, y);

   return EINA_TRUE;
}

Eina_Bool
bitmap_tile_set(Editor  *ed,
                int      x,
                int      y,
                uint8_t  tl,
                uint8_t  tr,
                uint8_t  bl,
                uint8_t  br,
                uint8_t  seed)
{
   Cell *c = &(ed->cells[y][x]);
   uint16_t tile;

   /* Set tile internals */
   if (tl != TILE_NONE) c->tile_tl = tl;
   if (tr != TILE_NONE) c->tile_tr = tr;
   if (bl != TILE_NONE) c->tile_bl = bl;
   if (br != TILE_NONE) c->tile_br = br;

   tile = tile_calculate(c->tile_tl, c->tile_tr,
                         c->tile_bl, c->tile_br,
                         seed, ed->pud->era);

   return bitmap_full_tile_set(ed, x, y, tile);
}

Eina_Bool
bitmap_add(Editor *ed)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);

   Evas *const e = evas_object_evas_get(ed->win);
   Eo *o;
   Eina_Bool chk;
   const char group[] = "war2edit/cursor";
   unsigned char *pixels;

   DBG("Adding bitmap");

   /* The scroller*/
   evas_object_event_callback_add(ed->scroller, EVAS_CALLBACK_RESIZE,
                                  _bitmap_resize_cb, ed);

   /* Set dimensions */
   ed->bitmap.x_off = 0;
   ed->bitmap.y_off = 0;
   ed->bitmap.cell_w = 32;
   ed->bitmap.cell_h = 32;
   ed->bitmap.cx = -1;
   ed->bitmap.cy = -1;
   ed->bitmap.max_w = ed->bitmap.cell_w * ed->pud->map_w;
   ed->bitmap.max_h = ed->bitmap.cell_h * ed->pud->map_h;

   /* Bitmap image */
   o = ed->bitmap.img = evas_object_image_filled_add(e);
   evas_object_image_colorspace_set(o, EVAS_COLORSPACE_ARGB8888);
   evas_object_size_hint_align_set(o, 0.0, 0.0);
   evas_object_size_hint_weight_set(o, 0.0, 0.0);
   evas_object_size_hint_min_set(o, ed->bitmap.max_w, ed->bitmap.max_h);
   evas_object_size_hint_max_set(o, ed->bitmap.max_w, ed->bitmap.max_h);
   elm_object_content_set(ed->scroller, o);
   evas_object_smart_callback_add(o, "mouse,down", _mouse_down_cb, ed);
   evas_object_smart_callback_add(o, "mouse,move", _mouse_move_cb, ed);
   evas_object_smart_callback_add(o, "mouse,up", _mouse_up_cb, ed);
   evas_object_propagate_events_set(o, EINA_FALSE);
   evas_object_pass_events_set(o, EINA_FALSE);
   evas_object_show(o);

   /* Cairo surface for the game */
   ed->bitmap.surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                ed->bitmap.max_w,
                                                ed->bitmap.max_h);
   ed->bitmap.cr = cairo_create(ed->bitmap.surf);
   cairo_surface_flush(ed->bitmap.surf);
   pixels = cairo_image_surface_get_data(ed->bitmap.surf);
   evas_object_image_size_set(ed->bitmap.img,
                              cairo_image_surface_get_width(ed->bitmap.surf),
                              cairo_image_surface_get_height(ed->bitmap.surf));
   evas_object_image_data_set(ed->bitmap.img, pixels);


   /* Clip - to avoid the cursor overlapping with the scroller */
   o = ed->bitmap.clip = evas_object_rectangle_add(e);
   evas_object_size_hint_align_set(o, 0.0, 0.0);
   evas_object_size_hint_weight_set(o, 0.0, 0.0);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_pass_events_set(o, EINA_TRUE);
   evas_object_show(o);

   /* Cursor */
   o = ed->bitmap.cursor = edje_object_add(e);
   evas_object_pass_events_set(o, EINA_TRUE);
   evas_object_propagate_events_set(o, EINA_FALSE);
   chk = edje_object_file_set(o, ed->edje_file, group);
   if (EINA_UNLIKELY(!chk))
     {
        ERR("Failed to set edje with file %s, group %s", ed->edje_file, group);
        /* FIXME dealloc */
        return EINA_FALSE;
     }
   evas_object_clip_set(o, ed->bitmap.clip);

   bitmap_cursor_size_set(ed, 1, 1);
   bitmap_cursor_move(ed, 0, 0);
   bitmap_cursor_visibility_set(ed, EINA_TRUE);
   bitmap_cursor_enabled_set(ed, EINA_TRUE);

   ed->cells = cell_matrix_new(ed->pud->map_w, ed->pud->map_h);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed->cells, EINA_FALSE);

   sel_add(ed);

   cairo_set_source_rgb(ed->bitmap.cr, 1.0, 1.0, 0.0);
   cairo_rectangle(ed->bitmap.cr, 10, 10, 200, 200);
   cairo_fill(ed->bitmap.cr);

   _bitmap_autoresize(ed);

   return EINA_TRUE;
}

void
bitmap_cell_size_get(const Editor *ed,
                     int          *w,
                     int          *h)
{
   if (w) *w = ed->bitmap.cell_w;
   if (h) *h = ed->bitmap.cell_h;
}

void
bitmap_cursor_size_set(Editor *ed,
                       int     cw,
                       int     ch)
{
   evas_object_resize(ed->bitmap.cursor,
                      cw * ed->bitmap.cell_w,
                      ch * ed->bitmap.cell_h);
   ed->bitmap.cw = cw;
   ed->bitmap.ch = ch;
}

void
bitmap_cursor_size_get(const Editor *ed,
                       int          *w,
                       int          *h)
{
   if (w) *w = ed->bitmap.cw;
   if (h) *h = ed->bitmap.ch;
}

Eina_Bool
bitmap_cursor_enabled_get(const Editor *ed)
{
   return ed->bitmap.cursor_enabled;
}

void
bitmap_cursor_enabled_set(Editor     *ed,
                          Eina_Bool  enabled)
{
   enabled = !!enabled;

   if (ed->bitmap.cursor_enabled == enabled) return;
   ed->bitmap.cursor_enabled = enabled;
   edje_object_signal_emit(ed->bitmap.cursor, (enabled == EINA_TRUE)
                           ? "cursor,enabled"
                           : "cursor,disabled", "war2edit");
}

void
bitmap_cursor_visibility_set(Editor    *ed,
                             Eina_Bool  visible)
{
   if (visible)
     evas_object_show(ed->bitmap.cursor);
   else
     evas_object_hide(ed->bitmap.cursor);
}

void
bitmap_cursor_move(Editor *ed,
                   int     cx,
                   int     cy)
{
   int x, y;
   int ox, oy;


   if ((ed->bitmap.cx == cx) && (ed->bitmap.cy == cy)) return;

   evas_object_geometry_get(ed->bitmap.img, &ox, &oy, NULL, NULL);
   bitmap_cells_to_coords(ed, cx, cy, &x, &y);
   evas_object_move(ed->bitmap.cursor, ox + x, oy + y);

   ed->bitmap.cx = cx;
   ed->bitmap.cy = cy;
}

void
bitmap_coords_to_cells(const Editor *ed,
                       int           x,
                       int           y,
                       int          *cx,
                       int          *cy)
{
   if (cx) *cx = (x + ed->bitmap.x_off) / ed->bitmap.cell_w;
   if (cy) *cy = (y + ed->bitmap.y_off) / ed->bitmap.cell_h;
}

void
bitmap_cells_to_coords(const Editor *ed,
                       int           cx,
                       int           cy,
                       int          *x,
                       int          *y)
{
   if (x) *x = (ed->bitmap.cell_w * cx) - ed->bitmap.x_off;
   if (y) *y = (ed->bitmap.cell_h * cy) - ed->bitmap.y_off;
}

void
bitmap_refresh(Editor               *ed,
               const Eina_Rectangle *zone)
{
   Eina_Rectangle area;
   Eina_Rectangle img/*, update*/;
   int x2, y2;
   int i, j;

   bitmap_visible_zone_cells_get(ed, &area);
   DBG("Visible zone %"EINA_RECTANGLE_FORMAT, EINA_RECTANGLE_ARGS(&area));

   /*
    * If no zone is provided, we will use the whole visible bitmap
    */
   if (zone)
     {
        DBG("Intersection with zone %"EINA_RECTANGLE_FORMAT, EINA_RECTANGLE_ARGS(zone));
        if (!eina_rectangle_intersection(&area, zone))
          {
             WRN("Attempted to refresh a zone that is not visible.");
             return;
          }
     }
   DBG("Refreshing zone %"EINA_RECTANGLE_FORMAT, EINA_RECTANGLE_ARGS(&area));

   /* Pre-calculate loop invariants */
   x2 = area.x + area.w;
   y2 = area.y + area.h;

   /* Safety checks */
   if ((unsigned int)x2 > ed->pud->map_w)
     x2 = ed->pud->map_w;
   if ((unsigned int)y2 > ed->pud->map_h)
     y2 = ed->pud->map_h;

   /*
    * XXX Not great... Simple, Stupid yet
    */

   /* Tiles first (plus start locations) */
   for (j = area.y; j < y2; ++j)
     for (i = area.x; i < x2; ++i)
       {
          bitmap_tile_draw(ed, i, j);
          bitmap_unit_draw(ed, i, j, BITMAP_UNIT_START_LOCATION);
       }

   /* Units below */
   for (j = y2 - 1; j >= area.y; --j)
     for (i = x2 - 1; i >= area.x; --i)
       bitmap_unit_draw(ed, i, j, BITMAP_UNIT_BELOW);

   /* Units above */
   for (j = y2 - 1; j >= area.y; --j)
     for (i = x2 - 1; i >= area.x; --i)
       bitmap_unit_draw(ed, i, j, BITMAP_UNIT_ABOVE);

   /* (Pre)Selections last */
   bitmap_selections_draw(ed, area.x, area.y, area.w, area.h);

   /* Image rectangle (relative to the image itself) */
   img.x = 0;
   img.y = 0;
   evas_object_geometry_get(ed->bitmap.img, NULL, NULL, &img.w, &img.h);

   /*
    * FIXME
    */

   //bitmap_cells_to_coords(ed, area.x, area.y, &update.x, &update.y);
   //bitmap_cells_to_coords(ed, area.x + area.w, area.y + area.h, &update.w, &update.h);

   evas_object_image_data_update_add(ed->bitmap.img, 0, 0, img.w, img.h);
#if 0
   if (eina_rectangle_intersection(&update, &img))
     {
        DBG("Evas update: %"EINA_RECTANGLE_FORMAT, EINA_RECTANGLE_ARGS(&update));
     }
   else
     {
        WRN("Something is fishy... no intersection between the image "
            "contour and the refresh area... "
            "update is %"EINA_RECTANGLE_FORMAT", img is %"EINA_RECTANGLE_FORMAT,
            EINA_RECTANGLE_ARGS(&update), EINA_RECTANGLE_ARGS(&img));
     }
#endif
}

void
bitmap_visible_zone_cells_get(const Editor   *ed,
                              Eina_Rectangle *zone)
{
   int w, h;

   evas_object_geometry_get(ed->bitmap.img, NULL, NULL, &w, &h);

   bitmap_coords_to_cells(ed, 0, 0, &(zone->x), &(zone->y));
   bitmap_coords_to_cells(ed, w, h, &(zone->w), &(zone->h));

   zone->w -= (zone->x - 1);
   zone->h -= (zone->y - 1);
}

void
bitmap_move(Editor *ed,
            int     x,
            int     y)
{
 //  DBG("New xoff,yoff = %i,%i",ed->bitmap.x_off,ed->bitmap.y_off);

   return;
   int dx, dy, abs_dx, abs_dy,  i, j;
   int w, h;
   int start, end;
   int d_size, total_size;
   Eina_Rectangle zone;

   dx = ed->bitmap.x_off - x;
   dy = ed->bitmap.y_off - y;
   abs_dy = abs(dy);
   abs_dx = abs(dx);

   ed->bitmap.x_off = x;
   ed->bitmap.y_off = y;

   evas_object_geometry_get(ed->bitmap.img, NULL, NULL, &w, &h);

   if (dy != 0)
     {
        total_size = w * 4 * sizeof(uint8_t) * h;
        d_size = abs_dy * w * 4 * sizeof(uint8_t);

        /*
         * This corner case will happen when we will scroll more than the bitmap
         * height. We will refresh the whole bitmap then... so we need to truncate to
         * its maximum size.
         */
        if (total_size < d_size)
          d_size = total_size;

        zone.h = abs_dy;
        if (dy < 0) /* translate up */
          {
             end = 0;
             start = d_size;
             zone.y = h - abs_dy;
          }
        else /* translate down */
          {
             start = 0;
             end = d_size;
             zone.y = 0;
          }

        zone.x = 0;
        zone.w = w;

        memmove(&(ed->bitmap.pixels[end]), &(ed->bitmap.pixels[start]), total_size - d_size);

        bitmap_coords_to_cells(ed, zone.x, zone.y, &zone.x, &zone.y);
        bitmap_coords_to_cells(ed, zone.w, zone.h, &zone.w, &zone.h);
        zone.w++; zone.h++;

        INF("Refreshing zone: %"EINA_RECTANGLE_FORMAT, EINA_RECTANGLE_ARGS(&zone));
        bitmap_refresh(ed, &zone);
     }
   if (dx != 0)
     {
        total_size = w * 4 * sizeof(uint8_t); /* one line */
        d_size = abs_dx * 4 * sizeof(uint8_t);

        /*
         * This corner case will happen when we will scroll more than the bitmap
         * width. We will refresh the whole bitmap then... so we need to truncate to
         * its maximum size.
         */
        if (total_size < d_size)
          d_size = total_size;


        if (dx < 0) /* Translate to the left */
          {
             start = d_size;
             end = 0;
             zone.x = w - abs_dx;
          }
        else /* Translate to the right */
          {
             start = 0;
             end = d_size;
             zone.x = 0;
          }

        for (j = 0, i = 0; i < h; i++, j += total_size)
          {
             memmove(&(ed->bitmap.pixels[j + end]),
                     &(ed->bitmap.pixels[j + start]), total_size - d_size);
          }

        zone.y = 0;
        zone.h = h;
        zone.w = abs_dx;

        bitmap_coords_to_cells(ed, zone.x, zone.y, &zone.x, &zone.y);
        bitmap_coords_to_cells(ed, zone.w, zone.h, &zone.w, &zone.h);
        zone.w++; zone.h++;

        INF("Refreshing zone: %"EINA_RECTANGLE_FORMAT, EINA_RECTANGLE_ARGS(&zone));
        bitmap_refresh(ed, &zone);
     }

}
