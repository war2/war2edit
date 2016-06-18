/*
 * war2edit.h
 *
 * Copyright (c) 2015 - 2016 Jean Guyomarc'h
 */

#ifndef _WAR2EDIT_H_
#define _WAR2EDIT_H_

// FIXME Cmake
#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT


#include <pud.h>
#include <war2.h>
#if 0
#include <cairo.h>
#endif
#include <Elementary.h>

typedef struct _Cell Cell;
typedef struct _Editor Editor;

typedef struct
{
   Evas_Object *clip;
   Evas_Object *shallow;
   Evas_Image *img;

#if 0
   cairo_surface_t *surf;
   cairo_t *cr;
#endif

   int x_off;
   int y_off;

   int cell_w;
   int cell_h;
   int max_w;
   int max_h;

   unsigned char *pixels;

   Edje_Object *cursor;
   int cx, cy, cw, ch;
   Eina_Bool cursor_enabled;
} Bitmap;


Eina_Bool main_in_tree_is(void);

#include "str.h"
#include "prefs.h"
#include "log.h"
#include "tile.h"
#include "texture.h"
#include "mainconfig.h"
#include "toolbar.h"
#include "ipc.h"
#include "cell.h"
#include "undo.h"
#include "file.h"
#include "menu.h"
#include "inwin.h"
#include "sprite.h"
#include "bitmap.h"
#include "editor.h"
#include "minimap.h"
#include "sel.h"

#endif /* ! _WAR2EDIT_H_ */

