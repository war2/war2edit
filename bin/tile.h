/*
 * tile.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef _TILE_H_
#define _TILE_H_

#include "editor.h"

/* Values must not overflow 4 bits */
#define TILE_WATER_LIGHT                0
#define TILE_WATER_DARK                 1
#define TILE_GRASS_LIGHT                2
#define TILE_GRASS_DARK                 3
#define TILE_GROUND_LIGHT               4
#define TILE_GROUND_DARK                5
#define TILE_TREES                      6
#define TILE_ROCKS                      7

typedef enum
{
   TILE_PROPAGATE_NONE  = 0,
   TILE_PROPAGATE_TL    = (1 << 0), /* 0001 */
   TILE_PROPAGATE_TR    = (1 << 1), /* 0010 */
   TILE_PROPAGATE_BL    = (1 << 2), /* 0100 */
   TILE_PROPAGATE_BR    = (1 << 3)  /* 1000 */
} Tile_Propagate;

static inline Eina_Bool
tile_solid_is(const uint8_t tl,
              const uint8_t tr,
              const uint8_t bl,
              const uint8_t br)
{
   return ((tl == tr) && (tl == bl) && (tl == br));
}

static inline Eina_Bool
tile_water_is(const uint8_t tl,
              const uint8_t tr,
              const uint8_t bl,
              const uint8_t br)
{
   return (tile_solid_is(tl, tr, bl, br) &&
           ((tl == TILE_WATER_LIGHT) || (tl == TILE_WATER_DARK)));
}

static inline Eina_Bool
tile_ground_is(const uint8_t tl,
               const uint8_t tr,
               const uint8_t bl,
               const uint8_t br)
{
   return (tile_solid_is(tl, tr, bl, br) &&
           ((tl == TILE_GROUND_LIGHT) || (tl == TILE_GROUND_DARK)));
}

static inline Eina_Bool
tile_grass_is(const uint8_t tl,
              const uint8_t tr,
              const uint8_t bl,
              const uint8_t br)
{
   return (tile_solid_is(tl, tr, bl, br) &&
           ((tl == TILE_GRASS_LIGHT) || (tl == TILE_GRASS_DARK)));
}

static inline Eina_Bool
tile_trees_is(const uint8_t tl,
             const uint8_t tr,
             const uint8_t bl,
             const uint8_t br)
{
   return ((tl == TILE_TREES) || (tr == TILE_TREES) ||
           (bl == TILE_TREES) || (br == TILE_TREES));
}

static inline Eina_Bool
tile_rocks_is(const uint8_t tl,
             const uint8_t tr,
             const uint8_t bl,
             const uint8_t br)
{
   return ((tl == TILE_ROCKS) || (tr == TILE_ROCKS) ||
           (bl == TILE_ROCKS) || (br == TILE_ROCKS));
}

static inline Eina_Bool
tile_walkable_is(const uint8_t tl,
                 const uint8_t tr,
                 const uint8_t bl,
                 const uint8_t br)
{
   return (//!tile_wall_is(tl, tr, bl, br) &&
           !tile_trees_is(tl, tr, bl, br) &&
           !tile_rocks_is(tl, tr, bl, br) &&
           !tile_water_is(tl, tr, bl, br));
}

uint16_t
tile_calculate(const uint8_t tl,
               const uint8_t tr,
               const uint8_t bl,
               const uint8_t br,
               const uint8_t seed);

uint16_t
tile_mask_calculate(const uint8_t tl,
                    const uint8_t tr,
                    const uint8_t bl,
                    const uint8_t br);

void
tile_decompose(uint16_t  tile_code,
               uint8_t  *bl,
               uint8_t  *br,
               uint8_t  *tl,
               uint8_t  *tr,
               uint8_t  *seed);

/* Helpers */
#define TILE_SOLID_IS(cptr) \
   tile_solid_is(cptr->tile_tl, cptr->tile_tr, cptr->tile_bl, cptr->tile_br)

#define TILE_WATER_IS(cptr) \
   tile_water_is(cptr->tile_tl, cptr->tile_tr, cptr->tile_bl, cptr->tile_br)

#define TILE_GROUND_IS(cptr) \
   tile_ground_is(cptr->tile_tl, cptr->tile_tr, cptr->tile_bl, cptr->tile_br)

#define TILE_GRASS_IS(cptr) \
   tile_grass_is(cptr->tile_tl, cptr->tile_tr, cptr->tile_bl, cptr->tile_br)

#define TILE_TREES_IS(cptr) \
   tile_trees_is(cptr->tile_tl, cptr->tile_tr, cptr->tile_bl, cptr->tile_br)

#define TILE_ROCKS_IS(cptr) \
   tile_rocks_is(cptr->tile_tl, cptr->tile_tr, cptr->tile_bl, cptr->tile_br)

#define TILE_WALKABLE_IS(cptr) \
   tile_walkable_is(cptr->tile_tl, cptr->tile_tr, cptr->tile_bl, cptr->tile_br)


//static inline Eina_Bool
//tile_wall_is(unsigned int tile)
//{
//   const uint16_t solid = tile & 0x00f0;
//   const uint16_t boundry = tile & 0x0f00;
//   return ((solid == 0x00a0) ||
//           (solid == 0x00c0) ||
//           (boundry == 0x0900) ||
//           (boundry == 0x0800));
//}


#endif /* ! _TILE_H_ */

