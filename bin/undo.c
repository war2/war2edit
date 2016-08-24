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
