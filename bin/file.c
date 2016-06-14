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
   if (inwin_id_is(ed, INWIN_FILE_SELECTOR))
     inwin_activate(ed);
   else
     inwin_set(ed, ed->fs,
               INWIN_FILE_SELECTOR,
               NULL, NULL, NULL, NULL);
   evas_object_show(ed->fs);
}

static void
_fs_hide(Editor *ed)
{
   inwin_dismiss(ed);
   evas_object_hide(ed->fs);
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

   save = elm_interface_fileselector_is_save_get(obj);
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
        if (ed->pud)
          {
             // TODO
             CRI("TODO PROMPT FOR SAVE");
          }
        editor_load(ed, file);
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

   elm_interface_fileselector_folder_only_set(obj, EINA_FALSE);
   elm_interface_fileselector_hidden_visible_set(obj, EINA_FALSE);
   elm_interface_fileselector_sort_method_set(obj, ELM_FILESELECTOR_SORT_BY_FILENAME_ASC);
   elm_interface_fileselector_multi_select_set(obj, EINA_FALSE);
   elm_interface_fileselector_expandable_set(obj, EINA_TRUE);
   elm_interface_fileselector_mode_set(obj, ELM_FILESELECTOR_LIST);
   elm_obj_fileselector_buttons_ok_cancel_set(obj, EINA_TRUE);
   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(obj, "done", _done_cb, ed);

   ed->fs = obj;

   return EINA_TRUE;
}

Eina_Bool
file_save_prompt(Editor *ed)
{
   elm_interface_fileselector_is_save_set(ed->fs, EINA_TRUE);
   _fs_show(ed);
   return EINA_TRUE;
}

Eina_Bool
file_load_prompt(Editor *ed)
{
   elm_interface_fileselector_is_save_set(ed->fs, EINA_FALSE);
   _fs_show(ed);
   return EINA_TRUE;
}

