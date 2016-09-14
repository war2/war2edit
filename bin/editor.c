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

/*
 * There will be one unit descriptor per unit.
 * So, I prefer to optimize for size here...
 * We get 2 bytes per unit, which is quite nice.
 * We can also optimize by using a mempool, so
 * we have zero fragmentation
 */
typedef struct
{
   uint8_t x : 7;
   uint8_t y : 7;
   uint8_t type : 2;
} Unit_Descriptor;


static Eina_List *_editors = NULL;
static unsigned int _eds = 0;
static Editor *_focused = NULL;
static Elm_Genlist_Item_Class *_itcg = NULL;
static Elm_Genlist_Item_Class *_itc = NULL;


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

   /*
    * Because of the log window that is never closed, we need to query how
    * many editors are still open. If none, let's just terminate.
    */
   if (editors_count() == 0)
     elm_exit();
}

static void
_scroll_cb(void        *data,
           Evas_Object *obj  EINA_UNUSED,
           void        *info EINA_UNUSED)
{
   Editor *const ed = data;

   // FIXME BAAAAAAD!!!! when minimap changes the view,
   // it makes the scroller scroll, then this callback is called,
   // and it loops until bounce ends....
   bitmap_minimap_view_resize(ed);
}

static void
_key_down_cb(void        *data,
             Evas        *evas  EINA_UNUSED,
             Evas_Object *obj   EINA_UNUSED,
             void        *event)
{
   const Evas_Event_Key_Down *const ev = event;
   Editor *const ed = data;
   Eina_Bool ctrl;

   ctrl = evas_key_modifier_is_set(ev->modifiers, "Control");

   if (ctrl)
     {
        if (!strcmp(ev->key, "plus"))
          {
             ed->zoom += 0.25;
          }
        else if (!strcmp(ev->key, "minus"))
          {
             ed->zoom -= 0.25;
          }
     }
   else
     {
        if (!strcmp(ev->key, "BackSpace"))
          editor_handle_delete(ed);
     }
}

static void
_focus_in_cb(void        *data,
             Evas_Object *obj   EINA_UNUSED,
             void        *event EINA_UNUSED)
{
   Editor *const ed = data;
   _focused = ed;
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

static char *
_text_get_cb(void        *data,
             Evas_Object *obj,
             const char  *part EINA_UNUSED)
{
   Unit_Descriptor *const d = data;
   Editor *ed;
   const Cell *c;
   char buf[256];
   int bytes;
   Pud_Unit u;

   ed = evas_object_data_get(obj, "editor");
   c = &(ed->cells[d->y][d->x]);

   cell_unit_get(c, d->type, &u, NULL);

   bytes = snprintf(buf, sizeof(buf), "%s", pud_unit2str(u, PUD_TRUE));
   buf[sizeof(buf) - 1] = '\0';
   return strndup(buf, bytes);
}

static char *
_text_group_get_cb(void        *data,
                   Evas_Object *obj  EINA_UNUSED,
                   const char  *part EINA_UNUSED)
{
   char buf[256];
   int bytes;
   const int player = (int)((uintptr_t)data);

   if (player < 8)
     {
        int b;

        b = snprintf(buf, sizeof(buf), "Player %i (", player + 1);
        bytes = snprintf(buf + b, sizeof(buf) - b, "%s)", pud_color2str(player));
        bytes += b;
        buf[b] -= 0x20; // Uppercase
     }
   else
     {
        bytes = snprintf(buf, sizeof(buf), "Neutral");
     }
   buf[sizeof(buf) - 1] = '\0';

   return strndup(buf, bytes);;
}

static Evas_Object *
_content_get_cb(void        *data,
                Evas_Object *obj,
                const char  *part)
{
   Unit_Descriptor *const d = data;
   Editor *ed;
   Evas_Object *im = NULL;
   Pud_Unit unit;
   Pud_Player col;

   if (!strcmp(part, "elm.swallow.icon"))
     {
        ed = evas_object_data_get(obj, "editor");
        cell_unit_get(&(ed->cells[d->y][d->x]), d->type, &unit, &col);
        im = editor_icon_image_new(obj, pud_unit_icon_get(unit), ed->pud->era, col);
     }
   return im;
}

static Unit_Descriptor *
_unit_descriptor_new(unsigned int x,
                     unsigned int y,
                     Unit         type)
{
   Unit_Descriptor *d;

   d = malloc(sizeof(*d));
   if (EINA_UNLIKELY(!d))
     {
        CRI("Failed to allocate memory");
        return NULL;
     }

   d->x = x;
   d->y = y;
   d->type = type;

   return d;
}

static void
_unit_descriptor_free(Unit_Descriptor *d)
{
   free(d);
}

static void
_del_cb(void        *data,
        Evas_Object *obj  EINA_UNUSED)
{
   Unit_Descriptor *const ud = data;
   _unit_descriptor_free(ud);
}

static void
_unit_show_cb(void        *data,
              Evas_Object *obj  EINA_UNUSED,
              void        *info)
{
   Editor *const ed = data;
   Elm_Object_Item *const eoi = info;
   const Unit_Descriptor *d;
   int x, y, w, h;

   d = elm_object_item_data_get(eoi);
   bitmap_cells_to_coords(ed, d->x, d->y, &x, &y);

   elm_interface_scrollable_content_region_get(ed->scroller, NULL, NULL, &w, &h);
   elm_scroller_region_bring_in(ed->scroller, x - w/2, y - h/2, w, h);
}

/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

Eina_Bool
editor_init(void)
{
   _itc = elm_genlist_item_class_new();
   _itc->item_style = "default";
   _itc->func.text_get = _text_get_cb;
   _itc->func.content_get = _content_get_cb;
   _itc->func.del = _del_cb;

   _itcg = elm_genlist_item_class_new();
   _itcg->item_style = "group_index";
   _itcg->func.text_get = _text_group_get_cb;

   ipc_spawns_cb_set(_spawn_cb, NULL);
   return EINA_TRUE;
}

void
editor_shutdown(void)
{
   Editor *ed;

   EINA_LIST_FREE(_editors, ed)
      editor_free(ed);

   elm_genlist_item_class_free(_itc);
   _itc = NULL;
   elm_genlist_item_class_free(_itcg);
   _itcg = NULL;
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
   snapshot_del(ed);
   free(ed);
}

static void
_dismiss_cb(void        *data,
            Evas_Object *obj  EINA_UNUSED,
            void        *info EINA_UNUSED)
{
   Editor *const ed = data;
   evas_object_del(ed->pop);
   ed->pop = NULL;
}

void
editor_error(Editor     *ed,
             const char *fmt, ...)
{
   Evas_Object *b;
   char msg[4096];
   va_list args;

   va_start(args, fmt);
   vsnprintf(msg, sizeof(msg), fmt, args);
   va_end(args);
   msg[sizeof(msg) - 1] = '\0';

   ed->pop = elm_popup_add(ed->win);
   b = elm_button_add(ed->pop);
   elm_object_text_set(b, "Oops, Ok");
   elm_object_part_content_set(ed->pop, "button1", b);
   evas_object_smart_callback_add(b, "clicked", _dismiss_cb, ed);

   elm_object_text_set(ed->pop, msg);
   evas_object_show(ed->pop);
}

static void
_hover_show_cb(void        *data,
               Evas_Object *obj  EINA_UNUSED,
               void        *info EINA_UNUSED)
{
   Editor *const ed = data;
   elm_layout_signal_emit(ed->lay, "war2edit,tileselector,show", "war2edit");
}

static void
_pop_tileselector_cb(void        *data,
                       Evas_Object *obj      EINA_UNUSED,
                       const char  *emission EINA_UNUSED,
                       const char  *source   EINA_UNUSED)
{
   Editor *const ed = data;
   evas_object_show(ed->hover);
}

static void
_hover_dismissed_cb(void        *data,
                    Evas_Object *obj  EINA_UNUSED,
                    void        *info EINA_UNUSED)
{
   Editor *const ed = data;
   elm_layout_signal_emit(ed->lay, "war2edit,tileselector,hide", "war2edit");
}


Editor *
editor_new(const char   *pud_file,
           unsigned int  debug)
{
   Editor *ed;
   char title[512];
   Evas_Object *o;
   Eina_Bool open_pud = EINA_FALSE, chk;
   const char contents[] = "war2edit.main.contents";
   const char group[] = "war2edit/main";
   int i;

   DBG("Creating editor with path %s and debug flags 0x%x", pud_file, debug);

   ed = calloc(1, sizeof(Editor));
   EINA_SAFETY_ON_NULL_GOTO(ed, err_ret);

   ed->debug = debug;
   ed->zoom = 1.0;

     // FIXME cleanup on error + set max size
   ed->orc_menus = eina_array_new(4);
   ed->human_menus = eina_array_new(4);

   /* No previous click */
   ed->prev_x = -1;
   ed->prev_y = -1;

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
   evas_object_resize(ed->win, 960, 480);
   evas_event_callback_add(evas_object_evas_get(ed->win),
                           EVAS_CALLBACK_CANVAS_FOCUS_IN,
                           _focus_in_cb, ed);

   /* Get the main menu */
   menu_add(ed);

   o = ed->lay = elm_layout_add(ed->win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   chk = elm_layout_file_set(o, main_edje_file_get(), group);
   if (EINA_UNLIKELY(!chk))
     {
        CRI("Failed to set layout");
        goto err_win_del;
     }
   evas_object_show(o);
   elm_win_resize_object_add(ed->win, o);

   /* File selector */
   file_selector_add(ed);

   /* Add a box to put widgets in it */
   o = ed->mainbox = elm_box_add(ed->lay);
   EINA_SAFETY_ON_NULL_GOTO(o, err_win_del);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(o, EINA_FALSE);
   elm_layout_content_set(ed->lay, contents, o);
   evas_object_show(o);

   /* Scroller */
   ed->scroller = elm_scroller_add(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(ed->scroller, err_win_del);
   evas_object_size_hint_weight_set(ed->scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ed->scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_event_callback_add(ed->scroller, EVAS_CALLBACK_KEY_DOWN, _key_down_cb, ed);
   evas_object_smart_callback_add(ed->scroller, "scroll", _scroll_cb, ed);
   elm_box_pack_end(o, ed->scroller);
   evas_object_show(ed->scroller);

   o = ed->units_genlist = elm_genlist_add(ed->win);
   evas_object_data_set(o, "editor", ed);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_content_set(ed->lay, "war2edit.main.sidepanel", o);
   evas_object_show(o);

   for (i = 0; i < (int) EINA_C_ARRAY_LENGTH(ed->gen_group_players); i++)
     {
        ed->gen_group_players[i] =
           elm_genlist_item_append(ed->units_genlist, _itcg,
                                   (void *)(uintptr_t)i,
                                   NULL, ELM_GENLIST_ITEM_GROUP,
                                   NULL, NULL);
     }
   ed->gen_group_neutral =
      elm_genlist_item_append(ed->units_genlist, _itcg,
                              (void *)(uintptr_t)PUD_PLAYER_NEUTRAL,
                              NULL, ELM_GENLIST_ITEM_GROUP,
                              NULL, NULL);

   ed->tileselector = elm_button_add(ed->lay);
   o = elm_icon_add(ed->tileselector);
   elm_icon_standard_set(o, "view-list-icons");
   elm_object_part_content_set(ed->tileselector, "icon", o);
   evas_object_size_hint_align_set(ed->tileselector, 0.0, EVAS_HINT_FILL);
   elm_layout_content_set(ed->lay, "war2edit.main.tileselector", ed->tileselector);

   ed->hover = elm_hover_add(ed->win);
   elm_object_style_set(ed->hover, "popout");
   elm_hover_parent_set(ed->hover, ed->win);
   elm_hover_target_set(ed->hover, ed->tileselector);
   evas_object_smart_callback_add(ed->tileselector, "clicked", _hover_show_cb, ed);
   elm_layout_signal_callback_add(ed->lay, "war2edit,tileselector,pop",
                                  "war2edit", _pop_tileselector_cb, ed);
   evas_object_smart_callback_add(ed->hover, "dismissed", _hover_dismissed_cb, ed);

   toolbar_add(ed, ed->hover);

   elm_object_part_content_set(ed->hover, "right", ed->segs[3]);
   elm_object_part_content_set(ed->hover, "top", ed->segs[1]);
   elm_object_part_content_set(ed->hover, "bottom", ed->segs[2]);
   elm_object_part_content_set(ed->hover, "middle", ed->segs[0]);

   /* Add inwin */
   inwin_add(ed);

   unitselector_add(ed);

   /* Show window */
   evas_object_show(ed->win);

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

   /* Add to list of editor windows */
   _editors = eina_list_append(_editors, ed);
   return ed;

err_win_del:
   evas_object_del(ed->win);
err_free:
   free(ed);
err_ret:
   return NULL;
}

Eina_Bool
editor_save(Editor     *ed,
            const char *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);

   Eina_Bool chk;
   Pud_Error_Description err;
   Pud *pud = ed->pud; /* Save indirections... */

   /* Sync the hot changes to the Pud structure */
   if (EINA_UNLIKELY(!editor_sync(ed)))
     {
        CRI("Failed to sync edtor");
        return EINA_FALSE;
     }

   /* Verify it is ok */
   pud_check(pud, &err);
   switch (err.type)
     {
      case PUD_ERROR_NONE:
         DBG("Consistancy check detected nothing wrong");
         break;

      case PUD_ERROR_TOO_MUCH_START_LOCATIONS:
         EDITOR_ERROR(ed,
                      "An extra start location was found at %u,%u (%s)",
                      err.data.unit->x, err.data.unit->y,
                      pud_color2str(err.data.unit->owner));
         return EINA_FALSE;

      case PUD_ERROR_EMPTY_PLAYER:
         EDITOR_ERROR(ed,
                      "Player %i (%s) has a start location but no units",
                      err.data.player + 1, pud_color2str(err.data.player));
         return EINA_FALSE;

      case PUD_ERROR_NO_START_LOCATION:
         EDITOR_ERROR(ed,
                      "Player %i (%s) has units but no start location",
                      err.data.player + 1, pud_color2str(err.data.player));
         return EINA_FALSE;

      case PUD_ERROR_NOT_ENOUGH_START_LOCATIONS:
         EDITOR_ERROR(ed,
                      "There is %u start locations. At least 2 are expected",
                      err.data.count);
         return EINA_FALSE;

      case PUD_ERROR_NOT_INITIALIZED:
      case PUD_ERROR_UNDEFINED:
      default:
         EDITOR_ERROR(ed, "Internal error 0x%x. Please report error", err.type);
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

   editor_notif_send(ed, "Saved");
   INF("Map has been saved to \"%s\"", file);
   return EINA_TRUE;

panic:
   return EINA_FALSE;
}

Eina_Bool
editor_sync(Editor *ed)
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
   Pud_Unit_Data *u;
   void *tmp;

   /* We never known th exact amount of units because they are modified
    * on the fly. Here, we stop edition to sync */
   tmp = realloc(pud->units, pud->units_count * sizeof(*u));
   if (EINA_UNLIKELY(!tmp))
     {
        CRI("Failed to realloc(%u) units", pud->units_count);
        goto fail;
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
                       goto fail;
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
                       goto fail;
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
                       goto fail;
                    }

                  u = &(pud->units[i++]);
                  u->x = x;
                  u->y = y;
                  if (c->start_location_human == 1)
                    u->type = PUD_UNIT_HUMAN_START;
                  else
                    u->type = PUD_UNIT_ORC_START;
                  u->owner = c->start_location;
                  u->alter = 0;
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
   if (EINA_UNLIKELY(k != pud->tiles))
     {
        CRI("Only %u tiles have been synced. Expected %u",
            k, pud->tiles);
        goto fail;
     }

   if (EINA_UNLIKELY(i != pud->units_count))
     {
        CRI("File may have been corrupted. %i units have been written."
            " Expected %i", i, pud->units_count);
        goto fail;
     }

   return EINA_TRUE;

fail:
   EDITOR_ERROR(ed, "Failed to synchronize PUD file. Please open console for details.");
   return EINA_FALSE;
}

static Unit
_unit_to_type(Pud_Unit unit)
{
   if (pud_unit_flying_is(unit))
     return UNIT_ABOVE;
   else if (pud_unit_start_location_is(unit))
     return UNIT_START_LOCATION;
   else
     return UNIT_BELOW;
}

Eina_Bool
editor_load(Editor     *ed,
            const char *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);

   unsigned int i, j, sw, sh, count;
   Pud *pud;
   Pud_Unit_Data *u;
   uint16_t tile;
   uint8_t bl, br, tl, tr, seed;

   DBG("Loading %s\n", file);

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

   atlas_texture_open(pud->era);
   atlas_icon_open(pud->era);
   sprite_buildings_open(pud->era);

   if (EINA_UNLIKELY(!bitmap_add(ed)))
     {
        CRI("Failed to create bitmap");
        return EINA_FALSE;
     }

   minimap_add(ed);

   // TODO split the map into parts, and do a parallel load
   for (j = 0; j < pud->map_h; j++)
     for (i = 0; i < pud->map_w; i++)
       {
          tile = pud_tile_get(pud, i, j);
          tile_decompose(tile, &tl, &tr, &bl, &br, &seed);
          bitmap_tile_set(ed, i, j, tl, tr, bl, br, seed, EINA_TRUE);
       }

   for (i = 0; i < pud->units_count; ++i)
     {
        u = &(pud->units[i]);
        sprite_tile_size_get(u->type, &sw, &sh);
        bitmap_unit_set(ed, u->type, u->owner,
                        sprite_info_random_get(), u->x, u->y, sw, sh,
                        u->alter);
     }
   snapshot_add(ed);
   bitmap_refresh(ed, NULL);
   minimap_show(ed);
   minimap_render(ed, 0, 0, ed->pud->map_w, ed->pud->map_h);

   count = pud->units_count;
   pud->units_count = 0;
   for (i = 0; i < count; i++)
     {
        const Pud_Unit_Data *const ud = &(pud->units[i]);
        editor_unit_ref(ed, ud->x, ud->y, _unit_to_type(ud->type));
     }
   if (EINA_UNLIKELY(pud->units_count != count))
     {
        CRI("Failed to recount units");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

void
editor_units_recount(Editor *ed)
{
   unsigned int count = 0, i, j;
   const Cell *c;

   for (j = 0; j < ed->pud->map_h; j++)
     for (i = 0; i < ed->pud->map_w; i++)
       {
          c = &(ed->cells[j][i]);
          if (c->anchor_above) count++;
          if (c->anchor_below) count++;
          if (c->start_location != CELL_NOT_START_LOCATION) count++;
       }

   DBG("Recounting units... before: %u, now: %u", ed->pud->units_count, count);
   ed->pud->units_count = count;
}

Eina_Bool
editor_unit_ref(Editor       *ed,
                unsigned int  x,
                unsigned int  y,
                Unit          type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);
   if (EINA_UNLIKELY(ed->pud->units_count >= UINT16_MAX))
     return EINA_FALSE;

   Unit_Descriptor *d;
   Eina_Bool ret = EINA_FALSE;
   unsigned int player;
   const Cell *c;
   Elm_Object_Item *eoi;

   d = _unit_descriptor_new(x, y, type);
   if (EINA_UNLIKELY(!d))
     {
        CRI("Failed to create unit descriptor");
        goto end;
     }

   c = &(ed->cells[y][x]);
   cell_unit_get(c, type, NULL, &player);

   if (player < 8)
     eoi = ed->gen_group_players[player];
   else if (player == PUD_PLAYER_NEUTRAL)
     eoi = ed->gen_group_neutral;
   else
     {
        ERR("Invalid player number %i at %u,%u (0x%x)",
            player, x, y, type);
        goto end;
     }

   elm_genlist_item_append(ed->units_genlist, _itc,
                           d, eoi, ELM_GENLIST_ITEM_NONE,
                           _unit_show_cb, ed);

   ret = EINA_TRUE;
end:
   ed->pud->units_count++;
   return ret;
}

static Elm_Object_Item *
_unit_find(const Editor *ed,
           unsigned int x,
           unsigned int y,
           Unit type)
{
   Unit_Descriptor *d;
   Elm_Object_Item *eoi;

   for (eoi = elm_genlist_first_item_get(ed->units_genlist);
        eoi != NULL;
        eoi = elm_genlist_item_next_get(eoi))
     {
        d = elm_object_item_data_get(eoi);
        /*
         * This is so dirty!! For group items, we pass data as an integer
         * value. If will never be greater than 0xf in those cases.
         * It avoids to allocate memory, and has no chance to be assimilated
         * as another address.
         */
        if (d > (Unit_Descriptor *)PUD_PLAYER_NEUTRAL)
          {
             if ((d->x == x) && (d->y == y) && (d->type == type))
               {
                  return eoi;
               }
          }
     }

   return NULL;
}

Eina_Bool
editor_units_list_update(Editor *ed)
{
   elm_genlist_realized_items_update(ed->units_genlist);
   return EINA_TRUE;
}

Eina_Bool
editor_unit_unref(Editor       *ed,
                  unsigned int  x,
                  unsigned int  y,
                  Unit          type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);
   if (EINA_UNLIKELY(ed->pud->units_count <= 0))
     {
        CRI("Attempt to unref, but units count is already %u", ed->pud->units_count);
        return EINA_FALSE;
     }

   Elm_Object_Item *eoi;

   eoi = _unit_find(ed, x, y, type);
   elm_object_item_del(eoi);

   DBG("Deleting unit: (%u, %u, 0x%x)", x, y, type);

   ed->pud->units_count--;
   return EINA_TRUE;
}

void
editor_name_set(Editor     *ed,
                const char *name)
{
   EINA_SAFETY_ON_NULL_RETURN(ed);
   EINA_SAFETY_ON_NULL_RETURN(name);

   elm_win_title_set(ed->win, name);
}

uint16_t
editor_alter_defaults_get(const Editor   *ed    EINA_UNUSED,
                          const Pud_Unit  unit)
{
   switch (unit)
     {
      case PUD_UNIT_GOLD_MINE:
         return 24;

      case PUD_UNIT_OIL_PATCH:
      case PUD_UNIT_HUMAN_OIL_WELL:
      case PUD_UNIT_ORC_OIL_WELL:
         return 10;

      case PUD_UNIT_CRITTER:
         return 0;

      default:
         return 1;
     }
}

void
editor_tb_sel_set(Editor     *ed,
                  Editor_Sel  sel)
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
editor_handle_delete(Editor *ed)
{
   if (!sel_empty_is(ed))
     sel_del(ed);
}

Editor *
editor_focused_get(void)
{
   return _focused;
}

static Eina_Bool
_dismiss_notif_cb(void *data)
{
   Editor *const ed = data;

   elm_layout_signal_emit(ed->lay, "war2edit,notif,hide", "war2edit");
   return ECORE_CALLBACK_CANCEL;
}

void
editor_notif_send(Editor *ed,
                  const char *msg, ...)
{
   va_list args;
   char buf[512];

   va_start(args, msg);
   vsnprintf(buf, sizeof(buf), msg, args);
   va_end(args);
   buf[sizeof(buf) - 1] = '\0';

   elm_layout_text_set(ed->lay, "war2edit.main.notif", buf);
   elm_layout_signal_emit(ed->lay, "war2edit,notif,show", "war2edit");

   ecore_timer_add(5.0, _dismiss_notif_cb, ed);
}

unsigned int
editors_count(void)
{
   return eina_list_count(_editors);
}

static void
_free_surf_cb(void        *data,
         Evas        *e    EINA_UNUSED,
         Evas_Object *obj  EINA_UNUSED,
         void        *info EINA_UNUSED)
{
   cairo_surface_t *const surf = data;
   cairo_surface_destroy(surf);
}

Evas_Object *
editor_icon_image_new(Evas_Object *parent,
                      Pud_Icon     icon,
                      Pud_Era      era,
                      Pud_Player   color)
{
   Evas_Object *im;
   cairo_surface_t *surf;
   unsigned char *px;

   surf = atlas_icon_colorized_get(era, icon, color);
   if (EINA_UNLIKELY(!surf))
     {
        CRI("Failed to get surface for icon 0x%x, era 0x%x, color 0x%x",
            icon, era, color);
        goto fail;
     }
   px = cairo_image_surface_get_data(surf);
   if (EINA_UNLIKELY(!px))
     {
        CRI("Failed to get pixels from surface %p", surf);
        goto fail;
     }
   im = evas_object_image_filled_add(evas_object_evas_get(parent));
   if (EINA_UNLIKELY(!px))
     {
        CRI("Failed to create evas_object_image");
        goto fail;
     }
   evas_object_image_colorspace_set(im, EVAS_COLORSPACE_ARGB8888);
   evas_object_image_size_set(im, ICON_WIDTH, ICON_HEIGHT);
   evas_object_image_data_set(im, px);
   evas_object_show(im);
   evas_object_event_callback_add(im, EVAS_CALLBACK_FREE, _free_surf_cb, surf);

   return im;

fail:
   return NULL;
}

Eina_Bool
editor_player_switch_race(Editor     *ed,
                          Pud_Player  player)
{
   Cell *c;
   unsigned int i, j;

   snapshot_push(ed);
   for (j = 0; j < ed->pud->map_h; j++)
     for (i = 0; i < ed->pud->map_w; i++)
       {
          c = &(ed->cells[j][i]);
          if (c->player_above == player)
            c->unit_above = pud_unit_switch_side(c->unit_above);
          if (c->player_below == player)
            c->unit_below = pud_unit_switch_side(c->unit_below);
       }
   snapshot_push_done(ed);

   editor_units_list_update(ed);
   bitmap_refresh(ed, NULL);
   return EINA_TRUE;
}

void
editor_tileselector_hide(Editor *ed)
{
   elm_hover_dismiss(ed->hover);
}
