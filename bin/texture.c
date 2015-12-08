/*
 * texture.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

static Eina_Hash *_textures = NULL;
static Eet_File *_tilesets[4] = { NULL, NULL, NULL, NULL };


void *
texture_load(Eet_File     *src,
             unsigned int  key,
             Eina_Bool    *missing)
{
   /* 4 channels (rgba) of 1 byte each */
   const int expected_size = TEXTURE_WIDTH * TEXTURE_HEIGHT * 4 * sizeof(unsigned char);
   void *mem;
   char key_str[8];
   int size;

   snprintf(key_str, sizeof(key_str), "0x%04x", key);

   /* Get raw data (uncompressed). */
   mem = eet_read(src, key_str, &size);
   if (!mem)
     {
        DBG("Cannot find key \"%s\"", key_str);
        /* Some tiles may not exist on some tilesets
         * Not finding the tile is likely not to be an error. */
        if (missing) *missing = EINA_TRUE;
        return NULL;
     }
   if (missing) *missing = EINA_FALSE;

   /* Check the size */
   if (EINA_UNLIKELY(size != expected_size))
     {
        CRI("Image raw data was loaded with size [%i], expected [%i].",
            size, expected_size);
        free(mem);
        return NULL;
     }

   //DBG("Loaded texture [%s] at <%p>", key_str, mem);

   return mem;
}

Eet_File *
texture_tileset_open(Pud_Era era)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((era >= 0) && (era <= 3), NULL);

   Eet_File *ef;
   const char *file;

   /* Don't load a tileset twice */
   if (_tilesets[era])
     return _tilesets[era];

   switch (era)
     {
      case PUD_ERA_FOREST:    file = DATA_DIR"/tiles/forest.eet";    break;
      case PUD_ERA_WINTER:    file = DATA_DIR"/tiles/winter.eet";    break;
      case PUD_ERA_WASTELAND: file = DATA_DIR"/tiles/wasteland.eet"; break;
      case PUD_ERA_SWAMP:     file = DATA_DIR"/tiles/swamp.eet";     break;
     }

   ef = eet_open(file, EET_FILE_MODE_READ);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ef, NULL);
   DBG("Open tileset file [%s]", file);

   _tilesets[era] = ef;

   return ef;
}

Eina_Bool
texture_init(void)
{
   /* Used to load textures only when they are required */
   _textures = eina_hash_int32_new(EINA_FREE_CB(free));
   if (EINA_UNLIKELY(!_textures))
     {
        CRI("Failed to create Hash for textures");
        goto fail;
     }
   return EINA_TRUE;

fail:
   return EINA_FALSE;
}

void
texture_shutdown(void)
{
   unsigned int i;

   eina_hash_free(_textures);
   for (i = 0; i < EINA_C_ARRAY_LENGTH(_tilesets); ++i)
     {
        if (_tilesets[i])
          {
             eet_close(_tilesets[i]);
             _tilesets[i] = NULL;
          }
     }
}


unsigned char *
texture_get(unsigned int  key,
            Pud_Era       tileset,
            Eina_Bool    *missing)
{
   Eina_Bool chk;
   unsigned char *tex;

   tex = eina_hash_find(_textures, &key);
   if (tex == NULL)
     {
        tex = texture_load(_tilesets[tileset], key, missing);
        if (tex == NULL)
          {
             /* See texture_load() */
             if (EINA_LIKELY(missing && *missing))
               return NULL;
             else
               {
                  ERR("Failed to load texture for key [%u]", key);
                  return NULL;
               }
          }
        chk = eina_hash_add(_textures, &key, tex);
        if (chk == EINA_FALSE)
          {
             ERR("Failed to add texture <%p> to hash", tex);
             free(tex);
             return NULL;
          }
        //DBG("Access key: [%u] (not yet registered). TEX = <%p>", key, tex);
        return tex;
     }
   else
     {
        //DBG("Access key: [%u] (already registered). TEX = <%p>", key, tex);
        return tex;
     }
}


