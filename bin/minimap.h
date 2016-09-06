/*
 * Copyright (c) 2015-2016 Jean Guyomarc'h
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _MINIMAP_H_
#define _MINIMAP_H_

Eina_Bool minimap_add(Editor *ed);
void minimap_del(Editor *ed);
Eina_Bool minimap_update(Editor *ed, unsigned int x, unsigned int y);
void
minimap_render(unsigned int  x,
               unsigned int  y,
               unsigned int  w,
               unsigned int  h);

void
minimap_render_unit(const Editor *ed,
                    unsigned int  x,
                    unsigned int  y,
                    Pud_Unit      u);

void
minimap_view_move(Editor    *ed,
                  int        x,
                  int        y,
                  Eina_Bool  clicked);

void minimap_view_resize(Editor *ed, unsigned int w, unsigned int h);
Eina_Bool minimap_init(void);
void minimap_shutdown(void);

void minimap_show(void);
Eina_Bool minimap_attach(Editor *ed);
Eina_Bool minimap_reload(Editor *ed);

typedef struct
{
   unsigned char **data;
   unsigned int ratio;
   unsigned int w;
   unsigned int h;
} Minimap_Data;

#endif /* ! _MINIMAP_H_ */
