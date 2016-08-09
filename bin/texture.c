/*
 * texture.c
 *
 * Copyright (c) 2015 - 2016 Jean Guyomarc'h
 */

#include "war2edit.h"

static cairo_surface_t *_atlases[4] = { NULL, NULL, NULL, NULL };

Eina_Bool
texture_tileset_open(Pud_Era era)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((era >= 0) && (era <= 3), EINA_FALSE);

   const char *file = NULL;
   char path[PATH_MAX];
   cairo_surface_t *surf;

   /* Don't load a tileset twice */
   if (_atlases[era])
     return EINA_TRUE;

   switch (era)
     {
      case PUD_ERA_FOREST:    file = "tiles/forest.png";    break;
      case PUD_ERA_WINTER:    file = "tiles/winter.png";    break;
      case PUD_ERA_WASTELAND: file = "tiles/wasteland.png"; break;
      case PUD_ERA_SWAMP:     file = "tiles/swamp.png";     break;
     }

   snprintf(path, sizeof(path), "%s/%s", elm_app_data_dir_get(), file);

   if (EINA_UNLIKELY(!ecore_file_exists(path)))
     {
        CRI("Atlas path \"%s\" does not exist", path);
        return EINA_FALSE;
     }
   surf = cairo_image_surface_create_from_png(path);
   if (EINA_UNLIKELY(!surf))
     {
        CRI("Failed to open tileset at path \"%s\"", path);
        return EINA_FALSE;
     }
   DBG("Open atlas file [%s]", path);
   _atlases[era] = surf;

   return EINA_TRUE;
}

Eina_Bool
texture_init(void)
{
   return EINA_TRUE;
}

void
texture_shutdown(void)
{
   unsigned int i;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(_atlases); i++)
     {
        if (_atlases[i])
          {
             cairo_surface_destroy(_atlases[i]);
             _atlases[i] = NULL;
          }
     }
}

Eina_Bool
texture_access_test(uint16_t         tile,
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
texture_atlas_get(Pud_Era era)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((era >= 0) && (era <= 3), NULL);
   return _atlases[era];
}
