/*
 * tile.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef _TILE_H_
#define _TILE_H_

#include "editor.h"

uint16_t tile_solid_mask_get(Editor_Sel action, Editor_Sel tint);

static inline Eina_Bool
tile_solid_is(unsigned int tile)
{
   return (tile & 0x00f0);
}

static inline Eina_Bool
tile_water_is(unsigned int tile)
{
   const uint16_t solid = tile & 0x00f0;
   return ((solid == 0x0010) ||
           (solid == 0x0020));
}

static inline Eina_Bool
tile_constructible_is(unsigned int tile)
{
   const uint16_t solid = tile & 0x00f0;
   return ((solid == 0x0050) ||
           (solid == 0x0060));
}

static inline Eina_Bool
tile_wall_is(unsigned int tile)
{
   const uint16_t solid = tile & 0x00f0;
   const uint16_t boundry = tile & 0x0f00;
   return ((solid == 0x00a0) ||
           (solid == 0x00c0) ||
           (boundry == 0x0900) ||
           (boundry == 0x0800));
}

static inline Eina_Bool
tile_tree_is(unsigned int tile)
{
   return (((tile & 0x00f0) == 0x0070) ||
           ((tile & 0x0f00) == 0x0700));
}

static inline Eina_Bool
tile_rock_is(unsigned int tile)
{
   return (((tile & 0x00f0) == 0x0080) ||
           ((tile & 0x0f00) == 0x0400));
}

static inline Eina_Bool
tile_walkable_is(unsigned int tile)
{
   return (!tile_wall_is(tile) &&
           !tile_tree_is(tile) &&
           !tile_rock_is(tile) &&
           !tile_water_is(tile));
}


#endif /* ! _TILE_H_ */

