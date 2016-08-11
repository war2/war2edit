/*
 * sprite.h
 *
 * Copyright (c) 2015 - 2016 Jean Guyomarc'h
 */

#ifndef _SPRITE_H_
#define _SPRITE_H_

typedef enum
{
   SPRITE_INFO_NORTH            = 0,
   SPRITE_INFO_NORTH_EAST       = 1,
   SPRITE_INFO_EAST             = 2,
   SPRITE_INFO_SOUTH_EAST       = 3,
   SPRITE_INFO_SOUTH            = 4,
   SPRITE_INFO_SOUTH_WEST,
   SPRITE_INFO_WEST,
   SPRITE_INFO_NORTH_WEST,
   SPRITE_INFO_ICON , /* special value for icons */
} Sprite_Info;

typedef struct
{
   unsigned char *data;
   unsigned int w;
   unsigned int h;
   Pud_Player   color;
} Sprite_Descriptor;


Sprite_Descriptor *sprite_get(Pud_Unit unit, Pud_Era era, Sprite_Info info,
                              Eina_Bool *flip_me);
Eet_File *sprite_buildings_open(Pud_Era era);
Eet_File *sprite_units_open(void);
Sprite_Info sprite_info_random_get(void);

Eina_Bool sprite_init(void);
void sprite_shutdown(void);
void sprite_tile_size_get(Pud_Unit unit, unsigned int *w, unsigned int *h);
cairo_surface_t *sprite_selection_get(unsigned int edge);

#endif /* ! _SPRITE_H_ */

