/*
 * file.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"
#include <elm_interface_fileselector.h>

/*============================================================================*
 *                                 Private API                                *
 *============================================================================*/

static void
_fs_show(Editor *ed)
{
   elm_win_inwin_content_set(ed->inwin, ed->fs);
   elm_win_inwin_activate(ed->inwin);
   evas_object_show(ed->inwin);
}

static void
_fs_hide(Editor *ed)
{
   evas_object_hide(ed->inwin);
}


/*============================================================================*
 *                                  Callbacks                                 *
 *============================================================================*/

static void
_done_cb(void        *data,
         Evas_Object *obj   EINA_UNUSED,
         void        *event)
{
   Editor *ed = data;
   Eina_Stringshare *file = event;
   Eina_Bool save;

   if (!file) goto hide_fileselector;

   eo_do(obj, save = elm_interface_fileselector_is_save_get());
   if (save)
     {
        if (ecore_file_is_dir(file))
          {
             ERR("You have selected a directory dumbass!");
             return;
          }

        INF("Saving at path \"%s\"...", file);
        editor_save(ed, file);
     }
   else /* Load */
     {
    // XXX    if (ed->pud)
    // XXX      {
    // XXX         // TODO
    // XXX         CRI("TODO PROMPT FOR SAVE");
    // XXX         pud_close(ed->pud);
    // XXX      }
    // XXX    ed->pud = pud_open(file, PUD_OPEN_MODE_R);
    // XXX    if (EINA_UNLIKELY(!ed->pud))
    // XXX      {
    // XXX         ERR("Failed to open PUD from file \"%s\"", file);
    // XXX         goto hide_fileselector;
    // XXX      }
    // XXX    file_load(ed, ed->pud);
     }

hide_fileselector:
   _fs_hide(ed);
}


/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

Eina_Bool
file_selector_add(Editor *ed)
{
   Evas_Object *obj;

   obj = elm_fileselector_add(ed->win);
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);

   eo_do(
      obj,
      elm_interface_fileselector_folder_only_set(EINA_FALSE),
      elm_interface_fileselector_hidden_visible_set(EINA_FALSE),
      elm_interface_fileselector_sort_method_set(ELM_FILESELECTOR_SORT_BY_FILENAME_ASC),
      elm_interface_fileselector_multi_select_set(EINA_FALSE),
      elm_interface_fileselector_expandable_set(EINA_TRUE),
      elm_interface_fileselector_mode_set(ELM_FILESELECTOR_LIST),
      elm_obj_fileselector_buttons_ok_cancel_set(EINA_TRUE),
      evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
      evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL)
   );
   evas_object_smart_callback_add(obj, "done", _done_cb, ed);

   ed->fs = obj;

   return EINA_TRUE;
}

Eina_Bool
file_save_prompt(Editor *ed)
{
   eo_do(ed->fs, elm_interface_fileselector_is_save_set(EINA_TRUE));
   _fs_show(ed);
   return EINA_TRUE;
}

Eina_Bool
file_load_prompt(Editor *ed)
{
   eo_do(ed->fs, elm_interface_fileselector_is_save_set(EINA_FALSE));
   _fs_show(ed);
   return EINA_TRUE;
}

