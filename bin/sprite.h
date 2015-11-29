/*
 * sprite.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
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


unsigned char *sprite_get(Pud_Unit unit, Pud_Era era, Sprite_Info info, int *x, int *y, unsigned int *w, unsigned int *h, Eina_Bool *flip_me);
Eet_File *sprite_buildings_open(Pud_Era era);
Eet_File *sprite_units_open(void);
void *sprite_load(Eet_File *src, const char *key, int *x_ret, int *y_ret, unsigned int *w_ret, unsigned int *h_ret);
Sprite_Info sprite_info_random_get(void);

Eina_Bool sprite_init(void);
void sprite_shutdown(void);
void sprite_tile_size_get(Pud_Unit unit, unsigned int *w, unsigned int *h);
unsigned char *sprite_selection_get(unsigned int edge);

#endif /* ! _SPRITE_H_ */

