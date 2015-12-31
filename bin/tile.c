/*
 * tile.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

static const uint8_t _tiles_compatible[__TILE_LAST][__TILE_LAST] =
{
   /*             F   GL  CL  WL  WD  GD  CD  R */
   /*    */ {  0,  0,  0,  0,  0,  0,  0,  0,  0 },
   /*  F */ {  0,  1,  1,  0,  0,  0,  0,  0,  0 },
   /* GL */ {  0,  1,  1,  1,  0,  0,  1,  0,  0 },
   /* CL */ {  0,  0,  1,  1,  1,  0,  0,  1,  1 },
   /* WL */ {  0,  0,  0,  1,  1,  1,  0,  0,  0 },
   /* WD */ {  0,  0,  0,  0,  1,  1,  0,  0,  0 },
   /* GD */ {  0,  0,  1,  0,  0,  0,  1,  0,  0 },
   /* CD */ {  0,  0,  0,  1,  0,  0,  0,  1,  0 },
   /*  R */ {  0,  0,  0,  1,  0,  0,  0,  0,  1 }
};

static const uint8_t _tiles_conflicts[__TILE_LAST] =
{
   /*    */ TILE_NONE,
   /*  F */ TILE_GRASS_LIGHT,
   /* GL */ TILE_GROUND_LIGHT,
   /* CL */ TILE_GRASS_LIGHT,
   /* WL */ TILE_GROUND_LIGHT,
   /* WD */ TILE_WATER_LIGHT,
   /* GD */ TILE_GRASS_LIGHT,
   /* CD */ TILE_GROUND_LIGHT,
   /*  R */ TILE_GROUND_LIGHT
};

/*============================================================================*
 *                                 Private API                                *
 *============================================================================*/

static inline EINA_CONST Eina_Bool
_tile_has_type(const uint8_t tl,
               const uint8_t tr,
               const uint8_t bl,
               const uint8_t br,
               const uint8_t type)
{
   return ((tl == type) || (tr == type) || (bl == type) || (br == type));
}

static EINA_CONST uint16_t
_tile_boundry_low_mask_get(const uint8_t tl,
                           const uint8_t tr,
                           const uint8_t bl,
                           const uint8_t br,
                           const uint8_t first_type)
{
   uint16_t mask = 0x0000;
   const uint16_t reference = tl;

   if (tl == reference) mask |= 0x0001; /* 0001 */
   if (tr == reference) mask |= 0x0002; /* 0010 */
   if (bl == reference) mask |= 0x0005; /* 0100 */
   if (br == reference) mask |= 0x0008; /* 1000 */

   if (reference != first_type)
     mask = (~mask) & 0x000f;

   return (mask - 0x0001) << 4;
}


/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

uint16_t
tile_mask_calculate(const uint8_t tl,
                    const uint8_t tr,
                    const uint8_t bl,
                    const uint8_t br)
{
   /* Helpers */
#define TILE_HAS(type) _tile_has_type(tl, tr, bl, br, type)
#define LOW_MASK(first) _tile_boundry_low_mask_get(tl, tr, bl, br, first)

   uint16_t tile = 0x0000;

   if (tile_solid_is(tl, tr, bl, br))
     {
        switch (tl) /* tl == tr == bl == br */
          {
           case TILE_WATER_LIGHT:  tile = 0x0010; break;
           case TILE_WATER_DARK:   tile = 0x0020; break;
           case TILE_GROUND_LIGHT: tile = 0x0030; break;
           case TILE_GROUND_DARK:  tile = 0x0040; break;
           case TILE_GRASS_LIGHT:  tile = 0x0050; break;
           case TILE_GRASS_DARK:   tile = 0x0060; break;
           case TILE_TREES:        tile = 0x0070; break;
           case TILE_ROCKS:        tile = 0x0080; break;

           default:
              CRI("Unhandled solid tile %x", tl);
              goto fail;
          }
     }
   else
     {
        if (TILE_HAS(TILE_GRASS_LIGHT))
          {
             if (TILE_HAS(TILE_TREES))
               {
                  tile = 0x0700 | LOW_MASK(TILE_TREES);
               }
             else if (TILE_HAS(TILE_GRASS_DARK))
               {
                  tile = 0x0600 | LOW_MASK(TILE_GROUND_DARK);
               }
             else if (TILE_HAS(TILE_GROUND_LIGHT))
               {
                  tile = 0x0500 | LOW_MASK(TILE_GROUND_LIGHT);
               }
             else
               {
                  CRI("Invalid disposition of tiles (with grass light)");
                  goto fail;
               }
          }
        else if (TILE_HAS(TILE_GROUND_LIGHT))
          {
             if (TILE_HAS(TILE_GROUND_DARK))
               {
                  tile = 0x0300 | LOW_MASK(TILE_GROUND_DARK);
               }
             else if (TILE_HAS(TILE_ROCKS))
               {
                  tile = 0x0400 | LOW_MASK(TILE_ROCKS);
               }
             else if (TILE_HAS(TILE_WATER_LIGHT))
               {
                  tile = 0x0200 | LOW_MASK(TILE_WATER_LIGHT);
               }
             else
               {
                  CRI("Invalid disposition of tiles (with ground light)");
                  goto fail;
               }
          }
        else if (TILE_HAS(TILE_WATER_LIGHT))
          {
             if (TILE_HAS(TILE_WATER_DARK))
               {
                  tile = 0x0100 | LOW_MASK(TILE_WATER_DARK);
               }
             else
               {
                  CRI("Invalid disposition of tiles (with water light)");
                  goto fail;
               }
          }
        else
          {
             CRI("Uncovered case");
             goto fail;
          }
     }

   return tile;

fail:
   CRI("Analysis of tile {%i %i %i %i} failed", tl, tr, bl, br);
   return 0x0000;

#undef LOW_MASK
#undef TILE_HAS
}

uint16_t
tile_calculate(const uint8_t tl,
               const uint8_t tr,
               const uint8_t bl,
               const uint8_t br,
               const uint8_t seed,
               const Pud_Era era)
{
   uint16_t tile_code, rtile;

   tile_code = tile_mask_calculate(tl, tr, bl, br);

   if ((seed & TILE_RANDOMIZE) == TILE_RANDOMIZE)
     {
        rtile = pud_random_get(tile_code);
        if (((tile_code & 0x0f00) == 0x0000) &&
            ((tile_code & 0x0030) ||
             (tile_code & 0x0040) ||
             (tile_code & 0x0050) ||
             (tile_code & 0x0060)))
          {
             if (!(seed & TILE_SPECIAL))
               rtile = rand() % 3;
             else
               {
                  if (rtile <= 2)
                    rtile += 0x0004;
               }
          }
     }
   else
     rtile = (uint16_t)seed & 0x000f;
   tile_code |= rtile;

   switch (tile_code)
     {
      case 0x0015:
      case 0x0016:
      case 0x0017:
      case 0x0025:
      case 0x0026:
      case 0x0027:
         /* These tiles exist only in winter and wasteland (floating
          * ice/different water tile).
          * They must be remapped in forest and swamp. */
         if ((era == PUD_ERA_FOREST) || (era == PUD_ERA_SWAMP))
           tile_code -= 0x0004;
         break;

      case 0x003a:
      case 0x003b:
      case 0x004a:
      case 0x004b:
         /* These tiles do not exist in swamp */
         if (era == PUD_ERA_SWAMP)
           tile_code -= 0x0005;
         break;

      default:
         break;
     }

   return tile_code;
}

void
tile_decompose(uint16_t  tile_code,
               uint8_t  *bl,
               uint8_t  *br,
               uint8_t  *tl,
               uint8_t  *tr,
               uint8_t  *seed)
{
   *seed = tile_code & 0x000f;

   if ((tile_code & 0xff00) == 0x0000) /* Solid */
     {
        uint8_t code = TILE_NONE;
        switch (tile_code & 0x00f0)
          {
           case 0x0010: code = TILE_WATER_LIGHT; break;
           case 0x0020: code = TILE_WATER_DARK; break;
           case 0x0030: code = TILE_GROUND_LIGHT; break;
           case 0x0040: code = TILE_GROUND_LIGHT; break;
           case 0x0050: code = TILE_GRASS_LIGHT; break;
           case 0x0060: code = TILE_GRASS_LIGHT; break;
           case 0x0070: code = TILE_TREES; break;
           case 0x0080: code = TILE_ROCKS; break;
           default: CRI("Unhandled tile: 0x%x", tile_code); break;
          }
        *bl = code; *br = code; *tl = code; *tr = code;
     }
   else /* Boundry */
     {
     }
}

Eina_Bool
tile_fragments_compatible_are(const uint8_t t1,
                              const uint8_t t2)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((t1 < __TILE_LAST) &&
                                   (t2 < __TILE_LAST),
                                   EINA_FALSE);

   return _tiles_compatible[t1][t2];
}

Eina_Bool
tile_compatible_is(const uint8_t tl,
                   const uint8_t tr,
                   const uint8_t bl,
                   const uint8_t br)
{
   return (tile_fragments_compatible_are(tl, tr) &&
           tile_fragments_compatible_are(tl, bl) &&
           tile_fragments_compatible_are(tl, br));

}

uint8_t
tile_conflict_resolve_get(const uint8_t t)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(t < __TILE_LAST,  TILE_NONE);
   return _tiles_conflicts[t];
}

