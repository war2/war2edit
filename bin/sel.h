/*
 * sel.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef __SEL_H__
#define __SEL_H__

Evas_Object *sel_add(Editor * ed);

void
sel_start(Editor * ed,
          const int        x,
          const int        y,
          Eina_Bool        inclusive);

void
sel_update(Editor * ed,
           int              w,
           int              h);

void sel_end(Editor * ed);

void sel_del(Editor * ed);

Eina_Bool sel_active_is(const Editor * ed);
Eina_Bool sel_empty_is(const Editor * ed);

#endif /* ! __SEL_H__ */

