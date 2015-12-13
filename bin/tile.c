/*
 * tile.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#include "war2edit.h"

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
               const uint8_t seed)
{
   // FIXME randomize if seed is 0xff
   const uint16_t seed_mask = (seed == 0xff) ? 0x0001 : (uint16_t)seed & 0x000f;
   uint16_t tile_code;

   tile_code = tile_mask_calculate(tl, tr, bl, br) | seed_mask;
   switch (tile_code)
     {
      case 0x003a:
      case 0x003b:
      case 0x004a:
      case 0x004b:
         CRI("BLACK PLAGUE TILE <0x%x>! REMAP THIS!!!!", tile_code);
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
   if ((tile_code & 0xff00) == 0x0000) /* Solid */
     {
        uint8_t code = 0xff;
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
        *seed = tile_code & 0x000f;
     }
   else /* Boundry */
     {
        CRI("IMPLEMENT ME");
     }
}

