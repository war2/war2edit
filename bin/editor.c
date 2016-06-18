/*
 * editor.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

static Eina_List *_editors = NULL;
static Ecore_Event_Handler *_handler = NULL;
static unsigned int _eds = 0;
static Editor *_focused = NULL;


/*============================================================================*
 *                                  Callbacks                                 *
 *============================================================================*/

static void
_win_del_cb(void        *data,
            Evas_Object *obj   EINA_UNUSED,
            void        *event EINA_UNUSED)
{
   Editor *ed = data;
   editor_free(ed);
}

static void
_error_close_cb(void        *data,
                Evas_Object *obj  EINA_UNUSED,
                void        *info EINA_UNUSED)
{
   Editor *ed = data;
   DBG("Closing Editor %p after error...", ed);
   editor_free(ed);
}

static void
_win_resize_cb(void        *data,
               Evas        *e    EINA_UNUSED,
               Evas_Object *obj  EINA_UNUSED,
               void        *info EINA_UNUSED)
{
   editor_view_update(data);
}

static void
_scroll_cb(void        *data,
           Evas_Object *obj  EINA_UNUSED,
           void        *info EINA_UNUSED)
{
   Editor *ed = data;

   // FIXME BAAAAAAD!!!! when minimap changes the view,
   // it makes the scroller scroll, then this callback is called,
   // and it loops until bounce ends....
   editor_view_update(ed);
}

static Eina_Bool
_key_down_cb(void *data  EINA_UNUSED,
             int   type  EINA_UNUSED,
             void *event)
{
   Ecore_Event_Key *ev = event;
   if (!strcmp(ev->keyname, "BackSpace"))
     editor_handle_delete(_focused);

   return ECORE_CALLBACK_PASS_ON;
}

static void
_focus_in_cb(void        *data,
             Evas_Object *obj   EINA_UNUSED,
             void        *event EINA_UNUSED)
{
   _focused = data;
}

static void
_spawn_cb(void         *data  EINA_UNUSED,
          unsigned int  spawns)
{
   Eina_List *l;
   Editor *ed;
   const Eina_Bool select = (spawns == 0) ? EINA_FALSE : EINA_TRUE;

   EINA_LIST_FOREACH(_editors, l, ed)
      toolbar_runner_segment_selected_set(ed, select);
}

/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

Eina_Bool
editor_init(void)
{
   _handler = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, _key_down_cb, NULL);
   if (EINA_UNLIKELY(!_handler))
     {
        CRI("Failed to attach key down handler");
        return EINA_FALSE;
     }

   ipc_spawns_cb_set(_spawn_cb, NULL);
   return EINA_TRUE;
}

void
editor_shutdown(void)
{
   Editor *ed;

   EINA_LIST_FREE(_editors, ed)
      editor_free(ed);
   ecore_event_handler_del(_handler);
}

void
editor_free(Editor *ed)
{
   EINA_SAFETY_ON_NULL_RETURN(ed);

   _editors = eina_list_remove(_editors, ed);
   cell_matrix_free(ed->cells);
   pud_close(ed->pud);
   evas_object_del(ed->win);
   minimap_del(ed);
   eina_array_free(ed->orc_menus);
   eina_array_free(ed->human_menus);
   undo_del(ed);
   free(ed->edje_file);
   free(ed);
}

void
editor_error(Editor     *ed,
             const char *msg)
{
   Evas_Object *box, *o, *e;

   if (inwin_id_is(ed, INWIN_EDITOR_ERROR))
     {
        inwin_activate(ed);
        return;
     }

   /* Confirm button */
   o = elm_button_add(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(o, end);
   elm_object_text_set(o, "Ok");
   evas_object_smart_callback_add(o, "clicked", _error_close_cb, ed);

   /* Info label */
   e = elm_label_add(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(e, end);
   elm_object_text_set(e, msg);
   evas_object_size_hint_weight_set(e, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(e, EVAS_HINT_FILL, EVAS_HINT_FILL);

   /* Box to content the UI */
   box = elm_box_add(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(box, end);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(box, EINA_FALSE);
   elm_box_homogeneous_set(box, EINA_FALSE);
   elm_box_pack_start(box, e);
   elm_box_pack_end(box, o);

   inwin_set(ed, box, INWIN_EDITOR_ERROR, "Ok", NULL, NULL, NULL);
   evas_object_show(box);
   evas_object_show(o);
   evas_object_show(e);

   return;
end:
   CRI("Failed to generate error UI with message [%s]", msg);
   editor_free(ed);
}


Editor *
editor_new(const char *pud_file,
           Eina_Bool   xdebug)
{
   Editor *ed;
   char title[512];
   Evas_Object *o, *box;
   Eina_Bool open_pud = EINA_FALSE;
   char path[PATH_MAX];
   const char theme[] = "default";
   int i, len;

   ed = calloc(1, sizeof(Editor));
   EINA_SAFETY_ON_NULL_GOTO(ed, err_ret);

   ed->xdebug = xdebug;

   /* Get theme */
   if (main_in_tree_is())
     len = snprintf(path, sizeof(path), "%s/themes/%s.edj", BUILD_DATA_DIR, theme);
   else
     {
        len = snprintf(path, sizeof(path),
                       "%s/war2edit/themes/%s.edj", PACKAGE_DATA_DIR, theme);
     }
   path[sizeof(path) - 1] = '\0';
   ed->edje_file = strndup(path, len);
   EINA_SAFETY_ON_NULL_GOTO(ed->edje_file, err_free);

   // FIXME cleanup on error + set max size
   ed->orc_menus = eina_array_new(4);
   ed->human_menus = eina_array_new(4);

   for (i = 0; i < 8; i++)
     {
        /* No start location */
        ed->start_locations[i].x = -1;
        ed->start_locations[i].y = -1;
     }

   /* Create window and set callbacks */
   ed->win = elm_win_util_standard_add("win-editor", "war2edit");
   EINA_SAFETY_ON_NULL_GOTO(ed->win, err_free);
   elm_win_focus_highlight_enabled_set(ed->win, EINA_FALSE);
   evas_object_smart_callback_add(ed->win, "delete,request", _win_del_cb, ed);
   evas_object_event_callback_add(ed->win, EVAS_CALLBACK_RESIZE,
                                  _win_resize_cb, ed);
   evas_object_resize(ed->win, 640, 480);
   evas_event_callback_add(evas_object_evas_get(ed->win),
                           EVAS_CALLBACK_CANVAS_FOCUS_IN,
                           _focus_in_cb, ed);

   /* File selector */
   file_selector_add(ed);

   /* Add a box to put widgets in it */
   o = elm_box_add(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(o, err_win_del);
   ed->mainbox = o;
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(o, EINA_FALSE);
   elm_win_resize_object_add(ed->win, o);
   evas_object_show(o);

   /* Get the main menu */
   menu_add(ed);

   /* Toolbar */
   box = elm_box_add(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(box, err_win_del);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(box, 0.0, EVAS_HINT_FILL);
   elm_box_align_set(box, 0.0, EVAS_HINT_FILL);
   elm_box_horizontal_set(box, EINA_TRUE);
   elm_box_homogeneous_set(box, EINA_FALSE);
   evas_object_show(box);
   elm_box_pack_end(o, box);
   toolbar_add(ed, box);


   /* Scroller */
   ed->scroller = elm_scroller_add(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(ed->scroller, err_win_del);
   evas_object_size_hint_weight_set(ed->scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ed->scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(ed->scroller, "scroll", _scroll_cb, ed);
   elm_box_pack_end(o, ed->scroller);
   evas_object_show(ed->scroller);

   undo_add(ed);

   /* Add inwin */
   inwin_add(ed);

   /* Show window */
   evas_object_show(ed->win);

   /* Add to list of editor windows */
   _editors = eina_list_append(_editors, ed);


   menu_unit_selection_reset(ed);

   /* Are we opening a (supposed) PUD? */
   if (ecore_file_exists(pud_file)) /* NULL case is handled */
     {
        if (ecore_file_is_dir(pud_file))
          {
             CRI("You wanted to open a directory! Please select a file");
             goto err_win_del;
          }
        open_pud = EINA_TRUE;
     }

   if (open_pud)
     {
        INF("Opening editor for file %s", pud_file);
        if (!editor_load(ed, pud_file))
          {
             CRI("Failed to load editor");
             goto err_win_del;
          }
        snprintf(title, sizeof(title), "%s", pud_file);
     }
   else
     {
        /* Create PUD file */
        ed->pud = pud_open_new(pud_file, PUD_OPEN_MODE_R | PUD_OPEN_MODE_W);
        if (EINA_UNLIKELY(!ed->pud))
          {
             CRI("Failed to create generic PUD file");
             goto err_win_del;
          }

        snprintf(title, sizeof(title), "Untitled - %u", _eds++);

        /* Mainconfig: get user input for config parameters */
        mainconfig_show(ed);
     }

   menu_units_side_enable(ed, ed->pud->side.players[ed->sel_player]);

   /* Set window's title */
   editor_name_set(ed, title);

   return ed;

err_win_del:
   evas_object_del(ed->win);
err_free:
   free(ed);
err_ret:
   return NULL;
}

Eina_Bool
editor_save(Editor * restrict ed,
            const char *      file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);

   Eina_Bool chk;
   Pud_Error err;
   Pud *pud = ed->pud; /* Save indirections... */

   /* Sync the hot changes to the Pud structure */
   if (EINA_UNLIKELY(!editor_sync(ed)))
     {
        CRI("Failed to sync edtor");
        return EINA_FALSE;
     }

   /* Verify it is ok */
   err = pud_check(pud);
   if (err != PUD_ERROR_NONE)
     {
        CRI("Pud is not valid. Error: %i", err);
        return EINA_FALSE;
     }

   /* Attribute a filename to the pud itself if none where attributed
    * before */
   if (!pud->filename)
     {
        pud->filename = strdup(file);
        if (!pud->filename)
          {
             CRI("Failed to strdup(\"%s\")", pud->filename);
             goto panic;
          }
        elm_win_title_set(ed->win, file);
     }

   /* Write the PUD to disk */
   chk = pud_write(pud, file);
   if (EINA_UNLIKELY(chk == EINA_FALSE))
     {
        CRI("Failed to save pud!!");
        return EINA_FALSE;
     }

   INF("Map has been saved to \"%s\"", pud->filename);
   return EINA_TRUE;

panic:
   return EINA_FALSE;
}

Eina_Bool
editor_sync(Editor * restrict ed)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed->pud, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed->cells, EINA_FALSE);

   /*
    * Syncs from the ed->cell to the ed->pud
    * This function can be called by an Ecore_Timer for instance,
    * to make regular auto-saves of the Pud
    */

   unsigned int x, y, k, i;
   Pud *pud = ed->pud;
   Cell **cells = ed->cells;
   const Cell *c;
   struct _Pud_Unit *u;
   void *tmp;

   /* We never known th exact amount of units because they are modified
    * on the fly. Here, we stop edition to sync */
   tmp = realloc(pud->units, pud->units_count * sizeof(struct _Pud_Unit));
   if (EINA_UNLIKELY(!tmp))
     {
        CRI("Failed to realloc(%u) units", pud->units_count);
        return EINA_FALSE;
     }
   else
     pud->units = tmp;

   for (k = 0, i = 0, y = 0; y < pud->map_h; ++y)
     {
        for (x = 0; x < pud->map_w; ++x)
          {
             c = &(cells[y][x]);

             if (c->anchor_below)
               {
                  if (EINA_UNLIKELY(i >= pud->units_count))
                    {
                       CRI("Attempt to overflow units");
                       return EINA_FALSE;
                    }

                  u = &(pud->units[i++]);
                  u->x = x;
                  u->y = y;
                  u->type = c->unit_below;
                  u->owner = c->player_below;
                  u->alter = c->alter_below;
               }
             if (c->anchor_above)
               {
                  if (EINA_UNLIKELY(i >= pud->units_count))
                    {
                       CRI("Attempt to overflow units");
                       return EINA_FALSE;
                    }

                  u = &(pud->units[i++]);
                  u->x = x;
                  u->y = y;
                  u->type = c->unit_above;
                  u->owner = c->player_above;
                  u->alter = c->alter_above;
               }
             if (c->start_location != CELL_NOT_START_LOCATION)
               {
                  if (EINA_UNLIKELY(i >= pud->units_count))
                    {
                       CRI("Attempt to overflow units");
                       return EINA_FALSE;
                    }

                  u = &(pud->units[i++]);
                  u->x = x;
                  u->y = y;
                  if (c->start_location_human == 1)
                    u->type = PUD_UNIT_HUMAN_START;
                  else
                    u->type = PUD_UNIT_ORC_START;
                  u->owner = c->start_location;
                  u->alter = c->alter_start_location;
               }

             /* I'm not using pud_tile_set() because I know what I'm doing,
              * and this function if much less performant... */
             pud->tiles_map[k] = c->tile;

             /* Determine action and movement map */
             pud->action_map[k] = TILE_ACTION_GET(c);
             pud->movement_map[k] = TILE_MOVEMENT_GET(c);

             k++;
          }
     }

   if (EINA_UNLIKELY(i != pud->units_count))
     {
        CRI("File may have been corrupted. %i units have been written."
            " Expected %i", i, pud->units_count);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool
editor_load(Editor * restrict  ed,
            const char        *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);

   unsigned int i, j, sw, sh;
   const Pud *pud;
   struct _Pud_Unit *u;
   uint16_t tile;
   uint8_t bl, br, tl, tr, seed;

   printf("%s(%s)\n", __func__, file);

   if (file)
     {
        if (ed->pud) pud_close(ed->pud);
        ed->pud = pud_open(file, PUD_OPEN_MODE_R | PUD_OPEN_MODE_W);
        if (EINA_UNLIKELY(!ed->pud))
          {
             ERR("Failed to load editor from file \"%s\"", file);
             return EINA_FALSE;
          }
     }
   else
     {
        if (EINA_UNLIKELY(!ed->pud))
          {
             CRI("Loading pud without filename and no initialized PUD");
             return EINA_FALSE;
          }
     }

   pud = ed->pud;

   texture_tileset_open(pud->era);
   sprite_buildings_open(pud->era);

   if (!ed->bitmap.img)
     {
        if (EINA_UNLIKELY(!bitmap_add(ed)))
          {
             CRI("Failed to create bitmap");
             return EINA_FALSE;
          }
     }

   minimap_add(ed);

   // TODO split the map into parts, and do a parallel load
   for (j = 0; j < pud->map_h; j++)
     for (i = 0; i < pud->map_w; i++)
       {
          tile = pud_tile_get(pud, i, j);
          tile_decompose(tile, &tl, &tr, &bl, &br, &seed);
          bitmap_tile_set(ed, i, j, tl, tr, bl, br, seed);
       }

   for (i = 0; i < pud->units_count; ++i)
     {
        u = &(pud->units[i]);
        sprite_tile_size_get(u->type, &sw, &sh);
        bitmap_unit_set(ed, u->type, u->owner,
                        sprite_info_random_get(), u->x, u->y, sw, sh,
                        u->alter);
     }
   bitmap_refresh(ed, NULL);
   minimap_render(ed, 0, 0, pud->map_w, pud->map_h);

   return EINA_TRUE;
}

Eina_Bool
editor_unit_ref(Editor * restrict ed)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);
   if (EINA_UNLIKELY(ed->pud->units_count >= UINT16_MAX))
     return EINA_FALSE;

   ed->pud->units_count++;
   return EINA_TRUE;
}

Eina_Bool
editor_unit_unref(Editor * restrict ed)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);
   if (ed->pud->units_count <= 0)
     return EINA_FALSE;

   ed->pud->units_count--;
   return EINA_TRUE;
}

unsigned char *
editor_texture_tile_access(const Editor * restrict ed,
                           unsigned int            x,
                           unsigned int            y)
{
   unsigned int key;

   key = ed->cells[y][x].tile;
   return texture_get(key, ed->pud->era);
}

void
editor_name_set(Editor * restrict  ed,
                const char        *name)
{
   EINA_SAFETY_ON_NULL_RETURN(ed);
   EINA_SAFETY_ON_NULL_RETURN(name);

   elm_win_title_set(ed->win, name);
}

uint16_t
editor_alter_defaults_get(const Editor * restrict ed,
                          const Pud_Unit          unit)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, 0x00);

   // TODO lookup defaults (editor)
   if ((unit == PUD_UNIT_GOLD_MINE) || (unit == PUD_UNIT_OIL_PATCH))
     {
        return 2500;
     }
   else if (unit == PUD_UNIT_CRITTER)
     {
        return 0;
     }
   else
     return 1;
}

void
editor_view_update(Editor *restrict ed)
{
   int rx, ry, rw, rh;
   int cx, cy, cw, ch;
   int cell_w = 0, cell_h = 0;
   float wf, hf;

   elm_interface_scrollable_content_region_get(ed->scroller, &rx, &ry, &rw, &rh);
   bitmap_cell_size_get(ed, &cell_w, &cell_h);
   /* Happens mostly at init time, when UI is unstable */
   if (EINA_UNLIKELY((cell_w == 0) || (cell_h == 0)))
     return;

   wf = (float)cell_w;
   hf = (float)cell_h;

   cw = rintf((float)rw / wf);
   ch = rintf((float)rh / hf);
   cx = rintf((float)rx / wf);
   cy = rintf((float)ry / hf);

   minimap_view_move(ed, cx, cy, EINA_FALSE);
   minimap_view_resize(ed, cw, ch);
}

void
editor_tb_sel_set(Editor *restrict ed,
                  Editor_Sel       sel)
{
   if (sel & EDITOR_SEL_ACTION_MASK)
     editor_sel_action_set(ed, sel);
   if (sel & EDITOR_SEL_SPREAD_MASK)
     editor_sel_spread_set(ed, sel);
   if (sel & EDITOR_SEL_RADIUS_MASK)
     editor_sel_radius_set(ed, sel);
   if (sel & EDITOR_SEL_TINT_MASK)
     editor_sel_tint_set(ed, sel);
}

void
editor_handle_delete(Editor *restrict ed)
{
   if (!sel_empty_is(ed))
     sel_del(ed);
}

Editor *
editor_focused_get(void)
{
   return _focused;
}
