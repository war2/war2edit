/*
 * undo.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

Eina_Bool
undo_add(Editor *ed)
{
   ed->undo = eina_array_new(8);
   if (EINA_UNLIKELY(!ed->undo))
     {
        CRI("Failed to allocate Array");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

void
undo_del(Editor *ed)
{
   eina_array_free(ed->undo);
}

void
undo_menu_connect(Editor          *ed   EINA_UNUSED,
                  Elm_Object_Item *undo EINA_UNUSED,
                  Elm_Object_Item *redo EINA_UNUSED)
{
}

void
undo_push(Editor     *ed      EINA_UNUSED,
          const char *message EINA_UNUSED)
{
}

