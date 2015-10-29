#include "war2edit.h"

static Eet_File *_units_ef = NULL;
static Eet_File *_buildings[4] = { NULL, NULL, NULL, NULL };
static Eina_Hash *_sprites = NULL;

void *
sprite_load(Eet_File   *src,
            const char *key,
            int        *x_ret,
            int        *y_ret,
            int        *w_ret,
            int        *h_ret)
{
   unsigned char *mem;
   int size, expected_size;
   uint16_t x, y, w, h;

   mem = eet_read(src, key, &size);
   EINA_SAFETY_ON_NULL_RETURN_VAL(mem, NULL);

   memcpy(&x, mem + 0, sizeof(uint16_t));
   memcpy(&y, mem + 2, sizeof(uint16_t));
   memcpy(&w, mem + 4, sizeof(uint16_t));
   memcpy(&h, mem + 6, sizeof(uint16_t));

   expected_size = (w * h * 4) + 8;
   if (expected_size != size)
     {
        CRI("Sprite data was loaded with size [%i], expected [%i]",
            size, expected_size);
        free(mem);
        return NULL;
     }

   //DBG("Loaded sprite [%s] of size %ix%i (offsets: %i,%i)\n", key, w, h, x, y);

   if (x_ret) *x_ret = x;
   if (y_ret) *y_ret = y;
   if (w_ret) *w_ret = w;
   if (h_ret) *h_ret = h;
   return mem + 8;
}

Eet_File *
sprite_units_open(void)
{
   Eet_File *ef;
   const char *file;

   /* Don't open the units file twice */
   if  (_units_ef)
     return _units_ef;

   file = DATA_DIR"/sprites/units/units.eet";
   ef = eet_open(file, EET_FILE_MODE_READ);
   if (EINA_UNLIKELY(ef == NULL))
     {
        CRI("Failed to open [%s]", file);
        return NULL;
     }
   DBG("Open units file [%s]", file);
   _units_ef = ef;

   return ef;
}

Eet_File *
sprite_buildings_open(Pud_Era era)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((era >= 0) && (era <= 3), NULL);

   Eet_File *ef;
   const char *file;

   /* Don't load buildings file twice */
   if (_buildings[era])
     return _buildings[era];

   switch (era)
     {
      case PUD_ERA_FOREST:    file = DATA_DIR"/sprites/buildings/forest.eet";    break;
      case PUD_ERA_WINTER:    file = DATA_DIR"/sprites/buildings/winter.eet";    break;
      case PUD_ERA_WASTELAND: file = DATA_DIR"/sprites/buildings/wasteland.eet"; break;
      case PUD_ERA_SWAMP:     file = DATA_DIR"/sprites/buildings/swamp.eet";     break;
     }

   ef = eet_open(file, EET_FILE_MODE_READ);
   if (EINA_UNLIKELY(ef == NULL))
     {
        CRI("Failed to open [%s]", file);
        return NULL;
     }
   DBG("Open buildings file [%s]", file);

   _buildings[era] = ef;

   return ef;
}

unsigned char *
sprite_get(Pud_Unit       unit,
           Pud_Era        era,
           Sprite_Info    info,
           int           *x,
           int           *y,
           int           *w,
           int           *h,
           Eina_Bool     *flip_me)
{
   unsigned char *data;
   char key[64];
   Eet_File *ef;
   Eina_Bool chk;
   int orient;
   Eina_Bool flip;
   uint16_t sx, sy, sw, sh;

   if (pud_unit_building_is(unit))
     {
        ef = _buildings[era];
        snprintf(key, sizeof(key), "%s", pud_unit2str(unit));
        flip = EINA_FALSE;
     }
   else
     {
        ef = _units_ef;

        if (info != SPRITE_INFO_ICON)
          {
             switch (info)
               {
                case SPRITE_INFO_SOUTH_WEST:
                   orient = SPRITE_INFO_SOUTH_EAST;
                   flip = EINA_TRUE;
                   break;

                case SPRITE_INFO_WEST:
                   orient = SPRITE_INFO_EAST;
                   flip = EINA_TRUE;
                   break;

                case SPRITE_INFO_NORTH_WEST:
                   orient = SPRITE_INFO_NORTH_EAST;
                   flip = EINA_TRUE;
                   break;

                default:
                   orient = info;
                   flip = EINA_FALSE;
                   break;
               }

             if ((unit == PUD_UNIT_GNOMISH_SUBMARINE) ||
                 (unit == PUD_UNIT_GIANT_TURTLE))
               {
                  snprintf(key, sizeof(key), "%s/%s/%i",
                           pud_unit2str(unit), pud_era2str(era), orient);
               }
             else if ((unit == PUD_UNIT_HUMAN_START) ||
                      (unit == PUD_UNIT_ORC_START))
               {
                  snprintf(key, sizeof(key), "%s/0",
                           pud_unit2str(unit));
               }
             else
               {
                  snprintf(key, sizeof(key), "%s/%i",
                           pud_unit2str(unit), orient);
               }
          }
        else
          {
             CRI("ICONS not implemented!");
             return NULL;
          }
     }
   if (flip_me) *flip_me = flip;

   data = eina_hash_find(_sprites, key);
   if (data == NULL)
     {
        data = sprite_load(ef, key, x, y, w, h);
        if (EINA_UNLIKELY(data == NULL))
          {
             ERR("Failed to load sprite for key [%s]", key);
             return NULL;
          }
        chk = eina_hash_add(_sprites, key, data);
        if (chk == EINA_FALSE)
          {
             ERR("Failed to add sprite <%p> to hash", data);
             free(data);
             return NULL;
          }
        //DBG("Access key [%s] (not yet registered). SRT = <%p>", key, data);
        return data;
     }
   else
     {
        if (x) { memcpy(&sx, data - 8, sizeof(uint16_t)); *x = sx; }
        if (y) { memcpy(&sy, data - 6, sizeof(uint16_t)); *y = sy; }
        if (w) { memcpy(&sw, data - 4, sizeof(uint16_t)); *w = sw; }
        if (h) { memcpy(&sh, data - 2, sizeof(uint16_t)); *h = sh; }
        //DBG("Access key [%s] (already registered). SRT = <%p>", key, data);
        return data;
     }
}

Sprite_Info
sprite_info_random_get(void)
{
   /* Does not return 4 */
   return rand() % (SPRITE_INFO_NORTH_WEST - SPRITE_INFO_NORTH) + SPRITE_INFO_NORTH;
}


static void
_free_cb(void *data)
{
   /* There is an 8 bytes offset */
   data -= 8;
   free(data);
}

Eina_Bool
sprite_init(void)
{
   if (EINA_UNLIKELY(!sprite_units_open()))
     {
        CRI("Failed to open units file");
        goto fail;
     }

   _sprites = eina_hash_string_superfast_new(_free_cb);
   if (EINA_UNLIKELY(!_sprites))
     {
        CRI("Failed to create Hash for sprites");
        goto sprites_fail;
     }

   if (EINA_UNLIKELY(!sprite_units_open()))
     {
        CRI("Failed to load units");
        goto units_fail;
     }

   return EINA_TRUE;

units_fail:
   eina_hash_free(_sprites);
sprites_fail:
   eet_close(_units_ef);
fail:
   return EINA_FALSE;
}

void
sprite_shutdown(void)
{
   unsigned int i;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(_buildings); ++i)
     {
        if (_buildings[i])
          {
             eet_close(_buildings[i]);
             _buildings[i] = NULL;
          }
     }

   eet_close(_units_ef);
   _units_ef = NULL;
   eina_hash_free(_sprites);
}

