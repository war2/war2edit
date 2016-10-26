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


#ifndef _WAR2EDIT_H_
#define _WAR2EDIT_H_

#include <pud.h>
#include <war2.h>
#include <Eina.h>
#include <cairo.h>
#include <Elementary.h>
#include <lzma.h>

typedef struct _Cell Cell;
typedef struct _Editor Editor;

typedef enum
{
   UNIT_NONE             = 0,
   UNIT_BELOW            = 1,
   UNIT_ABOVE            = 2,
   UNIT_START_LOCATION   = 3,
} Unit;


typedef struct
{
   Evas_Object *clip;
   Evas_Image *img;

   cairo_surface_t *surf;
   cairo_t *cr;

   int x_off;
   int y_off;

   int cell_w;
   int cell_h;
   int max_w;
   int max_h;

   unsigned char *pixels;

   int cx, cy, cw, ch;
   Eina_Bool cursor_enabled;
   Eina_Bool cursor_visible;
   Eina_Bool norender;
} Bitmap;


Eina_Bool main_in_tree_is(void);
const char *main_edje_file_get(void);

#include "str.h"
#include "plugins.h"
#include "log.h"
#include "tile.h"
#include "atlas.h"
#include "mainconfig.h"
#include "toolbar.h"
#include "cell.h"
#include "snapshot.h"
#include "menu.h"
#include "sprite.h"
#include "bitmap.h"
#include "minimap.h"
#include "editor.h"
#include "unitselector.h"
#include "sel.h"

#endif /* ! _WAR2EDIT_H_ */
