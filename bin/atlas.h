/*
 * Copyright (c) 2016 Jean Guyomarc'h
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

#ifndef __ATLAS_H__
#define __ATLAS_H__

#define TEXTURE_WIDTH  32
#define TEXTURE_HEIGHT 32
#define ICON_WIDTH 46
#define ICON_HEIGHT 38

typedef enum
{
   ATLAS_TILES_FOREST           = 0,
   ATLAS_TILES_WINTER           = 1,
   ATLAS_TILES_WASTELAND        = 2,
   ATLAS_TILES_SWAMP            = 3,
   ATLAS_ICONS_FOREST           = 4,
   ATLAS_ICONS_WINTER           = 5,
   ATLAS_ICONS_WASTELAND        = 6,
   ATLAS_ICONS_SWAMP            = 7,

   __ATLAS_LAST /* Sentinel */
} Atlas;


Eina_Bool atlas_init(void);
void atlas_shutdown(void);
Eina_Bool atlas_open(Atlas atlas);
void atlas_close(Atlas atlas);
cairo_surface_t *atlas_get(Atlas atlas);

cairo_surface_t *atlas_texture_get(Pud_Era era);
cairo_surface_t *atlas_icon_get(Pud_Era era);
Eina_Bool
atlas_texture_access_test(uint16_t         tile,
                          cairo_surface_t *atlas,
                          unsigned int    *x_off,
                          unsigned int    *y_off);

Eina_Bool atlas_texture_open(Pud_Era era);
Eina_Bool atlas_icon_open(Pud_Era era);

#endif /* ! __ATLAS_H__ */
