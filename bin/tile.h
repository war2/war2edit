#ifndef _TILE_H_
#define _TILE_H_

typedef enum
{

   TILE_ORC_WALL        = 0x00,
   TILE_HUMAN_WALL      = 0x01,
   TILE_TREES           = 0x02,
   TILE_CONSTRUCTIBLE   = 0x03,
   TILE_UNCONSTRUCTIBLE = 0x04,
   TILE_ROCKS           = 0x05,
   TILE_WATER           = 0x06,
   /* ==== Up to 0b110 (3 bits) */

   /* 2 bits for the tile ID */

   /* Bit 5: is it a dark or a light tile */
   TILE_DARK            = (1 << 5),

   /* Bits 6 to 9: what part of the tile is FILLED (everything = solid) */
   TILE_UP              = (1 << 6),
   TILE_DOWN            = (1 << 7),
   TILE_LEFT            = (1 << 8),
   TILE_RIGHT           = (1 << 9)

} Tile;


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
   return (t & fill);
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
   return t | ((id & 0x3) << 3);
}

#endif /* ! _TILE_H_ */

