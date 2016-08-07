/*
 * texture.h
 *
 * Copyright (c) 2015 - 2016 Jean Guyomarc'h
 */

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#define TEXTURE_WIDTH  32
#define TEXTURE_HEIGHT 32

Eina_Bool texture_init(void);
void texture_shutdown(void);
Eina_Bool texture_tileset_open(Pud_Era era);
Eina_Bool texture_access_test(uint16_t tile, cairo_surface_t *atlas, unsigned int *x_off, unsigned int *y_off);
cairo_surface_t *texture_atlas_get(Pud_Era era);

#endif /* ! _TEXTURE_H_ */
