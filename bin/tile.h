/*
 * tile.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef _TILE_H_
#define _TILE_H_

typedef unsigned int Tile;

#define TILE_ORC_WALL         ((Tile) 0x00)
#define TILE_HUMAN_WALL       ((Tile) 0x01)
#define TILE_TREES            ((Tile) 0x02)
#define TILE_CONSTRUCTIBLE    ((Tile) 0x03)
#define TILE_UNCONSTRUCTIBLE  ((Tile) 0x04)
#define TILE_ROCKS            ((Tile) 0x05)
#define TILE_WATER            ((Tile) 0x06)
   /* ==== Up to 0b110 (3 bits) */

   /* 2 bits for the tile ID */

   /* Bit 5: is it a dark or a light tile */
#define TILE_DARK             ((Tile) (1 << 5))

   /* Bits 6 to 9: what part of the tile is FILLED (everything = solid) */
#define TILE_UP               ((Tile) (1 << 6))
#define TILE_DOWN             ((Tile) (1 << 7))
#define TILE_LEFT             ((Tile) (1 << 8))
#define TILE_RIGHT            ((Tile) (1 << 9))



/*============================================================================*
 *                              Tile Inlined API                              *
 *============================================================================*/

static inline Eina_Bool
tile_solid_is(Tile t)
{
   return ((t & TILE_UP) && (t & TILE_DOWN) &&
           (t & TILE_LEFT) && (t & TILE_RIGHT));
}

static inline Tile
tile_fill_set(Tile t,
              Tile side)
{
   return t | side;
}

static inline Eina_Bool
tile_fill_is(Tile t,
             Tile fill)
{
   return !!(t & fill);
}

static inline Tile
tile_fill_unset(Tile t,
                Tile side)
{
   return t & (~side);
}

static inline Tile
tile_type_set(Tile t,
              Tile type)
{
   return t | (0x7 & type);
}

static inline Tile
tile_type_get(Tile t)
{
   return t & 0x7;
}

static inline Eina_Bool
tile_dark_is(Tile t)
{
   return (t & TILE_DARK);
}

static inline Tile
tile_dark_set(Tile t)
{
   return t | TILE_DARK;
}

static inline Tile
tile_dark_unset(Tile t)
{
   return t & (~TILE_DARK);
}

static inline unsigned char
tile_id_get(Tile t)
{
   return (t >> 3) & 0x3;
}

static inline Tile
tile_id_set(Tile          t,
            unsigned char id)
{
   return t | (((Tile)id & 0x3) << 3);
}

#endif /* ! _TILE_H_ */

