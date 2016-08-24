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

#ifndef __INWIN_H__
#define __INWIN_H__

typedef enum
{
   INWIN_GENERIC        = 0,
   INWIN_MAINCONFIG,
   INWIN_MAP_PROPERTIES,
   INWIN_PLAYER_PROPERTIES,
   INWIN_STARTING_PROPERTIES,
   INWIN_UNITS_PROPERTIES,
   INWIN_EDITOR_ERROR,
   INWIN_FILE_SELECTOR,
   INWIN_PREFS_DOSBOX
} Inwin;

Evas_Object *inwin_add(Editor *ed);
void
inwin_set(Editor        *ed,
          Evas_Object   *obj,
          Inwin          id,
          const char    *ok_label,
          Evas_Smart_Cb  ok_smart_cb,
          const char    *cancel_label,
          Evas_Smart_Cb  cancel_smart_cb);
void inwin_dismiss(Editor *ed);
Eina_Bool inwin_id_is(const Editor *ed, const Inwin id);
void inwin_activate(Editor *ed);

#endif /* ! __INWIN_H__ */
