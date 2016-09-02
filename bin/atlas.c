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

#include "war2edit.h"

static cairo_surface_t *_atlases[__ATLAS_LAST];

static const char * _atlases_files[__ATLAS_LAST] =
{
   [ATLAS_TILES_FOREST]         = "tiles/forest.png",
   [ATLAS_TILES_WINTER]         = "tiles/winter.png",
   [ATLAS_TILES_WASTELAND]      = "tiles/wasteland.png",
   [ATLAS_TILES_SWAMP]          = "tiles/swamp.png",
   [ATLAS_ICONS_FOREST]         = "icons/forest.png",
   [ATLAS_ICONS_WINTER]         = "icons/winter.png",
   [ATLAS_ICONS_WASTELAND]      = "icons/wasteland.png",
   [ATLAS_ICONS_SWAMP]          = "icons/swamp.png",
};


/*============================================================================*
 *                                Init/Shutdown                               *
 *============================================================================*/

Eina_Bool
atlas_init(void)
{
   return EINA_TRUE;
}

void
atlas_shutdown(void)
{
   unsigned int i;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(_atlases); i++)
     {
        atlas_close(i);
     }
}


/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

Eina_Bool
atlas_open(Atlas atlas)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL((unsigned) atlas >= __ATLAS_LAST, EINA_FALSE);

   char path[PATH_MAX];
   cairo_surface_t *surf;

   if (_atlases[atlas] != NULL) return EINA_TRUE;

   snprintf(path, sizeof(path), "%s/%s",
            elm_app_data_dir_get(), _atlases_files[atlas]);
   path[sizeof(path) - 1] = '\0';

   if (EINA_UNLIKELY(!ecore_file_exists(path)))
     {
        CRI("Atlas path \"%s\" does not exist", path);
        return EINA_FALSE;
     }

   surf = cairo_image_surface_create_from_png(path);
   if (EINA_UNLIKELY(!surf))
     {
        CRI("Failed to open atlas at path \"%s\"", path);
        return EINA_FALSE;
     }
   DBG("Opening atlas at path \"%s\"", path);
   _atlases[atlas] = surf;

   return EINA_TRUE;
}

void
atlas_close(Atlas atlas)
{
   EINA_SAFETY_ON_TRUE_RETURN((unsigned) atlas >= __ATLAS_LAST);

   if (_atlases[atlas])
     {
        cairo_surface_destroy(_atlases[atlas]);
        _atlases[atlas] = NULL;
     }
}

cairo_surface_t *
atlas_get(Atlas atlas)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL((unsigned) atlas >= __ATLAS_LAST, NULL);
   return _atlases[atlas];
}


/*============================================================================*
 *                            Texture Specific API                            *
 *============================================================================*/

Eina_Bool
atlas_texture_access_test(uint16_t         tile,
                          cairo_surface_t *atlas EINA_UNUSED,
                          unsigned int    *x_off,
                          unsigned int    *y_off)
{
   unsigned int major, minor;
   const unsigned int solid_offset = (0xd + 1) * 0x9 * TEXTURE_HEIGHT;
   unsigned int x, y;
   //unsigned char *pixels, *px;

   major = (tile & 0x0f00) >> 8;
   minor = (tile & 0x00f0) >> 4;

   if (major == 0) /* Solid tiles */
     {
        y = solid_offset + ((minor - 1) * TEXTURE_HEIGHT);
        x = ((int)tile & 0x000f) * TEXTURE_WIDTH;
     }
   else
     {
        y = ((major - 1) * TEXTURE_HEIGHT * (0xd + 1)) + (minor * TEXTURE_HEIGHT);
        x = (tile & 0x000f) * TEXTURE_WIDTH;
     }

   if (x_off) *x_off = x;
   if (y_off) *y_off = y;

   //pixels = cairo_image_surface_get_data(atlas);
   //px = &pixels[(x * 4) + (y * cairo_image_surface_get_width(atlas))];
   //if (px[0] != 0xff)
   //  return EINA_FALSE;

   return EINA_TRUE;
}

cairo_surface_t *
atlas_texture_get(Pud_Era era)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL((unsigned) era > PUD_ERA_SWAMP, NULL);
   return _atlases[era];
}

cairo_surface_t *
atlas_icon_get(Pud_Era era)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL((unsigned) era >= PUD_ERA_SWAMP, NULL);
   return _atlases[era + 4];
}
