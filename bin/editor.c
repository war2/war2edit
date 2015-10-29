#include "war2edit.h"

static Eina_List *_editors = NULL;


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


/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

Eina_Bool
editor_init(void)
{
   return EINA_TRUE;
}

void
editor_shutdown(void)
{
   Editor *ed;

   EINA_LIST_FREE(_editors, ed)
      editor_free(ed);
}

void
editor_free(Editor *ed)
{
   EINA_SAFETY_ON_NULL_RETURN(ed);

   _editors = eina_list_remove(_editors, ed);
   cell_matrix_free(ed->cells);
   pud_close(ed->pud);
   evas_object_del(ed->win);
   free(ed);
}

void
editor_error(Editor     *ed,
             const char *msg)
{
   Evas_Object *box, *o, *e;

   /* Confirm button */
   o = elm_button_add(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(o, end);
   elm_object_text_set(o, "Ok");
   evas_object_smart_callback_add(o, "clicked", _error_close_cb, ed);

   /* Info label */
   e = elm_label_add(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(e, end);
   elm_object_text_set(e, msg);
   eo_do(
      e,
      evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
      evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL)
   );

   /* Box to content the UI */
   box = elm_box_add(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(box, end);
   eo_do(
      box,
      evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
      evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL),
      elm_obj_box_horizontal_set(EINA_FALSE),
      elm_obj_box_homogeneous_set(EINA_FALSE),
      elm_obj_box_pack_start(e),
      elm_obj_box_pack_end(o)
   );

   elm_win_inwin_content_set(ed->inwin, box);
   elm_win_inwin_activate(ed->inwin);
   evas_object_show(ed->inwin);
   evas_object_show(box);
   evas_object_show(o);
   evas_object_show(e);

   return;
end:
   CRI("Failed to generate error UI with message [%s]", msg);
   editor_free(ed);
}


Editor *
editor_new(const char *pud_file)
{
   Editor *ed;
   char title[128], wins[32];
   Evas_Object *o, *box;
   int i;

   ed = calloc(1, sizeof(Editor));
   EINA_SAFETY_ON_NULL_GOTO(ed, err_ret);

   for (i = 0; i < 8; i++)
     {
        /* No start location */
        ed->start_locations[i].x = -1;
        ed->start_locations[i].y = -1;

        /* Set initial races */
        if (i % 2 == 0) ed->sides[i] = PUD_SIDE_HUMAN;
        else ed->sides[i] = PUD_SIDE_ORC;
     }


   /* Create window's title */
   snprintf(wins, sizeof(wins), " - %i", eina_list_count(_editors));
   snprintf(title, sizeof(title), "Untitled%s",
            (eina_list_count(_editors) == 0) ? "" : wins);

   /* Create window and set callbacks */
   ed->win = elm_win_util_standard_add("win-editor", title);
   EINA_SAFETY_ON_NULL_GOTO(ed->win, err_free);
   elm_win_focus_highlight_enabled_set(ed->win, EINA_FALSE);
   evas_object_smart_callback_add(ed->win, "delete,request", _win_del_cb, ed);
   evas_object_resize(ed->win, 640, 480);

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
   eo_do(
         box,
         evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, 0.0),
         evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
         elm_obj_box_horizontal_set(EINA_TRUE),
         elm_obj_box_homogeneous_set(EINA_FALSE),
         efl_gfx_visible_set(EINA_TRUE)
      );
   elm_box_pack_end(o, box);
   toolbar_add(ed, box);


   /* Scroller */
   ed->scroller = elm_scroller_add(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(ed->scroller, err_win_del);
   evas_object_size_hint_weight_set(ed->scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ed->scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(o, ed->scroller);
   evas_object_show(ed->scroller);

   /* Mainconfig: get user input for various mainstream parameters */
   mainconfig_add(ed);

   /* Add inwin */
   ed->inwin = elm_win_inwin_add(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(ed->inwin, err_win_del);

   /* Show window */
   evas_object_show(ed->win);

   /* Add to list of editor windows */
   _editors = eina_list_append(_editors, ed);


   menu_unit_selection_reset(ed);

   if (pud_file)
     {
        INF("Opening editor for file %s", pud_file);
        ed->pud = pud_open(pud_file, PUD_OPEN_MODE_R); // XXX mode
        editor_finalize(ed);
        editor_reload(ed);
     }
   else
     {
        /* Create PUD file */
        ed->pud = pud_new();
        if (EINA_UNLIKELY(!ed->pud))
          {
             CRI("Failed to create generic PUD file");
             goto err_win_del;
          }
        mainconfig_show(ed);
     }

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
   unsigned int x, y, k = 0;
   struct _unit *u;
   Cell c;
   Pud *pud = ed->pud; /* Save indirections... */

   eina_stringshare_replace((Eina_Stringshare **)(&pud->filename) ,file);

   //   /* Will never change, so set it once and for all */
   //   pud_dimensions_set(ed->pud, ed->size);
   //   pud_version_set(ed->pud, (ed->has_extension) ? 0x13 : 0x11);
   //   pud_tag_set(ed->pud, rand() % UINT32_MAX);
   //   strncpy(description, "No description available", sizeof(description));
   //   pud_description_set(ed->pud, description);
   //   pud_era_set(ed->pud, ed->era);

   /* Set units count and allocate units */
   ed->pud->units = realloc(ed->pud->units, ed->pud->units_count * sizeof((*ed->pud->units)));
   if (EINA_UNLIKELY(ed->pud->units == NULL))
     {
        CRI("Failed to allocate memory!!");
        goto panic;
     }

   // XXX owner ?
   for (x = 0; x < 8; x++)
     {
        if (ed->start_locations[x].x >= 0)
          ed->pud->starting_points++;
        ed->pud->side.players[x] = ed->sides[x];
     }
   /* FIXME This is by default. Needs to be implemented */
   ed->pud->human_players = 1;
   ed->pud->computer_players = 1;


   for (y = 0; y < ed->pud->map_h; y++)
     {
        for (x = 0; x < ed->pud->map_w; x++)
          {
             c = ed->cells[y][x];

             if (c.unit_below != PUD_UNIT_NONE)
               {
                  u = &(ed->pud->units[k++]);
                  u->x = x;
                  u->y = y;
                  u->type = c.unit_below;
                  u->owner = c.player_below;
                  u->alter = c.alter;
               }
             if (c.unit_above != PUD_UNIT_NONE)
               {
                  u = &(ed->pud->units[k++]);
                  u->x = x;
                  u->y = y;
                  u->type = c.unit_above;
                  u->owner = c.player_above;
                  u->alter = c.alter;
               }
             // FIXME c.tile is wrong
             pud_tile_set(ed->pud, x, y, c.tile);
          }
     }

   if (!pud_check(ed->pud))
     {
        CRI("Pud is not valid.");
        return EINA_FALSE;
     }

   chk = pud_write(ed->pud);
   if (EINA_UNLIKELY(chk == EINA_FALSE))
     {
        CRI("Failed to save pud!!");
        return EINA_FALSE;
     }

   return EINA_TRUE;

panic:
   return EINA_FALSE;
}

Eina_Bool
editor_load(Editor * restrict ed,
            const char *      file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ed, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);

   ed->pud = pud_open(file, PUD_OPEN_MODE_R);
   if (EINA_UNLIKELY(!ed->pud))
     {
        ERR("Failed to load editor from file \"%s\"", file);
        goto fail;
     }

   return EINA_TRUE;

fail:
   return EINA_FALSE;
}

void
editor_reload(Editor *ed)
{
   EINA_SAFETY_ON_NULL_RETURN(ed);

   INF("Editor reload");
   unsigned int i, j, k;
   const Pud *pud = ed->pud;

   if (EINA_UNLIKELY(!pud))
     {
        ERR("Attempt to reload an editor, but no pud was attached");
        return;
     }

   bitmap_reset(ed);

   for (j = 0; j < pud->map_h; ++j)
     {
        k = j * pud->map_w;
        for (i = 0; i < pud->map_w; ++i)
          bitmap_tile_set(ed, i, j, pud->tiles_map[i + k]);
     }

   /* TODO
    *
    * foreach (pud->tiles_map)
    * foreach (pud->units)
    */
}

unsigned char *
editor_texture_tile_access(const Editor * restrict ed,
                           unsigned int            x,
                           unsigned int            y)
{
   unsigned int key;

   key = ed->cells[y][x].tile;
   return texture_get(key, ed->pud->era, NULL);
}

void
editor_finalize(Editor * restrict ed)
{
   texture_tileset_open(ed->pud->era);
   sprite_units_open();
   bitmap_add(ed);
}

