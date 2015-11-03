/*
 * sel.c
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef __SEL_H__
#define __SEL_H__

Evas_Object *sel_add(Editor *restrict ed);

void
sel_start(Editor *restrict ed,
          const int        x,
          const int        y);

void
sel_update(Editor *restrict ed,
           int              w,
           int              h);

void sel_end(Editor *restrict ed);

Eina_Bool sel_active_is(const Editor *restrict ed);

#endif /* ! __SEL_H__ */

